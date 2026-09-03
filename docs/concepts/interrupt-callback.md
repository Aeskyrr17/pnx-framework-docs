# 通用回调

回调是“发生事件时，底层调用你提供的函数”。它让通信、外设或模块只负责发现事件；机器人应用决定收到事件后怎样处理。这样更换 CAN 报文、遥控器或上层控制逻辑时，不需要把应用逻辑写进 BSP 或模块，也不需要让应用了解其内部实现。

PnX 的 `core::callback` 是一个轻量的非拥有回调：内部只保存对象指针和函数指针，不分配内存，也不使用 mutex。

## 什么时候使用

当 API 接受 `rx_callback`、`update_callback`、`interrupt_callback` 或 `on_*_callback` 时，应用可以注册自己的处理函数。例如：CAN 收到一帧数据、遥控器更新了状态、裁判系统识别出一个数据包时，模块会调用该函数。

回调只负责把“事件已经发生”交给上层。业务处理复杂时，回调应快速复制必要数据或通知应用线程，由线程完成控制计算。

## 最小写法

下面是 `core::callback` 的使用片段；具体把它传给哪个接口，参见对应模块的 API 页面。

```cpp
#include "callback.hpp"

#include <cstdint>

namespace robot::application
{
namespace
{

struct command_receiver
{
    void on_command(std::uint8_t command) noexcept
    {
        // 把 command 交给自己的应用逻辑。
    }
};

command_receiver receiver{}; // 注册期间必须一直存在

const auto on_command =
    core::callback<void(std::uint8_t)>::bind<command_receiver,
                                               &command_receiver::on_command>(&receiver);

} // namespace
} // namespace robot::application
```

静态函数也可以绑定：

```cpp
void on_command(std::uint8_t command) noexcept;

const auto callback = core::callback<void(std::uint8_t)>::bind<&on_command>();
```

## 使用边界

- 回调不拥有成员函数所绑定的对象。对象必须在注册到注销期间持续有效；文件内 `static` 对象通常最直接。
- 回调的运行上下文由注册它的模块决定，并不总是中断。CAN、USART、EXTI 的接收或中断回调处于 HAL 中断路径，不能等待、获取 ThreadX mutex、调用可能阻塞的 API 或进行耗时计算。
- Referee 和 Remoter 的更新回调运行在各自的服务线程中，不是中断；但它们会占用服务的接收处理流程，也应尽快返回。
- 传入回调的引用参数通常只保证在本次调用中有效。若要留到之后的线程处理，复制所需内容。

具体回调的调用上下文和数据有效期，以对应 API 页面为准。
