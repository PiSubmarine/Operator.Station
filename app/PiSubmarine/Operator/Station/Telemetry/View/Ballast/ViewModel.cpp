#include "PiSubmarine/Operator/Station/Telemetry/View/Ballast/ViewModel.h"

namespace PiSubmarine::Operator::Station::Telemetry::View::Ballast
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    bool ViewModel::GetHasPosition() const { return m_HasPosition; }
    double ViewModel::GetPosition() const { return m_Position; }

    void ViewModel::SetSnapshot(const bool hasPosition, const double position)
    {
        if (m_HasPosition == hasPosition && m_Position == position)
        {
            return;
        }

        m_HasPosition = hasPosition;
        m_Position = position;
        emit SnapshotChanged();
    }
}
