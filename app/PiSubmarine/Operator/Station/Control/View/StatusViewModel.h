#pragma once

#include <QObject>
#include <QString>

#include "PiSubmarine/Operator/Station/Composition/LeaseState.h"

namespace PiSubmarine::Operator::Station::Control::View
{
    class StatusViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString statusKind READ GetStatusKind NOTIFY StateChanged)
        Q_PROPERTY(bool bindingDialogVisible READ GetBindingDialogVisible NOTIFY StateChanged)
        Q_PROPERTY(QString symbol READ GetSymbol CONSTANT)

    public:
        explicit StatusViewModel(QObject* parent = nullptr);

        [[nodiscard]] QString GetStatusKind() const;
        [[nodiscard]] bool GetBindingDialogVisible() const;
        [[nodiscard]] QString GetSymbol() const;

        Q_INVOKABLE void OpenBindingDialog();
        Q_INVOKABLE void CloseBindingDialog();
        Q_INVOKABLE void ToggleBindingDialog();

    public slots:
        void LeaseStateChanged(const ::PiSubmarine::Operator::Station::Composition::OptionalLeaseId& leaseId);
        void SetAllBindingsConfigured(bool allBindingsConfigured);

    signals:
        void StateChanged();

    private:
        bool m_HasLease = false;
        bool m_AllBindingsConfigured = false;
        bool m_BindingDialogVisible = false;
    };
}
