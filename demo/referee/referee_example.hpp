#pragma once

#include "referee.hpp"

#include <cstdint>

namespace demo::referee
{

struct state
{
    std::uint32_t update_count = 0U;
    std::uint16_t last_command_id = 0U;
    GameRobotStatus_t robot_status{};
};

types::status init() noexcept;
const state& latest() noexcept;

} // namespace demo::referee

