#include "PiSubmarine/Operator/Station/Control/View/ViewModel.h"

namespace PiSubmarine::Operator::Station::Control::View
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    double ViewModel::GetLampIntensity() const { return m_LampIntensity; }
    bool ViewModel::GetCameraEnabled() const { return m_CameraEnabled; }
    bool ViewModel::GetAutoFocusEnabled() const { return m_AutoFocusEnabled; }
    double ViewModel::GetManualFocusPosition() const { return m_ManualFocusPosition; }
    int ViewModel::GetStreamProfile() const { return m_StreamProfile; }
    double ViewModel::GetGimbalPitchDegrees() const { return m_GimbalPitchDegrees; }
    bool ViewModel::GetHoldPositionMode() const { return m_HoldPositionMode; }
    int ViewModel::GetVerticalMode() const { return m_VerticalMode; }
    double ViewModel::GetBallastPosition() const { return m_BallastPosition; }
    double ViewModel::GetDepthTargetMeters() const { return m_DepthTargetMeters; }

    void ViewModel::IncreaseLampIntensity()
    {
        emit LampChangeRequested(0.25);
    }

    void ViewModel::DecreaseLampIntensity()
    {
        emit LampChangeRequested(-0.25);
    }

    void ViewModel::SetLampIntensity(const double lampIntensity)
    {
        if (m_LampIntensity == lampIntensity)
        {
            return;
        }
        m_LampIntensity = lampIntensity;
        emit LampIntensityChanged();
    }

    void ViewModel::EnableCamera()
    {
        emit CameraEnableRequested();
    }

    void ViewModel::DisableCamera()
    {
        emit CameraDisableRequested();
    }

    void ViewModel::SetAutoFocusEnabled()
    {
        emit AutoFocusRequested();
    }

    void ViewModel::SetManualFocusEnabled()
    {
        emit ManualFocusRequested();
    }

    void ViewModel::SetManualFocusPosition(const double position)
    {
        emit ManualFocusPositionRequested(position);
    }

    void ViewModel::SetLowQualityStreamProfile()
    {
        emit LowQualityProfileRequested();
    }

    void ViewModel::SetMediumQualityStreamProfile()
    {
        emit MediumQualityProfileRequested();
    }

    void ViewModel::SetHighQualityStreamProfile()
    {
        emit HighQualityProfileRequested();
    }

    void ViewModel::SetCameraIntent(
        const bool isEnabled,
        const bool isAutoFocus,
        const double focusPosition,
        const int streamProfile)
    {
        if (m_CameraEnabled == isEnabled &&
            m_AutoFocusEnabled == isAutoFocus &&
            m_ManualFocusPosition == focusPosition &&
            m_StreamProfile == streamProfile)
        {
            return;
        }

        m_CameraEnabled = isEnabled;
        m_AutoFocusEnabled = isAutoFocus;
        m_ManualFocusPosition = focusPosition;
        m_StreamProfile = streamProfile;
        emit CameraIntentChanged();
    }

    void ViewModel::SetGimbalPitchDegrees(const double pitchDegrees)
    {
        if (m_GimbalPitchDegrees == pitchDegrees)
        {
            return;
        }

        m_GimbalPitchDegrees = pitchDegrees;
        emit GimbalIntentChanged();
    }

    void ViewModel::SetManualMode()
    {
        emit ManualModeRequested();
    }

    void ViewModel::SetHoldPositionMode()
    {
        emit HoldPositionModeRequested();
    }

    void ViewModel::SetModeIntent(const bool isHoldPosition)
    {
        if (m_HoldPositionMode == isHoldPosition)
        {
            return;
        }

        m_HoldPositionMode = isHoldPosition;
        emit ModeIntentChanged();
    }

    void ViewModel::SetVerticalKeepCurrentMode()
    {
        emit VerticalKeepCurrentRequested();
    }

    void ViewModel::SetVerticalBallastPositionMode()
    {
        emit VerticalBallastPositionRequested();
    }

    void ViewModel::SetVerticalDepthTargetMode()
    {
        emit VerticalDepthTargetRequested();
    }

    void ViewModel::SetVerticalIntent(
        const int verticalMode,
        const double ballastPosition,
        const double depthTargetMeters)
    {
        if (m_VerticalMode == verticalMode &&
            m_BallastPosition == ballastPosition &&
            m_DepthTargetMeters == depthTargetMeters)
        {
            return;
        }

        m_VerticalMode = verticalMode;
        m_BallastPosition = ballastPosition;
        m_DepthTargetMeters = depthTargetMeters;
        emit VerticalIntentChanged();
    }
}
