# bsp::adc

`bsp::adc` 读取已在板卡配置中声明的模拟输入。

## 注意

当前实现只支持单通道、软件触发、非扫描模式的 ADC 转换。

## Header / Namespace

- Header：`bsp_adc.hpp`
- Namespace：`bsp::adc`

## 初始化

在启动阶段调用 `init()`；需要校准时再调用 `calibrate()`。应用可以在 `params.json` 的 `bindings.adc_channels` 中为 IOC 已配置的单通道转换取角色名，生成 `app::adc::<角色>`。

```json
{ "bindings": { "adc_channels": { "battery_voltage": { "adc": "adc1", "channel": 4 } } } }
```

```cpp
// 使用片段：先在 params.json 配置 battery_voltage
#include "bsp_adc.hpp"

std::uint32_t raw_value = 0U;
constexpr auto channel = app::adc::battery_voltage;

types::status read_sensor() noexcept
{
    types::status status = bsp::adc::init(channel);
    if (status != types::status::ok)
    {
        return status;
    }
    return bsp::adc::read_raw(channel, raw_value, 10U);
}
```

## 核心类型

| 类型 | 用途 |
| --- | --- |
| `conversion_state` | 查询中断或 DMA 转换是否完成，以及最后一次结果或状态。 |
| `bsp::dma::buffer<N>` | DMA 采样缓冲区；`N` 必须是 4 的倍数。 |

## 常用接口

| 接口 | 用途与注意事项 |
| --- | --- |
| `init(channel)` | 检查该通道及当前 ADC 配置是否可用。 |
| `calibrate(channel)` | 执行单端偏移校准；在开始测量前调用。 |
| `read_raw(channel, value, timeout_ms)` | 阻塞读取一次原始 ADC 值。 |
| `start_it(channel)` | 异步开始一次转换；完成后查看 `state(channel).last_value`。 |
| `start_dma(channel, buffer)` | 异步采样到 DMA 缓冲区；缓冲区必须为长期存在的 `bsp::dma::buffer`。 |
| `stop_dma(channel)` | 停止当前 DMA 采样。 |

## 常见错误

- 在 ADC 配成扫描、外部触发或多通道时调用：`init()` 会返回 `not_configured`。
- 在 `read_raw()` 的短超时内做不完转换：该接口会阻塞到超时。
- DMA 缓冲区不是静态对象，或大小不是 4 的倍数。

## 相关内容

- [DMA 缓冲区](bsp-dma.md)
- [配置](../configuration.md)
