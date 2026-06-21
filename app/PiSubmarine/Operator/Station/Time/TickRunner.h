#pragma once

#include <atomic>
#include <chrono>
#include <memory>

#include <QObject>

#include "PiSubmarine/Error/Api/Result.h"
#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Time/ITickable.h"
#include "PiSubmarine/Time/Manager.h"

namespace spdlog
{
    class logger;
}

namespace PiSubmarine::Operator::Station::Time
{
    class TickRunner final : public QObject
    {
        Q_OBJECT

    public:
        TickRunner(
            std::chrono::nanoseconds tickPeriod,
            PiSubmarine::Logging::Api::IFactory& loggerFactory,
            QObject* parent = nullptr);
        ~TickRunner() override;

        [[nodiscard]] Error::Api::Result<void> AddTickable(::PiSubmarine::Time::ITickable& tickable);

    public slots:
        void Start();
        void Stop();

    private:
        void SleepUntil(const ::PiSubmarine::Time::Manager::TimePoint& deadline);

        ::PiSubmarine::Time::Manager m_Manager;
        std::shared_ptr<spdlog::logger> m_Logger;
        std::atomic_bool m_StopRequested = false;
    };
}
