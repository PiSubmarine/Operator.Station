#include "PiSubmarine/Operator/Station/Video/View/QmlVideoSinkTailFactory.h"

#include <QMetaObject>
#include <QQuickWindow>
#include <QThread>

#include <gst/gst.h>
#include <spdlog/spdlog.h>

#include "PiSubmarine/Error/Api/MakeError.h"

namespace PiSubmarine::Operator::Station::Video::View
{
    namespace
    {
        [[nodiscard]] Error::Api::Error MakeDeviceError()
        {
            return Error::Api::MakeError(Error::Api::ErrorCondition::DeviceError);
        }

        [[nodiscard]] Error::Api::Error MakeNotReadyError()
        {
            return Error::Api::MakeError(Error::Api::ErrorCondition::NotReady);
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
        auto* itemThread = m_VideoSurfaceItem.thread();
        if (itemThread == nullptr)
        {
            SPDLOG_LOGGER_ERROR(m_Logger, "Video surface item is not attached to a Qt thread");
            return std::unexpected(MakeDeviceError());
        }

        if (QThread::currentThread() == itemThread)
        {
            return CreatePipelineTailOnItemThread();
        }

        Error::Api::Result<GstElement*> result = std::unexpected(MakeDeviceError());
        const auto invoked = QMetaObject::invokeMethod(
            &m_VideoSurfaceItem,
            [this, &result]()
            {
                result = CreatePipelineTailOnItemThread();
            },
            Qt::BlockingQueuedConnection);

        if (!invoked)
        {
            SPDLOG_LOGGER_ERROR(m_Logger, "Failed to invoke video tail creation on the QML item thread");
            return std::unexpected(MakeDeviceError());
        }

        return result;
    }

    Error::Api::Result<GstElement*> QmlVideoSinkTailFactory::CreatePipelineTailOnItemThread()
    {
        auto* window = m_VideoSurfaceItem.window();
        if (window == nullptr)
        {
            SPDLOG_LOGGER_DEBUG(m_Logger, "Video surface item is not attached to a QQuickWindow yet");
            return std::unexpected(MakeNotReadyError());
        }

        if (!window->isExposed())
        {
            SPDLOG_LOGGER_DEBUG(m_Logger, "Video surface window is not exposed yet");
            return std::unexpected(MakeNotReadyError());
        }

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
        return sink;
    }

    Error::Api::Result<GstElement*> QmlVideoSinkTailFactory::CreateLinuxTail()
    {
        auto* bin = gst_bin_new("video_tail");
        if (bin == nullptr)
        {
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
            gst_object_unref(GST_OBJECT(bin));
            return std::unexpected(MakeDeviceError());
        }

        auto* sinkPad = gst_element_get_static_pad(upload, "sink");
        if (sinkPad == nullptr)
        {
            gst_object_unref(GST_OBJECT(bin));
            return std::unexpected(MakeDeviceError());
        }

        auto* ghostPad = gst_ghost_pad_new("sink", sinkPad);
        gst_object_unref(GST_OBJECT(sinkPad));
        if (ghostPad == nullptr)
        {
            gst_object_unref(GST_OBJECT(bin));
            return std::unexpected(MakeDeviceError());
        }

        if (!gst_element_add_pad(bin, ghostPad))
        {
            gst_object_unref(GST_OBJECT(ghostPad));
            gst_object_unref(GST_OBJECT(bin));
            return std::unexpected(MakeDeviceError());
        }

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
            SPDLOG_LOGGER_ERROR(logger, "Failed to create GStreamer element '{}'", factoryName);
            return std::unexpected(MakeDeviceError());
        }

        return element;
    }
}
