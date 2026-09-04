# PnX_template

PnX_template 是一个基于 STM32 和 ThreadX 的嵌入式下位机模板，整合了板级外设、常用设备和可复用的基础库。

文档最后更新：2026-09-04

## 建议阅读顺序

1. [项目结构](project-structure.md)：架构思想与结构
2. [配置](configuration.md)：config是什么，怎么用
3. [启动流程](startup.md)：程序的启动顺序，应用代码应该从哪里开始运行
4. [创建应用线程](thread.md)：从 `app_start()` 创建自己的控制线程
5. [API Reference](api/index.md)：按需要选择模块，参考真实接口

## 按功能查找

- [使用 Motor](api/index.md)：电机的注册、控制和状态读取
- [使用 IMU 或 AHRS](api/index.md)：姿态数据的获取和 AHRS Service
- [使用 Remoter](api/index.md)：遥控器数据接收和使用
- [使用 CAN、USART等外设](api/index.md)：通信外设的初始化和收发接口
- [使用 DMA 缓冲区](api/bsp-dma.md)：为需要 DMA 收发的 ADC、SPI 或 USART 准备合法的长期缓冲区
- [使用 USBX](api/index.md): 如何收发和视觉通信的USB数据
- [查询 API](api/index.md)：按 BSP、设备、模块和通用库查找公开接口

## Concepts

- [通用回调](concepts/interrupt-callback.md)：了解为什么要用回调，模块怎样把事件交给上层，以及回调中的代码约束

## 给 Agent 的资料

[给 Agent 的项目资料](agent-context.md)：在没有本地 `AGENT.md` 时，提供项目结构、配置、启动、API 与诊断资料的阅读顺序。
