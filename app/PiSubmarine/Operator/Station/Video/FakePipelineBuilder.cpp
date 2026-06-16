#include "PiSubmarine/Operator/Station/Video/FakePipelineBuilder.h"

#include <memory>
#include <stdexcept>
#include <utility>

#include <gst/gst.h>
#include <spdlog/logger.h>

#include "PiSubmarine/Operator/Station/Video/IPipeline.h"

namespace PiSubmarine::Operator::Station::Video
{
    namespace
    {
        class FakePipeline final : public IPipeline
        {
        public:
            FakePipeline(
                IVideoPipelineTailFactory& tailFactory,
                std::shared_ptr<spdlog::logger> logger)
                : m_Logger(std::move(logger))
            {
                if (!m_Logger)
                {
                    throw std::invalid_argument("Operator.Station.Video fake pipeline requires a logger");
                }

                GError* error = nullptr;
                if (!gst_init_check(nullptr, nullptr, &error))
                {
                    if (error != nullptr)
                    {
                        g_error_free(error);
                    }

                    throw std::runtime_error("Failed to initialize GStreamer");
                }

                auto* pipeline = gst_pipeline_new("operator-station-fake-video");
                auto* source = gst_element_factory_make("videotestsrc", "source");
                auto* converter = gst_element_factory_make("videoconvert", "converter");
                auto* queue = gst_element_factory_make("queue", "tail_input");
                const auto tailResult = tailFactory.CreatePipelineTail();
                auto* tail = tailResult.has_value() ? *tailResult : nullptr;
                if (pipeline == nullptr || source == nullptr || converter == nullptr || queue == nullptr || tail == nullptr)
                {
                    if (pipeline != nullptr)
                    {
                        gst_object_unref(GST_OBJECT(pipeline));
                    }

                    throw std::runtime_error("Failed to create fake video pipeline elements");
                }

                g_object_set(source, "pattern", 18, nullptr);
                gst_bin_add_many(GST_BIN(pipeline), source, converter, queue, tail, nullptr);
                if (!gst_element_link_many(source, converter, queue, tail, nullptr))
                {
                    gst_object_unref(GST_OBJECT(pipeline));
                    throw std::runtime_error("Failed to link fake video pipeline");
                }

                if (gst_element_set_state(pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
                {
                    gst_object_unref(GST_OBJECT(pipeline));
                    throw std::runtime_error("Failed to start fake video pipeline");
                }

                m_Pipeline.reset(pipeline);
            }

            [[nodiscard]] Error::Api::Result<void> Stop() override
            {
                if (!m_Pipeline)
                {
                    return {};
                }

                gst_element_set_state(m_Pipeline.get(), GST_STATE_NULL);
                m_Pipeline.reset();
                return {};
            }

            void PollBus() override
            {
                if (!m_Pipeline)
                {
                    return;
                }

                auto* bus = gst_element_get_bus(m_Pipeline.get());
                if (bus == nullptr)
                {
                    return;
                }

                while (auto* message = gst_bus_pop(bus))
                {
                    if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR)
                    {
                        GError* error = nullptr;
                        gchar* debug = nullptr;
                        gst_message_parse_error(message, &error, &debug);
                        if (error != nullptr)
                        {
                            g_error_free(error);
                        }
                        if (debug != nullptr)
                        {
                            g_free(debug);
                        }
                    }

                    gst_message_unref(message);
                }

                gst_object_unref(bus);
            }

            [[nodiscard]] bool IsRunning() const noexcept override
            {
                return m_Pipeline != nullptr;
            }

        private:
            struct GstElementDeleter
            {
                void operator()(GstElement* element) const noexcept
                {
                    if (element != nullptr)
                    {
                        gst_object_unref(GST_OBJECT(element));
                    }
                }
            };

            std::shared_ptr<spdlog::logger> m_Logger;
            std::unique_ptr<GstElement, GstElementDeleter> m_Pipeline;
        };

        class FakePipelineBuilder final : public IPipelineBuilder
        {
        public:
            explicit FakePipelineBuilder(Logging::Api::IFactory& loggerFactory)
                : m_LoggerFactory(loggerFactory)
            {
            }

            [[nodiscard]] std::unique_ptr<IPipeline> Build(
                const ReceiveEndpoint& receiveEndpoint,
                IVideoPipelineTailFactory& tailFactory) override
            {
                static_cast<void>(receiveEndpoint);

                auto logger = m_LoggerFactory.CreateLogger("Operator.Station.Video.FakeGst");
                if (!logger)
                {
                    throw std::invalid_argument("Operator.Station.Video fake pipeline requires a logger");
                }

                return std::make_unique<FakePipeline>(tailFactory, std::move(logger));
            }

        private:
            Logging::Api::IFactory& m_LoggerFactory;
        };
    }

    std::shared_ptr<IPipelineBuilder> CreateFakePipelineBuilder(Logging::Api::IFactory& loggerFactory)
    {
        return std::make_shared<FakePipelineBuilder>(loggerFactory);
    }
}
