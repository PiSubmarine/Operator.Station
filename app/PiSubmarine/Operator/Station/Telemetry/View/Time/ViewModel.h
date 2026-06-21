#pragma once

#include <QObject>
#include <QString>

namespace PiSubmarine::Operator::Station::Telemetry::View::Time
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(bool hasLease READ GetHasLease NOTIFY SnapshotChanged)
        Q_PROPERTY(QString displayText READ GetDisplayText NOTIFY SnapshotChanged)
        Q_PROPERTY(QString backgroundColor READ GetBackgroundColor NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] bool GetHasLease() const;
        [[nodiscard]] QString GetDisplayText() const;
        [[nodiscard]] QString GetBackgroundColor() const;

    public slots:
        void SetSnapshot(bool hasLease, const QString& displayText, const QString& backgroundColor);

    signals:
        void SnapshotChanged();

    private:
        bool m_HasLease = false;
        QString m_DisplayText{"NO LEASE"};
        QString m_BackgroundColor{"#8f1d1d"};
    };
}
