#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <QObject>
#include <QString>

#include "PiSubmarine/Input/Api/IBinder.h"
#include "PiSubmarine/Input/Api/IManager.h"
#include "PiSubmarine/Input/Api/ISerializer.h"
#include "PiSubmarine/Operator/Station/Input/BindingDescriptor.h"

namespace PiSubmarine::Operator::Station::Input
{
    class Controller final : public QObject
    {
        Q_OBJECT

    public:
        Controller(
            ::PiSubmarine::Input::Api::IManager& manager,
            ::PiSubmarine::Input::Api::IBinder& binder,
            ::PiSubmarine::Input::Api::ISerializer& serializer,
            std::filesystem::path bindingFilePath,
            std::vector<BindingDescriptor> bindings,
            QObject* parent = nullptr);

    public slots:
        void Start();
        void Capture(const QString& name);
        void CancelCapture();

    signals:
        void BindingHintChanged(const QString& name, const QString& hint);
        void AllBindingsConfiguredChanged(bool allBindingsConfigured);
        void CaptureTargetChanged(const QString& name);
        void CaptureInProgressChanged(bool captureInProgress);
        void StatusMessageChanged(const QString& message);
        void OnAxisBound(const QString& name, ::PiSubmarine::Input::Api::IAxis* axis);
        void OnKeyBound(const QString& name, ::PiSubmarine::Input::Api::IKey* key);

    private:
        struct BindingState
        {
            BindingDescriptor Descriptor;
            std::vector<std::byte> Serialized;
            std::unique_ptr<::PiSubmarine::Input::Api::IAxisBinding> AxisBinding;
            std::unique_ptr<::PiSubmarine::Input::Api::IKeyBinding> KeyBinding;
            std::unique_ptr<::PiSubmarine::Input::Api::IAxis> Axis;
            std::unique_ptr<::PiSubmarine::Input::Api::IKey> Key;
        };

        void LoadBindingFile();
        void SaveBindingFile();
        void RestoreBinding(BindingState& state);
        void StartAxisCapture(BindingState& state);
        void StartKeyCapture(BindingState& state);
        void CompleteAxisCapture(
            const std::string& name,
            ::PiSubmarine::Input::Api::IBinder::CaptureStatus status,
            std::unique_ptr<::PiSubmarine::Input::Api::IAxisBinding> binding);
        void CompleteKeyCapture(
            const std::string& name,
            ::PiSubmarine::Input::Api::IBinder::CaptureStatus status,
            std::unique_ptr<::PiSubmarine::Input::Api::IKeyBinding> binding);
        void PublishAllBindingsConfigured();
        [[nodiscard]] bool AreAllBindingsConfigured() const;
        static QString DescribeStatus(::PiSubmarine::Input::Api::IBinder::CaptureStatus status);
        static std::string EncodeBytes(const std::vector<std::byte>& data);
        static std::vector<std::byte> DecodeBytes(const std::string& text);

        ::PiSubmarine::Input::Api::IManager& m_Manager;
        ::PiSubmarine::Input::Api::IBinder& m_Binder;
        ::PiSubmarine::Input::Api::ISerializer& m_Serializer;
        std::filesystem::path m_BindingFilePath;
        std::vector<BindingState> m_Bindings;
        std::unordered_map<std::string, std::size_t> m_BindingIndexByName;
        std::string m_PendingCaptureName;
        bool m_LastAllBindingsConfigured = false;
    };
}
