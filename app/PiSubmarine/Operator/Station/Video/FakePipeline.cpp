#include "PiSubmarine/Operator/Station/Video/FakePipeline.h"

#include <stdexcept>
#include <utility>

#include "PiSubmarine/Operator/Station/Video/IVideoPipelineTailFactory.h"

namespace PiSubmarine::Operator::Station::Video
{
    FakePipeline::FakePipeline(
        IVideoPipelineTailFactory& tailFactory,
        std::shared_ptr<spdlog::logger> logger)
        : GstreamerPipeline(std::move(logger))
    {
        if (!EnsureGstreamerInitialized(GetLogger()))
        {
            throw std::runtime_error("Failed to initialize GStreamer");
        }

        const auto buildResult = BuildPipeline(tailFactory);
        if (!buildResult.has_value())
        {
            throw std::runtime_error("Failed to construct fake video pipeline");
        }

        const auto playResult = Play();
        if (!playResult.has_value())
        {
            throw std::runtime_error("Failed to start fake video pipeline");
        }
    }

    FakePipeline::~FakePipeline()
    {
        static_cast<void>(Stop());
    }

    Error::Api::Result<void> FakePipeline::BuildPipeline(IVideoPipelineTailFactory& tailFactory)
    {
        auto pipeline = MakePipeline("operator-station-fake-video");
        auto source = CreateElement("videotestsrc", "source");
        auto converter = CreateElement("videoconvert", "converter");
        auto queue = CreateElement("queue", "tail_input");
        auto tailResult = CreateTail(tailFactory);

        if (!pipeline || !source || !converter || !queue || !tailResult.has_value())
        {
            return std::unexpected(tailResult.has_value() ? MakeDeviceError() : tailResult.error());
        }

        auto tail = std::move(*tailResult);
        auto* sourceElement = source.get();
        auto* converterElement = converter.get();
        auto* queueElement = queue.get();
        auto* tailElement = tail.get();

        g_object_set(sourceElement, "pattern", 18, nullptr);
        gst_bin_add_many(GST_BIN(pipeline.get()), sourceElement, converterElement, queueElement, tailElement, nullptr);
        if (!gst_element_link_many(sourceElement, converterElement, queueElement, tailElement, nullptr))
        {
            return std::unexpected(MakeDeviceError());
        }

        ReleaseOwnership(source);
        ReleaseOwnership(converter);
        ReleaseOwnership(queue);
        ReleaseOwnership(tail);

        SetPipeline(std::move(pipeline));
        return {};
    }
}
