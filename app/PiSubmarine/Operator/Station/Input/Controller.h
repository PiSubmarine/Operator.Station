#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <QObject>
#include <QString>

#include "PiSubmarine/Input/Api/IBinder.h"
#include "PiSubmarine/Input/Api/IManager.h"
#include "PiSubmarine/Input/Api/ISerializer.h"
#include "PiSubmarine/Operator/Station/Input/AxisMultiplexor.h"
#include "PiSubmarine/Operator/Station/Input/BindingDescriptor.h"
#include "PiSubmarine/Operator/Station/Input/BindingDevice.h"
#include "PiSubmarine/Operator/Station/Input/KeyMultiplexor.h"
#include "PiSubmarine/Time/ITickable.h"

namespace PiSubmarine::Operator::Station::Input
{
    class Controller final : public QObject, public ::PiSubmarine::Time::ITickable
    {
        Q_OBJECT

    public:
        struct InputSystem
        {
            ::PiSubmarine::Input::Api::IManager& Manager;
            ::PiSubmarine::Input::Api::IBinder& Binder;
            ::PiSubmarine::Input::Api::ISerializer& Serializer;
            std::filesystem::path BindingFilePath;
        };

        Controller(
            InputSystem keyboardInputSystem,
            InputSystem gamepadInputSystem,
            std::vector<BindingDescriptor> bindings,
            QObject* parent = nullptr);

        void Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime) override;

    public slots:
        void Start();
        void Capture(const QString& name, BindingDevice device);
        void CancelCapture();

    signals:
        void BindingHintChanged(const QString& name, BindingDevice device, const QString& hint);
        void AllBindingsConfiguredChanged(bool allBindingsConfigured);
        void CaptureTargetChanged(const QString& name, BindingDevice device);
        void CaptureInProgressChanged(bool captureInProgress);
        void StatusMessageChanged(const QString& message);
        void OnAxisBound(const QString& name, ::PiSubmarine::Input::Api::IAxis* axis);
        void OnKeyBound(const QString& name, ::PiSubmarine::Input::Api::IKey* key);

    private:
        struct DeviceBindingState
        {
            std::vector<std::byte> Serialized;
            std::unique_ptr<::PiSubmarine::Input::Api::IAxisBinding> AxisBinding;
            std::unique_ptr<::PiSubmarine::Input::Api::IKeyBinding> KeyBinding;
            std::unique_ptr<::PiSubmarine::Input::Api::IAxis> Axis;
            std::unique_ptr<::PiSubmarine::Input::Api::IKey> Key;
        };

        struct BindingState
        {
            BindingDescriptor Descriptor;
            DeviceBindingState Keyboard;
            DeviceBindingState Gamepad;
            std::unique_ptr<AxisMultiplexor> AxisProxy;
            std::unique_ptr<KeyMultiplexor> KeyProxy;
        };

        struct PendingCapture
        {
            std::string Name;
            BindingDevice Device;
        };

        void ValidateInputSystems();
        void LoadBindingFile(BindingDevice device);
        void LoadSharedBindingFile();
        void SaveBindingFile(BindingDevice device) const;
        void SaveSharedBindingFile() const;
        void RestoreBinding(BindingState& state);
        void RestoreDeviceBinding(BindingState& state, BindingDevice device);
        void PublishBoundInput(BindingState& state);
        void UpdateProxy(BindingState& state);
        void StartAxisCapture(BindingState& state, BindingDevice device);
        void StartKeyCapture(BindingState& state, BindingDevice device);
        void CompleteAxisCapture(
            const std::string& name,
            BindingDevice device,
            ::PiSubmarine::Input::Api::IBinder::CaptureStatus status,
            std::unique_ptr<::PiSubmarine::Input::Api::IAxisBinding> binding);
        void CompleteKeyCapture(
            const std::string& name,
            BindingDevice device,
            ::PiSubmarine::Input::Api::IBinder::CaptureStatus status,
            std::unique_ptr<::PiSubmarine::Input::Api::IKeyBinding> binding);
        void PublishAllBindingsConfigured();
        [[nodiscard]] bool AreAllBindingsConfigured() const;
        [[nodiscard]] static QString DescribeStatus(::PiSubmarine::Input::Api::IBinder::CaptureStatus status);
        [[nodiscard]] static QString DescribeDevice(BindingDevice device);
        [[nodiscard]] static std::string EncodeDevice(BindingDevice device);
        [[nodiscard]] static std::optional<BindingDevice> DecodeDevice(const std::string& token);
        [[nodiscard]] static std::string EncodeBytes(const std::vector<std::byte>& data);
        [[nodiscard]] static std::vector<std::byte> DecodeBytes(const std::string& text);
        [[nodiscard]] InputSystem& GetInputSystem(BindingDevice device);
        [[nodiscard]] const InputSystem& GetInputSystem(BindingDevice device) const;
        [[nodiscard]] DeviceBindingState& GetDeviceBinding(BindingState& state, BindingDevice device);
        [[nodiscard]] const DeviceBindingState& GetDeviceBinding(const BindingState& state, BindingDevice device) const;

        InputSystem m_KeyboardInputSystem;
        InputSystem m_GamepadInputSystem;
        bool m_InputSystemsAreShared = false;
        std::vector<BindingState> m_Bindings;
        std::unordered_map<std::string, std::size_t> m_BindingIndexByName;
        std::optional<PendingCapture> m_PendingCapture;
        bool m_LastAllBindingsConfigured = false;
    };
}
