#pragma once

#include <QObject>

#include "PiSubmarine/Control/Api/Input/ISink.h"
#include "PiSubmarine/Logging/Api/IFactory.h"

namespace spdlog
{
    class logger;
}

namespace PiSubmarine::Operator::Station::Input
{
    class Controller final : public QObject
    {
        Q_OBJECT

    public:
        Controller(
            Control::Api::Input::ISink& sink,
            PiSubmarine::Logging::Api::IFactory& loggerFactory,
            QObject* parent = nullptr);

    public slots:
        void SubmitIntent(double surge, double yaw, double ballast, double lampIntensity, bool holdPosition);

    private:
        ::PiSubmarine::Control::Api::Input::ISink& m_Sink;
        std::shared_ptr<spdlog::logger> m_Logger;
    };
}
