# 审查记录

本 Review 依据真实源码调用和头文件得出。未经过上板测试的硬件属性不会被写成事实。

## 已确认代码问题 — 已修复

**UART receive-to-idle 启动竞态**

- 位置：`pnx_bsp/usart/src/bsp_usart.cpp`，`bsp::usart::start_rx_to_idle`。
- 修改前行为：DMA 已启动，但接收 buffer 和 callback 尚未写入 `port_state`。
- 问题与风险：快速到达的 IDLE/DMA 事件可能看到空的或旧的接收 owner，导致丢帧或通知错误的 callback。
- 修改：先发布新 owner，再启动 DMA；HAL 调用失败时恢复原状态。

## 重要设计风险

**消息总线的 `from_isr` 并不等于 ISR 安全**

- 位置：`pnx_libs/msg/include/msg.hpp`（`publish_opts::from_isr`）和 `pnx_libs/msg/src/msg.cpp`（`detail::publish`）。
- 当前行为：`from_isr` 只将 mutex 等待参数改为 `TX_NO_WAIT`，但仍会调用 `tx_mutex_get` 和 `tx_mutex_put`。
- 风险：调用者可能将该字段理解为“允许从 ISR publish”。ThreadX mutex 是线程上下文的同步工具，不是 ISR 交接机制。
- 建议：在具备 ISR 安全原语前删除或改名该选项；或者实现专用的无锁/信号量 ISR-to-thread 队列。

**电机掉线检测不会阻止输出**

- 位置：`demo/app.cpp` 调用 `motor_service::alive_check()`；`motor_service` 和协议 handler 仍独立调用 `send_control()`。
- 当前行为：没有反馈时，每个电机的 `state` 会变为 `offline`，但未发现应用层或 service 中的最终命令门控。
- 风险：反馈过期后仍可能继续发送控制帧。这不是完整的执行器安全策略。
- 建议：让应用层拥有明确的最终命令门控：检查遥控器新鲜度、模式、电机健康状态和紧急停止状态；任一失败时发送安全/禁用命令。

## 已补齐的文档缺口

- 新人此前无法从 CubeMX/ThreadX 启动流程追到 `app_start()`。
- DMA、cache、callback 上下文和 buffer lifetime 规则分散在注释中，缺少统一可用的说明。
- 顶层文档没有明确配置的唯一来源和生成文件规则。
- 没有说明可编译的 `demo/` 装配示例是规范示例。

## 需要验证

- **Watchdog：**在已 Review 的 PNX 代码中未发现独立 watchdog 的使用或喂狗策略。需要确认板级启动/option byte 配置和预期复位策略。
- **紧急停止和最终执行器禁用：**部分电机协议存在 disable 函数，但未发现全仓库统一的 emergency-stop 路径或最终命令门控。需要先定义机器人安全策略，再在使能电源后测试。
- **CAN bus-off 恢复：**`bsp::can::handle_error` 在 HAL error callback 中停止并启动 classic CAN。需上板确认这种中断上下文恢复安全，并确认恢复后 filter/notification 仍有效。
- **线程时序：**服务循环使用 `tx_thread_sleep()`。它只是延时，不保证严格控制周期；需在目标负载上测量抖动和优先级关系。
