#pragma once

#include <QImage>
#include <QQuickPaintedItem>

class QPainter;

namespace PiSubmarine::Operator::Station::Qt
{
    class VideoItem : public QQuickPaintedItem
    {
        Q_OBJECT

    public:
        explicit VideoItem(QQuickItem* parent = nullptr);

        void paint(QPainter* painter) override;

    public slots:
        void PresentFrame(const QImage& frame);

    private:
        QImage m_Frame;
    };
}
