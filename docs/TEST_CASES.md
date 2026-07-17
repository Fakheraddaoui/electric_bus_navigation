# Test Cases — Electric Bus Navigation

Executable sources: `firmware/primary/test/test_cases.c` (TC-1..4) and
`firmware/safety_monitor/test/test_cases.c` (TC-5..7). Both run in CI.
"Actual" values are measured outputs from running the real functions on the host.

| # | Function under test | Input | Expected result | Actual result | Status |
|---|---|---|---|---|---|
| TC-1 | `long_step()` | v_target = v_actual = 10 m/s | accel command **0.0 m/s²** | 0.000 | ✅ PASS |
| TC-2 | `long_step()` jerk limit | 30 m/s error, first 10 ms step, jerk limit 0.8 m/s³ | capped at **0.008 m/s²** (passenger comfort) | 0.008 | ✅ PASS |
| TC-3 | `long_step()` saturation | sustained 30 m/s error, 10 s | saturates at **accel_max = 1.5 m/s²** | 1.500 | ✅ PASS |
| TC-4 | `health_encode()` → `health_decode()` | seq=12345, overruns=2, cpu=65.3 %, accel=−1500 mm/s², 18-byte frame | decode **true**, fields bit-exact | seq=12345, accel=−1500, flags exact | ✅ PASS |
| TC-5 | `health_decode()` CRC check | valid frame with byte 5 XOR 0xA5 | decode returns **false** — monitor never acts on bad data | false | ✅ PASS |
| TC-6 | `sm_tick()` frame-loss reaction | timeout 100 ms; last frame t=0; tick at t=200 ms @12 m/s | **SM_TAKEOVER**, monitor authority, decel **−2.0 m/s²** | TAKEOVER, authority=true, −2.0 | ✅ PASS |
| TC-7 | `sm_on_frame()` + `sm_tick()` fault escalation | primary reports ETHERCAT_DOWN twice, then standstill (0.05 m/s) | **CONTROLLED_STOP** at **−1.2 m/s²** (primary steers) → **SM_SAFE_STOPPED**, latched | CONTROLLED_STOP/−1.2 → SAFE_STOPPED, stays latched | ✅ PASS |

Suite result: **7/7 passed** (part of 21 total firmware tests across both MCUs).
