#include "imu_example.hpp"
#include "motor_example.hpp"
#include "referee_example.hpp"
#include "remoter_example.hpp"
#include "topic_example.hpp"
#include "usart_example.hpp"
#include "usb_example.hpp"

#include "config.hpp"
#include "tx_api.h"
#include "usertypes.hpp"

#include <cstdint>

namespace demo::application
{
namespace
{

constexpr ULONG application_period_ticks = 2U;
constexpr ULONG alive_check_period_ticks = 100U;
constexpr std::uint32_t application_priority = 6U;

TX_THREAD application_thread{};
alignas(8) std::uint8_t application_stack[2048]{};
bool started = false;

types::status init_examples() noexcept
{
    types::status status = topic::init();
    if (status != types::status::ok)
    {
        return status;
    }
#if HAS_AHRS
    status = imu::init();
    if (status != types::status::ok)
    {
        return status;
    }
#endif
#if ROBOT_MOTOR_COUNT > 0
    status = motor::init();
    if (status != types::status::ok)
    {
        return status;
    }
#endif
#if HAS_REFEREE
    status = referee::init();
    if (status != types::status::ok)
    {
        return status;
    }
#endif
#if ENABLE_DR16 || ENABLE_VT03 || ENABLE_PS2
    status = remoter::init();
    if (status != types::status::ok)
    {
        return status;
    }
#endif
    status = usart::init();
    if (status != types::status::ok)
    {
        return status;
    }
#if ENABLE_USBX
    status = usb::init();
    if (status != types::status::ok)
    {
        return status;
    }
#endif
    return usart::send_hello();
}

void thread_entry(ULONG /*arg*/)
{
    if (init_examples() != types::status::ok)
    {
        return;
    }

    topic::sample published{};
    topic::sample received{};
    ::imu::state imu_state{};
    ULONG last_alive_check = tx_time_get() - alive_check_period_ticks;

    for (;;)
    {
        published.sequence++;
        published.value = static_cast<float>(published.sequence);
        (void)topic::publish(published);
        (void)topic::read(received);

        // The AHRS service owns the sensor and publishes solved IMU state.
        // The upper layer only consumes its channel.
#if HAS_AHRS
        (void)imu::read(imu_state);
#else
        (void)imu_state;
#endif

        // motor_service groups all registered protocol handlers into one send.
#if ROBOT_MOTOR_COUNT > 0
        motor::send_control();
        const ULONG now = tx_time_get();
        if ((now - last_alive_check) >= alive_check_period_ticks)
        {
            motor::alive_check();
            last_alive_check = now;
        }
#else
        (void)last_alive_check;
#endif

        tx_thread_sleep(application_period_ticks);
    }
}

} // namespace

types::status start() noexcept
{
    if (started)
    {
        return types::status::ok;
    }

    // This is the application-owned upper thread. Module/service threads remain
    // internal implementation details and are started through their init APIs.
    const UINT status = tx_thread_create(
        &application_thread, const_cast<CHAR*>("application"), thread_entry, 0U,
        application_stack, sizeof(application_stack), application_priority,
        application_priority, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        return types::status::error;
    }
    started = true;
    return types::status::ok;
}

} // namespace demo::application

extern "C" void app_start()
{
    (void)demo::application::start();
}
