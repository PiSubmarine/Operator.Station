#include "PiSubmarine/Operator/Station/Input/QtKeyboard/System.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <QString>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>

#include <QKeySequence>

namespace PiSubmarine::Operator::Station::Input::QtKeyboard
{
    namespace
    {
        [[nodiscard]] std::string DescribeKey(const int key)
        {
            const auto text = QKeySequence(key).toString(QKeySequence::NativeText);
            if (!text.isEmpty())
            {
                return text.toStdString();
            }

            return "Key " + std::to_string(key);
        }

        [[nodiscard]] std::string ToString(const std::vector<std::byte>& data)
        {
            return {reinterpret_cast<const char*>(data.data()), data.size()};
        }

        [[nodiscard]] std::vector<std::byte> ToBytes(const std::string& text)
        {
            std::vector<std::byte> result;
            result.reserve(text.size());
            for (const char character : text)
            {
                result.push_back(static_cast<std::byte>(character));
            }

            return result;
        }

        [[nodiscard]] int ParseSingleKey(const std::string& text)
        {
            const auto sequence = QKeySequence::fromString(QString::fromStdString(text), QKeySequence::NativeText);
            const auto key = sequence[0];
            if (key == 0)
            {
                throw std::invalid_argument("Failed to parse Qt keyboard key from hint.");
            }

            return key;
        }
    }

    class System::Impl
    {
    public:
        struct KeyEvent
        {
            int Key = 0;
            bool Pressed = false;
        };

        class KeyBinding final : public ::PiSubmarine::Input::Api::IKeyBinding
        {
        public:
            explicit KeyBinding(const int key)
                : m_Key(key)
            {
            }

            [[nodiscard]] int GetKey() const
            {
                return m_Key;
            }

            [[nodiscard]] std::string GetHint() const override
            {
                return DescribeKey(m_Key);
            }

        private:
            int m_Key = 0;
        };

        class AxisBinding final : public ::PiSubmarine::Input::Api::IAxisBinding
        {
        public:
            AxisBinding(const int positiveKey, const int negativeKey)
                : m_PositiveKey(positiveKey)
                , m_NegativeKey(negativeKey)
            {
            }

            [[nodiscard]] int GetPositiveKey() const
            {
                return m_PositiveKey;
            }

            [[nodiscard]] int GetNegativeKey() const
            {
                return m_NegativeKey;
            }

            [[nodiscard]] std::string GetHint() const override
            {
                return "+" + DescribeKey(m_PositiveKey) + ", -" + DescribeKey(m_NegativeKey);
            }

        private:
            int m_PositiveKey = 0;
            int m_NegativeKey = 0;
        };

        class Axis final : public ::PiSubmarine::Input::Api::IAxis
        {
        public:
            Axis(std::shared_ptr<Impl> state, AxisBinding binding)
                : m_State(std::move(state))
                , m_Binding(std::move(binding))
            {
            }

            [[nodiscard]] SignedNormalizedFraction GetValue() const override
            {
                const bool positivePressed = m_State->IsPressed(m_Binding.GetPositiveKey());
                const bool negativePressed = m_State->IsPressed(m_Binding.GetNegativeKey());

                if (positivePressed == negativePressed)
                {
                    return SignedNormalizedFraction(0.0);
                }

                return SignedNormalizedFraction(positivePressed ? 1.0 : -1.0);
            }

        private:
            std::shared_ptr<Impl> m_State;
            AxisBinding m_Binding;
        };

        class Key final : public ::PiSubmarine::Input::Api::IKey
        {
        public:
            Key(std::shared_ptr<Impl> state, KeyBinding binding)
                : m_State(std::move(state))
                , m_Binding(std::move(binding))
            {
            }

            [[nodiscard]] ::PiSubmarine::Input::Api::KeyState GetState() const override
            {
                return m_State->IsPressed(m_Binding.GetKey())
                    ? ::PiSubmarine::Input::Api::KeyState::Pressed
                    : ::PiSubmarine::Input::Api::KeyState::Released;
            }

        private:
            std::shared_ptr<Impl> m_State;
            KeyBinding m_Binding;
        };

        struct PendingAxisCapture
        {
            AxisCallback Callback;
            std::optional<int> PositiveKey;
        };

        void EnqueueKeyEvent(const int key, const bool pressed)
        {
            std::lock_guard lock(m_EventMutex);

            const bool alreadyPressed = m_ObservedPressedKeys.contains(key);
            if (pressed)
            {
                if (alreadyPressed)
                {
                    return;
                }

                m_ObservedPressedKeys.insert(key);
            }
            else
            {
                if (!alreadyPressed)
                {
                    return;
                }

                m_ObservedPressedKeys.erase(key);
            }

            m_PendingEvents.push_back(KeyEvent{
                .Key = key,
                .Pressed = pressed
            });
        }

