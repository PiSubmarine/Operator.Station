#include "PiSubmarine/Operator/Station/Telemetry/View/Time/ViewModel.h"

namespace PiSubmarine::Operator::Station::Telemetry::View::Time
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    bool ViewModel::GetHasLease() const { return m_HasLease; }
    QString ViewModel::GetDisplayText() const { return m_DisplayText; }
    int ViewModel::GetFaultState() const { return static_cast<int>(m_FaultState); }

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

    void ViewModel::SetSnapshot(
        const QString& displayText,
        const ::PiSubmarine::Operator::Station::Telemetry::View::Time::FaultState faultState)
    {
        if (m_DisplayText == displayText &&
            m_FaultState == faultState)
        {
            return;
        }

        m_DisplayText = displayText;
        m_FaultState = faultState;
        emit SnapshotChanged();
    }
}
