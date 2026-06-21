#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>

#include "PiSubmarine/Grpc/Client/Channel.h"
#include "PiSubmarine/Operator/Station/Composition/IVideo.h"
#include "PiSubmarine/Operator/Station/Video/RtpPipelineBuilder.h"
#include "PiSubmarine/Video/Subscription/Grpc/Client/Client.h"

namespace PiSubmarine::Operator::Station::Video::View
{
    class QmlVideoSinkTailFactory;
}

namespace PiSubmarine::Operator::Station::Composition
{
    struct RemoteVideoConfig
    {
        std::string GrpcTarget;
        std::filesystem::path CertificateAuthorityPath;
        std::filesystem::path ClientCertificateChainPath;
        std::filesystem::path ClientPrivateKeyPath;
        std::string ServerAuthorityOverride;
        std::chrono::milliseconds RpcTimeout{5000};
    };

    class RemoteVideo final : public IVideo
    {
    public:
        RemoteVideo(
            ::PiSubmarine::Logging::Api::IFactory& loggerFactory,
            ::PiSubmarine::Operator::Station::Video::View::QmlVideoSinkTailFactory& tailFactory,
            RemoteVideoConfig config);

        [[nodiscard]] std::shared_ptr<::PiSubmarine::Operator::Station::Video::IPipelineBuilder> GetPipelineBuilder() override;
        [[nodiscard]] ::PiSubmarine::Video::Subscription::Api::IService& GetSubscriptionService() override;

    private:
        ::PiSubmarine::Grpc::Client::Channel m_Channel;
        ::PiSubmarine::Video::Subscription::Grpc::Client::Client m_SubscriptionService;
        std::shared_ptr<::PiSubmarine::Operator::Station::Video::IPipelineBuilder> m_PipelineBuilder;
    };
}
