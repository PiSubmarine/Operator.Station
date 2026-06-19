#include "PiSubmarine/Operator/Station/Video/RtpPipelineBuilder.h"

#include <stdexcept>
#include <utility>

#include "PiSubmarine/Operator/Station/Video/RtpPipeline.h"

namespace PiSubmarine::Operator::Station::Video
{
    namespace
    {
        class RtpPipelineBuilder final : public IPipelineBuilder
        {
        public:
            explicit RtpPipelineBuilder(PiSubmarine::Logging::Api::IFactory& loggerFactory)
                : m_LoggerFactory(loggerFactory)
            {
            }

            [[nodiscard]] std::unique_ptr<IPipeline> Build(
                const ReceiveEndpoint& receiveEndpoint,
                IVideoPipelineTailFactory& tailFactory) override
            {
                auto logger = m_LoggerFactory.CreateLogger("Operator.Station.Video.Gst");
                if (!logger)
                {
                    throw std::invalid_argument("Operator.Station.Video requires a GStreamer logger");
                }

                return std::make_unique<RtpPipeline>(receiveEndpoint, tailFactory, std::move(logger));
            }

        private:
            PiSubmarine::Logging::Api::IFactory& m_LoggerFactory;
        };
    }

    std::shared_ptr<IPipelineBuilder> CreateRtpPipelineBuilder(PiSubmarine::Logging::Api::IFactory& loggerFactory)
    {
        return std::make_shared<RtpPipelineBuilder>(loggerFactory);
    }
}
