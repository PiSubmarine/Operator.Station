#pragma once

#include <memory>

#include <gst/gst.h>
#include <spdlog/logger.h>

#include "PiSubmarine/Operator/Station/Video/Config.h"
#include "PiSubmarine/Operator/Station/Video/IPipeline.h"

namespace PiSubmarine::Operator::Station::Video
{
    class IVideoPipelineTailFactory;

    class GstreamerPipeline final : public IPipeline
    {
    public:
        GstreamerPipeline(
            const ReceiveEndpoint& receiveEndpoint,
            IVideoPipelineTailFactory& tailFactory,
            std::shared_ptr<spdlog::logger> logger);
        ~GstreamerPipeline() override;

        [[nodiscard]] Error::Api::Result<void> Stop() override;
        void PollBus() override;
        [[nodiscard]] bool IsRunning() const noexcept override;

    private:
        struct GstObjectDeleter
        {
            void operator()(GstObject* object) const noexcept;
        };

        struct GstElementDeleter
        {
            void operator()(GstElement* element) const noexcept;
        };

        using GstObjectPtr = std::unique_ptr<GstObject, GstObjectDeleter>;
        using GstElementPtr = std::unique_ptr<GstElement, GstElementDeleter>;

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
