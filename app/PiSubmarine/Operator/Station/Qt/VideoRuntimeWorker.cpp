#include "PiSubmarine/Operator/Station/Qt/VideoRuntimeWorker.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

#include "PiSubmarine/Operator/Station/Qt/VideoTailFactory.h"

namespace PiSubmarine::Operator::Station::Qt
{
    VideoRuntimeWorker::VideoRuntimeWorker(
        Video::Config config,
        Logging::Api::IFactory& loggerFactory,
        Lease::Api::ILeaseIssuer& leaseIssuer,
        ::PiSubmarine::Video::Subscription::Api::IService& subscriptionService,
        std::shared_ptr<Video::IPipelineBuilder> pipelineBuilder,
        VideoTailFactory& tailFactory,
        QObject* parent)
        : QObject(parent)
        , m_Config(std::move(config))
        , m_LoggerFactory(loggerFactory)
        , m_LeaseIssuer(leaseIssuer)
        , m_SubscriptionService(subscriptionService)
        , m_PipelineBuilder(std::move(pipelineBuilder))
        , m_TailFactory(tailFactory)
        , m_Logger(loggerFactory.CreateLogger("Operator.Station.VideoRuntimeWorker"))
    {
        if (!m_Logger)
        {
            throw std::invalid_argument("Operator.Station.VideoRuntimeWorker requires a logger");
        }
    }

    void VideoRuntimeWorker::Start()
    {
        try
        {
            if (m_Timer == nullptr)
            {
                m_Timer = new QTimer(this);
                m_Timer->setInterval(16);
                connect(m_Timer, &QTimer::timeout, this, &VideoRuntimeWorker::Tick);
            }

            m_Controller = std::make_unique<Video::VideoController>(
                m_Config,
                m_LoggerFactory,
                m_LeaseIssuer,
                m_SubscriptionService,
                m_PipelineBuilder);

            const auto tailFactoryResult = m_Controller->SetPipelineTailFactory(m_TailFactory);
            if (!tailFactoryResult.has_value())
            {
                SPDLOG_LOGGER_ERROR(m_Logger, "Failed to set video tail factory");
                return;
            }

            const auto startResult = m_Controller->Start();
            if (!startResult.has_value())
            {
                SPDLOG_LOGGER_ERROR(m_Logger, "Failed to start video controller");
                return;
            }

            m_StartTime = std::chrono::steady_clock::now();
            m_LastTickTime = m_StartTime;
            m_Timer->start();
            SPDLOG_LOGGER_INFO(m_Logger, "Video runtime worker started");
        }
        catch (const std::exception& exception)
        {
            SPDLOG_LOGGER_ERROR(m_Logger, "Video runtime worker start failed: {}", exception.what());
        }
    }

    void VideoRuntimeWorker::Stop()
    {
        if (m_Timer != nullptr)
        {
            m_Timer->stop();
        }

        if (m_Controller)
        {
            const auto stopResult = m_Controller->Stop();
            if (!stopResult.has_value())
            {
                SPDLOG_LOGGER_WARN(m_Logger, "Video controller stop reported an error");
            }

            m_Controller.reset();
        }

        SPDLOG_LOGGER_INFO(m_Logger, "Video runtime worker stopped");
    }

    void VideoRuntimeWorker::Tick()
    {
        if (!m_Controller)
        {
            return;
        }

        const auto now = std::chrono::steady_clock::now();
        const auto uptime = std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_StartTime);
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_LastTickTime);
        m_LastTickTime = now;
        m_Controller->Tick(uptime, delta);
    }
}
