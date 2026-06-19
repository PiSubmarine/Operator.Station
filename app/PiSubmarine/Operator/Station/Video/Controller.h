#pragma once

#include <chrono>
#include <memory>
#include <optional>

#include <QObject>
#include <QString>
#include <QTimer>

#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"
#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Operator/Station/Video/Config.h"
#include "PiSubmarine/Operator/Station/Video/IPipelineBuilder.h"
#include "PiSubmarine/Operator/Station/Video/Status.h"
#include "PiSubmarine/Time/ITickable.h"
#include "PiSubmarine/Video/Subscription/Api/IService.h"

namespace spdlog
{
	class logger;
}

class QQuickItem;

namespace PiSubmarine::Operator::Station::Video
{
	class IPipeline;
	class IVideoPipelineTailFactory;

	class Controller final : public QObject, public Time::ITickable
	{
		Q_OBJECT

	public:
		Controller(
			Config config,
			PiSubmarine::Logging::Api::IFactory& loggerFactory,
			::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
			::PiSubmarine::Video::Subscription::Api::IService& subscriptionService,
			std::shared_ptr<IPipelineBuilder> pipelineBuilder,
			// TODO tail factory is only used to feed IPipelineBuilder::Build. Consider moving IVideoPipelineTailFactory dependency from Controller to the pipeline builders. Controller should not care about the pipeline structure.
			IVideoPipelineTailFactory& tailFactory,
			QObject* parent = nullptr);
		~Controller() override;

		// TODO Error::Api::Result is intended for cross-domain communication. No need to use it in internal methods, especially for methods that cannot fail.
		[[nodiscard]] Error::Api::Result<Status> GetStatus() const;
		void Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime) override;

	public slots:
		void Start();
		void Stop();
		void SetReceiveEndpoint(const QString& bindAddress, quint16 port);
		void SetSubscriptionEndpoint(const QString& host, quint16 port);

	private slots:
		void TickNow();

	private:
		[[nodiscard]] static Error::Api::ErrorCondition GetNotReadyCondition();
		[[nodiscard]] static bool IsNotReadyError(const Error::Api::Error& error);
		[[nodiscard]] Error::Api::Result<void> AcquireLease(const std::chrono::nanoseconds& uptime);
		[[nodiscard]] Error::Api::Result<void> RenewLease(const std::chrono::nanoseconds& uptime);
		[[nodiscard]] Error::Api::Result<void> EnsureSubscribed();
		[[nodiscard]] Error::Api::Result<void> RebuildPipeline();
		void ResetLeaseState() noexcept;
		void ScheduleRetry(const std::chrono::nanoseconds& uptime) noexcept;

		Config m_Config;
		std::shared_ptr<spdlog::logger> m_Logger;
		::PiSubmarine::Lease::Api::ILeaseIssuer& m_LeaseIssuer;
		::PiSubmarine::Video::Subscription::Api::IService& m_SubscriptionService;
		std::shared_ptr<IPipelineBuilder> m_PipelineBuilder;
		IVideoPipelineTailFactory& m_TailFactory;
		std::unique_ptr<IPipeline> m_Pipeline;
		QTimer m_Timer;
		std::chrono::steady_clock::time_point m_StartTime{};
		std::chrono::steady_clock::time_point m_LastTickTime{};
		std::optional<::PiSubmarine::Lease::Api::LeaseGrant> m_LeaseGrant;
		std::chrono::nanoseconds m_NextLeaseRenewal{0};
		std::chrono::nanoseconds m_NextRetryTime{0};
		bool m_IsStarted = false;
		bool m_IsSubscribed = false;
		bool m_IsDirty = true;
	};
}
