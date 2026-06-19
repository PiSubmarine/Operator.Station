#pragma once

#include <memory>

#include <gst/gstelement.h>
#include <spdlog/logger.h>

#include "PiSubmarine/Error/Api/Error.h"
#include "PiSubmarine/Operator/Station/Video/IPipeline.h"

namespace PiSubmarine::Operator::Station::Video
{
    class IVideoPipelineTailFactory;

	class GstreamerPipeline : public IPipeline
	{
    public:
        explicit GstreamerPipeline(std::shared_ptr<spdlog::logger> logger);

        [[nodiscard]] Error::Api::Result<void> Play() override;
        [[nodiscard]] Error::Api::Result<void> Stop() override;
        void PollBus() override;
        [[nodiscard]] bool IsRunning() const noexcept override;

        [[nodiscard]] static bool EnsureGstreamerInitialized(const std::shared_ptr<spdlog::logger>& logger);

	protected:
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

        [[nodiscard]] Error::Api::Result<GstElementPtr> CreateTail(IVideoPipelineTailFactory& tailFactory) const;
        [[nodiscard]] GstElementPtr CreateElement(const char* factoryName, const char* elementName) const;
        [[nodiscard]] static Error::Api::Error MakeDeviceError();
        [[nodiscard]] static GstElementPtr MakePipeline(const char* name);
        [[nodiscard]] const std::shared_ptr<spdlog::logger>& GetLogger() const noexcept;
        [[nodiscard]] GstElement* GetPipeline() const noexcept;
        void SetPipeline(GstElementPtr pipeline);

		static void ReleaseOwnership(GstElementPtr& element)
		{
			const auto raw = element.release();
			(void)raw;
		}

    private:
        void DrainBusMessages(GstElement& pipeline);

        std::shared_ptr<spdlog::logger> m_Logger;
        GstElementPtr m_Pipeline;
	};
}
