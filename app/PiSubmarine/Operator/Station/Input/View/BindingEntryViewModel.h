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

    public:
        explicit BindingEntryViewModel(QString name, QObject* parent = nullptr);

        [[nodiscard]] QString GetName() const;
        [[nodiscard]] QString GetHint() const;

    public slots:
        void SetHint(const QString& hint);

    signals:
        void HintChanged();

    private:
        QString m_Name;
        QString m_Hint = "Unbound";
    };
}
