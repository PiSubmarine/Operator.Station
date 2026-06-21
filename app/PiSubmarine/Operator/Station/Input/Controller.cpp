#include "PiSubmarine/Operator/Station/Input/Controller.h"

#include <cctype>
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
        ::PiSubmarine::Input::Api::IManager& manager,
        ::PiSubmarine::Input::Api::IBinder& binder,
        ::PiSubmarine::Input::Api::ISerializer& serializer,
        std::filesystem::path bindingFilePath,
        std::vector<BindingDescriptor> bindings,
        QObject* parent)
        : QObject(parent)
        , m_Manager(manager)
        , m_Binder(binder)
        , m_Serializer(serializer)
        , m_BindingFilePath(std::move(bindingFilePath))
    {
        m_Bindings.reserve(bindings.size());
        for (auto& binding : bindings)
        {
            m_BindingIndexByName.emplace(binding.Name, m_Bindings.size());
            m_Bindings.push_back(BindingState{
                .Descriptor = std::move(binding)
            });
        }

        LoadBindingFile();
    }

    void Controller::Start()
    {
        for (auto& binding : m_Bindings)
        {
            RestoreBinding(binding);
        }

        PublishAllBindingsConfigured();
        emit StatusMessageChanged("Ready to bind fake inputs.");
    }

    void Controller::Capture(const QString& name)
    {
        if (!m_PendingCaptureName.empty())
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
        m_PendingCaptureName = binding.Descriptor.Name;
        emit CaptureTargetChanged(name);
        emit CaptureInProgressChanged(true);

        if (binding.Descriptor.Type == BindingType::Axis)
        {
            emit StatusMessageChanged(QString("Capturing axis for %1...").arg(name));
            StartAxisCapture(binding);
            return;
        }

        emit StatusMessageChanged(QString("Capturing key for %1...").arg(name));
        StartKeyCapture(binding);
    }

    void Controller::CancelCapture()
    {
        if (m_PendingCaptureName.empty())
        {
            return;
        }

        m_Binder.StopCapture();
    }

    void Controller::LoadBindingFile()
    {
        if (!std::filesystem::exists(m_BindingFilePath))
        {
            return;
        }

        std::ifstream stream(m_BindingFilePath);
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
                binding.Serialized = DecodeBytes(encoded);
            }
            catch (const std::exception&)
            {
                binding.Serialized.clear();
            }
        }
    }

    void Controller::SaveBindingFile()
    {
        if (m_BindingFilePath.has_parent_path())
        {
            std::filesystem::create_directories(m_BindingFilePath.parent_path());
        }

        std::ofstream stream(m_BindingFilePath, std::ios::trunc);
        for (const auto& binding : m_Bindings)
        {
            if (binding.Serialized.empty())
            {
                continue;
            }

            stream << (binding.Descriptor.Type == BindingType::Axis ? "axis" : "key")
                   << '\t'
                   << binding.Descriptor.Name
                   << '\t'
                   << EncodeBytes(binding.Serialized)
                   << '\n';
        }
    }

    void Controller::RestoreBinding(BindingState& state)
    {
        if (state.Serialized.empty())
        {
            emit BindingHintChanged(QString::fromStdString(state.Descriptor.Name), "Unbound");
            return;
        }

        try
        {
            if (state.Descriptor.Type == BindingType::Axis)
            {
                state.AxisBinding = m_Serializer.DeserializeAxis(state.Serialized);
                state.Axis = m_Manager.CreateAxis(*state.AxisBinding);
                emit BindingHintChanged(
                    QString::fromStdString(state.Descriptor.Name),
                    QString::fromStdString(state.AxisBinding->GetHint()));
                emit OnAxisBound(QString::fromStdString(state.Descriptor.Name), state.Axis.get());
                return;
            }

            state.KeyBinding = m_Serializer.DeserializeKey(state.Serialized);
            state.Key = m_Manager.CreateKey(*state.KeyBinding);
            emit BindingHintChanged(
                QString::fromStdString(state.Descriptor.Name),
                QString::fromStdString(state.KeyBinding->GetHint()));
            emit OnKeyBound(QString::fromStdString(state.Descriptor.Name), state.Key.get());
        }
        catch (const std::exception&)
        {
            state.Serialized.clear();
            state.AxisBinding.reset();
            state.KeyBinding.reset();
            state.Axis.reset();
            state.Key.reset();
            emit BindingHintChanged(QString::fromStdString(state.Descriptor.Name), "Unbound");
        }

        PublishAllBindingsConfigured();
    }

    void Controller::StartAxisCapture(BindingState& state)
    {
        m_Binder.StartCapture([this, name = state.Descriptor.Name](
            const ::PiSubmarine::Input::Api::IBinder::CaptureStatus status,
            std::unique_ptr<::PiSubmarine::Input::Api::IAxisBinding> binding)
        {
            CompleteAxisCapture(name, status, std::move(binding));
        });
    }

    void Controller::StartKeyCapture(BindingState& state)
    {
        m_Binder.StartCapture([this, name = state.Descriptor.Name](
            const ::PiSubmarine::Input::Api::IBinder::CaptureStatus status,
            std::unique_ptr<::PiSubmarine::Input::Api::IKeyBinding> binding)
        {
            CompleteKeyCapture(name, status, std::move(binding));
        });
    }

    void Controller::CompleteAxisCapture(
        const std::string& name,
        const ::PiSubmarine::Input::Api::IBinder::CaptureStatus status,
        std::unique_ptr<::PiSubmarine::Input::Api::IAxisBinding> binding)
    {
        if (m_PendingCaptureName != name)
        {
            return;
        }

        m_PendingCaptureName.clear();
        emit CaptureTargetChanged({});
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
        state.Serialized = m_Serializer.Serialize(*binding);
        state.AxisBinding.reset();
        state.Axis.reset();
        RestoreBinding(state);
        SaveBindingFile();
        PublishAllBindingsConfigured();
        emit StatusMessageChanged(QString("Bound %1").arg(QString::fromStdString(name)));
    }

    void Controller::CompleteKeyCapture(
        const std::string& name,
        const ::PiSubmarine::Input::Api::IBinder::CaptureStatus status,
        std::unique_ptr<::PiSubmarine::Input::Api::IKeyBinding> binding)
    {
        if (m_PendingCaptureName != name)
        {
            return;
        }

        m_PendingCaptureName.clear();
        emit CaptureTargetChanged({});
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
        state.Serialized = m_Serializer.Serialize(*binding);
        state.KeyBinding.reset();
        state.Key.reset();
        RestoreBinding(state);
        SaveBindingFile();
        PublishAllBindingsConfigured();
        emit StatusMessageChanged(QString("Bound %1").arg(QString::fromStdString(name)));
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
            if (binding.Descriptor.Type == BindingType::Axis)
            {
                if (binding.Axis == nullptr)
                {
                    return false;
                }
            }
            else if (binding.Key == nullptr)
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
            const auto high = decodeNibble(text[index]);
            const auto low = decodeNibble(text[index + 1]);
            result.push_back(static_cast<std::byte>((high << 4) | low));
        }
        return result;
    }
}
