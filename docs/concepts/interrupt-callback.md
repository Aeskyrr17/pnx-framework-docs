# 回调

回调（Callback）常用来处理“什么时候发生由下层决定，但发生之后怎么处理由上层决定”的情况。

例如 CAN 收到数据时，BSP 负责发现“收到了一帧数据”，但具体怎么处理这帧数据应该由上层决定。上层可以提前注册一个回调函数，收到数据后由 BSP 调用。

这样可以避免把具体业务逻辑写进 BSP、Device 或 Module 中。

PnX 中很多接口都使用这种方式，例如 CAN 接收、USART 接收、遥控器更新、Host Protocol 收到请求等。

## `core::callback`

PnX 提供了 `core::callback` 作为通用回调类型。

它可以绑定普通函数：

```cpp
void on_command(std::uint8_t* command) noexcept
{
    // 处理 command
}

const auto callback =
    core::callback<void(std::uint8_t*)>::bind<&on_command>();
```

也可以绑定对象的成员函数：

```cpp
struct command_receiver
{
    void on_command(std::uint8_t* command) noexcept
    {
        // 处理 command
    }
};


static command_receiver receiver{};

const auto callback =
    core::callback<void(std::uint8_t*)>::bind<
        command_receiver,
        &command_receiver::on_command>(&receiver);
```

之后将这个 callback 传给需要它的接口即可。

例如：

```
host_protocol::usb::config cfg{};

cfg.on_request =
    host_protocol::usb::request_callback::bind<&on_usb_request>();

host_protocol::usb::service::instance().init(cfg);
```

使用时需要注意

`core::callback` 不拥有它绑定的对象。

如果绑定的是成员函数，对象必须在 `callback` 仍可能被调用时保持有效。因此长期注册的 `callback` 通常会绑定到静态对象或其他生命周期足够长的对象。

**因此具体 `callback` 用法应以对应 API 页面为准。**

通常情况下，`callback` 应尽快完成必要的处理并返回。