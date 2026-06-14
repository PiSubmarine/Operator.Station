#include "PiSubmarine/Operator/Station/Qt/VideoTailFactory.h"

#include <QImage>

#include <gst/app/gstappsink.h>

#include "PiSubmarine/Error/Api/MakeError.h"

namespace PiSubmarine::Operator::Station::Qt
{
    VideoTailFactory::VideoTailFactory(QObject* parent)
        : QObject(parent)
    {
    }

    Error::Api::Result<GstElement*> VideoTailFactory::CreatePipelineTail()
    {
        auto* sink = gst_element_factory_make("appsink", nullptr);
        if (sink == nullptr)
        {
            return std::unexpected(Error::Api::MakeError(Error::Api::ErrorCondition::DeviceError));
        }

        auto* appSink = GST_APP_SINK(sink);
        auto* caps = gst_caps_from_string("video/x-raw,format=BGRA");
        gst_app_sink_set_caps(appSink, caps);
        gst_caps_unref(caps);
        gst_app_sink_set_emit_signals(appSink, false);
        gst_app_sink_set_drop(appSink, true);
        gst_app_sink_set_max_buffers(appSink, 2);

        GstAppSinkCallbacks callbacks{};
        callbacks.new_sample = &VideoTailFactory::OnNewSample;
        gst_app_sink_set_callbacks(appSink, &callbacks, this, nullptr);
        g_object_set(sink, "sync", FALSE, nullptr);
        return sink;
    }

    GstFlowReturn VideoTailFactory::OnNewSample(GstAppSink* sink, gpointer userData)
    {
        return static_cast<VideoTailFactory*>(userData)->HandleNewSample(sink);
    }

    GstFlowReturn VideoTailFactory::HandleNewSample(GstAppSink* sink)
    {
        GstSample* sample = gst_app_sink_pull_sample(sink);
        if (sample == nullptr)
        {
            return GST_FLOW_ERROR;
        }

        GstCaps* caps = gst_sample_get_caps(sample);
        GstBuffer* buffer = gst_sample_get_buffer(sample);
        if (caps == nullptr || buffer == nullptr)
        {
            gst_sample_unref(sample);
            return GST_FLOW_ERROR;
        }

        GstStructure* structure = gst_caps_get_structure(caps, 0);
        int width = 0;
        int height = 0;
        if (!gst_structure_get_int(structure, "width", &width) ||
            !gst_structure_get_int(structure, "height", &height))
        {
            gst_sample_unref(sample);
            return GST_FLOW_ERROR;
        }

        GstMapInfo mapInfo{};
        if (!gst_buffer_map(buffer, &mapInfo, GST_MAP_READ))
        {
            gst_sample_unref(sample);
            return GST_FLOW_ERROR;
        }

        const QImage frame(
            reinterpret_cast<const uchar*>(mapInfo.data),
            width,
            height,
            QImage::Format_ARGB32);
        emit FrameReady(frame.copy());

        gst_buffer_unmap(buffer, &mapInfo);
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }
}
