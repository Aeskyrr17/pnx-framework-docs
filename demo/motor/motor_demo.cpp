#include "motor_demo.hpp"

#include "demo_debug.hpp"
#include "motor.hpp"
#include "motorservice.hpp"
#include "motortraits.hpp"
#include "robot_config.hpp"
#include "tx_api.h"

#include <cstdint>

namespace demo::motor
{

namespace devices
{

namespace
{
    // Define the motor types based on the robot configuration.
    using motor1_type =
        ::robot::devices::motors::model_type_t<robot::motors::motor1_model>;
    using motor2_type =
        ::robot::devices::motors::model_type_t<robot::motors::motor2_model>;
    using motor3_type =
        ::robot::devices::motors::model_type_t<robot::motors::motor3_model>;

    //create motor instances using motor types and robot configuration
    motor1_type motor1_device{robot::motors::motor1};
    motor2_type motor2_device{robot::motors::motor2};
    motor3_type motor3_device{robot::motors::motor3};

    //create a motor service instance for all motors
    ::motors::motor_service service{};
    bool initialized = false;
}

bool initialize() noexcept
{
    if (initialized)
    {
        return true;
    }
    if (!service.register_motor(motor1_device) ||
        !service.register_motor(motor2_device) ||
        !service.register_motor(motor3_device))
    {
        return false;
    }
    if (service.start() != types::status::ok)
    {
        return false;
    }
    initialized = true;
    return true;
}

} // namespace devices

namespace
{

enum stage : std::uint32_t
{
    devices_initialized = 1U << 0U,
    control_thread_started = 1U << 1U,
    control_tx_running = 1U << 2U,
    motor1_online = 1U << 3U,
    motor2_online = 1U << 4U,
};

enum failure : std::uint32_t
{
    device_init_failed = 1U << 0U,
    thread_create_failed = 1U << 1U,
    motor1_offline_timeout = 1U << 2U,
    motor2_offline_timeout = 1U << 3U,
};

constexpr ULONG control_period_ticks = 2U;
constexpr ULONG alive_check_period_ticks = 100U;
constexpr ULONG online_timeout_ticks = 1000U;
constexpr bool require_motor1_online = false;
constexpr int16_t motor1_test_current = 3000;
constexpr float motor2_mit_position = 0.0f;
constexpr float motor2_mit_velocity = 0.0f;
constexpr float motor2_mit_kp = 0.0f;
constexpr float motor2_mit_kd = 0.0f;
constexpr float motor2_mit_torque = 0.2f;

TX_THREAD control_thread{};
alignas(8) std::uint8_t control_stack[1024]{};
bool initialized = false;
bool thread_started = false;

void set_commands() noexcept
{
    ::motors::command motor1_command{};
    motor1_command.current = motor1_test_current;
    devices::motor1_device.set_command(motor1_command, ::motors::mode::current);

    ::motors::command motor2_command{};
    motor2_command.position = motor2_mit_position;
    motor2_command.velocity = motor2_mit_velocity;
    motor2_command.kp = motor2_mit_kp;
    motor2_command.kd = motor2_mit_kd;
    motor2_command.torque = motor2_mit_torque;
    devices::motor2_device.set_command(motor2_command, ::motors::mode::mit);
}

void sync_debug(std::uint32_t stages, std::uint32_t send_count, bool timed_out) noexcept
{
    auto& state = demo::debug::debug_instance.motor_unit;
    auto& motor1 = devices::motor1_device;
    auto& motor2 = devices::motor2_device;
    const bool motor1_is_online = motor1.status() == ::motors::state::online;
    const bool motor2_is_online = motor2.status() == ::motors::state::online;

    if (motor1_is_online)
    {
        stages |= motor1_online;
    }
    if (motor2_is_online)
    {
        stages |= motor2_online;
    }

    state.stage_mask = stages;
    state.last_step = stages;
    state.total_count = send_count;
    state.observed_count = static_cast<std::uint32_t>(motor1_is_online) +
                           static_cast<std::uint32_t>(motor2_is_online);
    state.value_a = motor1.get_feedback().current;
    state.value_b = motor2.get_feedback().velocity;
    state.value_c = static_cast<float>(motor2.get_feedback().error_code);

    state.failure_mask = 0U;
    if ((stages & devices_initialized) == 0U)
    {
        state.failure_mask |= device_init_failed;
    }
    if ((stages & control_thread_started) == 0U)
    {
        state.failure_mask |= thread_create_failed;
    }
    if (require_motor1_online && timed_out && !motor1_is_online)
    {
        state.failure_mask |= motor1_offline_timeout;
    }
    if (timed_out && !motor2_is_online)
    {
        state.failure_mask |= motor2_offline_timeout;
    }

    state.failed_count = state.failure_mask == 0U ? 0U : 1U;
    const bool motor1_requirement_met = !require_motor1_online || motor1_is_online;
    state.passed = state.failure_mask == 0U && motor1_requirement_met && motor2_is_online;
    state.passed_count = state.passed ? state.total_count : 0U;
}

void control_entry(ULONG started_at_arg)
{
    std::uint32_t send_count = 0U;
    std::uint32_t stages = devices_initialized | control_thread_started;
    const ULONG started_at = started_at_arg;
    ULONG last_alive_check = started_at - alive_check_period_ticks;

    for (;;)
    {
        set_commands();
        devices::service.send_control();
        const ULONG now = tx_time_get();
        if ((now - last_alive_check) >= alive_check_period_ticks)
        {
            devices::service.alive_check();
            last_alive_check = now;
        }
        ++send_count;
        stages |= control_tx_running;
        sync_debug(stages, send_count, (now - started_at) > online_timeout_ticks);

        tx_thread_sleep(control_period_ticks);
    }
}

} // namespace

void run() noexcept
{
    auto& state = demo::debug::debug_instance.motor_unit;
    state = {};
    state.started = true;

    if (!initialized)
    {
        initialized = devices::initialize();
        if (!initialized)
        {
            state.failure_mask = device_init_failed;
            state.failed_count = 1U;
            return;
        }
    }

    if (!thread_started)
    {
        const UINT status = tx_thread_create(&control_thread, const_cast<CHAR*>("motor_demo"),
                                             control_entry, tx_time_get(),
                                             control_stack, sizeof(control_stack),
                                             6U, 6U, TX_NO_TIME_SLICE, TX_AUTO_START);
        if (status != TX_SUCCESS)
        {
            state.failure_mask = thread_create_failed;
            state.failed_count = 1U;
            return;
        }
        thread_started = true;
    }

    sync_debug(devices_initialized | control_thread_started, 0U, false);
}

} // namespace demo::motor
