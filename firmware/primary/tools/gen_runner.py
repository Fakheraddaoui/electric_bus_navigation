#!/usr/bin/env python3
"""Generate a Unity test runner (main) for a test_*.c file.
Scans for `void test_xxx(void)` definitions and emits RUN_TEST calls."""
import re, sys, pathlib

def main(test_file, out_file):
    src = pathlib.Path(test_file).read_text()
    tests = re.findall(r"^\s*void\s+(test_\w+)\s*\(\s*void\s*\)", src, re.M)
    if not tests:
        sys.exit(f"no test functions found in {test_file}")
    with open(out_file, "w") as f:
        f.write('#include "unity.h"\n\n')
        for t in tests:
            f.write(f"void {t}(void);\n")
        f.write("void setUp(void);\nvoid tearDown(void);\n\n")
        f.write("int main(void)\n{\n    UNITY_BEGIN();\n")
        for t in tests:
            f.write(f"    RUN_TEST({t});\n")
        f.write("    return UNITY_END();\n}\n")
    print(f"generated {out_file}: {len(tests)} tests")

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
