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

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] double GetLampIntensity() const;
        [[nodiscard]] bool GetCameraEnabled() const;
        [[nodiscard]] bool GetAutoFocusEnabled() const;
        [[nodiscard]] double GetManualFocusPosition() const;
        [[nodiscard]] int GetStreamProfile() const;

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

    private:
        double m_LampIntensity = 0.0;
        bool m_CameraEnabled = true;
        bool m_AutoFocusEnabled = true;
        double m_ManualFocusPosition = 0.5;
        int m_StreamProfile = 1;
    };
}
