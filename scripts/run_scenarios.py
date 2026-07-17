#!/usr/bin/env python3
"""Scenario regression gate: validates the manifest — every scenario maps to
a known expected behavior. The full closed-loop replay (bus_sim) runs on the
simulation rig; this gate keeps the manifest and expectations consistent."""
import re, sys, pathlib

BEHAVIORS = {"CRUISE", "FOLLOW", "STOP_FOR_LIGHT", "YIELD_PEDESTRIAN",
             "EMERGENCY_STOP", "HOLD"}

def main(manifest_path):
    text = pathlib.Path(manifest_path).read_text()
    rows = re.findall(
        r"\{id:\s*(\d+),\s*name:\s*([\w\-]+),\s*expect:\s*(\w+)\}", text)
    if not rows:
        sys.exit("no scenarios parsed from manifest")
    failures = []
    ids = set()
    for sid, name, expect in rows:
        if expect not in BEHAVIORS:
            failures.append(f"{sid} ({name}): unknown expectation '{expect}'")
        if sid in ids:
            failures.append(f"{sid}: duplicate scenario id")
        ids.add(sid)
        print(f"[{sid:>3}] {name:<32} expect={expect}")
    if failures:
        print("\n".join("FAIL " + f for f in failures), file=sys.stderr)
        sys.exit(1)
    print(f"\n{len(rows)} scenarios validated")

if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "scenarios/scenario_manifest.yaml")
