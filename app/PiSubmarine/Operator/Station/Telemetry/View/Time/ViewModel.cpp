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

    void ViewModel::SetSnapshot(const bool hasLease, const QString& displayText, const QString& backgroundColor)
    {
        if (m_HasLease == hasLease &&
            m_DisplayText == displayText &&
            m_BackgroundColor == backgroundColor)
        {
            return;
        }

        m_HasLease = hasLease;
        m_DisplayText = displayText;
        m_BackgroundColor = backgroundColor;
        emit SnapshotChanged();
    }
}
