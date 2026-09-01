#include "usb_example.hpp"

#include "bsp_usb.hpp"
#include "config.hpp"

namespace demo::usb
{

#if ENABLE_USBX
namespace
{

void on_rx(const std::uint8_t* data, std::uint16_t len) noexcept
{
    // CDC is a byte stream. This minimal example simply echoes each received
    // chunk; an application protocol should own any framing or parser state.
    (void)bsp::usb::try_send(data, len);
}

} // namespace
#endif

types::status init() noexcept
{
#if ENABLE_USBX
    bsp::usb::config config{};
    config.on_rx_callback = bsp::usb::rx_callback::bind<&on_rx>();
    return bsp::usb::init(config);
#else
    return types::status::not_configured;
#endif
}

} // namespace demo::usb

