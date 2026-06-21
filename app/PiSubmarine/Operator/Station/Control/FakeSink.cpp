#include "PiSubmarine/Operator/Station/Control/FakeSink.h"

namespace PiSubmarine::Operator::Station::Control
{
    Error::Api::Result<void> FakeSink::Submit(const ::PiSubmarine::Control::Api::Input::OperatorCommand& command)
    {
        std::lock_guard lock(m_Mutex);
        m_LastCommand = command;
        return {};
    }

    std::optional<::PiSubmarine::Control::Api::Input::OperatorCommand> FakeSink::GetLastCommand() const
    {
        std::lock_guard lock(m_Mutex);
        return m_LastCommand;
    }
}
