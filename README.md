# ble-repairing-audit

Checking whether cheap BLE modules (found: BT05/AT-09/MLT-BT05 clone on CC2541, sold as "HC-10") do real BLE Security Manager pairing, or just a homebrew PIN check on top of an open link.

## Folders

- `arduino-uno-at-rtx` — UART↔USB bridge on Arduino Uno, just for sending AT commands to HC-10/BT05 (config only, not the test subject).
- `esp8266-at-rtx` — same bridge, on ESP8266.
- `esp-like-uart-usb` — alt version of the ESP8266 bridge.
- `stm32wb55-spoof-logger` — the actual test rig. STM32WB55 (Nucleo) as a real BLE client (honest HCI/GAP/GATT stack, not an AT wrapper). Connects to the target BT05 by known MAC (scanning is disabled, see below), triggers pairing, logs all GAP/GATT events over UART (PB6/PB7 → ST-LINK VCP, 115200 8N1).

## Known issue

Stock ST `HCI_LE_ADVERTISING_REPORT_SUBEVT_CODE` parsing only reads report `[0]`, ignoring `Num_HCI_Advertising_Reports > 1` — garbage/shifted addresses when multiple devices are around. Workaround: scanning disabled, connect directly to a known MAC (`APP_BLE_Init_3` in `app_ble.c`).

## Results

**Test 1 — explicit `aci_gap_send_pairing_req` right after connect:**
fails.
```
GAP PAIRING COMPLETE, status: 0x2, reason: 0xc   (Numeric Comparison Failed)
```

**Test 2 — no explicit pairing call, let GATT's security requirement trigger it naturally:**
succeeds, real bonding (LTK written to NVM).
```
GATT: Start Searching Primary Services
ACI_GATT_PROC_COMPLETE_VSEVT_CODE
NVM_START_WRITE (1 word)
GAP PAIRING COMPLETE, status: 0x0, reason: 0x0
NVM_END_WRITE
NVM_START_WRITE (22 words)
NVM_END_WRITE
```

**Takeaway:** BT05 does real BLE SM pairing — not a homebrew PIN check like assumed earlier. But *when/who* triggers pairing matters: forcing it early fails, letting the stack request it naturally (via GATT security) succeeds and bonds properly.

**Test 3 — Fixed PIN = 111111:** TBD.