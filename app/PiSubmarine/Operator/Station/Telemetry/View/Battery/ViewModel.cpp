#include "PiSubmarine/Operator/Station/Telemetry/View/Battery/ViewModel.h"

#include <algorithm>

namespace PiSubmarine::Operator::Station::Telemetry::View::Battery
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    double ViewModel::GetPackVoltage() const { return m_PackVoltage; }
    double ViewModel::GetPackCurrent() const { return m_PackCurrent; }
    double ViewModel::GetPackTemperature() const { return m_PackTemperature; }
    double ViewModel::GetChargerVoltage() const { return m_ChargerVoltage; }
    double ViewModel::GetChargerCurrent() const { return m_ChargerCurrent; }
    double ViewModel::GetChargerTemperature() const { return m_ChargerTemperature; }
    double ViewModel::GetStateOfCharge() const { return m_StateOfCharge; }
    QString ViewModel::GetTimeToFullText() const { return m_TimeToFullText; }
    QString ViewModel::GetTimeToEmptyText() const { return m_TimeToEmptyText; }

    void ViewModel::SetSnapshot(
        const double packVoltage,
        const double packCurrent,
        const double packTemperature,
        const double chargerVoltage,
        const double chargerCurrent,
        const double chargerTemperature,
        const double stateOfCharge,
        const bool hasTimeToFull,
        const qint64 timeToFullMilliseconds,
        const bool hasTimeToEmpty,
        const qint64 timeToEmptyMilliseconds)
    {
        const auto timeToFullText = FormatDuration(hasTimeToFull, timeToFullMilliseconds);
        const auto timeToEmptyText = FormatDuration(hasTimeToEmpty, timeToEmptyMilliseconds);

        if (m_PackVoltage == packVoltage &&
            m_PackCurrent == packCurrent &&
            m_PackTemperature == packTemperature &&
            m_ChargerVoltage == chargerVoltage &&
            m_ChargerCurrent == chargerCurrent &&
            m_ChargerTemperature == chargerTemperature &&
            m_StateOfCharge == stateOfCharge &&
            m_TimeToFullText == timeToFullText &&
            m_TimeToEmptyText == timeToEmptyText)
        {
            return;
        }

        m_PackVoltage = packVoltage;
        m_PackCurrent = packCurrent;
        m_PackTemperature = packTemperature;
        m_ChargerVoltage = chargerVoltage;
        m_ChargerCurrent = chargerCurrent;
        m_ChargerTemperature = chargerTemperature;
        m_StateOfCharge = stateOfCharge;
        m_TimeToFullText = timeToFullText;
        m_TimeToEmptyText = timeToEmptyText;
        emit SnapshotChanged();
    }

    QString ViewModel::FormatDuration(const bool hasValue, const qint64 milliseconds)
    {
        if (!hasValue)
        {
            return "--:--";
        }

        const auto totalMinutes = std::max<qint64>(0, milliseconds / 60000);
        const auto hours = totalMinutes / 60;
        const auto minutes = totalMinutes % 60;
        return QString("%1:%2")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'));
    }
}
