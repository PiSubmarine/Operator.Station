#include "PiSubmarine/Operator/Station/Input/Controller.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace PiSubmarine::Operator::Station::Input
{
    namespace
    {
        constexpr char HexDigits[] = "0123456789ABCDEF";
    }

    Controller::Controller(
        InputSystem keyboardInputSystem,
        InputSystem gamepadInputSystem,
        std::vector<BindingDescriptor> bindings,
        QObject* parent)
        : QObject(parent)
        , m_KeyboardInputSystem(std::move(keyboardInputSystem))
        , m_GamepadInputSystem(std::move(gamepadInputSystem))
    {
        m_Bindings.reserve(bindings.size());
        for (auto& binding : bindings)
        {
            BindingState state{
                .Descriptor = std::move(binding)
            };

            if (state.Descriptor.Type == BindingType::Axis)
            {
                state.AxisProxy = std::make_unique<AxisMultiplexor>();
            }
            else
            {
                state.KeyProxy = std::make_unique<KeyMultiplexor>();
            }

            m_BindingIndexByName.emplace(state.Descriptor.Name, m_Bindings.size());
            m_Bindings.push_back(std::move(state));
        }

        ValidateInputSystems();
        if (m_InputSystemsAreShared)
        {
            LoadSharedBindingFile();
        }
        else
        {
            LoadBindingFile(BindingDevice::Keyboard);
            LoadBindingFile(BindingDevice::Gamepad);
        }
    }

    void Controller::Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime)
    {
        for (auto& binding : m_Bindings)
        {
            if (binding.AxisProxy != nullptr)
            {
                binding.AxisProxy->Tick(uptime, deltaTime);
            }

            if (binding.KeyProxy != nullptr)
            {
                binding.KeyProxy->Tick(uptime, deltaTime);
            }
        }
    }

    void Controller::Start()
    {
        for (auto& binding : m_Bindings)
        {
            RestoreBinding(binding);
            PublishBoundInput(binding);
        }

        PublishAllBindingsConfigured();
        emit StatusMessageChanged("Ready to bind inputs.");
    }

    void Controller::Capture(const QString& name, const BindingDevice device)
    {
        if (m_PendingCapture.has_value())
        {
            emit StatusMessageChanged("Capture already in progress.");
            return;
        }

        const auto bindingIt = m_BindingIndexByName.find(name.toStdString());
        if (bindingIt == m_BindingIndexByName.end())
        {
            emit StatusMessageChanged("Unknown binding path.");
            return;
        }

        auto& binding = m_Bindings.at(bindingIt->second);
        m_PendingCapture = PendingCapture{
            .Name = binding.Descriptor.Name,
            .Device = device
        };
        emit CaptureTargetChanged(name, device);
        emit CaptureInProgressChanged(true);

        if (binding.Descriptor.Type == BindingType::Axis)
        {
            emit StatusMessageChanged(QString("Capturing %1 axis for %2...").arg(DescribeDevice(device), name));
            StartAxisCapture(binding, device);
            return;
        }

        emit StatusMessageChanged(QString("Capturing %1 key for %2...").arg(DescribeDevice(device), name));
        StartKeyCapture(binding, device);
    }

    void Controller::CancelCapture()
    {
        if (!m_PendingCapture.has_value())
        {
            return;
        }

        GetInputSystem(m_PendingCapture->Device).Binder.StopCapture();
    }

    void Controller::ValidateInputSystems()
    {
        const bool managerEqual = &m_KeyboardInputSystem.Manager == &m_GamepadInputSystem.Manager;
        const bool binderEqual = &m_KeyboardInputSystem.Binder == &m_GamepadInputSystem.Binder;
        const bool serializerEqual = &m_KeyboardInputSystem.Serializer == &m_GamepadInputSystem.Serializer;
        const bool pathEqual = m_KeyboardInputSystem.BindingFilePath == m_GamepadInputSystem.BindingFilePath;

        const bool allEqual = managerEqual && binderEqual && serializerEqual && pathEqual;
        const bool allUnequal = !managerEqual && !binderEqual && !serializerEqual && !pathEqual;

        if (!allEqual && !allUnequal)
        {
            throw std::invalid_argument(
                "Keyboard and gamepad InputSystems must be either fully shared or fully distinct.");
        }

        m_InputSystemsAreShared = allEqual;
    }

    void Controller::LoadBindingFile(const BindingDevice device)
    {
        const auto& inputSystem = GetInputSystem(device);
        if (!std::filesystem::exists(inputSystem.BindingFilePath))
        {
            return;
        }

        std::ifstream stream(inputSystem.BindingFilePath);
        std::string line;
        while (std::getline(stream, line))
        {
            if (line.empty())
            {
                continue;
            }

            std::istringstream lineStream(line);
            std::string type;
            std::string name;
            std::string encoded;

            if (!std::getline(lineStream, type, '\t') ||
                !std::getline(lineStream, name, '\t') ||
                !std::getline(lineStream, encoded))
            {
                continue;
            }

            const auto bindingIt = m_BindingIndexByName.find(name);
            if (bindingIt == m_BindingIndexByName.end())
            {
                continue;
            }

            auto& binding = m_Bindings.at(bindingIt->second);
            const auto expectedType = binding.Descriptor.Type == BindingType::Axis ? "axis" : "key";
            if (type != expectedType)
            {
                continue;
            }

            try
            {
                GetDeviceBinding(binding, device).Serialized = DecodeBytes(encoded);
            }
            catch (const std::exception&)
            {
                GetDeviceBinding(binding, device).Serialized.clear();
            }
        }
    }

    void Controller::LoadSharedBindingFile()
    {
        const auto& inputSystem = m_KeyboardInputSystem;
        if (!std::filesystem::exists(inputSystem.BindingFilePath))
        {
            return;
        }

        std::ifstream stream(inputSystem.BindingFilePath);
        std::string line;
        while (std::getline(stream, line))
        {
            if (line.empty())
            {
                continue;
            }

            std::istringstream lineStream(line);
            std::string type;
            std::string name;
            std::string thirdColumn;
            std::string fourthColumn;

            if (!std::getline(lineStream, type, '\t') ||
                !std::getline(lineStream, name, '\t') ||
                !std::getline(lineStream, thirdColumn, '\t'))
            {
                continue;
            }

            BindingDevice device = BindingDevice::Keyboard;
            std::string encoded = thirdColumn;
            if (std::getline(lineStream, fourthColumn))
            {
                const auto decodedDevice = DecodeDevice(thirdColumn);
                if (!decodedDevice.has_value())
                {
                    continue;
                }

                device = *decodedDevice;
                encoded = fourthColumn;
            }

            const auto bindingIt = m_BindingIndexByName.find(name);
            if (bindingIt == m_BindingIndexByName.end())
            {
                continue;
            }

            auto& binding = m_Bindings.at(bindingIt->second);
            const auto expectedType = binding.Descriptor.Type == BindingType::Axis ? "axis" : "key";
            if (type != expectedType)
            {
                continue;
            }

            try
            {
                GetDeviceBinding(binding, device).Serialized = DecodeBytes(encoded);
            }
            catch (const std::exception&)
            {
                GetDeviceBinding(binding, device).Serialized.clear();
            }
        }
    }

    void Controller::SaveBindingFile(const BindingDevice device) const
    {
        const auto& inputSystem = GetInputSystem(device);
        if (inputSystem.BindingFilePath.has_parent_path())
        {
            std::filesystem::create_directories(inputSystem.BindingFilePath.parent_path());
        }

        std::ofstream stream(inputSystem.BindingFilePath, std::ios::trunc);
        for (const auto& binding : m_Bindings)
        {
            const auto& deviceBinding = GetDeviceBinding(binding, device);
            if (deviceBinding.Serialized.empty())
            {
                continue;
            }

            stream << (binding.Descriptor.Type == BindingType::Axis ? "axis" : "key")
                   << '\t'
                   << binding.Descriptor.Name
                   << '\t'
                   << EncodeBytes(deviceBinding.Serialized)
                   << '\n';
        }
    }

    void Controller::SaveSharedBindingFile() const
    {
        const auto& inputSystem = m_KeyboardInputSystem;
        if (inputSystem.BindingFilePath.has_parent_path())
        {
            std::filesystem::create_directories(inputSystem.BindingFilePath.parent_path());
        }

        std::ofstream stream(inputSystem.BindingFilePath, std::ios::trunc);
        for (const auto& binding : m_Bindings)
        {
            for (const auto device : {BindingDevice::Keyboard, BindingDevice::Gamepad})
            {
                const auto& deviceBinding = GetDeviceBinding(binding, device);
                if (deviceBinding.Serialized.empty())
                {
                    continue;
                }

                stream << (binding.Descriptor.Type == BindingType::Axis ? "axis" : "key")
                       << '\t'
                       << binding.Descriptor.Name
                       << '\t'
                       << EncodeDevice(device)
                       << '\t'
                       << EncodeBytes(deviceBinding.Serialized)
                       << '\n';
            }
        }
    }

    void Controller::RestoreBinding(BindingState& state)
    {
        RestoreDeviceBinding(state, BindingDevice::Keyboard);
        RestoreDeviceBinding(state, BindingDevice::Gamepad);
        UpdateProxy(state);
    }

    void Controller::RestoreDeviceBinding(BindingState& state, const BindingDevice device)
    {
        auto& deviceBinding = GetDeviceBinding(state, device);
        auto& inputSystem = GetInputSystem(device);
        deviceBinding.AxisBinding.reset();
        deviceBinding.KeyBinding.reset();
        deviceBinding.Axis.reset();
        deviceBinding.Key.reset();

        if (deviceBinding.Serialized.empty())
        {
            emit BindingHintChanged(QString::fromStdString(state.Descriptor.Name), device, "Unbound");
            return;
        }

        try
        {
            if (state.Descriptor.Type == BindingType::Axis)
            {
                deviceBinding.AxisBinding = inputSystem.Serializer.DeserializeAxis(deviceBinding.Serialized);
                deviceBinding.Axis = inputSystem.Manager.CreateAxis(*deviceBinding.AxisBinding);
                emit BindingHintChanged(
                    QString::fromStdString(state.Descriptor.Name),
                    device,
                    QString::fromStdString(deviceBinding.AxisBinding->GetHint()));
                return;
            }

            deviceBinding.KeyBinding = inputSystem.Serializer.DeserializeKey(deviceBinding.Serialized);
            deviceBinding.Key = inputSystem.Manager.CreateKey(*deviceBinding.KeyBinding);
            emit BindingHintChanged(
                QString::fromStdString(state.Descriptor.Name),
                device,
                QString::fromStdString(deviceBinding.KeyBinding->GetHint()));
        }
        catch (const std::exception&)
        {
            deviceBinding.Serialized.clear();
            deviceBinding.AxisBinding.reset();
            deviceBinding.KeyBinding.reset();
            deviceBinding.Axis.reset();
            deviceBinding.Key.reset();
            emit BindingHintChanged(QString::fromStdString(state.Descriptor.Name), device, "Unbound");
        }
    }

    void Controller::PublishBoundInput(BindingState& state)
    {
        if (state.AxisProxy != nullptr)
        {
            emit OnAxisBound(QString::fromStdString(state.Descriptor.Name), state.AxisProxy.get());
            return;
        }

        emit OnKeyBound(QString::fromStdString(state.Descriptor.Name), state.KeyProxy.get());
    }

    void Controller::UpdateProxy(BindingState& state)
    {
        if (state.AxisProxy != nullptr)
        {
            std::vector<::PiSubmarine::Input::Api::IAxis*> sources;
            if (state.Keyboard.Axis != nullptr)
            {
                sources.push_back(state.Keyboard.Axis.get());
            }
            if (state.Gamepad.Axis != nullptr)
            {
                sources.push_back(state.Gamepad.Axis.get());
            }

            state.AxisProxy->SetSources(std::move(sources));
            return;
        }

        std::vector<::PiSubmarine::Input::Api::IKey*> sources;
        if (state.Keyboard.Key != nullptr)
        {
            sources.push_back(state.Keyboard.Key.get());
        }
        if (state.Gamepad.Key != nullptr)
        {
            sources.push_back(state.Gamepad.Key.get());
        }

        state.KeyProxy->SetSources(std::move(sources));
    }

    void Controller::StartAxisCapture(BindingState& state, const BindingDevice device)
    {
        GetInputSystem(device).Binder.StartCapture([this, name = state.Descriptor.Name, device](
            const ::PiSubmarine::Input::Api::IBinder::CaptureStatus status,
            std::unique_ptr<::PiSubmarine::Input::Api::IAxisBinding> binding)
        {
            CompleteAxisCapture(name, device, status, std::move(binding));
        });
    }

    void Controller::StartKeyCapture(BindingState& state, const BindingDevice device)
    {
        GetInputSystem(device).Binder.StartCapture([this, name = state.Descriptor.Name, device](
            const ::PiSubmarine::Input::Api::IBinder::CaptureStatus status,
            std::unique_ptr<::PiSubmarine::Input::Api::IKeyBinding> binding)
        {
            CompleteKeyCapture(name, device, status, std::move(binding));
        });
    }

    void Controller::CompleteAxisCapture(
        const std::string& name,
        const BindingDevice device,
        const ::PiSubmarine::Input::Api::IBinder::CaptureStatus status,
        std::unique_ptr<::PiSubmarine::Input::Api::IAxisBinding> binding)
    {
        if (!m_PendingCapture.has_value() || m_PendingCapture->Name != name || m_PendingCapture->Device != device)
        {
            return;
        }

        m_PendingCapture.reset();
        emit CaptureTargetChanged({}, device);
        emit CaptureInProgressChanged(false);

        const auto bindingIt = m_BindingIndexByName.find(name);
        if (bindingIt == m_BindingIndexByName.end())
        {
            emit StatusMessageChanged("Unknown binding path.");
            return;
        }

        if (status != ::PiSubmarine::Input::Api::IBinder::CaptureStatus::Ok || binding == nullptr)
        {
            emit StatusMessageChanged(DescribeStatus(status));
            return;
        }

        auto& state = m_Bindings.at(bindingIt->second);
        auto& deviceBinding = GetDeviceBinding(state, device);
        deviceBinding.Serialized = GetInputSystem(device).Serializer.Serialize(*binding);
        deviceBinding.AxisBinding.reset();
        deviceBinding.Axis.reset();
        RestoreDeviceBinding(state, device);
        UpdateProxy(state);
        if (m_InputSystemsAreShared)
        {
            SaveSharedBindingFile();
        }
        else
        {
            SaveBindingFile(device);
        }
        PublishAllBindingsConfigured();
        emit StatusMessageChanged(QString("Bound %1 %2").arg(DescribeDevice(device), QString::fromStdString(name)));
    }

    void Controller::CompleteKeyCapture(
        const std::string& name,
        const BindingDevice device,
        const ::PiSubmarine::Input::Api::IBinder::CaptureStatus status,
        std::unique_ptr<::PiSubmarine::Input::Api::IKeyBinding> binding)
    {
        if (!m_PendingCapture.has_value() || m_PendingCapture->Name != name || m_PendingCapture->Device != device)
        {
            return;
        }

        m_PendingCapture.reset();
        emit CaptureTargetChanged({}, device);
        emit CaptureInProgressChanged(false);

        const auto bindingIt = m_BindingIndexByName.find(name);
        if (bindingIt == m_BindingIndexByName.end())
        {
            emit StatusMessageChanged("Unknown binding path.");
            return;
        }

        if (status != ::PiSubmarine::Input::Api::IBinder::CaptureStatus::Ok || binding == nullptr)
        {
            emit StatusMessageChanged(DescribeStatus(status));
            return;
        }

        auto& state = m_Bindings.at(bindingIt->second);
        auto& deviceBinding = GetDeviceBinding(state, device);
        deviceBinding.Serialized = GetInputSystem(device).Serializer.Serialize(*binding);
        deviceBinding.KeyBinding.reset();
        deviceBinding.Key.reset();
        RestoreDeviceBinding(state, device);
        UpdateProxy(state);
        if (m_InputSystemsAreShared)
        {
            SaveSharedBindingFile();
        }
        else
        {
            SaveBindingFile(device);
        }
        PublishAllBindingsConfigured();
        emit StatusMessageChanged(QString("Bound %1 %2").arg(DescribeDevice(device), QString::fromStdString(name)));
    }

    void Controller::PublishAllBindingsConfigured()
    {
        const bool allBindingsConfigured = AreAllBindingsConfigured();
        if (m_LastAllBindingsConfigured == allBindingsConfigured)
        {
            return;
        }

        m_LastAllBindingsConfigured = allBindingsConfigured;
        emit AllBindingsConfiguredChanged(allBindingsConfigured);
    }

    bool Controller::AreAllBindingsConfigured() const
    {
        for (const auto& binding : m_Bindings)
        {
            const auto& keyboardBinding = GetDeviceBinding(binding, BindingDevice::Keyboard);
            const auto& gamepadBinding = GetDeviceBinding(binding, BindingDevice::Gamepad);

            if (binding.Descriptor.Type == BindingType::Axis)
            {
                if (keyboardBinding.Axis == nullptr || gamepadBinding.Axis == nullptr)
                {
                    return false;
                }

                continue;
            }

            if (keyboardBinding.Key == nullptr || gamepadBinding.Key == nullptr)
            {
                return false;
            }
        }

        return true;
    }

    QString Controller::DescribeStatus(const ::PiSubmarine::Input::Api::IBinder::CaptureStatus status)
    {
        switch (status)
        {
        case ::PiSubmarine::Input::Api::IBinder::CaptureStatus::Ok:
            return "Capture complete.";
        case ::PiSubmarine::Input::Api::IBinder::CaptureStatus::ActionNotSupported:
            return "Capture action is not supported.";
        case ::PiSubmarine::Input::Api::IBinder::CaptureStatus::DeviceNotSupported:
            return "Input device is not supported.";
        case ::PiSubmarine::Input::Api::IBinder::CaptureStatus::Cancelled:
            return "Capture cancelled.";
        case ::PiSubmarine::Input::Api::IBinder::CaptureStatus::UnknownError:
            return "Capture failed.";
        }

        return "Capture failed.";
    }

    QString Controller::DescribeDevice(const BindingDevice device)
    {
        return device == BindingDevice::Keyboard ? "keyboard" : "gamepad";
    }

    std::string Controller::EncodeDevice(const BindingDevice device)
    {
        return device == BindingDevice::Keyboard ? "keyboard" : "gamepad";
    }

    std::optional<BindingDevice> Controller::DecodeDevice(const std::string& token)
    {
        if (token == "keyboard")
        {
            return BindingDevice::Keyboard;
        }

        if (token == "gamepad")
        {
            return BindingDevice::Gamepad;
        }

        return std::nullopt;
    }

    std::string Controller::EncodeBytes(const std::vector<std::byte>& data)
    {
        std::string result;
        result.reserve(data.size() * 2);
        for (const auto value : data)
        {
            const auto byteValue = static_cast<unsigned char>(value);
            result.push_back(HexDigits[(byteValue >> 4) & 0x0F]);
            result.push_back(HexDigits[byteValue & 0x0F]);
        }
        return result;
    }

    std::vector<std::byte> Controller::DecodeBytes(const std::string& text)
    {
        if ((text.size() % 2) != 0)
        {
            throw std::invalid_argument("Hex data must contain an even number of characters.");
        }

        auto decodeNibble = [](const char character) -> unsigned char
        {
            if (character >= '0' && character <= '9')
            {
                return static_cast<unsigned char>(character - '0');
            }
            if (character >= 'A' && character <= 'F')
            {
                return static_cast<unsigned char>(character - 'A' + 10);
            }
            if (character >= 'a' && character <= 'f')
            {
                return static_cast<unsigned char>(character - 'a' + 10);
            }

            throw std::invalid_argument("Invalid hex character in binding file.");
        };

        std::vector<std::byte> result;
        result.reserve(text.size() / 2);
        for (std::size_t index = 0; index < text.size(); index += 2)
        {
            const auto high = decodeNibble(text.at(index));
            const auto low = decodeNibble(text.at(index + 1));
            result.push_back(static_cast<std::byte>((high << 4) | low));
        }
        return result;
    }

    Controller::InputSystem& Controller::GetInputSystem(const BindingDevice device)
    {
        return device == BindingDevice::Keyboard ? m_KeyboardInputSystem : m_GamepadInputSystem;
    }

    const Controller::InputSystem& Controller::GetInputSystem(const BindingDevice device) const
    {
        return device == BindingDevice::Keyboard ? m_KeyboardInputSystem : m_GamepadInputSystem;
    }

    Controller::DeviceBindingState& Controller::GetDeviceBinding(BindingState& state, const BindingDevice device)
    {
        return device == BindingDevice::Keyboard ? state.Keyboard : state.Gamepad;
    }

    const Controller::DeviceBindingState& Controller::GetDeviceBinding(
        const BindingState& state,
        const BindingDevice device) const
    {
        return device == BindingDevice::Keyboard ? state.Keyboard : state.Gamepad;
    }
}
