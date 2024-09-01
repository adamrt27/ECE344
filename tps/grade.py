#!/usr/bin/env python3

import json
import pathlib
import subprocess

tps_dir = pathlib.Path(__file__).resolve().parent
build_dir = tps_dir.joinpath('build')

if not build_dir.exists():
    subprocess.run(
        ['meson', 'setup', 'build'],
        cwd=tps_dir,
        stdout=subprocess.DEVNULL
    )
else:
    subprocess.run(
        ['meson', '--reconfigure', 'build'],
        cwd=tps_dir,
        stdout=subprocess.DEVNULL
    )

subprocess.run(
    ['meson', 'test', '-C', 'build'],
    cwd=tps_dir,
    stdout=subprocess.DEVNULL
)

testlog_path = build_dir.joinpath('meson-logs/testlog.json')

test_weights = {
    'tests/close_check.py': 20,
    'tests/execve_check.py': 0,
    'tests/ps_compare.py': 80,
}

grade = 0
with open(testlog_path, 'r') as f:
    for line in f:
        test = json.loads(line)
        weight = test_weights[test['name']]
        if test['result'] == 'OK':
            grade += weight
        elif test['result'] == 'FAIL':
            if weight == 0:
                grade = 0
                break
print(grade)
