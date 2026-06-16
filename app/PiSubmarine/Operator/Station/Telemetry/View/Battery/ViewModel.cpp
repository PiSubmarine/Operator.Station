#include "PiSubmarine/Operator/Station/Telemetry/View/Battery/ViewModel.h"

namespace PiSubmarine::Operator::Station::Telemetry::View::Battery
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    double ViewModel::GetPackVoltage() const { return m_PackVoltage; }
    double ViewModel::GetPackCurrent() const { return m_PackCurrent; }
    double ViewModel::GetStateOfCharge() const { return m_StateOfCharge; }
    double ViewModel::GetPackTemperature() const { return m_PackTemperature; }

    void ViewModel::SetSnapshot(
        const double packVoltage,
        const double packCurrent,
        const double stateOfCharge,
        const double packTemperature)
    {
        if (m_PackVoltage == packVoltage &&
            m_PackCurrent == packCurrent &&
            m_StateOfCharge == stateOfCharge &&
            m_PackTemperature == packTemperature)
        {
            return;
        }

        m_PackVoltage = packVoltage;
        m_PackCurrent = packCurrent;
        m_StateOfCharge = stateOfCharge;
        m_PackTemperature = packTemperature;
        emit SnapshotChanged();
    }
}
