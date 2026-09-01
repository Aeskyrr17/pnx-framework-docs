#include "imu_example.hpp"

#include "ahrs.hpp"
#include "config.hpp"
#include "msg.hpp"

namespace demo::imu
{
namespace
{

msg::subscriber input{};

} // namespace

types::status init() noexcept
{
    ahrs::config config{};
    config.imu_offset_x = params::ahrs::imu_offset_x;
    config.imu_thread_priority = params::ahrs::imu_thread_priority;
    config.temp_thread_priority = params::ahrs::temp_thread_priority;
    config.target_temp = params::ahrs::target_temp;

    const types::status status = ahrs::service::instance().init(config);
    if (status != types::status::ok)
    {
        return status;
    }

    input = msg::subscribe(ahrs::service::instance().output());
    return input.valid() ? types::status::ok : types::status::error;
}

bool read(::imu::state& data) noexcept
{
    return msg::read(input, data) == types::status::ok;
}

} // namespace demo::imu

