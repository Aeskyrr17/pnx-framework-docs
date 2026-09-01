#pragma once

#include "usertypes.hpp"

#include <cstdint>

namespace demo::topic
{

struct sample
{
    std::uint32_t sequence = 0U;
    float value = 0.0f;
};

types::status init() noexcept;
types::status publish(const sample& data) noexcept;
bool read(sample& data) noexcept;

} // namespace demo::topic

