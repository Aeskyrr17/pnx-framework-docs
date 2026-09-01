#include "imu_demo.hpp"
#include "motor_demo.hpp"
#include "referee_ui_demo.hpp"
#include "remoter_demo.hpp"
#include "usart_demo.hpp"
#include "usb_demo.hpp"

extern "C" void diagnose_start()
{
    diagnose::imu::run();
    diagnose::motor::run();
    // diagnose::remoter::run();
    // diagnose::referee_ui::run();
    // diagnose::usart::start();
    // diagnose::usb::start();
}
