#include "PiSubmarine/Operator/Station/Video/View/VideoSurfaceItem.h"

namespace PiSubmarine::Operator::Station::Video::View
{
    VideoSurfaceItem::VideoSurfaceItem(QQuickItem* parent)
        : QQuickItem(parent)
    {
        setFlag(QQuickItem::ItemHasContents, false);
        setClip(true);
    }
}
