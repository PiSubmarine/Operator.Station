#pragma once

#include <vector>

#include <QObject>
#include <QString>
#include <QVariantList>

#include "PiSubmarine/Operator/Station/Input/BindingDescriptor.h"
#include "PiSubmarine/Operator/Station/Input/BindingDevice.h"
#include "PiSubmarine/Operator/Station/Input/View/BindingEntryViewModel.h"

namespace PiSubmarine::Operator::Station::Input::View
{
    class BindingViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QVariantList entries READ GetEntries CONSTANT)
        Q_PROPERTY(bool captureInProgress READ GetCaptureInProgress NOTIFY CaptureInProgressChanged)

    public:
        explicit BindingViewModel(
            const std::vector<::PiSubmarine::Operator::Station::Input::BindingDescriptor>& descriptors,
            QObject* parent = nullptr);

        [[nodiscard]] QVariantList GetEntries() const;
        [[nodiscard]] bool GetCaptureInProgress() const;

        Q_INVOKABLE void Capture(const QString& name, int device);
        Q_INVOKABLE void CancelCapture();

    public slots:
        void SetBindingHint(const QString& name, BindingDevice device, const QString& hint);
        void SetCaptureTarget(const QString& name, BindingDevice device);
        void SetCaptureInProgress(bool captureInProgress);

    signals:
        void RequestCapture(const QString& name, BindingDevice device);
        void RequestCancelCapture();
        void CaptureInProgressChanged();

    private:
        std::vector<BindingEntryViewModel*> m_Entries;
        QVariantList m_EntryObjects;
        bool m_CaptureInProgress = false;
    };
}
