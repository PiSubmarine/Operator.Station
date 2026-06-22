#include "PiSubmarine/Operator/Station/Control/View/ViewModel.h"

namespace PiSubmarine::Operator::Station::Control::View
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    double ViewModel::GetLampIntensity() const { return m_LampIntensity; }

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
}
