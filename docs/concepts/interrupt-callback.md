# 通用回调

提供轻量化的通用回调系统 `core::callback`，使用`void*+指针`，不分配内存、不使用mutex。

## Callback
位于 `pnx_libs/common/callback.hpp`
```cpp
template <typename Result, typename... Args>
class callback<Result(Args...)>
```


## 使用约束
使用回调函数必须满足ISR原则
- 不阻塞：不能`tx_thread_sleep`、等mutex、调用阻塞类型的HAL/BSP API
- 不调用`msg::publish(...,{.from_isr = true})`,此实现会调用ThreadX mutex
- this / 回调目标必须在注册期间始终有效，通常应为 static 或长期存在的对象。
- 回调中的逻辑尽量轻量化，不要运行大量计算，实际工作应交给ThreadX线程

## 回调上下文

- `bsp::can::rx_callback` 运行在 FDCAN HAL 中断路径。
- `bsp::usart::rx_callback` 运行在 UART HAL 中断/DMA 回调路径。
- 这两类回调都不能阻塞、获取 ThreadX mutex、调用阻塞 BSP API，或执行耗时的解析/控制逻辑。

CAN 回调注册属于启动/线程上下文工作，应在总线开始接收帧之前完成。注册与注销不会和 FDCAN 中断同步；回调目标是借用的，不由 BSP 持有。

`referee::config::on_update_callback` 与 `remoter::config::on_update_callback` 不同：它们运行在各自服务线程中，而非直接运行在 ISR 内。但它们仍不能阻塞，因为会延迟接收处理。传入回调的引用只在本次调用期间有效。

## DMA 与 STM32H7 Cache

STM32H7 会在外设初始化之前启用 D-Cache。因此 DMA buffer 必须是 `bsp::dma::buffer<N>`，具有 static lifetime，并使用 `BSP_DMA_BUFFER` 放置。板级 profile 会将该段放到 DMA 可访问且按 cache line 对齐的内存。BSP 会验证 buffer，并在支持的 DMA 传输前后执行 cache clean 或 invalidate。

DMA buffer 及其 owner 必须保持有效，直到传输完成。SPI DMA 在 `bsp::spi::state(bus).complete` 为 true 前不能复用 buffer。UART receive-to-idle 的 buffer 归已注册接收方所有，直到停止接收，或用同一个 owner 重新配置。

DMA 模式下的 `bsp::usart::transmit()` 会将数据复制到内部 static staging buffer，因此调用方 buffer 只需在调用期间有效。SPI IT/DMA 不会复制调用方 buffer。

## 消息通道

`msg::channel<T>` 适用于允许丢失中间采样的线程间状态更新。它保存一个最新 payload，并为每个订阅者保存一个待读取标志。它不是命令队列，不应用于高频执行器路径。

不要把 `msg::publish(..., {.from_isr = true})` 当作 ISR 安全队列。实现仍使用 ThreadX mutex，因此 ISR 生产者需要独立的交接机制。
