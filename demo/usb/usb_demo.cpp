#include "usb_demo.hpp"

#include "bsp_usb.hpp"
#include "demo_debug.hpp"
#include "demo_protocol.hpp"

#include <array>
#include <cstring>

namespace demo::usb
{
namespace
{

bool started = false;
std::array<uint8_t, sizeof(protocol::host_packet)> rx_packet_bytes{};
std::size_t rx_packet_size = 0U;

void sync_bsp_state(debug::link_state& debug) noexcept
{
    const auto& usb_state = bsp::usb::state();
    debug.connected = usb_state.connected;
    debug.bsp_read_busy = usb_state.read_busy;
    debug.bsp_write_busy = usb_state.write_busy;
    debug.bsp_last_read_status = usb_state.last_read_status;
    debug.bsp_last_write_status = usb_state.last_write_status;
    debug.bsp_last_read_len = usb_state.last_read_len;
    debug.bsp_last_write_requested = usb_state.last_write_requested;
    debug.bsp_last_write_actual = usb_state.last_write_actual;
    debug.bsp_pending_write_len = usb_state.pending_write_len;
    debug.bsp_pending_write = usb_state.pending_write;
    debug.bsp_in_flight_write = usb_state.in_flight_write;
    debug.bsp_read_count = usb_state.read_count;
    debug.bsp_write_count = usb_state.write_count;
    debug.bsp_error_count = usb_state.error_count;
    debug.bsp_tx_wake_count = usb_state.tx_wake_count;
    debug.tx_pending = usb_state.pending_write || usb_state.in_flight_write;
}

void record_request(debug::link_state& debug, const protocol::host_packet& packet) noexcept
{
    debug.last_rx = packet;
    debug.last_seq = packet.seq;
    debug.last_counter = packet.counter;
    debug.last_value = packet.value;
    debug.last_flag = packet.flag != 0U;
}

void process_packet(const protocol::host_packet& packet)
{
    auto& state = debug::debug_instance.usb;
    record_request(state, packet);
    if (!protocol::validate(packet, protocol::usb_host_magic))
    {
        ++state.error_count;
        state.last_status = protocol::status_code(types::status::invalid_arg);
        return;
    }

    ++state.rx_count;
    const protocol::device_packet response = protocol::make_response(
        protocol::usb_device_magic, packet, types::status::ok, state.connected,
        state.rx_count, state.tx_count + 1U, state.error_count);
    const types::status status = bsp::usb::try_send(response);
    state.last_tx = response;
    state.last_status = protocol::status_code(status);
    if (status == types::status::ok)
    {
        ++state.tx_pending_seq;
        state.ready = true;
    }
    else
    {
        ++state.error_count;
    }
    sync_bsp_state(state);
}

void on_rx_callback(const uint8_t* data, uint16_t len)
{
    // CDC supplies a byte stream. This demo's fixed-size framing is handled here,
    // rather than assuming one on_rx_callback invocation contains one host_packet.
    for (uint16_t index = 0U; index < len; ++index)
    {
        rx_packet_bytes[rx_packet_size++] = data[index];
        if (rx_packet_size == rx_packet_bytes.size())
        {
            protocol::host_packet packet{};
            std::memcpy(&packet, rx_packet_bytes.data(), sizeof(packet));
            rx_packet_size = 0U;
            process_packet(packet);
        }
    }
}

void on_tx_result_callback(const bsp::usb::tx_result& result)
{
    auto& state = debug::debug_instance.usb;
    if (result.success() && result.requested_len == sizeof(protocol::device_packet) &&
        result.actual_len == result.requested_len)
    {
        ++state.tx_count;
        state.last_status = protocol::status_code(types::status::ok);
    }
    else
    {
        ++state.error_count;
        state.last_status = protocol::status_code(types::status::error);
    }
    sync_bsp_state(state);
}

// A real application can define arbitrary packet types and parser state here.
// The BSP only forwards byte-stream chunks and copies outgoing bytes.
// struct application_packet { ... };
// void on_application_rx(const uint8_t* data, uint16_t len) { ... }
// const auto status = bsp::usb::try_send(application_packet{});

} // namespace

types::status start() noexcept
{
    auto& state = debug::debug_instance.usb;
    if (started)
    {
        state.started = true;
        sync_bsp_state(state);
        return types::status::ok;
    }

    debug::reset(state);
    state.started = true;

    bsp::usb::config config{};
    config.on_rx_callback = bsp::usb::rx_callback::bind<&on_rx_callback>();
    config.on_tx_result_callback =
        core::callback<void(const bsp::usb::tx_result&)>::bind<&on_tx_result_callback>();
    const types::status status = bsp::usb::init(config);
    state.last_status = protocol::status_code(status);
    if (status != types::status::ok)
    {
        ++state.error_count;
        return status;
    }

    state.ready = true;
    sync_bsp_state(state);
    started = true;
    return types::status::ok;
}

void poll() noexcept
{
    sync_bsp_state(debug::debug_instance.usb);
}

} // namespace demo::usb
