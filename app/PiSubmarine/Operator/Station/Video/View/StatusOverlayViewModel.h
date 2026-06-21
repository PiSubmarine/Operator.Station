#pragma once

#include <QObject>
#include <QString>

#include "PiSubmarine/Operator/Station/Video/Status.h"

namespace PiSubmarine::Operator::Station::Video::View
{
    class StatusOverlayViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(bool isOverlayVisible READ GetIsOverlayVisible NOTIFY SnapshotChanged)
        Q_PROPERTY(QString overlayMessage READ GetOverlayMessage NOTIFY SnapshotChanged)
        Q_PROPERTY(QString overlayBackgroundColor READ GetOverlayBackgroundColor NOTIFY SnapshotChanged)

    public:
        explicit StatusOverlayViewModel(QObject* parent = nullptr);

        [[nodiscard]] bool GetIsOverlayVisible() const;
        [[nodiscard]] QString GetOverlayMessage() const;
        [[nodiscard]] QString GetOverlayBackgroundColor() const;

    public slots:
        void SetStatus(const PiSubmarine::Operator::Station::Video::Status& status);

    signals:
        void SnapshotChanged();

    private:
        Status m_Status{};
    };
}
