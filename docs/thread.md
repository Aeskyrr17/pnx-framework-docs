# 创建应用线程

当前工程会在启动时调用 `app_start()`，你只需在这里创建自己的应用线程。

AHRS、Referee、Remoter 和 USB 在各自的 `init()` 中创建所需后台线程；应用线程只负责初始化它们并使用结果。

## 最小写法

```cpp
#include "tx_api.h"
#include "usertypes.hpp"

#include <cstdint>

namespace robot::application
{
namespace
{

TX_THREAD control_thread{};
alignas(8) std::uint8_t control_stack[2048]{};
bool started = false;

void control_entry(ULONG /* arg */)
{
    // 先初始化本应用需要的模块和设备。

    for (;;)
    {
        // 周期性机器人控制逻辑。
        tx_thread_sleep(2U);
    }
}

types::status start() noexcept
{
    if (started)
    {
        return types::status::ok;
    }

    const UINT status = tx_thread_create(
        &control_thread, const_cast<CHAR*>("control"), control_entry, 0U,
        control_stack, sizeof(control_stack), 6U, 6U, //注意线程优先级不要和modules线程重复
        TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        return types::status::error;
    }

    started = true;
    return types::status::ok;
}

} // namespace
} // namespace robot::application

extern "C" void app_start()
{
    (void)robot::application::start();
}
```

## 使用时记住

栈过小时可能导致 HardFault 或随机异常；出现这类问题时先检查栈大小。也不要为 AHRS、USB、Referee 等已经自行管理线程的模块再创建“驱动线程”。

## 相关内容

- [启动流程](startup.md)
