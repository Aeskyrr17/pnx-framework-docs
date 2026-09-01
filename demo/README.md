# Minimal application examples

This directory is the smallest reference for assembling an upper application.
It intentionally contains no diagnostic verdicts, fault injection, protocol
test packets, or direct peripheral-driver details.

`app.cpp` creates one application-owned ThreadX thread. That thread initializes
the examples and performs the periodic work that belongs to the upper layer:

- publish and subscribe to an application-owned topic;
- subscribe to the AHRS-owned IMU channel;
- register one configured motor with `motor_service` and dispatch control;
- receive referee and remoter updates through service callbacks;
- send a minimal USART message;
- initialize a minimal USB CDC byte-stream echo.

The larger board validation programs and host scripts live in `diagnose/`.

