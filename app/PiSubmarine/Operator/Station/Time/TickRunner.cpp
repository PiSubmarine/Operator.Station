#include "PiSubmarine/Operator/Station/Time/TickRunner.h"

#include <thread>

#include <QCoreApplication>
#include <QEventLoop>
#include <QMetaObject>
#include <QThread>

#include <spdlog/spdlog.h>

namespace PiSubmarine::Operator::Station::Time
{
    TickRunner::TickRunner(
        const std::chrono::nanoseconds tickPeriod,
        PiSubmarine::Logging::Api::IFactory& loggerFactory,
        QObject* parent)
        : QObject(parent)
        , m_Manager(
            tickPeriod,
            [] { return std::chrono::steady_clock::now(); },
            [this](const ::PiSubmarine::Time::Manager::TimePoint& deadline)
            {
                SleepUntil(deadline);
            })
        , m_Logger(loggerFactory.CreateLogger("Operator.Station.Time.TickRunner"))
    {
    }

    TickRunner::~TickRunner()
    {
        Stop();
    }

    Error::Api::Result<void> TickRunner::AddTickable(::PiSubmarine::Time::ITickable& tickable)
    {
        return m_Manager.AddTickable(tickable);
    }

    void TickRunner::Start()
    {
        if (thread() != nullptr && QThread::currentThread() != thread())
        {
            QMetaObject::invokeMethod(this, &TickRunner::Start, Qt::QueuedConnection);
            return;
        }

        m_StopRequested = false;
        const auto runResult = m_Manager.Run();
        if (!runResult.has_value())
        {
            SPDLOG_LOGGER_ERROR(m_Logger, "Controllers tick loop stopped with an error");
        }
    }

    void TickRunner::Stop()
    {
        m_StopRequested = true;
        m_Manager.Stop();
    }

    void TickRunner::SleepUntil(const ::PiSubmarine::Time::Manager::TimePoint& deadline)
    {
        while (!m_StopRequested.load(std::memory_order_relaxed))
        {
            const auto now = std::chrono::steady_clock::now();
            if (now >= deadline)
            {
                return;
            }

            QCoreApplication::processEvents(QEventLoop::AllEvents, 1);

            const auto afterEvents = std::chrono::steady_clock::now();
            if (afterEvents >= deadline)
            {
                return;
            }

            const auto remaining = deadline - afterEvents;
            if (remaining > std::chrono::milliseconds(1))
            {
                QThread::msleep(1);
                continue;
            }

            if (remaining > std::chrono::microseconds(100))
            {
                QThread::usleep(
                    static_cast<unsigned long>(
                        std::chrono::duration_cast<std::chrono::microseconds>(remaining).count()));
                continue;
            }

            std::this_thread::yield();
        }
    }
}
