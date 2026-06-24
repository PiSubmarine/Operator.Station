#pragma once

#include <QObject>

namespace PiSubmarine::Operator::Station::Input::QtKeyboard
{
    class System;

    class EventFilter final : public QObject
    {
        Q_OBJECT

    public:
        explicit EventFilter(System& system, QObject* parent = nullptr);

    protected:
        bool eventFilter(QObject* watched, QEvent* event) override;

    private:
        System& m_System;
    };
}
