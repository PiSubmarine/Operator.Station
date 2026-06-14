#include "PiSubmarine/Operator/Station/Qt/VideoSurfaceItem.h"

namespace PiSubmarine::Operator::Station::Qt
{
    VideoSurfaceItem::VideoSurfaceItem(QQuickItem* parent)
        : QQuickItem(parent)
    {
        setFlag(QQuickItem::ItemHasContents, false);
        setClip(true);
    }
}
