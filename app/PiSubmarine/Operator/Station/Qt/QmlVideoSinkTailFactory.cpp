#include "PiSubmarine/Operator/Station/Qt/QmlVideoSinkTailFactory.h"

#include <gst/gst.h>
#include <spdlog/spdlog.h>

#include "PiSubmarine/Error/Api/MakeError.h"

namespace PiSubmarine::Operator::Station::Qt
{
    namespace
    {
        [[nodiscard]] Error::Api::Error MakeDeviceError()
        {
            return Error::Api::MakeError(Error::Api::ErrorCondition::DeviceError);
        }
    }

    QmlVideoSinkTailFactory::QmlVideoSinkTailFactory(
        QQuickItem& videoSurfaceItem,
        std::shared_ptr<spdlog::logger> logger)
        : m_VideoSurfaceItem(videoSurfaceItem)
        , m_Logger(std::move(logger))
    {
    }

    Error::Api::Result<GstElement*> QmlVideoSinkTailFactory::CreatePipelineTail()
    {
#if defined(_WIN32)
        return CreateWindowsTail();
#else
        return CreateLinuxTail();
#endif
    }

    Error::Api::Result<GstElement*> QmlVideoSinkTailFactory::CreateWindowsTail()
    {
        const auto sinkResult = MakeElement("qml6d3d11sink", "video_sink", m_Logger);
        if (!sinkResult.has_value())
        {
            return std::unexpected(sinkResult.error());
        }

        auto* sink = *sinkResult;
        g_object_set(
            G_OBJECT(sink),
            "widget",
            static_cast<gpointer>(&m_VideoSurfaceItem),
            "force-aspect-ratio",
            TRUE,
            nullptr);
        SPDLOG_LOGGER_INFO(m_Logger, "Using qml6d3d11sink video tail");
        return sink;
    }

    Error::Api::Result<GstElement*> QmlVideoSinkTailFactory::CreateLinuxTail()
    {
        auto* bin = gst_bin_new("video_tail");
        if (bin == nullptr)
        {
            SPDLOG_LOGGER_ERROR(m_Logger, "Failed to create Linux QML video sink bin");
            return std::unexpected(MakeDeviceError());
        }

        const auto uploadResult = MakeElement("glupload", "gl_upload", m_Logger);
        const auto convertResult = MakeElement("glcolorconvert", "gl_color_convert", m_Logger);
        const auto sinkResult = MakeElement("qml6glsink", "video_sink", m_Logger);
        if (!uploadResult.has_value() || !convertResult.has_value() || !sinkResult.has_value())
        {
            gst_object_unref(GST_OBJECT(bin));
            return std::unexpected(MakeDeviceError());
        }

        auto* upload = *uploadResult;
        auto* convert = *convertResult;
        auto* sink = *sinkResult;
        g_object_set(
            G_OBJECT(sink),
            "widget",
            static_cast<gpointer>(&m_VideoSurfaceItem),
            "force-aspect-ratio",
            TRUE,
            nullptr);

        gst_bin_add_many(GST_BIN(bin), upload, convert, sink, nullptr);
        if (!gst_element_link_many(upload, convert, sink, nullptr))
        {
            SPDLOG_LOGGER_ERROR(m_Logger, "Failed to link Linux QML video tail");
            gst_object_unref(GST_OBJECT(bin));
            return std::unexpected(MakeDeviceError());
        }

        auto* sinkPad = gst_element_get_static_pad(upload, "sink");
        if (sinkPad == nullptr)
        {
            SPDLOG_LOGGER_ERROR(m_Logger, "Failed to retrieve Linux QML video tail ghost pad");
            gst_object_unref(GST_OBJECT(bin));
            return std::unexpected(MakeDeviceError());
        }

        auto* ghostPad = gst_ghost_pad_new("sink", sinkPad);
        gst_object_unref(GST_OBJECT(sinkPad));
        if (ghostPad == nullptr)
        {
            SPDLOG_LOGGER_ERROR(m_Logger, "Failed to create Linux QML video tail ghost pad");
            gst_object_unref(GST_OBJECT(bin));
            return std::unexpected(MakeDeviceError());
        }

        if (!gst_element_add_pad(bin, ghostPad))
        {
            SPDLOG_LOGGER_ERROR(m_Logger, "Failed to add Linux QML video tail ghost pad");
            gst_object_unref(GST_OBJECT(ghostPad));
            gst_object_unref(GST_OBJECT(bin));
            return std::unexpected(MakeDeviceError());
        }

        SPDLOG_LOGGER_INFO(m_Logger, "Using qml6glsink video tail");
        return GST_ELEMENT(bin);
    }

    Error::Api::Result<GstElement*> QmlVideoSinkTailFactory::MakeElement(
        const char* factoryName,
        const char* elementName,
        const std::shared_ptr<spdlog::logger>& logger)
    {
        auto* element = gst_element_factory_make(factoryName, elementName);
        if (element == nullptr)
        {
            SPDLOG_LOGGER_ERROR(
                logger,
                "Failed to create GStreamer element '{}'. "
                "The current vcpkg GStreamer port disables Qt6 integration, so qml6 sinks may be unavailable.",
                factoryName);
            return std::unexpected(MakeDeviceError());
        }

        return element;
    }
}
