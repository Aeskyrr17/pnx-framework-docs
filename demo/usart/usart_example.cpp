#include "usart_example.hpp"

#include "bsp_usart.hpp"
#include "config.hpp"

#include <cstdint>

namespace demo::usart
{
namespace
{

constexpr std::uint8_t hello[] = "hello from pnx\r\n";

} // namespace

types::status init() noexcept
{
    return bsp::usart::init(app::uart::test_report, bsp::usart::mode::block);
}

types::status send_hello() noexcept
{
    return bsp::usart::transmit(app::uart::test_report, hello, sizeof(hello) - 1U, 50U);
}

} // namespace demo::usart

