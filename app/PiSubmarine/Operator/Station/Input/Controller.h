#pragma once

#include <QObject>

#include "PiSubmarine/Control/Api/Input/ISink.h"
#include "PiSubmarine/Logging/Api/IFactory.h"

namespace spdlog
{
    class logger;
}

namespace PiSubmarine::Operator::Station::Input::View
{
    class ViewModel;
}

namespace PiSubmarine::Operator::Station::Input
{
    class Controller final : public QObject
    {
        Q_OBJECT

    public:
        Controller(
            Control::Api::Input::ISink& sink,
            View::ViewModel& viewModel,
            PiSubmarine::Logging::Api::IFactory& loggerFactory,
            QObject* parent = nullptr);

    public slots:
        void SubmitCurrentIntent();

    private:
        ::PiSubmarine::Control::Api::Input::ISink& m_Sink;
        View::ViewModel& m_ViewModel;
        std::shared_ptr<spdlog::logger> m_Logger;
    };
}
