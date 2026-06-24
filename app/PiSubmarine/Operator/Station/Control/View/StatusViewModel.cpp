#include "PiSubmarine/Operator/Station/Control/View/StatusViewModel.h"

namespace PiSubmarine::Operator::Station::Control::View
{
    StatusViewModel::StatusViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    QString StatusViewModel::GetStatusKind() const
    {
        if (!m_HasLease)
        {
            return "fault";
        }

        if (!m_AllBindingsConfigured)
        {
            return "warning";
        }

        return "normal";
    }

    bool StatusViewModel::GetBindingDialogVisible() const
    {
        return m_BindingDialogVisible;
    }

    QString StatusViewModel::GetSymbol() const
    {
        return "C";
    }

    void StatusViewModel::OpenBindingDialog()
    {
        if (m_BindingDialogVisible)
        {
            return;
        }

        m_BindingDialogVisible = true;
        emit StateChanged();
    }

    void StatusViewModel::CloseBindingDialog()
    {
        if (!m_BindingDialogVisible)
        {
            return;
        }

        m_BindingDialogVisible = false;
        emit StateChanged();
    }

    void StatusViewModel::ToggleBindingDialog()
    {
        m_BindingDialogVisible = !m_BindingDialogVisible;
        emit StateChanged();
    }

    void StatusViewModel::LeaseStateChanged(const ::PiSubmarine::Operator::Station::Composition::OptionalLeaseId& leaseId)
    {
        const bool hasLease = leaseId.has_value();
        if (m_HasLease == hasLease)
        {
            return;
        }

        m_HasLease = hasLease;
        emit StateChanged();
    }

    void StatusViewModel::SetAllBindingsConfigured(const bool allBindingsConfigured)
    {
        if (m_AllBindingsConfigured == allBindingsConfigured)
        {
            return;
        }

        m_AllBindingsConfigured = allBindingsConfigured;
        emit StateChanged();
    }
}
