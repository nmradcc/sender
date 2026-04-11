# STM32 MCU Adapter

This folder contains the STM32H563 Sender MCU project.

- `563/Core/Inc/sender_*.h` + `563/Core/Src/sender_*.c`
  - Protocol, parser, engine, app, and transport files used by the H563 build.
- `563/Core/Src/sender_transport_usbx.c`
  - STM32H563 USBX CDC ACM transport implementation.

## H563 integration

The H563 project in `563/` contains the sender core directly and runs `sender_app_poll()` from the generated USBX device thread.

Key wiring:

1. `563/CMakeLists.txt` builds the local sender sources from `563/Core/Src`.
2. `563/USBX/App/app_usbx_device.c` initializes the sender app and polls it from the USBX thread.
3. `563/Core/Src/sender_transport_usbx.c` handles framed SHP traffic over USB CDC ACM.

## Porting to another STM32 target

1. Reuse the sender files from `563/Core/Inc` and `563/Core/Src`.
2. Provide a target-specific transport implementation alongside your CubeMX project.
3. Call `sender_app_init()` once and then service `sender_app_poll()` from your scheduler loop or RTOS thread.

Example:

```c
#include "sender_app.h"

int main(void)
{
    // HAL init, clock, peripherals, transport init via sender_app_init...
    sender_app_init();

    while (1)
    {
        sender_app_poll();
    }
}
```

## Notes

- The engine module is currently a deterministic stub for protocol bring-up.
- Remaining hardware work is in `sender_engine.c`: move DCC timing generation into timer/DMA logic.
