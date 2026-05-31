# Programs

A "program" is the swappable application logic running on a module (e.g. read an
eCoMAX boiler over UART, drive a WATTrouter over Modbus, poll an ETAtherm). The
server decides which program a module runs and delivers it via OTA
(see [ota.md](ota.md)); the firmware core just provides a lifecycle around it.

## The contract

Every program provides two symbols (declared in
[components/program/include/program.h](../components/program/include/program.h)):

```c
void program_Main(void *pvParameters);   // FreeRTOS task body — the program's entry point
void program_OnDestroy();                 // cleanup hook, called before the task is deleted
```

The default [components/program/program.c](../components/program/program.c) is an
empty stub. A real program implements these, typically using one of the device
driver components.

## Lifecycle (owned by `main.c`)

`main.c` runs the program as a FreeRTOS task named `program` (stack 20480) and
exposes control via server actions:

| Function | Server action | Behavior |
|----------|---------------|----------|
| `_ProgramRun` | `ProgramRun` | Create the task (first call) or `vTaskResume` it. |
| `_ProgramPause` | `ProgramPause` | `vTaskSuspend` the task. |
| `_ProgramDestroy` | — | Call `program_OnDestroy()`, then `vTaskDelete`. |
| `_ProgramRestart` | `ProgramRestart` | Destroy then run. |
| `_GetProgramStatus` | `ProgramStatus` (msg) | `2` running, `3` paused/stopped. |

The program is started automatically at boot (`_ProgramRun()` in `app_main`).

## Device driver components

These are libraries a program drives — not programs themselves. All speak to
external hardware over UART / RS485:

| Component | Device | Notes |
|-----------|--------|-------|
| `ecomax` | Plum eCoMAX boiler controller | PyPlumIO-style protocol; reads temps & O₂. `ecomax_GetData(ecomax_data_t*)`. |
| `etatherm` | ETAtherm heating regulator | Custom framed protocol with ADD+XOR CRC; read/set real/desired/OZ temps. |
| `wattrouter_mx` | SOLAR controls WATTrouter MX | Modbus RTU ([rs485_modbus_rtu.c](../components/wattrouter_mx/rs485_modbus_rtu.c)); power/energy reads, TUV/boiler state, feeding power. |

Each exposes `*_Init(uart_port, tx, rx)` / `*_Deinit(...)` plus typed getters/setters
returning `esp_err_t` (or a driver-specific error enum, e.g. `eta_err_t`).

## Adding a new program

1. Implement `program_Main` and `program_OnDestroy` (replace the stub in `program.c`, or add
   a new source the `program` component builds).
2. Use a device driver component as needed; add it to `REQUIRES` in
   [components/program/CMakeLists.txt](../components/program/CMakeLists.txt).
3. Surface program-specific data/controls through the server by registering
   `comm_AddMessage*` / `comm_AddAction*` (see [server-protocol.md](server-protocol.md)).
4. The server identifies the build by `ProgramName`; keep it consistent with what
   the OTA endpoint serves.
