#include "PiSubmarine/Operator/Station/Composition/RemoteVideo.h"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "PiSubmarine/Operator/Station/Video/View/QmlVideoSinkTailFactory.h"

namespace PiSubmarine::Operator::Station::Composition
{
    namespace
    {
        [[nodiscard]] std::string ReadTextFile(const std::filesystem::path& path)
        {
            std::ifstream stream(path, std::ios::binary);
            if (!stream.is_open())
            {
                throw std::runtime_error("Failed to open file: " + path.string());
            }

            return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
        }

        [[nodiscard]] ::PiSubmarine::Grpc::Client::TlsConfig MakeTlsConfig(RemoteVideoConfig config)
        {
            return ::PiSubmarine::Grpc::Client::TlsConfig{
                .Target = std::move(config.GrpcTarget),
                .CertificateAuthority = ReadTextFile(config.CertificateAuthorityPath),
                .ClientCertificateChain = ReadTextFile(config.ClientCertificateChainPath),
                .ClientPrivateKey = ReadTextFile(config.ClientPrivateKeyPath),
                .ServerAuthorityOverride = std::move(config.ServerAuthorityOverride),
                .RpcTimeout = config.RpcTimeout};
        }
    }

    RemoteVideo::RemoteVideo(
        ::PiSubmarine::Logging::Api::IFactory& loggerFactory,
        ::PiSubmarine::Operator::Station::Video::View::QmlVideoSinkTailFactory& tailFactory,
        RemoteVideoConfig config)
        : m_Channel(loggerFactory, MakeTlsConfig(std::move(config)))
        , m_SubscriptionService(loggerFactory, m_Channel)
        , m_PipelineBuilder(::PiSubmarine::Operator::Station::Video::CreateRtpPipelineBuilder(loggerFactory, tailFactory))
    {
    }

    std::shared_ptr<::PiSubmarine::Operator::Station::Video::IPipelineBuilder> RemoteVideo::GetPipelineBuilder()
    {
        return m_PipelineBuilder;
    }

    ::PiSubmarine::Video::Subscription::Api::IService& RemoteVideo::GetSubscriptionService()
    {
        return m_SubscriptionService;
    }
}
