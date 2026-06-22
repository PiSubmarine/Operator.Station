#include "PiSubmarine/Operator/Station/Telemetry/View/Time/ViewModel.h"

namespace PiSubmarine::Operator::Station::Telemetry::View::Time
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    bool ViewModel::GetHasLease() const { return m_HasLease; }
    QString ViewModel::GetDisplayText() const { return m_DisplayText; }
    QString ViewModel::GetBackgroundColor() const { return m_BackgroundColor; }

    void ViewModel::LeaseStateChanged(const ::PiSubmarine::Operator::Station::Composition::OptionalLeaseId& leaseId)
    {
        const bool hasLease = leaseId.has_value();
        if (m_HasLease == hasLease)
        {
            return;
        }

        m_HasLease = hasLease;
        emit SnapshotChanged();
    }

    void ViewModel::SetSnapshot(const QString& displayText, const QString& backgroundColor)
    {
        if (m_DisplayText == displayText &&
            m_BackgroundColor == backgroundColor)
        {
            return;
        }

        m_DisplayText = displayText;
        m_BackgroundColor = backgroundColor;
        emit SnapshotChanged();
    }
}
