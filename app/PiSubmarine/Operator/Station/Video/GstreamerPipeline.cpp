#include "PiSubmarine/Operator/Station/Video/GstreamerPipeline.h"

#include <mutex>

#include <gst/gstmessage.h>
#include <spdlog/spdlog.h>

#include "PiSubmarine/Error/Api/MakeError.h"
#include "PiSubmarine/Gstreamer/Build/Plugins.h"
#include "PiSubmarine/Operator/Station/Video/IVideoPipelineTailFactory.h"

namespace PiSubmarine::Operator::Station::Video
{
    namespace
    {
        std::once_flag GstreamerInitFlag;
        bool GstreamerInitialized = false;
    }

    GstreamerPipeline::GstreamerPipeline(std::shared_ptr<spdlog::logger> logger)
        : m_Logger(std::move(logger))
    {
    }

    Error::Api::Result<void> GstreamerPipeline::Play()
    {
        if (!m_Pipeline)
        {
            return std::unexpected(MakeDeviceError());
        }

        if (gst_element_set_state(m_Pipeline.get(), GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
        {
            DrainBusMessages(*m_Pipeline);
            return std::unexpected(MakeDeviceError());
        }

        return {};
    }

    Error::Api::Result<void> GstreamerPipeline::Stop()
    {
        if (!m_Pipeline)
        {
            return {};
        }

        gst_element_set_state(m_Pipeline.get(), GST_STATE_NULL);
        m_Pipeline.reset();
        return {};
    }

    void GstreamerPipeline::PollBus()
    {
        if (!m_Pipeline)
        {
            return;
        }

        DrainBusMessages(*m_Pipeline);
    }

    bool GstreamerPipeline::IsRunning() const noexcept
    {
        if (!m_Pipeline)
        {
            return false;
        }

        GstState state = GST_STATE_NULL;
        GstState pending = GST_STATE_VOID_PENDING;
        gst_element_get_state(m_Pipeline.get(), &state, &pending, 0);
        return state == GST_STATE_PLAYING || pending == GST_STATE_PLAYING;
    }

    bool GstreamerPipeline::EnsureGstreamerInitialized(const std::shared_ptr<spdlog::logger>& logger)
    {
        std::call_once(GstreamerInitFlag, [&logger]()
        {
            GError* error = nullptr;
            GstreamerInitialized = gst_init_check(nullptr, nullptr, &error);
            if (!GstreamerInitialized)
            {
                if (logger && error != nullptr)
                {
                    SPDLOG_LOGGER_ERROR(logger, "gst_init_check failed: {}", error->message);
                }

                if (error != nullptr)
                {
                    g_error_free(error);
                }
                return;
            }

            ::PiSubmarine::Gstreamer::Build::Plugins::RegisterStatic(logger);
        });

        return GstreamerInitialized;
    }

    Error::Api::Result<GstreamerPipeline::GstElementPtr> GstreamerPipeline::CreateTail(
        IVideoPipelineTailFactory& tailFactory) const
    {
        const auto tailResult = tailFactory.CreatePipelineTail();
        if (!tailResult.has_value())
        {
            return std::unexpected(tailResult.error());
        }

        if (*tailResult == nullptr)
        {
            return std::unexpected(MakeDeviceError());
        }

        return GstElementPtr(*tailResult);
    }

    GstreamerPipeline::GstElementPtr GstreamerPipeline::CreateElement(
        const char* factoryName,
        const char* elementName) const
    {
        auto element = GstElementPtr(gst_element_factory_make(factoryName, elementName));
        if (!element && m_Logger)
        {
            SPDLOG_LOGGER_ERROR(m_Logger, "Failed to create GStreamer element '{}'", factoryName);
        }
        return element;
    }

    Error::Api::Error GstreamerPipeline::MakeDeviceError()
    {
        return Error::Api::MakeError(Error::Api::ErrorCondition::DeviceError);
    }

    GstreamerPipeline::GstElementPtr GstreamerPipeline::MakePipeline(const char* name)
    {
        return GstElementPtr(gst_pipeline_new(name));
    }

    const std::shared_ptr<spdlog::logger>& GstreamerPipeline::GetLogger() const noexcept
    {
        return m_Logger;
    }

    GstElement* GstreamerPipeline::GetPipeline() const noexcept
    {
        return m_Pipeline.get();
    }

    void GstreamerPipeline::SetPipeline(GstElementPtr pipeline)
    {
        m_Pipeline = std::move(pipeline);
    }

    void GstreamerPipeline::DrainBusMessages(GstElement& pipeline)
    {
        GstObjectPtr bus(GST_OBJECT(gst_element_get_bus(&pipeline)));
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
                    if (m_Logger)
                    {
                        SPDLOG_LOGGER_ERROR(
                            m_Logger,
                            "{} ({})",
                            error != nullptr ? error->message : "unknown",
                            debug != nullptr ? debug : "no debug");
                    }
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
                    if (m_Logger)
                    {
                        SPDLOG_LOGGER_WARN(
                            m_Logger,
                            "{} ({})",
                            error != nullptr ? error->message : "unknown",
                            debug != nullptr ? debug : "no debug");
                    }
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
                    if (m_Logger)
                    {
                        SPDLOG_LOGGER_INFO(
                            m_Logger,
                            "{} ({})",
                            error != nullptr ? error->message : "unknown",
                            debug != nullptr ? debug : "no debug info");
                    }
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

	void GstreamerPipeline::GstObjectDeleter::operator()(GstObject* object) const noexcept
	{
		if (object != nullptr)
		{
			gst_object_unref(object);
		}
	}

	void GstreamerPipeline::GstElementDeleter::operator()(GstElement* element) const noexcept
	{
		if (element != nullptr)
		{
			gst_object_unref(GST_OBJECT(element));
		}
	}
}
