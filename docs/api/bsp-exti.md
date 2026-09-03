# bsp::exti

`bsp::exti` 为已配置为外部中断的 GPIO 输入注册回调。

## 注意

回调在 GPIO HAL 中断路径执行，只能快速记录状态或通知后续线程，不能阻塞。

## Header / Namespace

- Header：`bsp_exti.hpp`
- Namespace：`bsp::exti`

## 初始化

没有单独的 `init()`。在启动阶段为输入调用 `attach()`。

```cpp
// 使用片段：为配置的传感器就绪输入注册中断
#include "bsp_exti.hpp"

namespace robot::application
{
namespace
{

volatile bool sensor_data_ready = false;

void on_sensor_data_ready() noexcept
{
    sensor_data_ready = true;
}

types::status init_sensor_interrupt() noexcept
{
    return bsp::exti::attach(
        app::gpio::sensor_ready,
        bsp::exti::interrupt_callback::bind<&on_sensor_data_ready>());
}

} // namespace
} // namespace robot::application
```

## 核心类型

| 类型 | 用途 |
| --- | --- |
| `interrupt_callback` | 无参数回调；在 GPIO HAL 中断路径中执行。 |

## 常用接口

| 接口 | 用途与注意事项 |
| --- | --- |
| `attach(input, callback)` | 绑定一个输入和回调；同一引脚再次绑定会替换旧回调。 |

## 常见错误

- 在回调中等待信号量、打印日志或进行耗时计算：它处于中断路径，必须快速返回。
- 把短生命周期对象的成员函数注册为回调：回调不拥有目标对象。
- 在 IOC 中没有把该引脚配置为 EXTI：注册并不会替你开启硬件中断。

## 相关内容

- [通用回调](../concepts/interrupt-callback.md)
- [GPIO](bsp-gpio.md)
- [公开头文件](../../pnx_bsp/exti/include/bsp_exti.hpp)
- [实现](../../pnx_bsp/exti/src/bsp_exti.cpp)
