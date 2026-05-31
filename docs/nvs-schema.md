# NVS schema

All persistent device state lives in a single NVS namespace: **`config`**. Keys are
magic strings referenced from several files (`main.c`, `server_comm.c`), so this is
the canonical inventory. Always access via `nvs_open("config", ...)`.

## `config` namespace

| Key | Type | Written by | Read by / purpose |
|-----|------|-----------|-------------------|
| `ModuleId` | i64 | `SetModuleId` action | Sent as `Id` in device info. |
| `ModuleName` | str | `SetModuleName` action | Sent as `Name`. |
| `ModuleKey` | str | `SetModuleKey` action | Auth/identity, sent as `Key`. |
| `ServerAddress` | str | `SetServerAddress` action | Host for all server requests. |
| `CommInterval` | i32 | `SetCommInterval` action | Polling period (seconds). Default 60. |
| `EnableHttp` | i8 | (provisioning) | Truthy → use plain HTTP instead of HTTPS. |
| `WifiSsid` | str | `SetWifiSsid` action | Wi-Fi STA SSID. |
| `WifiPassword` | str | `SetWifiPassword` action | Wi-Fi STA password (also sent to server). |
| `ProgramName` | str | OTA / `SetProgramName` flow | Currently-assigned program. |
| `_ProgramName` | str | OTA + boot check | Rollback bookkeeping — see [ota.md](ota.md). |
| `ProgramVersion` | str | (program) | Reported as `ProgramVersion`. |

Notes:
- Several reads are guarded — a key is only used if `nvs_get_*` returns `ESP_OK`, so
  missing keys are tolerated (the corresponding device-info field is simply omitted).
- `_ProgramName` (leading underscore) is internal bookkeeping for the OTA
  pending-verify / rollback dance, not a server-facing value.

## Provisioning / seed file

[nvs_data.csv](../nvs_data.csv) is a `nvs_partition_gen.py` seed file. It declares the
`config` namespace and seeds `ServerAddress`, `WifiSsid`, `WifiPassword` (placeholder
values — replace before flashing). Keep its keys aligned with the `config` schema
above if you add more.

To flash NVS data manually:

```bash
python $IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py \
  generate nvs_data.csv nvs.bin 0x20000
esptool.py write_flash 0x9000 nvs.bin   # offset from partitions.csv
```
