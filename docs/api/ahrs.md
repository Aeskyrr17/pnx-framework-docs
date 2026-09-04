# AHRS

`ahrs::service` 使用板载 BMI088 解算姿态，并持续发布统一的 `imu::state`。应用从消息通道读取姿态，不需要自己管理 BMI088 的 SPI、数据就绪中断、温控或 EKF 解算。

## 注意

- 只在配置启用了 BMI088 时使用。`HAS_AHRS` 由板卡的 `board.json` 和 `board.ioc` 生成；
- `init()` 成功只表示服务线程已创建。它会继续在后台配置 BMI088、等待温度就绪并进行静止校准；应读到 `state.online == true` 后，再使用姿态控制机器人。

## 最小接入

在应用线程的初始化阶段启动服务、订阅它的输出通道；之后在自己的控制线程中读取最新消息。

下面是使用片段，不是完整程序。`subscriber` 需要长期存在，因此这里放在文件内静态对象中。

```cpp
#include "ahrs.hpp"
#include "imu.hpp"
#include "msg.hpp"

namespace robot::application
{
namespace
{

msg::subscriber imu_input{};

types::status init_ahrs() noexcept
{
    const types::status status = ahrs::service::instance().init();
    if (status != types::status::ok)
    {
        return status;
    }

    imu_input = msg::subscribe(ahrs::service::instance().output());
    return imu_input.valid() ? types::status::ok : types::status::error;
}

bool read_attitude(imu::state& output) noexcept
{
    return msg::read(imu_input, output) == types::status::ok;
}

} // namespace
} // namespace robot::application
```

`read_attitude()` 只在有新消息时返回 `true`；没有新消息时返回 `false`。`msg::read()` 会获取消息通道的 ThreadX mutex，因此只能从线程调用，不能在中断回调中调用。

默认 `ahrs::config` 已经使用生成的 `params::ahrs` 值，不需要在应用中重复赋值。只有需要临时覆盖默认行为时才创建 `cfg` 并传给 `init(cfg)`；服务会复制它。第一次成功 `init()` 后再次调用不会更新已经保存的配置。

## 姿态数据

`imu::state` 的常用字段如下：

| 成员 | 用处 |
| --- | --- |
| `online` | 本次 BMI088 读取和解算是否可用。控制前先检查它。 |
| `yaw`、`pitch`、`roll` | 当前姿态角，单位为弧度。 |
| `total_yaw` | 连续累计的 yaw，跨越一圈时不会回跳。 |
| `quaternion[4]` | 姿态四元数，顺序为 W、X、Y、Z。 |
| `gyro[3]`、`accel[3]` | 本次解算使用的角速度和加速度。 |
| `sequence`、`received_tick` | 本次输出的序号和 ThreadX tick 时间戳。 |

## 温控与启动

默认 `temperature_control_enabled` 为 `true`，温控目标和线程优先级来自 `params.json` 的 `ahrs`。板卡当前实现带有 BMI088 加热 PWM 时，服务会先等温度达到内部阈值，再开始校准和解算。

若你的板卡没有可用加热器，或调试时不需要加热，可在初始化前设置：

```cpp
ahrs::config cfg{};
cfg.temperature_control_enabled = false;
```

当前实现关闭温控后仍会等待传感器温度达到至少 20°C，随后才继续初始化。它不是“跳过所有启动检查”的开关。

## 使用外置 DMIMU

`ahrs::dmimu_service` 是独立服务：它接收外置 DMIMU 的 CAN 数据并发布另一条 `imu::state` 通道，不会和 BMI088 的 `ahrs::service` 自动合并或互相替代。

只有在 `robot.json` 启用 `devices.dmimu` 时，`HAS_DMIMU` 才为真，`robot_config.hpp` 才会生成 `robot::imu::dmimu` 的 CAN 传输配置。当前 DMIMU 只支持 Classic CAN。应用需要选择订阅哪一个服务的 `output()`；如果同时初始化两者，也必须自己决定控制逻辑使用哪一条姿态数据。

DMIMU 服务的完整初始化写法可参考诊断目录中的对应示例。它发布的 `yaw`、`pitch`、`roll` 也会转换为弧度，以与 BMI088 输出保持一致。

## 常见错误

- 收到 `init() == ok` 后立刻开始姿态控制，没有等待 `online` 数据。
- 直接调用 `service::imu()` 读 BMI088，或手动初始化它内部使用的外设。这会绕过服务的线程和解算流程。
- 在中断回调中调用 `msg::read()`。
- 同时启动 BMI088 和 DMIMU 后，把两条 `output()` 当作同一个自动切换的数据源。当前没有这样的仲裁功能。

## 相关内容

- [配置](../configuration.md)
- [通用回调](../concepts/interrupt-callback.md)
