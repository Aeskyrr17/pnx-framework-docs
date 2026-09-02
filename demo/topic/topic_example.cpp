#include "topic_example.hpp"

#include "msg.hpp"

namespace demo::topic
{
namespace
{

msg::channel<sample> output{};
msg::subscriber input{};

} // namespace

types::status init() noexcept
{
    types::status status = msg::init();
    if (status != types::status::ok)
    {
        return status;
    }

    status = msg::init(output);
    if (status != types::status::ok)
    {
        return status;
    }

    input = msg::subscribe(output);
    return input.valid() ? types::status::ok : types::status::error;
}

types::status publish(const sample& data) noexcept
{
    return msg::publish(output, data);
}

bool read(sample& data) noexcept
{
    return msg::read(input, data) == types::status::ok;
}

} // namespace demo::topic
