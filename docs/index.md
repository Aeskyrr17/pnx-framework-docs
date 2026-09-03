# PnX_template

PnX_template 是一套运行在 STM32 和 ThreadX 上的模板工程，组织了板级外设、常用设备和可复用library

## 第一次使用

建议按下面顺序阅读：

1. [项目结构](project-structure.md)：了解代码、配置和参考应用分别放在哪里。
2. [配置](configuration.md)：修改板卡、外设和机器人设备配置，并重新生成构建结果。
3. [启动流程](startup.md)：知道程序怎样启动，以及你的应用代码从哪里开始运行。
4. [API Reference](api/index.md)：按需要选择模块，参考真实接口把功能组合进自己的机器人逻辑。

## 按功能查找

- [使用 Motor](api/index.md)：查找已配置电机的注册、控制和状态读取接口。
- [使用 IMU 或 AHRS](api/index.md)：查找姿态数据的获取和 AHRS 服务的使用方式。
- [使用 Remoter](api/index.md)：查找遥控器数据接收和更新通知的使用方式。
- [使用 CAN、USART等外设](api/index.md)：查找通信外设的初始化和收发接口。
- [使用 DMA 缓冲区](api/bsp-dma.md)：为需要 DMA 收发的 ADC、SPI 或 USART 准备合法的长期缓冲区。
- [使用 USBX](api/index.md): 如何收发和视觉通信的USB数据
- [查询 API](api/index.md)：按 BSP、设备、模块和通用库查找公开接口。

## 理解架构

- [创建应用线程](concepts/thread.md)：知道怎样从 `app_start()` 创建自己的周期控制线程，以及对象应保持多久有效。
- [通用回调](concepts/interrupt-callback.md)：了解模块怎样把事件交给上层，以及回调中的代码应保持多轻量。

## 给 Agent 的资料

[给 Agent 的项目资料](agent-context.md)：在没有本地 `AGENT.md` 时，提供项目结构、配置、启动、API 与诊断资料的阅读顺序。
