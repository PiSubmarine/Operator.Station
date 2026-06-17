#include "PiSubmarine/Operator/Station/Video/RtpPipeline.h"

#include <mutex>
#include <stdexcept>

#include <spdlog/spdlog.h>

#include "PiSubmarine/Error/Api/MakeError.h"
#include "PiSubmarine/Operator/Station/Video/IVideoPipelineTailFactory.h"

namespace PiSubmarine::Operator::Station::Video
{
	namespace
	{
		std::once_flag GstreamerInitFlag;
		bool GstreamerInitialized = false;

		[[nodiscard]] Error::Api::Error MakePipelineError()
		{
			return Error::Api::MakeError(Error::Api::ErrorCondition::DeviceError);
		}

		[[nodiscard]] GstCaps* CreateRtpCaps()
		{
			return gst_caps_from_string("application/x-rtp,media=video,encoding-name=H264,payload=96");
		}
	}

	RtpPipeline::RtpPipeline(
		const ReceiveEndpoint& receiveEndpoint,
		IVideoPipelineTailFactory& tailFactory,
		std::shared_ptr<spdlog::logger> logger)
		: m_Logger(std::move(logger))
	{
		if (!InitializeGstreamer(m_Logger))
		{
			throw std::runtime_error("Failed to initialize GStreamer");
		}

		const auto buildResult = BuildPipeline(receiveEndpoint, tailFactory);
		if (!buildResult.has_value())
		{
			throw std::runtime_error("Failed to construct GStreamer video pipeline");
		}
	}

	RtpPipeline::~RtpPipeline()
	{
		static_cast<void>(Stop());
	}

	Error::Api::Result<void> RtpPipeline::Stop()
	{
		if (!m_Pipeline)
		{
			return {};
		}

		gst_element_set_state(m_Pipeline.get(), GST_STATE_NULL);
		m_Tail = nullptr;
		m_Pipeline.reset();
		return {};
	}

	void RtpPipeline::PollBus()
	{
		if (m_Pipeline)
		{
			DrainBusMessages();
		}
	}

	// FIXME do not rely on pointer. Check actual pipeline state.
	bool RtpPipeline::IsRunning() const noexcept
	{
		return m_Pipeline != nullptr;
	}

	bool RtpPipeline::InitializeGstreamer(const std::shared_ptr<spdlog::logger>& logger)
	{
		std::call_once(GstreamerInitFlag, [&logger]
		{
			GError* error = nullptr;
			GstreamerInitialized = gst_init_check(nullptr, nullptr, &error);
			if (!GstreamerInitialized && error != nullptr)
			{
				SPDLOG_LOGGER_ERROR(logger, "gst_init_check failed: {}", error->message);
				g_error_free(error);
			}
		});

		return GstreamerInitialized;
	}

	// TODO Move to GstreamerPipeline class
	RtpPipeline::GstElementPtr RtpPipeline::CreateElement(const char* factoryName, const char* elementName)
	{
		return GstElementPtr(gst_element_factory_make(factoryName, elementName));
	}

