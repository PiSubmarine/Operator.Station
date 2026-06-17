#pragma once

#include <memory>

#include <gst/gst.h>
#include <spdlog/logger.h>

#include "PiSubmarine/Operator/Station/Video/Config.h"
#include "PiSubmarine/Operator/Station/Video/GstreamerPipeline.h"

namespace PiSubmarine::Operator::Station::Video
{
    class IVideoPipelineTailFactory;

    class RtpPipeline final : public GstreamerPipeline
    {
    public:

        // TODO Inject Logger Factory instead of logger.
        RtpPipeline(
            const ReceiveEndpoint& receiveEndpoint,
            IVideoPipelineTailFactory& tailFactory,
            std::shared_ptr<spdlog::logger> logger);
        ~RtpPipeline() override;

        [[nodiscard]] Error::Api::Result<void> Stop() override;
        void PollBus() override;
        [[nodiscard]] bool IsRunning() const noexcept override;

    private:

        [[nodiscard]] static bool InitializeGstreamer(const std::shared_ptr<spdlog::logger>& logger);
        [[nodiscard]] static GstElementPtr CreateElement(const char* factoryName, const char* elementName);
        [[nodiscard]] Error::Api::Result<void> BuildPipeline(
            const ReceiveEndpoint& receiveEndpoint,
            IVideoPipelineTailFactory& tailFactory);
        void DrainBusMessages();

        std::shared_ptr<spdlog::logger> m_Logger;
        GstElementPtr m_Pipeline;
        GstElement* m_Tail = nullptr;
    };
}
