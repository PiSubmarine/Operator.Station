#include "PiSubmarine/Operator/Station/Video/View/StatusOverlayViewModel.h"

namespace PiSubmarine::Operator::Station::Video::View
{
    StatusOverlayViewModel::StatusOverlayViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    bool StatusOverlayViewModel::GetIsOverlayVisible() const
    {
        return m_Status.IsStarted && !m_Status.HasLease;
    }

    QString StatusOverlayViewModel::GetOverlayMessage() const
    {
        if (GetIsOverlayVisible())
        {
            return "NO CAMERA LEASE";
        }

        return {};
    }

    QString StatusOverlayViewModel::GetOverlayBackgroundColor() const
    {
        if (GetIsOverlayVisible())
        {
            return "#8f1d1d";
        }

        return "transparent";
    }

    void StatusOverlayViewModel::SetStatus(const PiSubmarine::Operator::Station::Video::Status& status)
    {
        if (m_Status == status)
        {
            return;
        }

        m_Status = status;
        emit SnapshotChanged();
    }
}
