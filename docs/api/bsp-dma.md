# bsp::dma

`bsp::dma` 提供适合 DMA 外设读写的缓冲区。

## 注意

只有 ADC、SPI、USART 的 DMA 接口需要它；CAN、GPIO 和阻塞式接口不需要 DMA 缓冲区。

DMA 外设会直接读写 RAM，不经过 CPU。当前 H723 启用了数据缓存：如果缓冲区位置不能被 DMA 访问，或地址没有按缓存行对齐，DMA 读到的数据可能不对。

`bsp::dma::buffer` 把这些要求集中起来：它会按缓存行对齐和补齐容量；配合 `BSP_DMA_BUFFER`，缓冲区会放进当前板卡的 DMA 专用内存段。调用者只需定义一次长期存在的 `buffer`，再把 `.view()` 交给 DMA 接口。

## Header / Namespace

- Header：`bsp_dma.hpp`；使用 `BSP_DMA_BUFFER` 时还要 include `memory.h`
- Namespace：`bsp::dma`

## 初始化

不需要初始化。创建缓冲区后，把 `.view()` 传给 DMA 接口。

```cpp
// 使用片段
#include "bsp_dma.hpp"
#include "memory.h"

namespace robot::application
{
namespace
{

// BSP_DMA_BUFFER 是变量属性：把 rx_buffer 放到 DMA 可访问的内存中。
bsp::dma::buffer<64U> rx_buffer BSP_DMA_BUFFER{};

} // namespace
} // namespace robot::application
```

## 核心类型

| 类型 | 用途 |
| --- | --- |
| `buffer<N>` | 拥有 `N` 个有效字节的 DMA 缓冲区；实际占用会按缓存行补齐。 |
| `buffer_view` | `buffer` 交给 DMA 接口时使用的视图，通过 `.view()` 获得。 |

## 常用接口

| 接口 | 用途与注意事项 |
| --- | --- |
| `buffer<N>::data()` | 访问实际数据。 |
| `buffer<N>::view()` | 传给 `*_dma()` 或 `start_rx_to_idle()`。 |
| `valid(view)` | 检查缓冲区是否对当前板卡的 DMA 合法；一般不需要手动调用。 |

## 常见错误

- 把栈上的 `buffer` 交给 DMA：传输结束前函数已返回，缓冲区会失效。
- 漏写 `BSP_DMA_BUFFER`：当前板卡启用了专用 DMA 段，缓冲区可能不在 DMA 可访问区域。
- 用普通数组替代 `bsp::dma::buffer`：它没有所需的对齐和容量信息。

## 相关内容
