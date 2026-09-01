#include "remoter_example.hpp"

#include "config.hpp"

namespace demo::remoter
{
namespace
{

::remoter::state latest_state{};

void on_update(const ::remoter::state& data) noexcept
{
    latest_state = data;
}

} // namespace

types::status init() noexcept
{
    ::remoter::config config{};
    config.on_update_callback = ::remoter::update_callback::bind<&on_update>();
    return ::remoter::service::instance().init(config);
}

const ::remoter::state& latest() noexcept
{
    return latest_state;
}

} // namespace demo::remoter
