# 配置参考

这页只说明 `configs/params.json` 和 `configs/robot.json`。前者选择 MCU 外设的应用角色和运行参数，后者描述这台机器人实际装了哪些设备。修改后执行 CMake configure；生成的 `config.hpp` 和 `robot_config.hpp` 只读，不能手改。完整生效步骤见[配置](configuration.md)。

## `params.json`：外设角色、功能和运行参数

### `build`

| 字段 | 示例 | 作用 |
| --- | --- | --- |
| `build.usbx` | `true` | 板卡同时具备 USB 时，编译 USBX 与 USB CDC BSP。应用仍需调用 `bsp::usb::init()`。 |

```json
{
  "build": { "usbx": true }
}
```

### `bindings`：给应用使用的外设取名字

这些绑定生成 `app::` 下的 C++ 常量。绑定目标必须已在 `board.ioc` 中启用；应用代码使用角色名，不写具体 UART、SPI、ADC 或 CAN 实例。

```json
{
  "bindings": {
    "remoter_uart": "uart5",
    "referee_uart": "usart1",
    "uart_ports": {
      "host_link": "uart7"
    },
    "spi_buses": {
      "custom_sensor": "spi2"
    },
    "adc_channels": {
      "battery_voltage": { "adc": "adc1", "channel": 1 }
    },
    "can_buses": {
      "chassis": {
        "bus": "fdcan2",
        "rx_header": "0x201",
        "tx_header": "0x200"
      },
      "supercap": "fdcan1"
    }
  }
}
```

| 字段 | 生成结果 | 注意 |
| --- | --- | --- |
| `remoter_uart` | `app::uart::dr16`、`app::uart::ps2` | DR16、PS2 使用它。 |
| `referee_uart` | `app::uart::referee` | Referee 服务使用它。 |
| `uart_ports.<角色>` | `app::uart::<角色>` | 自定义应用串口角色。角色必须是合法 C++ 名称，且不能与内置角色重名。 |
| `spi_buses.<角色>` | `app::spi::<角色>` | 自定义 SPI 总线角色。 |
| `adc_channels.<角色>` | `app::adc::<角色>` | 目标必须是 IOC 中的单通道常规 ADC 转换。 |
| `can_buses.<角色>` | `app::can::<角色>` | 可以只写总线字符串，也可写对象并附带可选 `rx_header`、`tx_header`。后两项会生成同名 `_rx_header`、`_tx_header` 常量。 |

VT03 是当前例外：它固定使用 `app::uart::vt03`，即 `uart7`，并要求 `board.ioc` 为 UART7 配置 RX DMA；不使用 `remoter_uart`。

### CAN 与诊断

```json
{
  "can": {
    "fdcan1": { "id_type": "standard" },
    "fdcan2": { "id_type": "extended" }
  },
  "can_diag": {
    "enabled": true,
    "sample_period_ms": 1000,
    "window_size": 60
  }
}
```

- `can.<FDCAN>.id_type` 可为 `standard` / `std` 或 `extended` / `ext`。它决定生成的 CAN 接收过滤器 ID 类型；CAN Classic、FD、以及 FD 是否开启 BRS 都直接由 `board.ioc` 的 `FrameFormat` 决定。
- 接收 FIFO 不在 `params.json`、`robot.json` 或板卡 profile 中配置。每个 FDCAN 的 FIFO0 / FIFO1 只在 `board.ioc` 中设置；当前 BSP 从 CubeMX 生成的 HAL handle 判断实际启用的 FIFO，并要求每个 FDCAN 恰好启用一个接收 FIFO。
- `can_diag.sample_period_ms` 和 `can_diag.window_size` 是 CAN 诊断实现实际使用的参数；`window_size` 必须在 1 到 3600。
- `can_diag.enabled` 会生成 `config::feature::can_diag`。为 `true` 时，首次成功 `bsp::can::init()` 会创建诊断采样定时器，并在 CAN 收发、错误和 FIFO 溢出路径统计指标；为 `false` 时这些路径不会启用诊断统计。

### AHRS、DMIMU、遥控器与 Referee

```json
{
  "ahrs": {
    "imu_offset_x": 0.0,
    "imu_thread_priority": 3,
    "temp_thread_priority": 4,
    "target_temp": 45.0
  },
  "dmimu": {
    "communication_mode": "active",
    "offline_timeout_ticks": 100,
    "thread_priority": 3,
    "receive_wait_ticks": 1,
    "request_period_ticks": 1
  },
  "remoter": {
    "source": "dr16",
    "thread_priority": 2,
    "rx_timeout_ticks": 100,
    "offline_timeout_ticks": 120,
    "ps2_offline_timeout_ticks": 600,
    "ps2_frame_timeout_ticks": 20,
    "ps2_deadzone": 0.08
  },
  "referee": {
    "thread_priority": 8
  }
}
```

