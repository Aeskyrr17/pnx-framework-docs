#pragma once

#include "usertypes.hpp"

namespace demo::motor
{

types::status init() noexcept;
void send_control() noexcept;
void alive_check() noexcept;

} // namespace demo::motor

