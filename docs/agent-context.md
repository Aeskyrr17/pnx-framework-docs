# 给 Agent 的项目资料

本页是可提交的项目阅读索引，可在没有本地 `AGENT.md` 时提供给 Agent。它描述资料位置，不替代具体任务要求；修改前仍应先阅读相关源码。

## 先读这些

1. [项目结构](project-structure.md)：目录职责和依赖方向。
2. [配置](configuration.md) 与 [配置参考](configuration-reference.md)：哪些文件可改、生成文件从哪里来。
3. [启动流程](startup.md)：`app_start()` 与 ThreadX 的真实入口关系。
4. [API Reference](api/index.md)：使用者可直接调用的接口；写示例或修改行为前先核对公开头文件和实现。

## 按任务补充阅读

| 任务 | 资料 |
| --- | --- |
| 修改板卡、外设、CAN 或生成逻辑 | `board/board.ioc`、`boards/<board>/board.json`、`configs/cmake/`、配置文档 |
| 修改机器人设备 | `configs/robot.json`、生成的 `robot_config.hpp`、对应 Device API 与源码 |
| 修改通信或回调 | [通用回调](concepts/interrupt-callback.md)、对应 BSP / Module API 和公开头文件 |
| 修改板端检测 | [diagnose/README.md](../diagnose/README.md) 与目标目录 README、实现源码 |
| 修改 CAN 诊断 | [pnx_bsp/can/README.md](../pnx_bsp/can/README.md)、`bsp_can.cpp`、`bsp_can_diag.cpp` |

## 约束

- `configs/generated/` 由 CMake 生成，不能手改。
- `.ioc` 改动后先用 CubeMX 生成板级代码，再 CMake configure。
- 文档面向框架使用者；API 行为、初始化顺序、线程与回调上下文必须以当前源码为准。
- `diagnose/` 是当前默认入口；开始机器人业务时应替换 `app_start()`，不要把诊断测试命令带进控制程序。
