# IMU 诊断

`imu_demo.cpp` 启动板载 BMI088 的 `ahrs::service`，并在启用 `HAS_DMIMU` 时启动独立的 `ahrs::dmimu_service`。监视线程读取两条 topic，把状态写入 `demo_debug_instance.imu_unit` 和 `dmimu_demo_debug`。

检查重点：`online`、姿态是否更新、四元数是否有效、温控是否就绪，以及 DMIMU 的收帧/掉线计数。

两条服务不会自动切换或仲裁。当前诊断启动顺序中，BMI088 初始化失败会阻止 DMIMU 启动。

使用接口和配置见 [AHRS API](../../docs/api/ahrs.md) 与 [配置](../../docs/configuration.md)。
