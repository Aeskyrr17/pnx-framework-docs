# PnX Libs

`pnx_libs` 是被 BSP、设备和模块共同使用的基础工具库。通常只有在你写自己的控制、数据交接或算法代码时，才需要直接包含它们的头文件。

## Namespace

| Namespace | 用来做什么 | 入口 |
| --- | --- | --- |
| `types` | 通用返回值，例如 `types::status::ok`、`error`、`busy`。 | [`usertypes.hpp`](../../pnx_libs/common/include/usertypes.hpp) |
| `core` | 不分配内存的回调包装 `core::callback`；BSP 和模块注册回调时会用到。 | [`callback.hpp`](../../pnx_libs/common/include/callback.hpp) |
| `msg` | 线程之间传递最新数据的消息通道。AHRS、Remoter 等模块使用它发布状态。 | [`msg.hpp`](../../pnx_libs/msg/include/msg.hpp) |
| `control` | PID 控制器。 | [`pid.hpp`](../../pnx_libs/control/include/pid.hpp) |
| `filter` | IIR 与一维卡尔曼滤波器。 | [`iir.hpp`](../../pnx_libs/filter/include/iir.hpp)、[`kalman_1d.hpp`](../../pnx_libs/filter/include/kalman_1d.hpp) |
| `math` | 常量、限幅、坐标或数值转换。 | [`constants.hpp`](../../pnx_libs/math/include/constants.hpp)、[`constrain.hpp`](../../pnx_libs/math/include/constrain.hpp)、[`trans.hpp`](../../pnx_libs/math/include/trans.hpp) |
| `crc` | CRC8、CRC16 校验；通常由协议模块内部使用。 | [`crc.hpp`](../../pnx_libs/crc/include/crc.hpp) |
| `runtime` | 统计一次循环的耗时和超时次数。 | [`runtime_monitor.hpp`](../../pnx_libs/runtime/include/runtime_monitor.hpp) |

`memory.h` 不提供 namespace；其中的 `BSP_DMA_BUFFER` 是 DMA 缓冲区用的变量属性。只有 DMA 相关缓冲区才需要使用它，详见 [DMA 缓冲区](bsp-dma.md)。

## `msg`：在线程之间传递最新状态

`msg::channel<Payload>` 是一个有类型的 topic。`Payload` 必须是可平凡复制的类型；同一 topic 最多可有 8 个订阅者。

```cpp
#include "msg.hpp"

#include <cstdint>

namespace robot::application
{
namespace
{

struct chassis_command
{
    float forward = 0.0f;
    float turn = 0.0f;
    bool relax = true;
};

msg::channel<chassis_command> command_topic{};
msg::subscriber control_sub{};

types::status init_command_topic() noexcept
{
    const types::status status = msg::init(command_topic);//创建topic
    if (status != types::status::ok)
    {
        return status;
    }

    control_sub = msg::subscribe(command_topic);//创建subscriber
    return control_sub.valid() ? types::status::ok : types::status::error;
}

void publish_command(float forward, float turn) noexcept
{
    chassis_command command{};
    command.forward = forward;
    command.turn = turn;
    command.relax = false;
    (void)msg::publish(command_topic, command);//发布
}

bool read_command(chassis_command& output) noexcept
{
    return msg::read(control_sub, output) == types::status::ok;//读
}

} // namespace
} // namespace robot::application
```

初始化顺序是：`msg::init(topic)` → `msg::subscribe(topic)` → `publish()` / `read()`。`msg::init()` 可以重复调用同一 topic；已初始化时返回 `ok`。

`publish()` 把数据复制到 topic；`read()` 把最新数据复制到调用者提供的对象。它不是 FIFO 队列：每个 topic 只保留最新一份数据。如果两次 `publish()` 之间订阅者还没读取，旧值会被覆盖。适合 command、姿态、遥控器状态等“只关心最新值”的数据，不适合必须逐条处理的事件。

`read()` 不会等待新消息；没有新数据时返回 `types::status::empty`。但 `publish()` 和 `read()` 都会取得 topic 的 ThreadX mutex，默认可能等待 mutex，所以应在线程中调用。注意：当前 `publish_opts::from_isr` 只会改成不等待 mutex，仍然会调用 mutex；它不是 ISR 安全的发布接口。ISR 需要使用独立的交接方式，把数据交给线程后再 `publish()`。

## `control::pid`：最小闭环

`control::pid` 的调用者直接写目标值 `ref` 和反馈值 `fdb`，调用 `update()` 后从 `result` 取输出。

```cpp
#include "pid.hpp"

namespace robot::application
{
namespace
{

// kp、ki、kd、最大输出、最大积分输出；默认是位置式 PID。
control::pid speed_pid{1.0f, 0.05f, 0.0f, 10000.0f, 3000.0f};

float update_speed_control(float target_speed, float measured_speed) noexcept
{
    speed_pid.ref = target_speed;
    speed_pid.fdb = measured_speed;
    speed_pid.update();
    return speed_pid.result;
}

} // namespace
} // namespace robot::application
```

`update()` 没有时间参数。请以稳定的控制周期调用它；改变调用频率会改变 `ki` 和 `kd` 的实际效果。`max_out` 限制最终输出，`max_iout` 限制积分项。

需要重新调参时调用 `tune(kp, ki, kd)`；切换控制目标、重新使能或希望清除累计积分时，可调用 `reset_state(reference, feedback)`。不要在多个线程同时读写同一个 `pid` 对象。

## 注意

- `msg::subscribe()` 和 `msg::read()` 会使用 ThreadX mutex，只能在线程中调用，不能放进 ISR 回调。
- `core::callback` 不拥有被绑定的对象；成员函数回调绑定的对象必须在回调注册期间一直存在。

## 相关内容

- [通用回调](../concepts/interrupt-callback.md)
- [DMA 缓冲区](bsp-dma.md)
- [PID 公开头文件](../../pnx_libs/control/include/pid.hpp)
