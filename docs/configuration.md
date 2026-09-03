# Configuration

配置顺序：通过STM32CubeMX进行必要配置，在工程中选择开发板(目前只有stm32h7)，进行外设等硬件配置，添加工程的设备树

## `board/board.ioc`：工程文件

使用 STM32CubeMX 修改。这里决定 MCU、引脚、时钟、DMA、中断和外设工作模式；CAN 的接收 FIFO0 / FIFO1 也只在这里配置。JSON 不记录 FIFO 选择。

## `boards/h723_v1/`
当前选中的profile 会通过 `boards/h723_v1/board.json` 为工程配置开发板的固定配置，大部分情况下不会修改：
例如 BMI088 使用的 SPI、CS 与 DRDY 引脚。
这些配置对于同一块开发板（mc02或C板）通用，特殊情况下可以配合`.ioc`进行修改：例如需要的引脚太多，需修改常用配置才能满足开发需求。需要保证`.ioc` 与具体配置保持一致。

## `configs/params.json`：绑定外设与构建选择

这里放置和MCU外设相关的配置与构建功能选择：是否构建 USBX、遥控器和裁判系统使用哪一路 UART、遥控器类型、CAN 诊断、服务线程优先级，以及 AHRS、USB 等运行参数。

例如，使用 DR16 时，UART 必须已经在 `board.ioc` 中存在并配置 RX DMA；然后在 `params.json` 选择它：

```json
{
  "bindings": {
    "remoter_uart": "uart5",
    "uart_ports": { "host_link": "uart7" },
    "spi_buses": { "custom_sensor": "spi2" }
  },
  "remoter": { "source": "dr16" }
}
```

`bindings.spi_buses` 为应用自定义 SPI 总线取角色名。上例生成
`app::spi::custom_sensor`；`spi2` 必须已经在 `board.ioc` 中启用，配置生成会检查。
BMI088、LED 等板载设备仍使用 `board::device::<设备>::spi`，不需要重复绑定。

`bindings.uart_ports` 同样为应用串口取角色名，例如上例生成
`app::uart::host_link`。应用代码应使用这个角色名，不直接写具体 UART 实例。

## `configs/robot.json`：机器人设备

这里描述当前机器人连接的"device"层设备：例如每台电机的具体配置，可选 DMIMU 的连接参数。

```json
{
  "devices": {
    "motors": {
      "list": [
        {
          "name": "motor1",
          "model": "dji_gm6020",
          "can_bus": "fdcan2",
          "can_type": "classic",
          "can_id": "0x205"
        }
      ]
    }
  }
}
```

这里的 `can_bus` 和 `can_type` 必须与 `board.ioc` 中的实际 CAN 配置一致。配置阶段会检查不匹配的情况。

更多分组、字段出口和可复制的配置片段见[配置参考](configuration-reference.md)。

# 生成结果和生效步骤

顶层 `CMakeLists.txt` 在 **CMake configure** 阶段运行 `configs/cmake/generate_config.cmake`。它读取：

- 当前板卡 profile 指向的 `board/board.ioc`；
- `boards/h723_v1/board.json`；
- `configs/params.json`；
- `configs/robot.json`。

随后生成这些文件：

| 文件 | 内容 | 能否手改 |
| --- | --- | --- |
| `configs/generated/config.hpp` | 外设枚举、板级绑定、功能开关和 `params::` 常量 | 否 |
| `configs/generated/robot_config.hpp` | 当前电机和 DMIMU 的 C++ 配置常量 | 否 |
| `configs/generated/bsp_bindings.cpp` | 从 IOC 得到的 ADC、PWM 与 HAL 句柄绑定 | 否 |

修改 `params.json`、`robot.json`、`board.json` 或 CMake 配置后需要重新执行：

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

修改 `board.ioc` 时，先在 CubeMX 中生成板级代码，再执行同样的 CMake configure 和 build。只改 `.ioc` 而不重新生成 CubeMX 代码，会让配置描述与实际编译的 HAL 初始化代码不一致。

# 当前实现的注意事项

配置只能生成常量或编译进驱动，具体任务需要在上层逻辑中初始化并调用，例如：

- `devices.motors.list` 中的每台电机都会生成配置并决定要编译哪些电机协议，电机仍然需要进行register等初始化行为

- TODO：`test.thread_priority` 和 `test.auto_run_on_boot` 当前会生成到 `config.hpp`，但仓库内没有运行代码读取它们；修改这两个字段目前不会改变运行行为。

# 快速查找

| 我想修改 | 应该修改 |
| --- | --- |
| MCU 型号、引脚、时钟、DMA、中断或外设模式 | `board/board.ioc`，然后用 CubeMX 生成代码 |
| 板载 BMI088、LED 的固定连接 | 当前板卡 profile 的 `boards/h723_v1/board.json`，并确认与 IOC 一致 |
| CAN 的接收 FIFO0 / FIFO1 | `board/board.ioc`。为每个 FDCAN 只启用一个接收 FIFO，再用 CubeMX 生成代码；不修改 JSON |
| 是否构建 USBX、遥控器类型与 UART、服务运行参数 | `configs/params.json` |
| 电机型号、CAN 总线、CAN ID、初始模式 | `configs/robot.json` 的 `devices.motors` |
| DMIMU 是否存在及其 CAN 连接 | `configs/robot.json` 的 `devices.dmimu` |
| 查看生成后的 C++ 名称和当前结果 | `configs/generated/config.hpp`、`robot_config.hpp`；只读，不修改 |