        void EnqueueReleaseAll()
        {
            std::lock_guard lock(m_EventMutex);
            m_ObservedPressedKeys.clear();
            m_ReleaseAllRequested = true;
        }

        void Tick()
        {
            std::vector<KeyEvent> events;
            bool releaseAllRequested = false;

            {
                std::lock_guard lock(m_EventMutex);
                events = std::exchange(m_PendingEvents, {});
                releaseAllRequested = std::exchange(m_ReleaseAllRequested, false);
            }

            if (releaseAllRequested)
            {
                m_KeyStates.clear();
            }

            for (const auto& event : events)
            {
                m_KeyStates[event.Key] = event.Pressed;

                if (!event.Pressed)
                {
                    continue;
                }

                if (m_PendingKeyCapture.has_value())
                {
                    auto callback = std::move(m_PendingKeyCapture.value());
                    m_PendingKeyCapture.reset();
                    callback(::PiSubmarine::Input::Api::IBinder::CaptureStatus::Ok, std::make_unique<KeyBinding>(event.Key));
                    continue;
                }

                if (!m_PendingAxisCapture.has_value())
                {
                    continue;
                }

                if (!m_PendingAxisCapture->PositiveKey.has_value())
                {
                    m_PendingAxisCapture->PositiveKey = event.Key;
                    continue;
                }

                auto callback = std::move(m_PendingAxisCapture->Callback);
                const auto positiveKey = *m_PendingAxisCapture->PositiveKey;
                m_PendingAxisCapture.reset();

                if (positiveKey == event.Key)
                {
                    callback(::PiSubmarine::Input::Api::IBinder::CaptureStatus::UnknownError, nullptr);
                    continue;
                }

                callback(
                    ::PiSubmarine::Input::Api::IBinder::CaptureStatus::Ok,
                    std::make_unique<AxisBinding>(positiveKey, event.Key));
            }
        }

        [[nodiscard]] bool IsPressed(const int key) const
        {
            const auto keyStateIt = m_KeyStates.find(key);
            return keyStateIt != m_KeyStates.end() && keyStateIt->second;
        }

        void StartCapture(KeyCallback callback)
        {
            m_PendingAxisCapture.reset();
            m_PendingKeyCapture = std::move(callback);
        }

        void StartCapture(AxisCallback callback)
        {
            m_PendingKeyCapture.reset();
            m_PendingAxisCapture = PendingAxisCapture{
                .Callback = std::move(callback)
            };
        }

        void StopCapture()
        {
            if (m_PendingKeyCapture.has_value())
            {
                auto callback = std::move(m_PendingKeyCapture.value());
                m_PendingKeyCapture.reset();
                callback(::PiSubmarine::Input::Api::IBinder::CaptureStatus::Cancelled, nullptr);
            }

            if (m_PendingAxisCapture.has_value())
            {
                auto callback = std::move(m_PendingAxisCapture->Callback);
                m_PendingAxisCapture.reset();
                callback(::PiSubmarine::Input::Api::IBinder::CaptureStatus::Cancelled, nullptr);
            }
        }

        [[nodiscard]] static AxisBinding RequireAxisBinding(::PiSubmarine::Input::Api::IAxisBinding& binding)
        {
            const auto* axisBinding = dynamic_cast<AxisBinding*>(&binding);
            if (axisBinding == nullptr)
            {
                const auto hint = binding.GetHint();
                const auto separator = hint.find(',');
                if (separator == std::string::npos || separator < 2 || separator + 3 > hint.size())
                {
                    throw std::invalid_argument("Qt keyboard axis binding type mismatch.");
                }

                const auto positiveHint = hint.substr(1, separator - 1);
                const auto negativePrefix = hint.substr(separator + 1, 2);
                if (negativePrefix != " -")
                {
                    throw std::invalid_argument("Qt keyboard axis hint has invalid format.");
                }

                const auto negativeHint = hint.substr(separator + 3);
                return AxisBinding(ParseSingleKey(positiveHint), ParseSingleKey(negativeHint));
            }

            return *axisBinding;
        }

        [[nodiscard]] static KeyBinding RequireKeyBinding(::PiSubmarine::Input::Api::IKeyBinding& binding)
        {
            const auto* keyBinding = dynamic_cast<KeyBinding*>(&binding);
            if (keyBinding == nullptr)
            {
                return KeyBinding(ParseSingleKey(binding.GetHint()));
            }

            return *keyBinding;
        }

