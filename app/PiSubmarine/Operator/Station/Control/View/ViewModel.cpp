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
}
