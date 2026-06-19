#include "PiSubmarine/Operator/Station/Video/RtpPipelineBuilder.h"

#include "PiSubmarine/Operator/Station/Video/RtpPipeline.h"

namespace PiSubmarine::Operator::Station::Video
{
    namespace
    {
        class RtpPipelineBuilder final : public IPipelineBuilder
        {
        public:
            RtpPipelineBuilder(
                PiSubmarine::Logging::Api::IFactory& loggerFactory,
                IVideoPipelineTailFactory& tailFactory)
                : m_LoggerFactory(loggerFactory)
                , m_TailFactory(tailFactory)
            {
            }

            [[nodiscard]] std::unique_ptr<IPipeline> Build(const ReceiveEndpoint& receiveEndpoint) override
            {
                return std::make_unique<RtpPipeline>(receiveEndpoint, m_TailFactory, m_LoggerFactory);
            }

        private:
            PiSubmarine::Logging::Api::IFactory& m_LoggerFactory;
            IVideoPipelineTailFactory& m_TailFactory;
        };
    }

    std::shared_ptr<IPipelineBuilder> CreateRtpPipelineBuilder(
        PiSubmarine::Logging::Api::IFactory& loggerFactory,
        IVideoPipelineTailFactory& tailFactory)
    {
        return std::make_shared<RtpPipelineBuilder>(loggerFactory, tailFactory);
    }
}
