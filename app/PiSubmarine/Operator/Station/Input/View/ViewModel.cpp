#include "PiSubmarine/Operator/Station/Input/View/ViewModel.h"

namespace PiSubmarine::Operator::Station::Input::View
{
    namespace
    {
        void EmitIntentSignals(ViewModel& viewModel)
        {
            emit viewModel.IntentChanged();
            emit viewModel.IntentUpdated(
                viewModel.GetSurge(),
                viewModel.GetYaw(),
                viewModel.GetBallast(),
                viewModel.GetLampIntensity(),
                viewModel.GetHoldPosition());
        }
    }

    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    double ViewModel::GetSurge() const { return m_Surge; }
    double ViewModel::GetYaw() const { return m_Yaw; }
    double ViewModel::GetBallast() const { return m_Ballast; }
    double ViewModel::GetLampIntensity() const { return m_LampIntensity; }
    bool ViewModel::GetHoldPosition() const { return m_HoldPosition; }

    void ViewModel::SetSurge(const double surge)
    {
        if (m_Surge == surge) return;
        m_Surge = surge;
        EmitIntentSignals(*this);
    }

    void ViewModel::SetYaw(const double yaw)
    {
        if (m_Yaw == yaw) return;
        m_Yaw = yaw;
        EmitIntentSignals(*this);
    }

    void ViewModel::SetBallast(const double ballast)
    {
        if (m_Ballast == ballast) return;
        m_Ballast = ballast;
        EmitIntentSignals(*this);
    }

    void ViewModel::SetLampIntensity(const double lampIntensity)
    {
        if (m_LampIntensity == lampIntensity) return;
        m_LampIntensity = lampIntensity;
        EmitIntentSignals(*this);
    }

    void ViewModel::SetHoldPosition(const bool holdPosition)
    {
        if (m_HoldPosition == holdPosition) return;
        m_HoldPosition = holdPosition;
        EmitIntentSignals(*this);
    }
}
