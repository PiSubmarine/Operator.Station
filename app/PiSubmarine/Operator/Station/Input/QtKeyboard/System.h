#pragma once

#include <cstddef>
#include <chrono>
#include <memory>
#include <vector>

#include "PiSubmarine/Input/Api/IBinder.h"
#include "PiSubmarine/Input/Api/IManager.h"
#include "PiSubmarine/Input/Api/ISerializer.h"
#include "PiSubmarine/Time/ITickable.h"

namespace PiSubmarine::Operator::Station::Input::QtKeyboard
{
    class System final
        : public ::PiSubmarine::Input::Api::IManager
        , public ::PiSubmarine::Input::Api::IBinder
        , public ::PiSubmarine::Input::Api::ISerializer
        , public ::PiSubmarine::Time::ITickable
    {
    public:
        System();
        ~System() override;

        [[nodiscard]] std::unique_ptr<::PiSubmarine::Input::Api::IAxis> CreateAxis(
            ::PiSubmarine::Input::Api::IAxisBinding& binding) override;
        [[nodiscard]] std::unique_ptr<::PiSubmarine::Input::Api::IKey> CreateKey(
            ::PiSubmarine::Input::Api::IKeyBinding& binding) override;

        void StartCapture(KeyCallback callback) override;
        void StartCapture(AxisCallback callback) override;
        void StopCapture() override;

        [[nodiscard]] std::vector<std::byte> Serialize(::PiSubmarine::Input::Api::IAxisBinding& binding) const override;
        [[nodiscard]] std::unique_ptr<::PiSubmarine::Input::Api::IAxisBinding> DeserializeAxis(
            const std::vector<std::byte>& data) const override;
        [[nodiscard]] std::vector<std::byte> Serialize(::PiSubmarine::Input::Api::IKeyBinding& binding) const override;
        [[nodiscard]] std::unique_ptr<::PiSubmarine::Input::Api::IKeyBinding> DeserializeKey(
            const std::vector<std::byte>& data) const override;

        void Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime) override;

        void PushKeyEvent(int key, bool pressed);
        void ReleaseAllKeys();

    private:
        class Impl;

        std::shared_ptr<Impl> m_Impl;
    };
}
