#pragma once

#include <QObject>

#include "PiSubmarine/Operator/Station/Video/IVideoPipelineTailFactory.h"

typedef struct _GstAppSink GstAppSink;

namespace PiSubmarine::Operator::Station::Qt
{
    class VideoTailFactory final
        : public QObject
        , public Video::IVideoPipelineTailFactory
    {
        Q_OBJECT

    public:
        explicit VideoTailFactory(QObject* parent = nullptr);

        [[nodiscard]] Error::Api::Result<GstElement*> CreatePipelineTail() override;

    signals:
        void FrameReady(const QImage& frame);

    private:
        static GstFlowReturn OnNewSample(GstAppSink* sink, gpointer userData);
        GstFlowReturn HandleNewSample(GstAppSink* sink);
    };
}
