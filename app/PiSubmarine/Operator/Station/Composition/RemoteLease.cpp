#include "PiSubmarine/Operator/Station/Composition/RemoteLease.h"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

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

        [[nodiscard]] ::PiSubmarine::Grpc::Client::TlsConfig MakeTlsConfig(RemoteLeaseConfig config)
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

    RemoteLease::RemoteLease(PiSubmarine::Logging::Api::IFactory& loggerFactory, RemoteLeaseConfig config)
        : m_Channel(loggerFactory, MakeTlsConfig(std::move(config)))
        , m_BlockingIssuer(loggerFactory, m_Channel)
        , m_Worker(std::make_unique<::PiSubmarine::Operator::Station::Lease::ThreadWorker>(
            m_BlockingIssuer,
            loggerFactory))
        , m_Issuer(std::make_unique<::PiSubmarine::Operator::Station::Lease::SyncLeaseIssuerProxy>(*m_Worker))
    {
    }

    ::PiSubmarine::Lease::Api::ILeaseIssuer& RemoteLease::GetIssuer()
    {
        return *m_Issuer;
    }

    QObject& RemoteLease::GetWorkerObject()
    {
        return *m_Worker;
    }
}
