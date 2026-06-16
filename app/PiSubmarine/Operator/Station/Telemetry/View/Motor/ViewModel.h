#pragma once

#include <QObject>
#include <QString>

namespace PiSubmarine::Operator::Station::Telemetry::View::Motor
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString operationalState READ GetOperationalState NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasFault READ HasFault NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasWarning READ HasWarning NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] QString GetOperationalState() const;
        [[nodiscard]] bool HasFault() const;
        [[nodiscard]] bool HasWarning() const;

    public slots:
        void SetSnapshot(const QString& operationalState, bool hasFault, bool hasWarning);

    signals:
        void SnapshotChanged();

    private:
        QString m_OperationalState{"Operational"};
        bool m_HasFault = false;
        bool m_HasWarning = false;
    };
}
