#pragma once

#include <QQuickItem>

namespace PiSubmarine::Operator::Station::Video::View
{
    class VideoSurfaceItem : public QQuickItem
    {
        Q_OBJECT

    public:
        explicit VideoSurfaceItem(QQuickItem* parent = nullptr);
    };
}
