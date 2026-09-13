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

### HC-10 ↔ HC-10 (native AT interface, both are BT05 clones)

One observed case: peripheral set to `AT+TYPE1` with `AT+PIN123456`, central connecting with a mismatched PIN → `Connected` followed by a self-triggered `Disconnected` a moment later. Connecting with the matching PIN → connection holds. On the surface this looks like a real PIN check + some form of bonding between two same-firmware devices, matching what TI's own CC2541 stack docs describe (bond established once, then reused).

**Not confirmed reproducible.** Repeated attempts at the exact same setup (fresh `AT+DEFAULT` + `AT+RENEW`, `TYPE1` re-applied, matching sequence) failed to reproduce the disconnect — connections held regardless of PIN value tried afterwards. Given this firmware's `AT+TYPE` is independently known to be flaky (doesn't always persist), the single disconnect event might be a real mechanism that only fired under some specific, unclear condition, or might be a firmware glitch. Status: open question, not a confirmed finding.

### STM32WB55 (honest BLE stack) ↔ HC-10

This part **is** solid and reproducible across multiple runs:

- Pairing method is Just Works — confirmed by changing `CFG_FIXED_PIN` (111111 → still succeeds, no difference).
- No real bonding on the HC-10 side — reconnecting with the exact same (already-bonded) STM32 address triggers a full pairing exchange again (`GAP PAIRING COMPLETE` + NVM write), instead of skipping straight to encryption from a stored LTK.
- Spoofing STM32's public address to a brand-new, never-seen value still connects and pairs successfully, with any PIN (matching or not) — the module does not gate on identity or on the passkey value.

```
GATT: Start Searching Primary Services
ACI_GATT_PROC_COMPLETE_VSEVT_CODE
NVM_START_WRITE (1 word)
GAP PAIRING COMPLETE, status: 0x0, reason: 0x0
NVM_END_WRITE
NVM_START_WRITE (22 words)
NVM_END_WRITE
```

**Takeaway:** against a real BLE client, this module provides no meaningful pairing security — no MAC-based trust, no PIN enforcement, no working bonding. Whatever the two clones do between themselves over their proprietary AT-level check (see above) doesn't carry over to a standards-compliant BLE central. This is the actual motivation for the AES+DH wrapper project — don't trust the module's own "security" at all, encrypt at the application layer instead.