# OTA updates & rollback

OTA is server-triggered and uses ESP-IDF's `esp_https_ota` against the dual app-slot
partition layout in [partitions.csv](../partitions.csv) (`ota_0` + `ota_1`, with
`otadata` tracking the boot slot). The flow is split between
[`_PerformOTA`](../components/server_comm/server_comm.c) (download) and
[`_CheckApp` / `_RunCheck`](../main/main.c) (verify-or-rollback on next boot).

## 1. Download (triggered by `PerformOTA` action)

`_PerformOTA(programName)`:

1. Requires Wi-Fi connected.
2. Builds `<ServerAddress>/api/firmware?program=<programName>`.
3. Runs `esp_https_ota` with the cert bundle (`esp_crt_bundle_attach`).
4. On success:
   - Saves the current `ProgramName` into `_ProgramName` (rollback record).
   - Sets `ProgramName` = new `programName`.
   - Commits NVS and `esp_restart()` into the new image.
5. On failure: logs and returns (stays on the current image).

## 2. First boot of the new image (pending-verify)

`_CheckApp()` runs early in `app_main`. It reads the running partition's OTA state:

- If state is **`ESP_OTA_IMG_PENDING_VERIFY`** (i.e. this is the first boot after an
  OTA), it spawns `_RunCheck` to decide commit vs. rollback.
- Otherwise (normal boot) it restores `ProgramName` from `_ProgramName` as a
  consistency safeguard.

## 3. Verify or roll back (`_RunCheck`)

1. Wait `CommInterval * 2 + 60` seconds — a soak window for the new image to prove it
   is healthy.
2. Evaluate `_CheckAppState()` (currently a stub returning `true` — **this is the
   hook to implement real health checks**, e.g. confirm server check-in succeeded).
3. If healthy:
   - Promote `_ProgramName` → `ProgramName` (confirm the new program name).
   - `esp_ota_mark_app_valid_cancel_rollback()` — make the new image permanent.
4. If unhealthy:
   - `esp_ota_mark_app_invalid_rollback_and_reboot()` — revert to the previous slot.

## Gotchas

- **`_CheckAppState()` is a stub.** Until it does a real check, an OTA'd image is
  always accepted after the soak delay; rollback never actually fires on a bad app.
- The soak window scales with `CommInterval`; a long interval means a long delay
  before an update is committed.
