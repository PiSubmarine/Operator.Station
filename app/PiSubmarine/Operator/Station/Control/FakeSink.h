#pragma once

#include <mutex>
#include <optional>

#include "PiSubmarine/Control/Api/Input/ISink.h"

namespace PiSubmarine::Operator::Station::Control
{
    class FakeSink final : public ::PiSubmarine::Control::Api::Input::ISink
    {
    public:
        [[nodiscard]] Error::Api::Result<void> Submit(
            const ::PiSubmarine::Control::Api::Input::OperatorCommand& command) override;

        [[nodiscard]] std::optional<::PiSubmarine::Control::Api::Input::OperatorCommand> GetLastCommand() const;

    private:
        mutable std::mutex m_Mutex;
        std::optional<::PiSubmarine::Control::Api::Input::OperatorCommand> m_LastCommand;
    };
}
