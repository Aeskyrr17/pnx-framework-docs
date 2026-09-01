#include "remoter_example.hpp"

#include "config.hpp"

namespace demo::remoter
{
namespace
{

::remoter::state latest_state{};

void on_update(const ::remoter::state& data) noexcept
{
    latest_state = data;
}

} // namespace

types::status init() noexcept
{
    ::remoter::config config{};
    config.dr16.thread_priority = params::remoter::thread_priority;
    config.dr16.rx_timeout_ticks = params::remoter::rx_timeout_ticks;
    config.vt03.thread_priority = params::remoter::thread_priority;
    config.ps2.thread_priority = params::remoter::thread_priority;
    config.ps2.receiver_offline_timeout_ticks = params::remoter::ps2_offline_timeout_ticks;
    config.ps2.frame_timeout_ticks = params::remoter::ps2_frame_timeout_ticks;
    config.ps2.deadzone = params::remoter::ps2_deadzone;
    config.thread_priority = params::remoter::thread_priority + 1U;
    config.on_update_callback = ::remoter::update_callback::bind<&on_update>();
    return ::remoter::service::instance().init(config);
}

const ::remoter::state& latest() noexcept
{
    return latest_state;
}

} // namespace demo::remoter