	Error::Api::Result<void> RtpPipeline::BuildPipeline(
		const ReceiveEndpoint& receiveEndpoint,
		IVideoPipelineTailFactory& tailFactory)
	{
		auto pipeline = GstElementPtr(gst_pipeline_new("operator-station-video"));
		auto source = CreateElement("udpsrc", "source");
		auto jitterBuffer = CreateElement("rtpjitterbuffer", "jitter_buffer");
		auto depayloader = CreateElement("rtph264depay", "depayloader");
		auto parser = CreateElement("h264parse", "parser");
		auto decoder = CreateElement("openh264dec", "decoder");
		auto converter = CreateElement("videoconvert", "converter");
		auto queue = CreateElement("queue", "tail_input");

		if (!pipeline || !source || !jitterBuffer || !depayloader || !parser || !decoder || !converter || !queue)
		{
			return std::unexpected(MakePipelineError());
		}

		// TODO make tail injection common through GstreamerPipeline class
		const auto tailResult = tailFactory.CreatePipelineTail();
		if (!tailResult.has_value() || *tailResult == nullptr)
		{
			return std::unexpected(tailResult.has_value() ? MakePipelineError() : tailResult.error());
		}

		GstElement* tail = *tailResult;
		GstCaps* caps = CreateRtpCaps();
		if (caps == nullptr)
		{
			gst_object_unref(tail);
			return std::unexpected(MakePipelineError());
		}

		g_object_set(
			source.get(),
			"address",
			receiveEndpoint.BindAddress.c_str(),
			"port",
			receiveEndpoint.Port,
			"caps",
			caps,
			nullptr);
		gst_caps_unref(caps);

		gst_bin_add_many(
			GST_BIN(pipeline.get()),
			source.get(),
			jitterBuffer.get(),
			depayloader.get(),
			parser.get(),
			decoder.get(),
			converter.get(),
			queue.get(),
			tail,
			nullptr);

		if (!gst_element_link_many(
				source.get(),
				jitterBuffer.get(),
				depayloader.get(),
				parser.get(),
				decoder.get(),
				converter.get(),
				queue.get(),
				nullptr) ||
			!gst_element_link(queue.get(), tail))
		{
			gst_object_unref(tail);
			return std::unexpected(MakePipelineError());
		}

		ReleaseOwnership(source);
		ReleaseOwnership(jitterBuffer);
		ReleaseOwnership(depayloader);
		ReleaseOwnership(parser);
		ReleaseOwnership(decoder);
		ReleaseOwnership(converter);
		ReleaseOwnership(queue);

		// TODO Since we already have Pipeline::Stop(), move this to Pipeline::Play(). Constructor should only construct, not change state.
		if (gst_element_set_state(pipeline.get(), GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
		{
			gst_object_unref(tail);
			// FIXME DrainBusMessages will not show any error because m_Pipeline = std::move(pipeline) is never reached
			return std::unexpected(MakePipelineError());
		}

		m_Tail = tail;
		m_Pipeline = std::move(pipeline);
		return {};
	}

	void RtpPipeline::DrainBusMessages()
	{
		GstObjectPtr bus(GST_OBJECT(gst_element_get_bus(m_Pipeline.get())));
		if (!bus)
		{
			return;
		}

		while (auto* message = gst_bus_pop(GST_BUS(bus.get())))
		{
			switch (GST_MESSAGE_TYPE(message))
			{
			case GST_MESSAGE_ERROR:
				{
					GError* error = nullptr;
					gchar* debug = nullptr;
					gst_message_parse_error(message, &error, &debug);
					SPDLOG_LOGGER_ERROR(
						m_Logger,
						"GStreamer pipeline error: {} ({})",
						error != nullptr ? error->message : "unknown",
						debug != nullptr ? debug : "no debug");
					if (error != nullptr)
					{
						g_error_free(error);
					}
					if (debug != nullptr)
					{
						g_free(debug);
					}
					break;
				}
			case GST_MESSAGE_WARNING:
				{
					GError* error = nullptr;
					gchar* debug = nullptr;
					gst_message_parse_warning(message, &error, &debug);
					SPDLOG_LOGGER_WARN(
						m_Logger,
						"GStreamer pipeline warning: {} ({})",
						error != nullptr ? error->message : "unknown",
						debug != nullptr ? debug : "no debug");
					if (error != nullptr)
					{
						g_error_free(error);
					}
					if (debug != nullptr)
					{
						g_free(debug);
					}
					break;
				}
			case GST_MESSAGE_INFO:
				{
					GError* error = nullptr;
					gchar* debug = nullptr;
					gst_message_parse_info(message, &error, &debug);
					SPDLOG_LOGGER_INFO(
						m_Logger,
						"GStreamer pipeline info: {} ({})",
						error != nullptr ? error->message : "unknown",
						debug != nullptr ? debug : "no debug info");
					if (error != nullptr)
					{
						g_error_free(error);
					}

					if (debug != nullptr)
					{
						g_free(debug);
					}

					break;
				}
			default:
				break;
			}

			gst_message_unref(message);
		}
	}
}
