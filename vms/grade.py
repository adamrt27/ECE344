#!/usr/bin/env python3

import json
import pathlib
import shutil
import subprocess

vms_dir = pathlib.Path(__file__).resolve().parent
build_dir = vms_dir.joinpath('build')

shutil.rmtree(build_dir, ignore_errors=True)

subprocess.run(
    ['meson', 'setup', 'build'],
    check=True,
    cwd=vms_dir,
    stdout=subprocess.DEVNULL
)

subprocess.run(
    ['meson', 'test', '-C', 'build'],
    cwd=vms_dir,
    stdout=subprocess.DEVNULL
)


testlog_path = build_dir.joinpath('meson-logs/testlog.json')
if not testlog_path.exists():
    print(f'grade,{grade}')
    # code doesn't compile?
    exit(1)

test_weights = {
    'copy-1': 8,
    'copy-2': 7,
    'copy-3': 7,
    'copy-4': 7,
    'copy-5': 7,
    'cow-1': 8,
    'cow-2': 7,
    'cow-3': 7,
    'cow-4': 7,
    'cow-5': 7,
    'cow-6': 7,
    'cow-7': 7,
    'cow-8': 7,
    'cow-9': 7,
}

grade = 0
with open(testlog_path, 'r') as f:
    for line in f:
        test = json.loads(line)
        weight = test_weights[test['name']]
        if test['result'] == 'OK':
            print(test['name'], weight, sep=',')
            grade += weight
        else:
            print(test['name'], 0, sep=',')
print(f'grade,{grade}')
