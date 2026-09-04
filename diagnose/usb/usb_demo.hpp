#pragma once

#include "usertypes.hpp"

namespace diagnose::usb
{

types::status start() noexcept;
void poll() noexcept;

} // namespace diagnose::usb