| 分组 | 关键字段 | 用处 |
| --- | --- | --- |
| `ahrs` | `imu_offset_x`、两个线程优先级、`target_temp` | 生成 `params::ahrs`，作为 AHRS 默认配置。 |
| `dmimu` | `communication_mode`、离线超时、线程优先级、接收等待、请求周期 | 生成 `params::dmimu`。模式只能是 `active` 或 `request`；超时与接收等待必须大于 0，`request` 模式的请求周期也必须大于 0。 |
| `remoter` | `source`、线程优先级、超时、PS2 死区 | `source` 只能是 `dr16`、`vt03` 或 `ps2`，并决定编译哪一种来源。其余字段生成 `params::remoter`。 |
| `referee` | `thread_priority` | 生成 `params::referee::thread_priority`。 |

### USB 与测试参数

```json
{
  "usb": {
    "read_thread_priority": 5,
    "write_thread_priority": 5,
    "period_ticks": 2
  },
  "test": {
    "thread_priority": 10,
    "report_uart": "uart7",
    "auto_run_on_boot": false
  }
}
```

- `usb` 三项生成 `params::usb`，是 `bsp::usb::config` 的默认收发线程参数。
- `test.report_uart` 会生成 `app::uart::test_report`，且不能与当前启用的遥控器 UART 相同。
- 注意：`test.thread_priority` 和 `test.auto_run_on_boot` 当前只生成常量，仓库内没有运行代码读取它们；修改不会改变运行行为。

## `robot.json`：当前机器人安装的设备

### 配置电机

`devices.motors.list` 中每一项就是一台电机。配置生成器据此选择要编译的协议，并生成 `robot::motors::<name>` 和 `robot::motors::<name>_model`。电机仍需在应用中创建、注册到 `motor_service` 并启动，详见[Motors](api/motors.md)。

```json
{
  "devices": {
    "motors": {
      "dm": {
        "id_base": "0x01",
        "master_id_base": "0x05",
        "max_motors": 4
      },
      "list": [
        {
          "name": "chassis_left",
          "model": "dji_gm6020",
          "can_bus": "fdcan2",
          "can_type": "classic",
          "can_id": "0x205",
          "control_mode": "relax"
        },
        {
          "name": "joint",
          "model": "dm_dm4310",
          "can_bus": "fdcan1",
          "can_type": "fd",
          "can_id": "0x01",
          "control_mode": "mit"
        }
      ]
    }
  }
}
```

| 字段 | 要求 |
| --- | --- |
| `name` | 必填。用于生成 C++ 名称，例如 `chassis_left`；应使用合法 C++ 标识符。 |
| `model` | 当前支持：`dji_m2006`、`dji_m3508`、`dji_gm6020`、`dji_xroll`、`dm_dm4310`、`dm_dm8009p`、`lk_lk8016`、`lk_lk9025`。 |
| `can_bus` | 必填，且必须存在于 `board.ioc`。 |
| `can_type` | 必填，`classic` 或 `fd`；必须与板卡该总线的实际能力一致。 |
| `can_id` | 必填，必须落在该总线配置的标准 / 扩展 ID 范围内。 |
| `control_mode` | 可选，省略时为 `relax`。可写 `relax`、`current`、`torque`、`mit`、`pos_speed` / `position_speed`、`speed` / `velocity`、`multi`。实际型号是否支持该模式，仍应在应用中用 `supports()` 判断。 |
| `motors.dm` | DM 协议的 ID 基值、主机 ID 基值和最大电机数；生成 `robot::motors::dm::*`。当前 DM handler 用它们匹配配置和反馈。 |

### 配置外置 DMIMU

```json
{
  "devices": {
    "dmimu": {
      "enabled": true,
      "can_bus": "fdcan3",
      "can_type": "classic",
      "can_id": "0x04",
      "master_id": "0x04"
    }
  }
}
```

`enabled: true` 才会生成 `HAS_DMIMU` 和 `robot::imu::dmimu`。其余四项在启用时都必填：

- `can_bus` 必须存在于 IOC；
- `can_type` 当前只能是 `classic`，并且必须与板卡总线配置一致；
- `can_id`、`master_id` 都必须在 `0x00` 到 `0xFF`；
- DMIMU 服务的通信模式、超时和线程参数仍写在 `params.json` 的 `dmimu` 分组。

## 修改后如何确认

重新执行：

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

Configure 阶段会检查名称、IOC 外设、CAN 类型和 ID 范围。成功后可只读查看：

- `configs/generated/config.hpp`：`params.json` 的生成结果；
- `configs/generated/robot_config.hpp`：`robot.json` 的生成结果。
