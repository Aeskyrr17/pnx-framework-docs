# USART 诊断

此诊断在 `app::uart::usart1` 上启动 receive-to-idle 接收，主机发送固定 `host_packet`，板端校验后回复 `device_packet`。状态在 `demo_debug_instance.usart`。

启用 `diagnose::usart::start()` 后，用调试器确认 `ready`、`rx_count`、`tx_count` 和 `error_count`；主机脚本位于 `../tools/run_usart_demo.py`。

它只验证这一路固定角色，不是任意 USART 的通用检测。接口说明见 [USART API](../../docs/api/bsp-usart.md)。
