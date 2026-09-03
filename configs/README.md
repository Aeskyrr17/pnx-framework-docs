# 配置生成

这里保存项目配置与生成器。

- `params.json`：构建开关、应用外设角色和运行参数。
- `robot.json`：当前机器人的电机、DMIMU 等设备。
- `cmake/generate_config.cmake`：在 CMake configure 阶段读取 JSON、板卡 profile 与 `.ioc`，生成 C++ 常量。
- `generated/`：生成结果，不能手改。

使用者应阅读 [配置入口](../docs/configuration.md) 和 [配置参考](../docs/configuration-reference.md)，不要以本 README 当作字段参考。

修改 JSON、板卡 profile 或 `.ioc` 后重新执行 CMake configure；修改 `.ioc` 时还要先在 CubeMX 生成板级代码。
