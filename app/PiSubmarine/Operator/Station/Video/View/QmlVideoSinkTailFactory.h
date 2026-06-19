#pragma once

#include <memory>

#include <QQuickItem>

#include <spdlog/logger.h>

#include "PiSubmarine/Operator/Station/Video/IVideoPipelineTailFactory.h"

namespace PiSubmarine::Operator::Station::Video::View
{
    class QmlVideoSinkTailFactory final : public IVideoPipelineTailFactory
    {
    public:
        QmlVideoSinkTailFactory(
            QQuickItem& videoSurfaceItem,
            std::shared_ptr<spdlog::logger> logger);

        [[nodiscard]] Error::Api::Result<GstElement*> CreatePipelineTail() override;

    private:
        [[nodiscard]] Error::Api::Result<GstElement*> CreatePipelineTailOnItemThread();
        [[nodiscard]] Error::Api::Result<GstElement*> CreateWindowsTail();
        [[nodiscard]] Error::Api::Result<GstElement*> CreateLinuxTail();
        [[nodiscard]] static Error::Api::Result<GstElement*> MakeElement(
            const char* factoryName,
            const char* elementName,
            const std::shared_ptr<spdlog::logger>& logger);

        QQuickItem& m_VideoSurfaceItem;
        std::shared_ptr<spdlog::logger> m_Logger;
    };
}
