#include "PiSubmarine/Operator/Station/Telemetry/View/Proximity/ViewModel.h"

namespace PiSubmarine::Operator::Station::Telemetry::View::Proximity
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    bool ViewModel::GetHasDistance() const { return m_HasDistance; }
    double ViewModel::GetDistanceMeters() const { return m_DistanceMeters; }

    void ViewModel::SetSnapshot(const bool hasDistance, const double distanceMeters)
    {
        if (m_HasDistance == hasDistance && m_DistanceMeters == distanceMeters)
        {
            return;
        }

        m_HasDistance = hasDistance;
        m_DistanceMeters = distanceMeters;
        emit SnapshotChanged();
    }
}
