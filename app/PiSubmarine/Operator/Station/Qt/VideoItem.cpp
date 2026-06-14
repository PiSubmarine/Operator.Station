#include "PiSubmarine/Operator/Station/Qt/VideoItem.h"

#include <QPainter>

namespace PiSubmarine::Operator::Station::Qt
{
    VideoItem::VideoItem(QQuickItem* parent)
        : QQuickPaintedItem(parent)
    {
        setFillColor(::Qt::black);
        setAntialiasing(false);
    }

    void VideoItem::paint(QPainter* painter)
    {
        painter->fillRect(boundingRect(), ::Qt::black);
        if (!m_Frame.isNull())
        {
            painter->drawImage(boundingRect(), m_Frame);
        }
    }

    void VideoItem::PresentFrame(const QImage& frame)
    {
        m_Frame = frame;
        update();
    }
}