        [[nodiscard]] static std::vector<std::byte> SerializeAxis(::PiSubmarine::Input::Api::IAxisBinding& binding)
        {
            const auto axisBinding = RequireAxisBinding(binding);

            std::ostringstream stream;
            stream << axisBinding.GetPositiveKey() << '\n'
                   << axisBinding.GetNegativeKey();
            return ToBytes(stream.str());
        }

        [[nodiscard]] static std::unique_ptr<::PiSubmarine::Input::Api::IAxisBinding> DeserializeAxis(
            const std::vector<std::byte>& data)
        {
            std::istringstream stream(ToString(data));

            int positiveKey = 0;
            int negativeKey = 0;
            if (!(stream >> positiveKey))
            {
                throw std::invalid_argument("Failed to deserialize Qt keyboard positive axis key.");
            }
            if (!(stream >> negativeKey))
            {
                throw std::invalid_argument("Failed to deserialize Qt keyboard negative axis key.");
            }

            if (positiveKey == negativeKey)
            {
                throw std::invalid_argument("Qt keyboard axis keys must be distinct.");
            }

            return std::make_unique<AxisBinding>(positiveKey, negativeKey);
        }

        [[nodiscard]] static std::vector<std::byte> SerializeKey(::PiSubmarine::Input::Api::IKeyBinding& binding)
        {
            const auto keyBinding = RequireKeyBinding(binding);
            return ToBytes(std::to_string(keyBinding.GetKey()));
        }

        [[nodiscard]] static std::unique_ptr<::PiSubmarine::Input::Api::IKeyBinding> DeserializeKey(
            const std::vector<std::byte>& data)
        {
            std::istringstream stream(ToString(data));

            int key = 0;
            if (!(stream >> key))
            {
                throw std::invalid_argument("Failed to deserialize Qt keyboard key binding.");
            }

            return std::make_unique<KeyBinding>(key);
        }

    private:
        mutable std::mutex m_EventMutex;
        std::vector<KeyEvent> m_PendingEvents;
        bool m_ReleaseAllRequested = false;
        std::unordered_set<int> m_ObservedPressedKeys;
        std::unordered_map<int, bool> m_KeyStates;
        std::optional<KeyCallback> m_PendingKeyCapture;
        std::optional<PendingAxisCapture> m_PendingAxisCapture;
    };

    System::System()
        : m_Impl(std::make_shared<Impl>())
    {
    }

    System::~System() = default;

    std::unique_ptr<::PiSubmarine::Input::Api::IAxis> System::CreateAxis(::PiSubmarine::Input::Api::IAxisBinding& binding)
    {
        return std::make_unique<Impl::Axis>(m_Impl, Impl::RequireAxisBinding(binding));
    }

    std::unique_ptr<::PiSubmarine::Input::Api::IKey> System::CreateKey(::PiSubmarine::Input::Api::IKeyBinding& binding)
    {
        return std::make_unique<Impl::Key>(m_Impl, Impl::RequireKeyBinding(binding));
    }

    void System::StartCapture(KeyCallback callback)
    {
        m_Impl->StartCapture(std::move(callback));
    }

    void System::StartCapture(AxisCallback callback)
    {
        m_Impl->StartCapture(std::move(callback));
    }

    void System::StopCapture()
    {
        m_Impl->StopCapture();
    }

    std::vector<std::byte> System::Serialize(::PiSubmarine::Input::Api::IAxisBinding& binding) const
    {
        return Impl::SerializeAxis(binding);
    }

    std::unique_ptr<::PiSubmarine::Input::Api::IAxisBinding> System::DeserializeAxis(const std::vector<std::byte>& data) const
    {
        return Impl::DeserializeAxis(data);
    }

    std::vector<std::byte> System::Serialize(::PiSubmarine::Input::Api::IKeyBinding& binding) const
    {
        return Impl::SerializeKey(binding);
    }

    std::unique_ptr<::PiSubmarine::Input::Api::IKeyBinding> System::DeserializeKey(const std::vector<std::byte>& data) const
    {
        return Impl::DeserializeKey(data);
    }

    void System::Tick(const std::chrono::nanoseconds&, const std::chrono::nanoseconds&)
    {
        m_Impl->Tick();
    }

    void System::PushKeyEvent(const int key, const bool pressed)
    {
        m_Impl->EnqueueKeyEvent(key, pressed);
    }

    void System::ReleaseAllKeys()
    {
        m_Impl->EnqueueReleaseAll();
    }
}
