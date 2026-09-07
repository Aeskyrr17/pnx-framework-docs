# PS2 diagnose

`ps2_demo.cpp` first runs the transport-independent PS2 protocol check, then
starts the board-bound `remoter::ps2` source. Add `demo_debug_instance.ps2_unit` to
Cortex Live Watch to inspect the latest raw frame, analog controller ID,
buttons, normalized axes, test stages and error counters.

The test passes only after an analog `0x53` or `0x73` frame is received. A
digital `0x41` reply is intentionally rejected by the analog-only driver.

## CubeMX pin configuration

For the GPIO backend, configure the pins selected by `params.json.ps2` as:

| Signal | CubeMX mode | Pull | Initial level |
| --- | --- | --- | --- |
| `CMD` | GPIO Output Push-Pull | No pull | High |
| `DATA` | GPIO Input | No pull | N/A |
| `CLK` | GPIO Output Push-Pull | No pull | High |
| `CS` / `ATT` | GPIO Output Push-Pull | No pull | High (inactive) |

For the SPI backend, configure the selected peripheral as **Master, 2-line,
Mode 3, LSB first**. `CMD` maps to MOSI, `DATA` maps to MISO, `CLK` maps to
SCK, and `CS` / `ATT` remains a software-controlled GPIO output.
