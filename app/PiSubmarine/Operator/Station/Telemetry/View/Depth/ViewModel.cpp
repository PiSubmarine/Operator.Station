#include "PiSubmarine/Operator/Station/Telemetry/View/Depth/ViewModel.h"

namespace PiSubmarine::Operator::Station::Telemetry::View::Depth
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    bool ViewModel::GetHasDepth() const { return m_HasDepth; }
    double ViewModel::GetDepthMeters() const { return m_DepthMeters; }

    void ViewModel::SetSnapshot(const bool hasDepth, const double depthMeters)
    {
        if (m_HasDepth == hasDepth && m_DepthMeters == depthMeters)
        {
            return;
        }

        m_HasDepth = hasDepth;
        m_DepthMeters = depthMeters;
        emit SnapshotChanged();
    }
}
