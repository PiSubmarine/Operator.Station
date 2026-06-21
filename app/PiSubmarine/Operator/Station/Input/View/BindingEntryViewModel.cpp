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

    QString BindingEntryViewModel::GetHint() const
    {
        return m_Hint;
    }

    bool BindingEntryViewModel::GetCapturing() const
    {
        return m_Capturing;
    }

    void BindingEntryViewModel::SetHint(const QString& hint)
    {
        if (m_Hint == hint)
        {
            return;
        }

        m_Hint = hint;
        emit HintChanged();
    }

    void BindingEntryViewModel::SetCapturing(const bool capturing)
    {
        if (m_Capturing == capturing)
        {
            return;
        }

        m_Capturing = capturing;
        emit CapturingChanged();
    }
}
