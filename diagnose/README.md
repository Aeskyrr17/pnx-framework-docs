# Module diagnostics

This directory contains board validation programs rather than application
examples. They expose detailed debug state, pass/fail masks, counters, host
packet protocols, and fault-oriented telemetry.

The normal firmware entry remains `app_start()` in `demo/app.cpp`.
`diagnose/app.cpp` provides a separate `diagnose_start()` entry for a board
bring-up target or a temporary call from the platform startup code.

