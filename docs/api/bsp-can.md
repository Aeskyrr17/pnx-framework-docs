# bsp::can

`bsp::can` 提供已在板卡配置中声明的 CAN 总线初始化、发送和中断接收回调入口。

## 什么时候使用

它位于 BSP 层，在需要直接收发自定义 CAN 协议时使用：例如自定义的上下板的通信数据；电机、DMIMU 等设备会在内部直接调用，不需要直接操作 `bsp_can` 接口；

使用前先确认[配置](../configuration.md)中的配置

## Header / Namespace

- Header：`bsp_can.hpp`
- Namespace：`bsp::can`

`bus`、`bus_type` 和 `id_type` 由生成的 `config.hpp` 提供。
## 初始化

在启动阶段或应用线程中，先注册接收回调，再初始化总线。`init()` 会配置过滤器、启用中断并启动 FDCAN

```cpp
// 使用片段：放在应用的初始化阶段。
#include "bsp_can.hpp"

namespace robot::application
{
namespace
{

void on_can_frame(bsp::can::bus bus, const bsp::can::rx_frame& frame) noexcept
{
    if (bus == bsp::can::bus::fdcan2 && frame.id == 0x201U && frame.len == 8U)
    {
        // 只做快速的收帧处理；不要阻塞。
    }
}

types::status init_custom_can() noexcept
{
    constexpr auto bus = bsp::can::bus::fdcan2;

    const auto callback =
        bsp::can::rx_callback::bind<&on_can_frame>();
    types::status status = bsp::can::register_rx_callback(bus, callback);
    if (status != types::status::ok)
    {
        return status;
    }

    return bsp::can::init(bus);
}

} // namespace
} // namespace robot::application
```

对同一总线再次调用成功过的 `init()` 会直接返回 `ok`，不会重新配置它。CAN Classic / CAN FD 类型由生成配置决定，应用不再传入第二份类型信息。

## 核心类型

### `rx_frame`

接收回调拿到的一帧 CAN 数据。

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `id` | `uint32_t` | 实际收到的报文 ID。 |
| `len` | `uint8_t` | `data` 中的有效字节数。 |
| `id_kind` | `id_type` | 这帧使用标准 ID 还是扩展 ID。 |
| `format` | `bus_type` | 这帧是 Classic CAN 还是 CAN FD。 |
| `bit_rate_switch` | `bool` | 这帧 CAN FD 是否使用比特率切换。 |
| `data` | `uint8_t[64]` | 报文数据；Classic CAN 最多使用前 8 字节。 |

`const rx_frame&` 只在本次回调调用期间有效。如需在线程中继续处理，应复制需要的数据，或使用适合 ISR 的交接方式。

### `rx_callback`

| 项目 | 内容 |
| --- | --- |
| 类型 | `core::callback<void(bus, const rx_frame&)>` |
| 第一个参数 | 接收到报文的总线。 |
| 第二个参数 | 本次接收的 `rx_frame`。 |
| 可绑定目标 | 静态函数，或长期存在对象的成员函数。 |
| 调用上下文 | FDCAN HAL 中断路径。 |

回调运行在 FDCAN HAL 中断路径。它不能等待、获取 ThreadX mutex、调用可能阻塞的 BSP API，或执行耗时解析。回调目标由调用者持有；BSP 不管理其生命周期。

### `bus`、`bus_type`、`id_type`

| 类型 | 调用者如何使用 |
| --- | --- |
| `bus` | 选择生成配置中的总线，例如 `bsp::can::bus::fdcan1`。 |
| `bus_type` | `classic` 或 `fd`。由生成配置决定；接收帧的 `format` 字段也使用这个类型。 |
| `id_type` | `standard` 或 `extended`。它是生成的接收过滤器 ID 类型。 |

此模块没有应用层的运行时 `config` 结构体；总线能力、接收 FIFO 和过滤器 ID 类型来自生成配置。

## 常用接口

先按目的查找：

| 我要做什么 | 接口 | 应在哪个上下文调用 |
| --- | --- | --- |
| 读取板卡配置 | `bus_enabled()`、`configured_bus_type()`、`filter_id_type_of()` | 启动或 ThreadX 线程 |
| 启动总线 | `init()` | 启动或 ThreadX 线程 |
| 发送一帧 | `transmit()` | 应用或设备线程 |
| 收到帧时处理 | `register_rx_callback()` | 启动或 ThreadX 线程，在 `init()` 前 |
| 移除所有接收处理 | `unregister_rx_callbacks()` | 接收 IRQ 已停止后 |
| 重启或监测错误 | `restart()`、`err_sem()` | ThreadX 线程 |

