#include "PiSubmarine/Operator/Station/Video/RtpPipeline.h"

#include <stdexcept>
#include <utility>

#include "PiSubmarine/Operator/Station/Video/IVideoPipelineTailFactory.h"

namespace PiSubmarine::Operator::Station::Video
{
	namespace
	{
		[[nodiscard]] GstCaps* CreateRtpCaps()
		{
			return gst_caps_from_string("application/x-rtp,media=video,encoding-name=H264,payload=96");
		}
	}

	RtpPipeline::RtpPipeline(
		const ReceiveEndpoint& receiveEndpoint,
		IVideoPipelineTailFactory& tailFactory,
		PiSubmarine::Logging::Api::IFactory& loggerFactory)
		: GstreamerPipeline(loggerFactory.CreateLogger("Operator.Station.Video.Gst"))
	{
		if (!EnsureGstreamerInitialized(GetLogger()))
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
        auto tailResult = CreateTail(tailFactory);

		if (!pipeline || !source || !jitterBuffer || !depayloader || !parser || !decoder || !converter || !queue || !tailResult.has_value())
		{
			return std::unexpected(tailResult.has_value() ? MakeDeviceError() : tailResult.error());
		}

		auto tail = std::move(*tailResult);
        auto* sourceElement = source.get();
        auto* jitterBufferElement = jitterBuffer.get();
        auto* depayloaderElement = depayloader.get();
        auto* parserElement = parser.get();
        auto* decoderElement = decoder.get();
        auto* converterElement = converter.get();
        auto* queueElement = queue.get();
        auto* tailElement = tail.get();
		GstCaps* caps = CreateRtpCaps();
		if (caps == nullptr)
		{
			return std::unexpected(MakeDeviceError());
		}

		g_object_set(
			sourceElement,
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
			sourceElement,
			jitterBufferElement,
			depayloaderElement,
			parserElement,
			decoderElement,
			converterElement,
			queueElement,
			tailElement,
			nullptr);

		if (!gst_element_link_many(
				sourceElement,
				jitterBufferElement,
				depayloaderElement,
				parserElement,
				decoderElement,
				converterElement,
				queueElement,
				nullptr) ||
			!gst_element_link(queueElement, tailElement))
		{
			return std::unexpected(MakeDeviceError());
		}

		ReleaseOwnership(source);
		ReleaseOwnership(jitterBuffer);
		ReleaseOwnership(depayloader);
		ReleaseOwnership(parser);
		ReleaseOwnership(decoder);
		ReleaseOwnership(converter);
		ReleaseOwnership(queue);
        ReleaseOwnership(tail);

        SetPipeline(std::move(pipeline));
        return {};
	}
}
