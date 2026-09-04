# bsp::dwt

`bsp::dwt` 使用 Cortex-M 的周期计数器提供高精度计时和短延时。

## 注意

它适合计算控制循环的时间间隔，或做很短的忙等延时；不是 ThreadX 的线程休眠接口。

## Header / Namespace

- Header：`bsp_dwt.hpp`
- Namespace：`bsp::dwt`

## 初始化

先调用一次 `init()`，之后再使用计时和延时接口。

```cpp
// 使用片段：在控制循环中计算 dt
#include "bsp_dwt.hpp"

std::uint32_t last_cycle_count = 0U;

types::status init_timing() noexcept
{
    return bsp::dwt::init();
}

float control_dt() noexcept
{
    return bsp::dwt::delta_s(&last_cycle_count);
}
```

## 核心类型

| 类型 | 用途 |
| --- | --- |
| `time` | `now()` 返回的秒、毫秒和微秒三部分时间。 |

## 常用接口

| 接口 | 用途与注意事项 |
| --- | --- |
| `now()`、`timeline_s()`、`timeline_ms()`、`timeline_us()` | 获取从 `init()` 起累计的时间。 |
| `delta_s(last_cnt)`、`delta_s64(last_cnt)` | 计算本次与上次调用的时间差，并更新调用者保存的计数值。 |
| `delay_s()`、`delay_ms()`、`delay_us()` | 忙等；会占用当前 CPU，不能代替 `tx_thread_sleep()`。 |

## 常见错误

- 忘记 `init()`：计时接口返回零，延时接口直接返回。
- 每次都传入新的 `last_cnt`：无法得到连续的时间差。
- 在普通线程中用较长 `delay_ms()`：它会阻塞该线程。

## 相关内容
