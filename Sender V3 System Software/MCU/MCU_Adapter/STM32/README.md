# STM32 MCU Adapter Scaffold

This folder contains an STM32-oriented split of the firmware stub:

- `Core/Inc/sender_protocol.h` + `Core/Src/sender_protocol.c`
  - Protocol constants and CRC16.
- `Core/Inc/sender_parser.h` + `Core/Src/sender_parser.c`
  - Request frame parsing and response frame building.
- `Core/Inc/sender_engine.h` + `Core/Src/sender_engine.c`
  - Waveform engine abstraction/state (stub behavior).
- `Core/Inc/sender_transport.h` + `Core/Src/sender_transport_stm32_stub.c`
  - Transport contract and stub hooks.
- `Core/Inc/sender_app.h` + `Core/Src/sender_app.c`
  - Dispatcher that ties transport/parser/engine together.

## CubeIDE integration

1. Add all files in `Core/Inc` and `Core/Src` to your STM32CubeIDE project.
2. Replace `sender_transport_stm32_stub.c` with your USB CDC implementation.
3. In `main.c`:

```c
#include "sender_app.h"

int main(void)
{
    // HAL init, clock, peripherals, USB CDC init...
    sender_app_init();

    while (1)
    {
        sender_app_poll();
    }
}
```

## Notes

- The engine module is currently a deterministic stub for protocol bring-up.
- Once transport is live, move DCC timing generation into timer/DMA logic in `sender_engine.c`.
