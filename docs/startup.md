# 启动流程

当前启动路径是：

```text
复位
  -> main()
  -> 启用 I-Cache 和 D-Cache
  -> HAL_Init()、时钟、CubeMX 外设初始化
  -> MX_ThreadX_Init()
  -> tx_kernel_enter()
  -> App_ThreadX_Init()
  -> app_start()
  -> diagnose_start()
  -> 选中的诊断模块及其 ThreadX 线程
```

前半段位于 `board/Core/Src/main.c`。ThreadX 创建应用资源时，`board/Core/Src/app_threadx.c` 会调用 `app_start()`。当前实现位于 `diagnose/app.cpp`：它调用 `diagnose_start()`，后者默认启动 IMU 和电机检测；其他检测入口暂时以注释保留，按需要手动启用。

各服务的 `init()` 会创建资源并自动启动内部 ThreadX 线程。因此，只要服务仍在运行，调用方就必须让配置对象、回调目标和设备对象保持有效。当前 demo 使用 singleton 服务，使用起来较简单，但这仍然是 API contract。

## 命名约定

为区分“配置一个可复用组件”和“启动应用行为”，入口名称按以下语义使用：

| 名称 | 适用层级 | 含义 |
| --- | --- | --- |
| `init()` | BSP、设备、可复用服务 | 进行一次性资源/硬件配置；服务可在内部创建并自动启动其私有线程。 |
| `start()` | 应用、task、诊断功能，或具备独立启停语义的设备动作 | 使一个上层功能开始工作，通常会创建其业务线程或启用已初始化资源。 |
| `run()` | 私有线程入口或阻塞循环 | 不作为会自行创建线程的公开功能入口。 |

因此，调用 `ahrs::service::init()`、`remoter::service::init()` 或 `bsp::usb::init()` 不需要由 task 再创建相应驱动线程；task 只为云台控制、协议组包等业务循环调用或实现 `start()`。诊断模块的公开入口同样统一为 `start()`。

不要把依赖外设的初始化移到 CubeMX 外设初始化之前。不要在 `app_start()` 阻塞；诊断模块自己的 `start()` 会创建需要的 ThreadX 线程。

## 回调与消息出口

组件的 `config` 只在第一次成功 `init()` 时固定；后续 `init(config)` 不会重新绑定回调。因此，应在首次启动前确定业务回调，且回调目标必须在组件运行期间保持有效。

| 组件 | 回调是否为初始化前提 | 推荐的上层接入方式 |
| --- | --- | --- |
| `bsp::usb` | 是。必须提供 RX 或 TX-result 回调之一。 | 由拥有 USB 协议的 task 在 `config` 中绑定回调；BSP 自行创建 CDC 收发线程。 |
| `remoter::service` | 否。`on_update_callback` 是可选增强。 | 优先订阅 `output()`；若使用回调，它运行在合并线程中且不得阻塞。 |
| `referee::service` | 否。`on_update_callback` 是可选增强。 | 当前没有公开消息 channel。业务若需接收稳定快照，应在回调中轻量复制或非阻塞发布到自己的 channel，而非跨线程直接长期读取 `packets()` 引用。 |

USB RX、遥控 USART RX 和裁判 USART RX 的底层接收回调是组件的内部实现；上表指的是应用层可选择绑定的业务回调。

## 入口
ThreadX 初始化阶段会从 `board/Core/Src/app_threadx.c` 调用：
```cpp
extern "C" void app_start();
```
- 当前诊断入口在 [`diagnose/app.cpp`](../diagnose/app.cpp)；开始写机器人控制逻辑时，替换 `app_start()` 的实现。
- 新传感器若需要 MCU 外设，先修改 `board/board.ioc`；缺少外设封装时再写 BSP；最后在 `pnx_devices/` 写设备驱动。
- 一个可复用的后台服务如果拥有线程，并向多个应用提供状态，应放进 `pnx_modules/`。
- 一次性的机器人行为属于应用层，不应放进 BSP 或设备驱动。
