#pragma once

#include "imu.hpp"
#include "usertypes.hpp"

namespace demo::imu
{

types::status init() noexcept;
bool read(::imu::state& data) noexcept;

} // namespace demo::imu
