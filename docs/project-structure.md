# 项目结构

| 目录 | 职责 |
| --- | --- | 
| `board/` | STM32CubeMX/HAL/ThreadX/USBX 生成工程、启动文件、链接脚本和工具链文件。 |
| `boards/h723_v1/` | 硬件板子的.json文件，用于绑定开发板引脚与内存等，一般在上层开发中不需要改动(TODO:stm32f4待补充) |
| `configs/` | JSON 和 CMake 生成器，用于进行机器人的必要配置，详见[[configuration]] | 
| `pnx_bsp/` | 对 HAL 外设的小型封装 |
| `pnx_devices/` | 电机、IMU、LED、UI 等基于 BSP 的具体设备与统一接口。 | 
| `pnx_modules/` | 	AHRS、遥控器、裁判系统等可持续运行的服务线程。 |
| `pnx_libs/` | 	状态码、消息、CRC、控制、滤波、数学和运行时监控等通用能力 | 
| `demo/` | 最小示例 | 
| `diagnose/` | 板级验证和调试状态 | 

 `pnx_bsp`、`pnx_devices`、`pnx_modules` 和 `pnx_libs` 子模块都由顶层 CMake 直接编译。

## 分层关系

实线表示推荐的代码依赖方向：上层可以依赖下层，下层不应依赖上层。虚线表示当前工程中已经存在的共享依赖或实现例外。

```mermaid
flowchart TB
    subgraph Config[配置与生成阶段]
        IOC[board/board.ioc\nCubeMX peripheral]
        BoardProfile[boards/h723_v1/\n板卡 profile 与 board.json]
        RobotConfig[configs/params.json 与 robot.json\n机器人参数与设备实例]
        Generator[configs/cmake/\n配置生成器]
        Generated[configs/generated/\n只读 C++ 绑定]

        IOC --> Generator
        BoardProfile --> Generator
        RobotConfig --> Generator
        Generator --> Generated
    end

    subgraph Runtime[ ]
        App[Application\ndemo/ 或机器人应用]
        Modules[pnx_modules/\nAHRS、remoter\referee...]
        Devices[pnx_devices/\nmotor、IMU、LED、UI]
        BSP[pnx_bsp/\nCAN、UART、SPI、DMA、PWM...]
        Board[board/\nCubeMX、HAL、ThreadX、USBX、IRQ]

        App --> Modules
        Modules --> Devices
        Devices --> BSP
        BSP --> Board
    end

    Generated --> App
    Generated --> Modules
    Generated --> Devices
    Generated --> BSP

    Libs[pnx_libs/\n消息、数学、滤波、运行时间测量]
    App -. 共享工具 .-> Libs
    Modules -. 共享工具 .-> Libs
    Devices -. 共享工具 .-> Libs
    BSP -. 共享工具 .-> Libs

    App -. 当前 demo 可直接使用设备 .-> Devices
    Modules -. 当前 AHRS 直接依赖 BMI088 .-> Devices
```


