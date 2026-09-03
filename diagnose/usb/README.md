# USB 诊断

此诊断初始化 USB CDC，按字节流拼接固定 `host_packet`，校验后用 `try_send()` 回复 `device_packet`。状态在 `demo_debug_instance.usb`。

启用 `diagnose::usb::start()` 后，确认电脑已枚举 CDC 端口，再查看 `connected`、`rx_count`、`tx_count` 和 `error_count`。主机脚本位于 `../tools/run_usb_demo.py`。

它验证 CDC 收发和单槽发送结果，不是应用协议模板。接口说明见 [USB CDC API](../../docs/api/bsp-usb.md)。
