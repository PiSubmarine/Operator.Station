#pragma once

#include <QQuickItem>

namespace PiSubmarine::Operator::Station::Qt
{
    class VideoSurfaceItem : public QQuickItem
    {
        Q_OBJECT

    public:
        explicit VideoSurfaceItem(QQuickItem* parent = nullptr);
    };
}
