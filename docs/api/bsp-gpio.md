# bsp::gpio

`bsp::gpio` 读取已配置的输入引脚，或控制已配置的输出引脚。

## 注意

优先使用 `is_active()` 和 `set_active()`，它们会根据配置处理有效电平。不要绕过它直接调用 HAL。

## Header / Namespace

- Header：`bsp_gpio.hpp`
- Namespace：`bsp::gpio`

## 初始化

不需要 BSP 初始化。为引脚在板卡配置中定义角色名后，应用代码只使用 `app::gpio::<角色>`。

```json
{ "bindings": { "gpio_inputs": { "limit_switch": { "pin": "pa0", "active_level": "low" } } } }
```

这是 `board.json` 中的配置片段；实际引脚仍由板卡配置决定，不进入应用代码。

```cpp
// 使用片段：读取配置为限位开关的输入
#include "bsp_gpio.hpp"

bool limit_pressed = false;

types::status poll_limit_switch() noexcept
{
    return bsp::gpio::is_active(app::gpio::limit_switch, limit_pressed);
}
```

`limit_switch` 是配置生成的角色名；应用代码不需要知道它实际连接到哪个引脚。

## 核心类型

| 类型 | 用途 |
| --- | --- |
| `input` / `output` | 生成配置中的输入、输出引脚标识。 |
| `active_level` | 配置中的有效电平；`is_active()` 和 `set_active()` 会自动处理它。 |

## 常用接口

| 接口 | 用途与注意事项 |
| --- | --- |
| `read(input, is_high)` | 读取实际高低电平。 |
| `is_active(input, active)` | 按配置中的有效电平读取输入。 |
| `write(output, is_high)` | 写实际高低电平。 |
| `set_active(output, active)` | 按配置中的有效电平控制输出。 |
| `toggle(output)` | 翻转输出的实际电平。 |

## 常见错误

- 把“有效”误认为始终高电平；应优先使用 `is_active()` 和 `set_active()`。
- 直接控制已由其他模块管理的引脚。
- 只在代码中写枚举，未在 IOC 和板卡配置中声明该引脚。

## 相关内容

- [配置](../configuration.md)
- [EXTI 外部中断](bsp-exti.md)
- [公开头文件](../../pnx_bsp/gpio/include/bsp_gpio.hpp)
- [实现](../../pnx_bsp/gpio/src/bsp_gpio.cpp)
