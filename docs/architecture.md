# Architecture — Electric Bus Navigation

```
 Jetson Orin ×N (ROS 2 + Autoware)
   perception (YOLO/seg/lights) → localization (RTK+IMU+LiDAR) →
   route/behavior/motion planning → target_motion
                     │ micro-ROS
 ┌───────────────────┴────────────────────┐   health frames (10 ms, CRC'd)
 │ PRIMARY MCU (RZ/N1, ThreadX)           │◄──────────────┐
 │  long_control (jerk-limited) + lateral │               │
 │  EtherCAT master: steer/brake/PT @1ms  │   ┌───────────┴───────────┐
 └────────────────────────────────────────┘   │ SAFETY MONITOR MCU    │
                                              │ takeover_fsm:         │
   actuators ◄── EtherCAT ◄── authority mux ◄─┤ NORMAL→DEGRADED→STOP  │
                                              │ →TAKEOVER→SAFE_STOPPED│
                                              └───────────────────────┘
```

## Fault handling matrix
| Observation (monitor) | Reaction |
|---|---|
| Health frames stop / seq frozen | TAKEOVER: monitor drives −2 m/s² controlled stop |
| Task overruns / EtherCAT down reported | CONTROLLED_STOP via primary (−1.2 m/s²) |
| CPU overload | DEGRADED: tightened limits, alert on /diagnostics |
| Standstill reached | SAFE_STOPPED (latched, maintenance reset required) |

## OTA (rolling)
Secondary first → self-test gate → primary, each on A/B slots with 3-strike
auto-rollback (`firmware/bootloader/ab_slot.c`). Versions and self-test results
published to ROS 2 after every boot.
