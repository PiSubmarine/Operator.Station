#pragma once

#include <QObject>

namespace PiSubmarine::Operator::Station::Telemetry::View::Lamp
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(bool isActive READ IsActive NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasFault READ HasFault NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasWarning READ HasWarning NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] bool IsActive() const;
        [[nodiscard]] bool HasFault() const;
        [[nodiscard]] bool HasWarning() const;

    public slots:
        void SetSnapshot(bool isActive, bool hasFault, bool hasWarning);

    signals:
        void SnapshotChanged();

    private:
        bool m_IsActive = false;
        bool m_HasFault = false;
        bool m_HasWarning = false;
    };
}
