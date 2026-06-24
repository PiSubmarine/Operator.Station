#include "PiSubmarine/Operator/Station/Input/View/BindingEntryViewModel.h"

namespace PiSubmarine::Operator::Station::Input::View
{
    BindingEntryViewModel::BindingEntryViewModel(QString name, QObject* parent)
        : QObject(parent)
        , m_Name(std::move(name))
    {
    }

    QString BindingEntryViewModel::GetName() const
    {
        return m_Name;
    }

    QString BindingEntryViewModel::GetKeyboardHint() const
    {
        return m_KeyboardHint;
    }

    QString BindingEntryViewModel::GetGamepadHint() const
    {
        return m_GamepadHint;
    }

    bool BindingEntryViewModel::GetKeyboardCapturing() const
    {
        return m_KeyboardCapturing;
    }

    bool BindingEntryViewModel::GetGamepadCapturing() const
    {
        return m_GamepadCapturing;
    }

    void BindingEntryViewModel::SetKeyboardHint(const QString& hint)
    {
        if (m_KeyboardHint == hint)
        {
            return;
        }

        m_KeyboardHint = hint;
        emit KeyboardHintChanged();
    }

    void BindingEntryViewModel::SetGamepadHint(const QString& hint)
    {
        if (m_GamepadHint == hint)
        {
            return;
        }

        m_GamepadHint = hint;
        emit GamepadHintChanged();
    }

    void BindingEntryViewModel::SetKeyboardCapturing(const bool capturing)
    {
        if (m_KeyboardCapturing == capturing)
        {
            return;
        }

        m_KeyboardCapturing = capturing;
        emit KeyboardCapturingChanged();
    }

    void BindingEntryViewModel::SetGamepadCapturing(const bool capturing)
    {
        if (m_GamepadCapturing == capturing)
        {
            return;
        }

        m_GamepadCapturing = capturing;
        emit GamepadCapturingChanged();
    }
}
