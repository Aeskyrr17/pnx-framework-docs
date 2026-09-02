# 配置

配置在 CMake *configure* 阶段生成，不在固件运行时生成。修改输入文件后需要重新执行 CMake configure；常规 preset build 检测到依赖变化时也会执行。

| 文件 | 唯一职责 |
| --- | --- |
| `boards/h723_v1/board.cmake` | 选择板卡 profile、IOC、工具链和链接脚本 |
| `board/board.ioc` | CubeMX 外设实例、引脚、时钟/DMA 设置和 CAN 帧格式 |
| `boards/h723_v1/board.json` | 板卡能力、DMA 内存策略和固定的板载设备绑定 |
| `configs/params.json` | 构建开关、UART 绑定、CAN 接收 ID 类型和服务参数 |
| `configs/robot.json` | 当前机器人的可选 DMIMU 和电机实例 |
| `configs/generated/config.hpp`、`robot_config.hpp`、`bsp_bindings.cpp` | 生成输出；禁止手动修改 |

生成器是 `configs/cmake/generate_config.cmake`。它会用 IOC 校验所选硬件名称，并生成 `app::uart::*`、`config::feature::*` 和 `robot::motors::*` 等 C++ 常量。

## 推荐流程

1. 使用 CubeMX 在 `board/board.ioc` 修改 MCU 引脚、DMA 或外设模式。
2. 在 `boards/h723_v1/board.json` 修改板级通用含义。
3. 在 `configs/robot.json` 修改当前机器人的设备实例；在 `configs/params.json` 修改运行参数。
4. 执行 `cmake --preset Debug` 和 `cmake --build --preset Debug`。
5. 生成头文件只用于检查结果，不能修改。

`params.json` 选择启用的遥控器来源及 UART。`robot.json` 根据 `devices.motors.list` 中的型号，选择 CMake 要编译哪些电机协议源码目录。当前生成器不使用独立的电机构建开关；继续维护这类开关会产生重复配置来源。

当前未发现 JSON Schema 或 CI JSON 验证目标。CMake 生成是现有验证点。未来若加入 schema，应以 JSON 文件作为字段的唯一来源，再由它生成参考文档，而不是维护第二张参数表。
