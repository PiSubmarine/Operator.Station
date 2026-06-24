#include "PiSubmarine/Operator/Station/Input/View/BindingViewModel.h"

namespace PiSubmarine::Operator::Station::Input::View
{
    BindingViewModel::BindingViewModel(
        const std::vector<::PiSubmarine::Operator::Station::Input::BindingDescriptor>& descriptors,
        QObject* parent)
        : QObject(parent)
    {
        m_Entries.reserve(descriptors.size());
        m_EntryObjects.reserve(static_cast<qsizetype>(descriptors.size()));

        for (const auto& descriptor : descriptors)
        {
            auto* entry = new BindingEntryViewModel(QString::fromStdString(descriptor.Name), this);
            m_Entries.push_back(entry);
            m_EntryObjects.push_back(QVariant::fromValue(static_cast<QObject*>(entry)));
        }
    }

    QVariantList BindingViewModel::GetEntries() const
    {
        return m_EntryObjects;
    }

    bool BindingViewModel::GetCaptureInProgress() const
    {
        return m_CaptureInProgress;
    }

    void BindingViewModel::Capture(const QString& name, const int device)
    {
        emit RequestCapture(name, static_cast<BindingDevice>(device));
    }

    void BindingViewModel::CancelCapture()
    {
        emit RequestCancelCapture();
    }

    void BindingViewModel::SetBindingHint(const QString& name, const BindingDevice device, const QString& hint)
    {
        for (auto* entry : m_Entries)
        {
            if (entry->GetName() != name)
            {
                continue;
            }

            if (device == BindingDevice::Keyboard)
            {
                entry->SetKeyboardHint(hint);
            }
            else
            {
                entry->SetGamepadHint(hint);
            }
            return;
        }
    }

    void BindingViewModel::SetCaptureTarget(const QString& name, const BindingDevice device)
    {
        for (auto* entry : m_Entries)
        {
            const bool isTarget = entry->GetName() == name && !name.isEmpty();
            entry->SetKeyboardCapturing(isTarget && device == BindingDevice::Keyboard);
            entry->SetGamepadCapturing(isTarget && device == BindingDevice::Gamepad);
        }
    }

    void BindingViewModel::SetCaptureInProgress(const bool captureInProgress)
    {
        if (m_CaptureInProgress == captureInProgress)
        {
            return;
        }

        m_CaptureInProgress = captureInProgress;
        emit CaptureInProgressChanged();
    }
}
