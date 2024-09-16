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
    'tests/arg0-ownership': 10,
    'tests/dev-null': 10,
    'tests/echo-cat-cat': 5,
    'tests/send-signal': 10,
    'tests/sigpipe': 5,
    'tests/single-cat': 10,
    'tests/single-sleep': 10,
    'tests/ssp-wait': 10,
    'tests/subreaper-child': 10,
    'tests/subreaper-grandchild': 10,
    'tests/true-false': 10,
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
