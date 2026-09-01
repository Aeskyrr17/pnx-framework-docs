#include "referee_example.hpp"

#include "config.hpp"

namespace demo::referee
{
namespace
{

state latest_state{};

void on_update(const ::referee::packet_store& packets,
               const ::referee::update_info& update) noexcept
{
    if (!update.got_valid_frame)
    {
        return;
    }
    latest_state.last_command_id = update.command_id;
    latest_state.robot_status = packets.game_robot_status;
    ++latest_state.update_count;
}

} // namespace

types::status init() noexcept
{
    ::referee::config config{};
    config.thread_priority = params::referee::thread_priority;
    config.on_update_callback = ::referee::update_callback::bind<&on_update>();
    return ::referee::service::instance().init(config);
}

const state& latest() noexcept
{
    return latest_state;
}

} // namespace demo::referee

