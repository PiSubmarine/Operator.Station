#pragma once

#include <vector>

#include <QObject>
#include <QString>
#include <QVariantList>

#include "PiSubmarine/Operator/Station/Input/BindingDescriptor.h"
#include "PiSubmarine/Operator/Station/Input/View/BindingEntryViewModel.h"

namespace PiSubmarine::Operator::Station::Input::View
{
    class BindingViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QVariantList entries READ GetEntries CONSTANT)
        Q_PROPERTY(bool captureInProgress READ GetCaptureInProgress NOTIFY CaptureInProgressChanged)
        Q_PROPERTY(QString statusMessage READ GetStatusMessage NOTIFY StatusMessageChanged)

    public:
        explicit BindingViewModel(const std::vector<::PiSubmarine::Operator::Station::Input::BindingDescriptor>& descriptors, QObject* parent = nullptr);

        [[nodiscard]] QVariantList GetEntries() const;
        [[nodiscard]] bool GetCaptureInProgress() const;
        [[nodiscard]] QString GetStatusMessage() const;

        Q_INVOKABLE void Capture(const QString& name);
        Q_INVOKABLE void CancelCapture();

    public slots:
        void SetBindingHint(const QString& name, const QString& hint);
        void SetCaptureInProgress(bool captureInProgress);
        void SetStatusMessage(const QString& statusMessage);

    signals:
        void RequestCapture(const QString& name);
        void RequestCancelCapture();
        void CaptureInProgressChanged();
        void StatusMessageChanged();

    private:
        std::vector<BindingEntryViewModel*> m_Entries;
        QVariantList m_EntryObjects;
        bool m_CaptureInProgress = false;
        QString m_StatusMessage = "Ready to bind fake inputs.";
    };
}
