#pragma once

#include <chrono>
#include <memory>

#include <QObject>
#include <QTimer>

#include <spdlog/logger.h>

#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"
#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Operator/Station/Video/Config.h"
#include "PiSubmarine/Operator/Station/Video/IPipelineBuilder.h"
#include "PiSubmarine/Operator/Station/Video/VideoController.h"
#include "PiSubmarine/Video/Subscription/Api/IService.h"

namespace PiSubmarine::Operator::Station::Qt
{
    class VideoTailFactory;

    class VideoRuntimeWorker final : public QObject
    {
        Q_OBJECT

    public:
        VideoRuntimeWorker(
            Video::Config config,
            Logging::Api::IFactory& loggerFactory,
            Lease::Api::ILeaseIssuer& leaseIssuer,
            ::PiSubmarine::Video::Subscription::Api::IService& subscriptionService,
            std::shared_ptr<Video::IPipelineBuilder> pipelineBuilder,
            VideoTailFactory& tailFactory,
            QObject* parent = nullptr);

    public slots:
        void Start();
        void Stop();

    private slots:
        void Tick();

    private:
        Video::Config m_Config;
        Logging::Api::IFactory& m_LoggerFactory;
        Lease::Api::ILeaseIssuer& m_LeaseIssuer;
        ::PiSubmarine::Video::Subscription::Api::IService& m_SubscriptionService;
        std::shared_ptr<Video::IPipelineBuilder> m_PipelineBuilder;
        VideoTailFactory& m_TailFactory;
        std::shared_ptr<spdlog::logger> m_Logger;
        std::unique_ptr<Video::VideoController> m_Controller;
        QTimer* m_Timer = nullptr;
        std::chrono::steady_clock::time_point m_StartTime;
        std::chrono::steady_clock::time_point m_LastTickTime;
    };
}
