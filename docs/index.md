# PnX_template

工程以 STM32CubeMX 生成的 `board.ioc` 工程为硬件基础，在其上提供配置驱动的 BSP、设备抽象、可复用业务模块、通用库和板端 demo，可以通过切换不同的 `pnx_bsp` 分支与`config` 端开关切换STM32H723(达妙mc-02）和STM32F407（大疆开发板C板）

建议从以下页面开始：

- [项目结构](project-structure.md)：项目结构与依赖关系
- [启动流程](startup.md)：从复位到 `app_start()` 和 ThreadX 线程。
- [配置](configuration.md)：每类配置的唯一来源。
- [API Reference](api/index.md)：BSP、Device、Module 和 Library 的公开接口。
- [运行时数据流](concepts/interrupt-callback.md)：数据从外设进入系统后如何被处理和传递
- [Review 记录](review.md)：已确认问题、设计风险和待上板验证项。

可编译的参考应用在 [`demo/`](../demo/README.md)。它的源码会编入固件目标，因此它是规范用法示例，而不是文档中容易失效的复制片段。
