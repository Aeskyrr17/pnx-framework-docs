#include "usb_demo.hpp"

#include "bsp_usb.hpp"
#include "demo_debug.hpp"
#include "demo_protocol.hpp"
#include "usb_host_service.hpp"

namespace demo::usb
{
namespace
{

bool started = false;

void sync_bsp_state(debug::link_state& debug) noexcept
{
    const auto& usb = bsp::usb::state();
    debug.connected = usb.connected;
    debug.bsp_read_busy = usb.read_busy;
    debug.bsp_write_busy = usb.write_busy;
    debug.bsp_last_read_status = usb.last_read_status;
    debug.bsp_last_write_status = usb.last_write_status;
    debug.bsp_last_read_len = usb.last_read_len;
    debug.bsp_last_write_requested = usb.last_write_requested;
    debug.bsp_last_write_actual = usb.last_write_actual;
    debug.bsp_pending_write_len = usb.pending_write_len;
    debug.bsp_pending_write = usb.pending_write;
    debug.bsp_in_flight_write = usb.in_flight_write;
    debug.bsp_read_count = usb.read_count;
    debug.bsp_write_count = usb.write_count;
    debug.bsp_error_count = usb.error_count;
    debug.bsp_tx_wake_count = usb.tx_wake_count;
    debug.tx_pending = usb.pending_write || usb.in_flight_write;
}

void sync_protocol_state(debug::link_state& debug) noexcept
{
    const auto& protocol_state = host_protocol::usb::service::instance().state();
    debug.rx_count = protocol_state.rx_count;
    debug.tx_count = protocol_state.tx_count;
    debug.error_count = protocol_state.error_count;
    debug.tx_pending_seq = protocol_state.tx_pending_seq;
    debug.tx_pending = protocol_state.tx_pending;
    debug.last_status = protocol::status_code(protocol_state.last_status);
    debug.last_rx = protocol_state.last_rx;
    debug.last_tx = protocol_state.last_tx;
    debug.last_seq = protocol_state.last_rx.seq;
    debug.last_counter = protocol_state.last_rx.counter;
    debug.last_value = protocol_state.last_rx.value;
    debug.last_flag = protocol_state.last_rx.flag != 0U;
}

types::status on_request(const protocol::host_packet&)
{
    // Add application-level request handling here.
    return types::status::ok;
}

// Minimal application startup example. It intentionally remains commented out:
// users normally place an equivalent callback beside their own application logic.
// types::status start_minimal_usb_demo() noexcept
// {
//     host_protocol::usb::config config{};
//     config.on_request = host_protocol::usb::request_callback::bind<&on_request>();
//     return host_protocol::usb::service::instance().init(config);
// }

} // namespace

types::status start() noexcept
{
    auto& state = debug::debug_instance.usb;
    if (started)
    {
        state.started = true;
        sync_protocol_state(state);
        sync_bsp_state(state);
        return types::status::ok;
    }

    debug::reset(state);
    state.started = true;

    host_protocol::usb::config config{};
    config.on_request = host_protocol::usb::request_callback::bind<&on_request>();
    const types::status status = host_protocol::usb::service::instance().init(config);
    state.last_status = protocol::status_code(status);
    if (status != types::status::ok)
    {
        ++state.error_count;
        return status;
    }

    state.ready = true;
    sync_protocol_state(state);
    sync_bsp_state(state);
    started = true;
    return types::status::ok;
}

void poll() noexcept
{
    auto& state = debug::debug_instance.usb;
    sync_protocol_state(state);
    sync_bsp_state(state);
}

} // namespace demo::usb
