#pragma once

#include <QObject>

namespace PiSubmarine::Operator::Station::Input::View
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(double surge READ GetSurge WRITE SetSurge NOTIFY IntentChanged)
        Q_PROPERTY(double yaw READ GetYaw WRITE SetYaw NOTIFY IntentChanged)
        Q_PROPERTY(double ballast READ GetBallast WRITE SetBallast NOTIFY IntentChanged)
        Q_PROPERTY(double lampIntensity READ GetLampIntensity WRITE SetLampIntensity NOTIFY IntentChanged)
        Q_PROPERTY(bool holdPosition READ GetHoldPosition WRITE SetHoldPosition NOTIFY IntentChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] double GetSurge() const;
        [[nodiscard]] double GetYaw() const;
        [[nodiscard]] double GetBallast() const;
        [[nodiscard]] double GetLampIntensity() const;
        [[nodiscard]] bool GetHoldPosition() const;

        void SetSurge(double surge);
        void SetYaw(double yaw);
        void SetBallast(double ballast);
        void SetLampIntensity(double lampIntensity);
        void SetHoldPosition(bool holdPosition);

    signals:
        void IntentChanged();

    private:
        double m_Surge = 0.0;
        double m_Yaw = 0.0;
        double m_Ballast = 0.5;
        double m_LampIntensity = 0.0;
        bool m_HoldPosition = false;
    };
}
