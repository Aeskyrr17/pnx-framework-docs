# bsp::usart

`bsp::usart` 在已配置的串口上发送数据，或以 receive-to-idle DMA 接收数据块。

## 注意

DR16、裁判系统等已有模块会自行使用串口；应用不应重复初始化或接收同一端口。

## Header / Namespace

- Header：`bsp_usart.hpp`
- Namespace：`bsp::usart`

## 初始化

先在 `params.json` 为应用链路定义角色名，再选好端口模式：普通阻塞发送用 `block`，DMA 发送或 receive-to-idle 接收用 `dma`。接收缓冲区和回调目标都必须长期存在。

```json
{ "bindings": { "uart_ports": { "host_link": "uart7" } } }
```

重新 CMake configure 后，生成 `app::uart::host_link`。

```cpp
// 使用片段：在应用链路上启动 receive-to-idle DMA
#include "bsp_dma.hpp"
#include "bsp_usart.hpp"
#include "memory.h"

namespace robot::application
{
namespace
{

// BSP_DMA_BUFFER 让这块接收内存可供 DMA 直接写入。
bsp::dma::buffer<64U> rx_buffer BSP_DMA_BUFFER{};
constexpr auto host_link = app::uart::host_link;

void on_rx(bsp::usart::port, const bsp::usart::rx_frame& frame) noexcept
{
    if (frame.len > 0U)
    {
        // 快速复制或投递数据；不要阻塞。
    }
}

types::status init_link() noexcept
{
    types::status status = bsp::usart::init(host_link, bsp::usart::mode::dma);
    if (status != types::status::ok)
    {
        return status;
    }
    return bsp::usart::start_rx_to_idle(
        host_link, rx_buffer.view(), bsp::usart::rx_callback::bind<&on_rx>());
    //rx_buffer.view() 把这块缓冲区交给 USART BSP；随后 on_rx() 中的 frame.data 指向它内部收到的数据。
}

} // namespace
} // namespace robot::application
```

## 核心类型

| 类型 | 用途 |
| --- | --- |
| `mode` | `block`、`dma` 或 `it`；当前发送接口实际区分阻塞和 DMA。 |
| `line_config` | 修改波特率、数据位、停止位、校验和收发方向。 |
| `rx_frame` | 接收回调给出的数据指针和本次长度；数据只在该回调期间保证可用。 |
| `rx_callback` | 在 UART HAL 中断/DMA 回调路径执行的接收回调。 |

## 常用接口

| 接口 | 用途与注意事项 |
| --- | --- |
| `configure(port, config)` | 修改串口参数；必须在开始接收前调用，之后还要重新 `init()`。 |
| `init(port, mode)` | 初始化端口状态；同一端口成功初始化后再次调用会直接返回 `ok`，不会切换模式。 |
| `transmit(port, data, len, timeout_ms)` | `block` 模式会阻塞；`dma` 模式异步发送，最大 256 字节，忙时返回 `busy`。 |
| `start_rx_to_idle(port, buffer, callback)` | 启动 DMA 接收；缓冲区、回调目标和可选信号量都需持续有效。 |
| `restart_rx(port)` | 使用之前注册的接收所有者重新启动接收。 |

## 常见错误

- 在接收回调里调用可能阻塞的接口；回调运行在 UART HAL 的中断/DMA 路径。
- 将局部 DMA 缓冲区或短生命周期对象注册给接收。
- 同一端口已启动接收后，用不同缓冲区或回调再次 `start_rx_to_idle()`；当前实现返回 `busy`。
- DMA 模式发送超过 256 字节。

## 相关内容

- [DMA 缓冲区](bsp-dma.md)
- [通用回调](../concepts/interrupt-callback.md)
