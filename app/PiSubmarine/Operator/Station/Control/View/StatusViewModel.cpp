#include "PiSubmarine/Operator/Station/Control/View/StatusViewModel.h"

namespace PiSubmarine::Operator::Station::Control::View
{
    StatusViewModel::StatusViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    QString StatusViewModel::GetBackgroundColor() const
    {
        if (!m_HasLease)
        {
            return "#8f1d1d";
        }

        if (!m_AllBindingsConfigured)
        {
            return "#b78a1e";
        }

        return "#1d7f3b";
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

    void StatusViewModel::SetLeaseState(const bool hasLease)
    {
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
