# Server communication protocol

The device talks to the server over HTTP(S) with JSON bodies, driven by a polling
loop in [`_MainLoop`](../components/server_comm/server_comm.c). One request/response
per cycle; the cycle repeats every `CommInterval` seconds (default 60).

## The cycle

1. Wait until Wi-Fi STA is connected.
2. Build a JSON object containing **device info** + all registered **messages**.
3. `POST` it to `<ServerAddress>/api/modules`.
4. If the response is `200`, treat the response JSON as a map of **actions** and
   dispatch each recognized key to its callback.
5. Discard the JSON, sleep `CommInterval` seconds, repeat.

HTTP vs HTTPS is chosen by the NVS `EnableHttp` flag (see
[nvs-schema.md](nvs-schema.md)). URLs are assembled by
`http_BuildUrl(ssl, host, port, path, query, ...)`.

## Outbound: POST body to `/api/modules`

Device info added by `_GetDeviceInfo` (only present if the NVS key exists):

| JSON field | Source | Type |
|------------|--------|------|
| `Id` | NVS `ModuleId` | number (i64) |
| `Name` | NVS `ModuleName` | string |
| `Key` | NVS `ModuleKey` | string |
| `ProgramName` | NVS `ProgramName` | string |
| `ProgramVersion` | NVS `ProgramVersion` | string |
| `WifiPassword` | NVS `WifiPassword` | string |
| `Chip` | runtime (`esp_chip_info`) | string |
| `FirmwareVersion` | app descriptor | string |
| `IDFVersion` | app descriptor | string |
| `FreeHeap` | `esp_get_free_heap_size()` | number |
| `CommInterval` | runtime | number |
| `WifiSsid` | `wifiGetSTASsid()` | string |

Plus any fields contributed by registered **messages** (`comm_AddMessage*`) and
one-shot values pushed with `comm_PushMessage(key, value)`.

## Inbound: action dispatch from the response

The response JSON is treated as `{ "<ActionKey>": <value>, ... }`. For each key the
matching registered action callback is invoked; the value is type-checked against
the registration type (`Str`/`Int32`/`Float`/`Bool`/`Json`/`Void`). Unknown keys are
logged and ignored.

### Built-in actions (registered in `server_comm`)

| Key | Arg | Effect |
|-----|-----|--------|
| `PerformOTA` | string (program name) | Download + flash firmware, see [ota.md](ota.md). |
| `SetModuleId` | string (parsed to i64) | Persist `ModuleId`. |
| `SetModuleName` | string | Persist `ModuleName`. |
| `SetModuleKey` | string | Persist `ModuleKey`. |
| `SetServerAddress` | string | Persist `ServerAddress`, apply immediately. |
| `SetCommInterval` | int32 | Persist `CommInterval`, apply immediately. |

### Built-in actions (registered in `main.c`)

| Key | Arg | Effect |
|-----|-----|--------|
| `ProgramRun` | void | Start/resume the program task. |
| `ProgramPause` | void | Suspend the program task. |
| `ProgramRestart` | void | Destroy + re-create the program task. |
| `SetWifiSsid` | string | Persist `WifiSsid`. |
| `SetWifiPassword` | string | Persist `WifiPassword`. |

### Built-in messages

| Key | Type | Meaning |
|-----|------|---------|
| `ProgramStatus` | int32 | `2` = running, `3` = not running. |

## Registering new actions/messages

In `app_main` (or component init), before `comm_Start()`:

```c
comm_AddActionStr("SetThing", _SetThing);     // void _SetThing(char*)
comm_AddActionVoid("DoThing",  _DoThing);      // void _DoThing(void)
comm_AddMessageI32("Sensor",   _ReadSensor);   // int32_t _ReadSensor(void)
```

Actions are inbound (server → device); messages are outbound (device → server).
Keys must be unique per list.
