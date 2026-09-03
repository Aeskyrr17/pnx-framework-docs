# Motors

`motors::motor_service` 统一管理已在 `robot.json` 配置的电机：它负责接收 CAN 反馈、组合控制报文和启动协议所需步骤；应用只创建电机对象、设置命令并周期调用发送。

## 注意

- 先在 `robot.json` 的 `devices.motors.list` 配置电机型号、CAN 总线、ID 和初始控制模式。
- `register_motor()` 会初始化该电机使用的 CAN 总线并登记接收回调。不要再为同一电机 CAN 链路手动调用 `bsp::can::init()` 
- 电机对象和 `motor_service` 只保存非拥有指针，必须在整个机器人运行期间存在；通常放在应用文件的静态存储期对象中。

## 最小接入

下面是使用片段，不是完整程序。示例使用 `robot.json` 中名为 `motor1` 的电机；把 `motor1` 换成你的配置名称即可。

```cpp
#include "motor.hpp"
#include "motorservice.hpp"
#include "motortraits.hpp"
#include "robot_config.hpp"

namespace robot::application
{
namespace
{

using yaw_motor_type =
    robot::devices::motors::model_type_t<robot::motors::motor1_model>;

yaw_motor_type yaw_motor{robot::motors::motor1};//创建电机实例
motors::motor_service motor_service{};//创建电机service实例

types::status init_motors() noexcept
{
    if (!motor_service.register_motor(yaw_motor))//注册电机，每个电机都需要单独注册
    {
        return types::status::error;
    }
    return motor_service.start();//使能
}

void update_motors() noexcept
{
    motors::command command{};
    command.position = 0.0f;
    command.velocity = 0.0f;
    command.kp = 10.0f;
    command.kd = 0.5f;
    command.torque = 0.0f;

    if (yaw_motor.supports(motors::mode::mit))
    {
        yaw_motor.set_command(command, motors::mode::mit);
    }
    else
    {
        yaw_motor.relax();
    }

    motor_service.send_control();//对所有注册过的电机发送控制指令
}

void check_motors_alive() noexcept
{
    motor_service.alive_check();
}

} // namespace
} // namespace robot::application
```

初始化顺序是：创建长期存在的电机和服务对象 → `register_motor()` 每台电机 → `start()` 一次。之后在应用控制周期中按顺序设置每台电机命令，再调用一次 `send_control()`。

`start()` 必须在线程中调用。当前 DM 协议会在这里等待电机的使能反馈，最长可等待多个 ThreadX tick；不要从回调或 ISR 调用它。

## 设置命令

设置命令不会立即发送 CAN 报文；必须由后续的 `motor_service.send_control()` 发出。优先使用下面的便捷函数，并用 `supports()` 确认当前型号支持该模式：

| 接口 | 命令内容 |
| --- | --- |
| `set_current(current)` | 电流控制。 |
| `set_torque(torque)` | 力矩控制；会按该型号的力矩常数换算电流。 |
| `set_mit(position, velocity, kp, kd, torque)` | MIT 位置、速度、刚度、阻尼和力矩控制。 |
| `set_pos_speed(position, velocity)` | 位置速度控制。 |
| `set_velocity(velocity)` | 速度控制。 |
| `set_command(command, mode)` | 一次填写完整 `motors::command`，适合 MIT 等多字段模式。 |
| `relax()` | 清空命令并切换为松弛模式。 |

当前能力由具体型号决定：DJI、LK 提供电流和力矩控制；DM 提供 MIT、位置速度和速度控制。不要把不支持的模式强行写入电机；使用 `supports(mode)` 进行判断。

## 状态与掉线

| 接口 | 用处 |
| --- | --- |
| `get_feedback()` | 返回位置、速度、力矩、电流、温度和错误码等最近反馈。 |
| `status()` | 返回 `offline`、`online` 或 `blocked`。 |
| `alive_check()` | 比较两次检查之间是否收到新反馈，并更新 `status()`。应用需要定期调用服务的同名接口。 |

当前实现会在 CAN 接收中断路径中更新反馈，而 `get_feedback()` 返回的是内部对象的引用，未提供跨上下文同步快照。不要保存这个引用或在多个执行上下文中无同步地读取它；如需稳定快照，需要在应用中设计自己的安全交接方式。

## 常见错误

- 只调用 `set_command()`，却忘了在控制周期调用 `send_control()`。
- 将局部变量电机注册到长期运行的 `motor_service`；函数返回后服务会保留悬空指针。
- 注册后没有调用 `start()`，特别是 DM 电机不会完成使能流程。
- 从 `status()` 读取掉线状态，却从不周期调用 `alive_check()`。
- 应用层手动配置 CAN ID、总线类型，导致与 `robot.json` 生成配置不一致。

## 相关内容

- [配置](../configuration.md)
- [CAN](bsp-can.md)
- [通用电机接口](../../pnx_devices/motors/motor/include/motor.hpp)
- [电机服务](../../pnx_devices/motors/motor/include/motorservice.hpp)
- [生成类型映射](../../pnx_devices/motors/motor/include/motortraits.hpp)
