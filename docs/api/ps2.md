# PS2 手柄

由于手柄有两种模式，目前直接屏蔽一种模式。如果发现遥控器没用请按一下MODE
它与 `remoter` 中旧的串口接收器相互独立；旧接收器现命名为
`ps2_uart`。

先在 CubeMX 中配置引脚。PS2 后端和 GPIO 引脚角色在编译期由
`configs/params.json` 选择。

GPIO bit-bang 配置示例：

```json
{
  "ps2": {
    "enabled": true,
    "backend": "gpio",
    "cmd": "pb5",
    "data": "pb4",
    "clk": "pb3",
    "cs": "pb2"
  }
}
```

硬件 SPI 配置示例：

```json
{
  "ps2": {
    "enabled": true,
    "backend": "spi",
    "spi": "spi3",
    "cs": "pb2"
  }
}
```

使用 SPI 后端时，CubeMX 必须将目标 SPI 配置为主机、双线、Mode 3、
LSB first

`configs/generated/bsp_bindings.hpp` 提供具体的编译期类型别名和
静态实例：

```cpp
#include "bsp_bindings.hpp"

auto& ps2 = devices::ps2::binding::instance();
if (ps2.init() == types::status::ok)
{
    devices::ps2::state state{};
    devices::ps2::raw_frame raw{};
    const types::status result = ps2.poll(state, &raw);
}
```

板级诊断会从 `diagnose_start()` 启动该驱动。可在 Cortex Live Watch 中
添加 `demo_debug_instance.ps2_unit`，观察原始帧、已解析按键、归一化
摇杆、协议测试结果以及硬件轮询阶段。


`devices::ps2::state` 的按钮值为true or false；摇杆值为`[-1.0f, 1.0f]`。


GPIO 后端采用阻塞式 bit-bang，时钟约为 67 kHz，CS 建立时间和保持时间
各为 20 µs。