### `init(bus)`

启动一条已配置的 CAN 总线。

- Context：启动阶段或 ThreadX 线程；不要在 ISR 中调用。
- Preconditions：`bus` 已由配置生成。
- Return：成功为 `ok`；未配置的总线为 `not_configured`；枚举值无效为 `invalid_arg`；HAL 或 ThreadX 资源创建失败为 `error`。

### `transmit(bus, id, data, len)`

把一个数据帧加入该总线的发送 FIFO。

- Context：应用或设备线程。当前接口没有发送锁；多个执行上下文不要同时调用它。
- Blocking：不会等待发送完成；FIFO 无法加入报文时返回 `error`。
- Ownership：在函数调用期间读取 `data`；调用返回后不再借用调用者缓冲区。
- Preconditions：总线已成功 `init()`；`data` 非空、`len` 非零；Classic CAN 最多 8 字节，CAN FD 最多 64 字节；`id` 必须在当前生成的标准/扩展 ID 范围内。
- Return：`ok` 只表示已加入发送 FIFO，不表示另一节点已经收到该帧。

当前实现会把**生成的接收过滤器 ID 类型**也用于发送帧的 ID 类型。因此，一条总线不能通过这个 API 在标准 ID 和扩展 ID 间按帧切换。

### `register_rx_callback(bus, callback)`

为一条总线添加一个接收处理函数。

- Context：启动或线程上下文，在 `init()` 前完成。
- Blocking：否。
- Lifetime：回调函数或绑定对象必须一直有效，直到调用 `unregister_rx_callbacks()`，或直到程序结束。
- Return：`ok` 表示已登记；无效回调或总线枚举为 `invalid_arg`；未配置为 `not_configured`；回调槽已满时为 `error`。当前每条总线最多的槽数由生成配置的 `max_rx_callbacks` 决定。

所有登记的回调都会收到该总线上每一帧；回调自身负责按 `bus`、`frame.id` 和长度筛选。

### `unregister_rx_callbacks(bus)`

清空一条总线上的**全部**接收回调，没有单独注销某一个回调的接口。

不要在接收中断可能运行时调用它。当前实现不与 IRQ 同步，注销、注册和回调执行并发时会发生数据竞争。

### `restart(bus)` 和 `err_sem(bus)`

`restart()` 停止后重新启动一条已经初始化的总线，适合由线程中的故障恢复逻辑调用；失败返回 `error`。

`err_sem()` 在 `init()` 成功后返回该总线的错误信号量，否则返回 `nullptr`。当前实现只会在 **Classic CAN** 的 warning、passive 或协议错误中置位这个信号量；CAN FD 和单独的 Bus-Off 不会通过它通知。除非你确实需要自己的错误恢复线程，通常不必使用它。

### 查询生成配置

`bus_enabled()`、`configured_bus_type()` 和 `filter_id_type_of()` 可用于读取生成配置；`handle_of()` 和 `bus_of()` 用于与 HAL 句柄衔接。普通应用通常不需要调用它们。

`receive()` 和 `handle_error()` 是 HAL 回调桥接入口；上层应用不应直接调用。

## 最小使用示例

下面是发送一帧的使用片段。它假设前面的 `init_custom_can()` 已成功完成。

```cpp
#include <cstdint>

const std::uint8_t command[] = {0x01U, 0x02U, 0x03U};
const types::status status = bsp::can::transmit(
    bsp::can::bus::fdcan2, 0x201U, command, sizeof(command));

if (status != types::status::ok)
{
    // 处理未初始化、参数错误或发送 FIFO 已满等失败情况。
}
```

## 常见错误

- 在回调里解析完整协议、等待信号量或发送 CAN。回调处于中断路径，应把后续工作交给线程。
- 先启动总线、后注册回调。当前注册不会与 IRQ 同步，启动窗口内的帧可能没有业务处理者。
- 把局部对象的成员函数注册为回调，然后离开作用域。回调保存的是非拥有引用。
- 配置了标准 ID 过滤器，却期望通过 `transmit()` 发送扩展 ID（或相反）。当前 API 没有逐帧指定 ID 类型的参数。
- 认为 `transmit()` 返回 `ok` 就代表报文已在总线上成功发送或对方已处理；它只表示进入发送 FIFO。

## 相关内容

- [第一个 PnX 应用](../guide/first-application.md)
- [配置](../configuration.md)
- [中断、回调、DMA 与消息通道](../concepts/interrupt-callback.md)
- [公开头文件](../../pnx_bsp/can/include/bsp_can.hpp)
- [实现](../../pnx_bsp/can/src/bsp_can.cpp)
