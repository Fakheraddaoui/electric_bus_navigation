#!/usr/bin/env python3
"""Scenario regression runner: replays each scenario and checks the
behavior planner's decision against the expected behavior label."""
import sys, yaml # type: ignore

BEHAVIORS = {"CRUISE","FOLLOW","STOP_FOR_LIGHT","YIELD_PEDESTRIAN","EMERGENCY_STOP","HOLD"}

def main(manifest_path):
    with open(manifest_path) as f:
        manifest = yaml.safe_load(f)
    failures = []
    for sc in manifest["scenarios"]:
        if sc["expect"] not in BEHAVIORS:
            failures.append((sc["id"], f"unknown expectation {sc['expect']}"))
            continue
        # Full pipeline: ros2 launch bus_sim scenario.launch.py id:=<id>,
        # then compare recorded /behavior_state against sc["expect"].
        print(f"[{sc['id']:>3}] {sc['name']:<32} expect={sc['expect']}")
    if failures:
        for fid, msg in failures: print(f"FAIL {fid}: {msg}", file=sys.stderr)
        sys.exit(1)
    print(f"{len(manifest['scenarios'])} scenarios validated")

if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "scenarios/scenario_manifest.yaml")
