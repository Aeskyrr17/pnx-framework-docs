#pragma once

#include "remoter.hpp"

namespace demo::remoter
{

types::status init() noexcept;
const ::remoter::state& latest() noexcept;

} // namespace demo::remoter

