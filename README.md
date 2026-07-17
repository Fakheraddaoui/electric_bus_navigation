# 🚌 Electric Bus Autonomous Navigation System

Full-stack autonomous navigation for an electric city bus with redundant,
fail-operational safety layers.

| Layer | Tech | Role |
|-------|------|------|
| Perception | ROS 2 (multiple Jetson Orins) | YOLO detection, segmentation, traffic lights |
| Localization | ROS 2 + micro-ROS | RTK-GPS + IMU + LiDAR odometry |
| Planning | ROS 2 + Autoware | Route / behavior / motion planning |
| Vehicle Control | ThreadX + EtherCAT (RZ/N1) | Steering, throttle, braking (1 ms cycles) |
| Safety Monitor | Independent ThreadX MCU | Watchdogs, takeover arbitration, controlled stop |

## Redundancy model
- **Primary MCU** runs the control algorithms.
- **Secondary (safety monitor) MCU** cross-checks the primary over a private
  heartbeat channel and commands a controlled stop / takeover on fault.
- Rolling OTA: secondary updates first, self-tests, then the primary follows;
  either can fall back to its previous A/B slot.

## Layout
```
firmware/primary/         Control MCU (lateral+longitudinal control, EtherCAT)
firmware/safety_monitor/  Independent monitor MCU (health voting, takeover FSM)
ros2_ws/src/
  bus_behavior_planner/   Behavior FSM: traffic lights, pedestrians, cut-ins (GTest)
  bus_diagnostics/        Aggregates /diagnostics from all micro-ROS + ROS 2 nodes
scenarios/                100+ urban scenario definitions for regression CI
```

## Tests
```bash
make -C firmware/primary test
make -C firmware/safety_monitor test
cd ros2_ws && colcon build && colcon test
```

> Firmware unit tests run with a vendored [Unity](https://github.com/ThrowTheSwitch/Unity) harness (`make test`) — no Ruby/Ceedling required. A legacy `project.yml` is kept for teams using Ceedling 0.31.x locally.

See [docs/TEST_CASES.md](docs/TEST_CASES.md) for seven documented test cases with concrete inputs, expected values, and measured results.
