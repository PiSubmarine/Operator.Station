#pragma once

#include <QObject>

namespace PiSubmarine::Operator::Station::Control::View
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(double lampIntensity READ GetLampIntensity NOTIFY LampIntensityChanged)
        Q_PROPERTY(bool cameraEnabled READ GetCameraEnabled NOTIFY CameraIntentChanged)
        Q_PROPERTY(bool autoFocusEnabled READ GetAutoFocusEnabled NOTIFY CameraIntentChanged)
        Q_PROPERTY(double manualFocusPosition READ GetManualFocusPosition NOTIFY CameraIntentChanged)
        Q_PROPERTY(int streamProfile READ GetStreamProfile NOTIFY CameraIntentChanged)
        Q_PROPERTY(double gimbalPitchDegrees READ GetGimbalPitchDegrees NOTIFY GimbalIntentChanged)
        Q_PROPERTY(bool holdPositionMode READ GetHoldPositionMode NOTIFY ModeIntentChanged)
        Q_PROPERTY(int verticalMode READ GetVerticalMode NOTIFY VerticalIntentChanged)
        Q_PROPERTY(double ballastPosition READ GetBallastPosition NOTIFY VerticalIntentChanged)
        Q_PROPERTY(double depthTargetMeters READ GetDepthTargetMeters NOTIFY VerticalIntentChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] double GetLampIntensity() const;
        [[nodiscard]] bool GetCameraEnabled() const;
        [[nodiscard]] bool GetAutoFocusEnabled() const;
        [[nodiscard]] double GetManualFocusPosition() const;
        [[nodiscard]] int GetStreamProfile() const;
        [[nodiscard]] double GetGimbalPitchDegrees() const;
        [[nodiscard]] bool GetHoldPositionMode() const;
        [[nodiscard]] int GetVerticalMode() const;
        [[nodiscard]] double GetBallastPosition() const;
        [[nodiscard]] double GetDepthTargetMeters() const;

    signals:
        void LampIntensityChanged();
        void LampChangeRequested(double step);
        void CameraIntentChanged();
        void CameraEnableRequested();
        void CameraDisableRequested();
        void AutoFocusRequested();
        void ManualFocusRequested();
        void ManualFocusPositionRequested(double position);
        void LowQualityProfileRequested();
        void MediumQualityProfileRequested();
        void HighQualityProfileRequested();
        void GimbalIntentChanged();
        void ModeIntentChanged();
        void ManualModeRequested();
        void HoldPositionModeRequested();
        void VerticalIntentChanged();
        void VerticalKeepCurrentRequested();
        void VerticalBallastPositionRequested();
        void VerticalDepthTargetRequested();

    public slots:
        void IncreaseLampIntensity();
        void DecreaseLampIntensity();
        void SetLampIntensity(double lampIntensity);
        void EnableCamera();
        void DisableCamera();
        void SetAutoFocusEnabled();
        void SetManualFocusEnabled();
        void SetManualFocusPosition(double position);
        void SetLowQualityStreamProfile();
        void SetMediumQualityStreamProfile();
        void SetHighQualityStreamProfile();
        void SetCameraIntent(bool isEnabled, bool isAutoFocus, double focusPosition, int streamProfile);
        void SetGimbalPitchDegrees(double pitchDegrees);
        void SetManualMode();
        void SetHoldPositionMode();
        void SetModeIntent(bool isHoldPosition);
        void SetVerticalKeepCurrentMode();
        void SetVerticalBallastPositionMode();
        void SetVerticalDepthTargetMode();
        void SetVerticalIntent(int verticalMode, double ballastPosition, double depthTargetMeters);

    private:
        double m_LampIntensity = 0.0;
        bool m_CameraEnabled = true;
        bool m_AutoFocusEnabled = true;
        double m_ManualFocusPosition = 0.5;
        int m_StreamProfile = 1;
        double m_GimbalPitchDegrees = 0.0;
        bool m_HoldPositionMode = false;
        int m_VerticalMode = 1;
        double m_BallastPosition = 0.5;
        double m_DepthTargetMeters = 0.0;
    };
}
