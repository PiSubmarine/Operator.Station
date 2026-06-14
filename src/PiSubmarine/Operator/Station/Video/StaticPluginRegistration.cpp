#include "PiSubmarine/Operator/Station/Video/StaticPluginRegistration.h"

#include <mutex>

#include <gst/gst.h>
#include <spdlog/spdlog.h>

#if PISUBMARINE_GSTREAMER_HAS_STATIC_PLUGINS
extern "C"
{
GST_PLUGIN_STATIC_DECLARE(app);
GST_PLUGIN_STATIC_DECLARE(coreelements);
GST_PLUGIN_STATIC_DECLARE(autodetect);
GST_PLUGIN_STATIC_DECLARE(videotestsrc);
GST_PLUGIN_STATIC_DECLARE(videoconvertscale);
GST_PLUGIN_STATIC_DECLARE(videoparsersbad);
GST_PLUGIN_STATIC_DECLARE(openh264);
GST_PLUGIN_STATIC_DECLARE(rtp);
GST_PLUGIN_STATIC_DECLARE(udp);
#if defined(_WIN32)
GST_PLUGIN_STATIC_DECLARE(mediafoundation);
#else
GST_PLUGIN_STATIC_DECLARE(video4linux2);
#endif
}
#endif

namespace PiSubmarine::Operator::Station::Video
{
    namespace
    {
        std::once_flag StaticPluginRegistrationFlag;
    }

    void RegisterStaticPlugins(const std::shared_ptr<spdlog::logger>& logger)
    {
        std::call_once(StaticPluginRegistrationFlag, [&logger]
        {
#if PISUBMARINE_GSTREAMER_HAS_STATIC_PLUGINS
            SPDLOG_LOGGER_INFO(logger, "Registering static GStreamer plugins for operator station");
            GST_PLUGIN_STATIC_REGISTER(app);
            GST_PLUGIN_STATIC_REGISTER(coreelements);
            GST_PLUGIN_STATIC_REGISTER(autodetect);
            GST_PLUGIN_STATIC_REGISTER(videotestsrc);
            GST_PLUGIN_STATIC_REGISTER(videoconvertscale);
            GST_PLUGIN_STATIC_REGISTER(videoparsersbad);
            GST_PLUGIN_STATIC_REGISTER(openh264);
            GST_PLUGIN_STATIC_REGISTER(rtp);
            GST_PLUGIN_STATIC_REGISTER(udp);
#if defined(_WIN32)
            GST_PLUGIN_STATIC_REGISTER(mediafoundation);
#else
            GST_PLUGIN_STATIC_REGISTER(video4linux2);
#endif
#else
            SPDLOG_LOGGER_INFO(logger, "Using dynamic GStreamer plugin discovery");
#endif
        });
    }
}
