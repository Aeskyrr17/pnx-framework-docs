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
  -> demo::application::start()
  -> 应用 ThreadX 线程
  -> demo 服务和周期性应用循环
```

前半段位于 `board/Core/Src/main.c`。ThreadX 创建应用资源时，`board/Core/Src/app_threadx.c` 会调用 `app_start()`。默认实现位于 `demo/app.cpp`，它只创建应用线程。该线程随后依次初始化 topic、可选 AHRS、已配置电机、裁判系统、遥控器、USART 和可选 USB。

各服务的 `init()` 会创建资源并自动启动内部 ThreadX 线程。因此，只要服务仍在运行，调用方就必须让配置对象、回调目标和设备对象保持有效。当前 demo 使用 singleton 服务，使用起来较简单，但这仍然是 API contract。

## 命名约定

为区分“配置一个可复用组件”和“启动应用行为”，入口名称按以下语义使用：

| 名称 | 适用层级 | 含义 |
| --- | --- | --- |
| `init()` | BSP、设备、可复用服务 | 进行一次性资源/硬件配置；服务可在内部创建并自动启动其私有线程。 |
| `start()` | 应用、task、诊断功能，或具备独立启停语义的设备动作 | 使一个上层功能开始工作，通常会创建其业务线程或启用已初始化资源。 |
| `run()` | 私有线程入口或阻塞循环 | 不作为会自行创建线程的公开功能入口。 |

因此，调用 `ahrs::service::init()`、`remoter::service::init()` 或 `bsp::usb::init()` 不需要由 task 再创建相应驱动线程；task 只为云台控制、协议组包等业务循环调用或实现 `start()`。诊断模块的公开入口同样统一为 `start()`。

不要把依赖外设的初始化移到 CubeMX 外设初始化之前。不要在 `app_start()` 阻塞；应像 `demo::application::start()` 一样创建 ThreadX 线程来运行应用工作。

## 入口
ThreadX 初始化阶段会从 `board/Core/Src/app_threadx.c` 调用：
```cpp
extern "C" void app_start();
```
- 新机器人控制循环从 [`demo/app.cpp`](../demo/app.cpp) 开始，或者替换 `app_start()` 的实现。
- 新传感器若需要 MCU 外设，先修改 `board/board.ioc`；缺少外设封装时再写 BSP；最后在 `pnx_devices/` 写设备驱动。
- 一个可复用的后台服务如果拥有线程，并向多个应用提供状态，应放进 `pnx_modules/`。
- 一次性的机器人行为属于应用层，不应放进 BSP 或设备驱动。
