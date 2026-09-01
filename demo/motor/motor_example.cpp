#include "motor_example.hpp"

#include "motor.hpp"
#include "motorservice.hpp"
#include "motortraits.hpp"
#include "robot_config.hpp"

namespace demo::motor
{
#if ROBOT_MOTOR_COUNT > 0
namespace
{

using configured_motor =
    robot::devices::motors::model_type_t<robot::motors::motor1_model>;

configured_motor motor{robot::motors::motor1};
motors::motor_service service{};

} // namespace
#endif

types::status init() noexcept
{
#if ROBOT_MOTOR_COUNT > 0
    if (!service.register_motor(motor))
    {
        return types::status::error;
    }
    return service.start();
#else
    return types::status::not_configured;
#endif
}

void send_control() noexcept
{
#if ROBOT_MOTOR_COUNT > 0
    // The minimal reference keeps the motor relaxed. Replace this command in
    // the upper application before selecting another control mode.
    motors::command command{};
    motor.set_command(command, motors::mode::relax);
    service.send_control();
#endif
}

void alive_check() noexcept
{
#if ROBOT_MOTOR_COUNT > 0
    service.alive_check();
#endif
}

} // namespace demo::motor
