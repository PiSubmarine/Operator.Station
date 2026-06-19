#include "PiSubmarine/Operator/Station/Video/FakePipelineBuilder.h"

#include <stdexcept>
#include <utility>

#include "PiSubmarine/Operator/Station/Video/FakePipeline.h"
#include "PiSubmarine/Operator/Station/Video/IVideoPipelineTailFactory.h"

namespace PiSubmarine::Operator::Station::Video
{
    namespace
    {
        class FakePipelineBuilder final : public IPipelineBuilder
        {
        public:
            FakePipelineBuilder(
                PiSubmarine::Logging::Api::IFactory& loggerFactory,
                IVideoPipelineTailFactory& tailFactory)
                : m_LoggerFactory(loggerFactory)
                , m_TailFactory(tailFactory)
            {
            }

            [[nodiscard]] std::unique_ptr<IPipeline> Build(const ReceiveEndpoint& receiveEndpoint) override
            {
                static_cast<void>(receiveEndpoint);

                auto logger = m_LoggerFactory.CreateLogger("Operator.Station.Video.FakeGst");
                if (!logger)
                {
                    throw std::invalid_argument("Operator.Station.Video fake pipeline requires a logger");
                }

                return std::make_unique<FakePipeline>(m_TailFactory, std::move(logger));
            }

        private:
            PiSubmarine::Logging::Api::IFactory& m_LoggerFactory;
            IVideoPipelineTailFactory& m_TailFactory;
        };
    }

    // TODO Method too simple to exist. Remove.
    std::shared_ptr<IPipelineBuilder> CreateFakePipelineBuilder(
        PiSubmarine::Logging::Api::IFactory& loggerFactory,
        IVideoPipelineTailFactory& tailFactory)
    {
        return std::make_shared<FakePipelineBuilder>(loggerFactory, tailFactory);
    }
}
