#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>

#include "PiSubmarine/Grpc/Client/Channel.h"
#include "PiSubmarine/Lease/Client/Grpc/Client.h"
#include "PiSubmarine/Operator/Station/Composition/ILease.h"
#include "PiSubmarine/Operator/Station/Lease/SyncLeaseIssuerProxy.h"
#include "PiSubmarine/Operator/Station/Lease/ThreadWorker.h"

namespace PiSubmarine::Operator::Station::Composition
{
    struct RemoteLeaseConfig
    {
        std::string GrpcTarget;
        std::filesystem::path CertificateAuthorityPath;
        std::filesystem::path ClientCertificateChainPath;
        std::filesystem::path ClientPrivateKeyPath;
        std::string ServerAuthorityOverride;
        std::chrono::milliseconds RpcTimeout{5000};
    };

    class RemoteLease final : public ILease
    {
    public:
        RemoteLease(PiSubmarine::Logging::Api::IFactory& loggerFactory, RemoteLeaseConfig config);

        [[nodiscard]] ::PiSubmarine::Lease::Api::ILeaseIssuer& GetIssuer() override;
        [[nodiscard]] QObject& GetWorkerObject() override;

    private:
        ::PiSubmarine::Grpc::Client::Channel m_Channel;
        ::PiSubmarine::Lease::Client::Grpc::Client m_BlockingIssuer;
        std::unique_ptr<::PiSubmarine::Operator::Station::Lease::ThreadWorker> m_Worker;
        std::unique_ptr<::PiSubmarine::Operator::Station::Lease::SyncLeaseIssuerProxy> m_Issuer;
    };
}
