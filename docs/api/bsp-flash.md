# bsp::flash

`bsp::flash` 擦除或写入 MCU 内部 Flash。

## 注意

只在已经为持久化数据预留独立 Flash 区域时使用，例如保存校准值。它不会验证地址是否安全，也没有读取接口。

## Header / Namespace

- Header：`bsp_flash.hpp`
- Namespace：`bsp::flash`

## 初始化

不需要初始化。写入前先擦除目标所在扇区，并确认该区域未与固件、配置或其他数据重叠。

```cpp
// 使用片段：reserved_flash_addr 必须来自项目的链接布局，不可随意填写。
#include "bsp_flash.hpp"

#include <array>
#include <cstdint>

alignas(bsp::flash::flash_word_size)
const std::array<std::uint8_t, bsp::flash::flash_word_size> calibration_data{};

types::status save_calibration(std::uint32_t reserved_flash_addr) noexcept
{
    types::status status = bsp::flash::erase_sector(reserved_flash_addr);
    if (status != types::status::ok)
    {
        return status;
    }
    return bsp::flash::write_flash_word(reserved_flash_addr, calibration_data.data());
}
```

## 核心类型

| 类型 | 用途 |
| --- | --- |
| `flash_word_size` | 一次写入的数据大小，当前为 32 字节。 |

## 常用接口

| 接口 | 用途与注意事项 |
| --- | --- |
| `erase_sector(addr)` | 擦除 `addr` 所在的一个扇区。当前实现按 STM32H7 的固定 128 KiB 扇区布局计算。 |
| `write_flash_word(addr, data)` | 写入一个 32 字节 Flash word；地址必须按 32 字节对齐，`data` 不能为 null。 |

## 常见错误

- 把固件镜像所在的扇区传给 `erase_sector()`：会删除程序内容。
- 未擦除就写，或数据不足 32 字节。
- 将该接口当成跨板卡通用存储：当前实现明确使用 STM32H7 的地址和扇区布局。

## 相关内容

- [配置](../configuration.md)
