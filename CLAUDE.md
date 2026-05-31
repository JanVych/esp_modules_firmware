# CLAUDE.md

Firmware for managed ESP32 IoT modules, written in C on **ESP-IDF**. Each device
connects to a central server (a Blazor Server app), reports state, and is remotely
managed: the server can push config, swap the running "program", and trigger OTA
updates. Part of a master's thesis.

## Build / flash / monitor

Standard ESP-IDF workflow (target chip: **ESP32**):

```bash
idf.py set-target esp32      # first time only
idf.py build
idf.py -p <PORT> flash
idf.py -p <PORT> monitor     # Ctrl-] to exit
```

- Project name `esp_modules_firmware`, `PROJECT_VER` in [CMakeLists.txt](CMakeLists.txt).
- Custom partition layout in [partitions.csv](partitions.csv) — dual OTA app slots.
- `cJSON` is pulled in as a managed component (see `managed_components/`).

## Architecture

`app_main` ([main/main.c](main/main.c)) initializes NVS + networking, connects
Wi-Fi, registers server actions/messages, starts the program task, then enters the
server communication loop. Functionality is split into components under
[components/](components/):

| Component | Role |
|-----------|------|
| `wifi` | STA connect/disconnect, connection state, current SSID. |
| `http_client` | Thin wrapper over `esp_http_client` + cJSON; URL builder, GET/POST JSON. |
| `server_comm` | The heart: polling loop that POSTs device state to the server and dispatches actions from the response. Also owns built-in actions (OTA, set config). |
| `program` | The swappable user program. Stub with `program_Main()` / `program_OnDestroy()` contract; real behavior comes via OTA-delivered firmware. |
| `ecomax` / `etatherm` / `wattrouter_mx` | Device drivers (UART / RS485 Modbus RTU) a program drives. |
| `helpers` | Small macros, e.g. `ESP_RETURN_ERROR`. |

## Conventions

- **All persistent state lives in the NVS `config` namespace** as magic-string keys.
  Always go through `nvs_open("config", ...)`. See [docs/nvs-schema.md](docs/nvs-schema.md).
- **Server actions/messages are registered by string key** via `comm_AddAction*` /
  `comm_AddMessage*` ([components/server_comm/include/server_comm.h](components/server_comm/include/server_comm.h)).
  Register them in `app_main` before `comm_Start()`.
- A **program** must implement `program_Main(void*)` (a FreeRTOS task body) and
  `program_OnDestroy()`. The lifecycle (run/pause/destroy/restart) is owned by
  `main.c`. See [docs/programs.md](docs/programs.md).
- Private/internal functions are prefixed `_` and marked `static`.
- Errors use `esp_err_t`; bubble up with `ESP_RETURN_ERROR(...)` from `helpers.h`.

## Gotchas

- The NVS `EnableHttp` flag is inverted into the `ssl` argument of
  `http_BuildUrl` — set it to use plain HTTP instead of HTTPS (e.g. local testing).

## Detailed references

@docs/server-protocol.md
@docs/nvs-schema.md
@docs/programs.md
@docs/ota.md
