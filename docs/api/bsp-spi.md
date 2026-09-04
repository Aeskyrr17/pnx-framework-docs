# bsp::spi

`bsp::spi` 在已配置的 SPI 总线上收发字节数据。

## 注意

只为自定义 SPI 设备直接调用。BMI088、LED 等已有设备由对应 Device/Module 管理，通常不需要直接使用本页接口。

## Header / Namespace

- Header：`bsp_spi.hpp`
- Namespace：`bsp::spi`

## 初始化

先在 `params.json` 配置应用角色，再调用 `init()`。例如：

```json
{ "bindings": { "spi_buses": { "custom_sensor": "spi2" } } }
```

```cpp
// 使用片段：发送一个设备命令
#include "bsp_spi.hpp"

constexpr std::uint8_t command[] = {0x80U, 0x01U};

types::status init_sensor_bus() noexcept
{
    return bsp::spi::init(app::spi::custom_sensor);
}

types::status send_sensor_command() noexcept
{
    return bsp::spi::transmit(app::spi::custom_sensor, command, sizeof(command), 10U);
}
```

## 核心类型

| 类型 | 用途 |
| --- | --- |
| `bus` | 生成配置中的 SPI 总线；应用代码优先使用 `app::spi::<角色>`。 |
| `transfer_state` | 查询中断或 DMA 传输的完成状态。 |
| `bsp::dma::buffer<N>` | DMA 接收或发送的长期缓冲区。 |

## 常用接口

| 接口 | 用途与注意事项 |
| --- | --- |
| `init(bus)` | 检查总线已在配置中启用。 |
| `transmit()` / `receive()` | 阻塞收发；最多等待传入的超时时间。 |
| `transmit_it()` / `receive_it()` | 异步中断收发；调用者的普通缓冲区必须保持有效到完成。 |
| `transmit_dma()` / `receive_dma()` | 异步 DMA 收发；传入静态 DMA 缓冲区。 |
| `state(bus)` | 查看异步传输是否 `complete`，以及 `last_status`。 |

## 常见错误

- 将局部数组传给 `*_it()`，函数返回后数组已失效。
- 用普通数组调用 `*_dma()`；必须使用 [DMA 缓冲区](bsp-dma.md)。
- 忽略 SPI 设备自己的 CS 引脚和传输协议；SPI BSP 只负责总线收发。

## 相关内容

- [DMA 缓冲区](bsp-dma.md)
- [配置](../configuration.md)
