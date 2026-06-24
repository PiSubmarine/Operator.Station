#include "PiSubmarine/Operator/Station/Input/QtKeyboard/EventFilter.h"

#include <QEvent>
#include <QKeyEvent>

#include "PiSubmarine/Operator/Station/Input/QtKeyboard/System.h"

namespace PiSubmarine::Operator::Station::Input::QtKeyboard
{
    EventFilter::EventFilter(System& system, QObject* parent)
        : QObject(parent)
        , m_System(system)
    {
    }

    bool EventFilter::eventFilter(QObject* watched, QEvent* event)
    {
        Q_UNUSED(watched);

        if (event == nullptr)
        {
            return false;
        }

        if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
        {
            const auto* keyEvent = dynamic_cast<QKeyEvent*>(event);
            if (keyEvent != nullptr && !keyEvent->isAutoRepeat())
            {
                m_System.PushKeyEvent(keyEvent->key(), event->type() == QEvent::KeyPress);
            }

            return false;
        }

        if (event->type() == QEvent::ApplicationDeactivate || event->type() == QEvent::WindowDeactivate)
        {
            m_System.ReleaseAllKeys();
        }

        return false;
    }
}
