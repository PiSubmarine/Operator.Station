#pragma once

#include <QObject>

namespace PiSubmarine::Operator::Station::Control::View
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(double lampIntensity READ GetLampIntensity NOTIFY LampIntensityChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] double GetLampIntensity() const;

    signals:
        void LampIntensityChanged();
        void LampChangeRequested(double step);

    public slots:
        void IncreaseLampIntensity();
        void DecreaseLampIntensity();
        void SetLampIntensity(double lampIntensity);

    private:
        double m_LampIntensity = 0.0;
    };
}
