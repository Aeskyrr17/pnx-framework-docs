# 启动流程

应用代码从 `app_start()` 开始

## 实际顺序

```text
复位
  -> main()
  -> HAL / CubeMX 外设初始化
  -> MX_ThreadX_Init()
  -> tx_kernel_enter()
  -> App_ThreadX_Init()
  -> app_start()
```

`App_ThreadX_Init()` 位于 `board/Core/Src/app_threadx.c`，它在 ThreadX 内核启动后调用 `app_start()`。

当前仓库的 `app_start()` 实际调用 `diagnose_start()`，用于启动诊断模块。开始写机器人程序时，把这里替换成自己的应用入口即可。

## 从哪里开始写

在 `app_start()` 中只做启动工作，例如创建应用线程、初始化需要的模块：

```cpp
extern "C" void app_start()
{
    robot::application::start();
}
```

耗时循环、周期控制和持续运行的逻辑应放在线程中，不要让 `app_start()` 自己阻塞。

各模块的 `init()` 会按自身实现创建所需资源和后台线程；应用只需按 API 要求初始化并保存仍在使用的对象。

## 修改启动入口时注意

- 不要在 `main()` 或 `app_threadx.c` 里加入机器人业务逻辑。
- 不要在 ThreadX 启动前使用依赖 HAL 外设的模块。
- 回调、线程控制块、线程栈以及仍被模块使用的配置对象，必须保持有效的生命周期。
- 修改 `board.ioc` 后，先重新生成 CubeMX 板级代码，再执行 CMake configure 和 build。

相关内容：[`项目结构`](project-structure.md)、[`配置`](configuration.md)、[`创建应用线程`](thread.md)。
