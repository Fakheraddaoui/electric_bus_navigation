# Rolling A/B OTA — Redundant MCU Pair

Both MCUs use A/B partitions with signed images. The ROS 2 `ota_orchestrator`
enforces this sequence — the bus never has both controllers updating at once:

1. **Stage secondary**: stream signed image into secondary's inactive slot.
2. **Swap + self-test secondary**: reboot into new slot; run built-in self test
   (RAM/flash CRC, EtherCAT link, health-frame loopback). Report to ROS 2.
3. **Gate**: if self-test fails → secondary auto-reverts (3-strike counter),
   update aborted, incident published on `/diagnostics`.
4. **Stage + swap primary** while secondary is in hot-standby with takeover
   authority armed.
5. **Confirm**: primary self-test passes → both slots confirmed; else primary
   reverts and secondary keeps authority until it comes back on the old image.

Version + self-test status of each MCU is published on `/firmware_status` after
every boot; the orchestrator refuses to start unless both report CONFIRMED.
