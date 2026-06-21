#pragma once

#include <QObject>
#include <QString>

namespace PiSubmarine::Operator::Station::Input::View
{
    class BindingEntryViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString name READ GetName CONSTANT)
        Q_PROPERTY(QString hint READ GetHint NOTIFY HintChanged)
        Q_PROPERTY(bool capturing READ GetCapturing NOTIFY CapturingChanged)

    public:
        explicit BindingEntryViewModel(QString name, QObject* parent = nullptr);

        [[nodiscard]] QString GetName() const;
        [[nodiscard]] QString GetHint() const;
        [[nodiscard]] bool GetCapturing() const;

    public slots:
        void SetHint(const QString& hint);
        void SetCapturing(bool capturing);

    signals:
        void HintChanged();
        void CapturingChanged();

    private:
        QString m_Name;
        QString m_Hint = "Unbound";
        bool m_Capturing = false;
    };
}
