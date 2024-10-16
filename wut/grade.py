#!/usr/bin/env python3

import json
import pathlib
import shutil
import subprocess

wut_dir = pathlib.Path(__file__).resolve().parent
build_dir = wut_dir.joinpath('build')

shutil.rmtree(build_dir, ignore_errors=True)

subprocess.run(
    ['meson', 'setup', 'build'],
    check=True,
    cwd=wut_dir,
    stdout=subprocess.DEVNULL
)

subprocess.run(
    ['meson', 'test', '-C', 'build'],
    cwd=wut_dir,
    stdout=subprocess.DEVNULL
)

grade = 0
try:
    with open(wut_dir / 'test.out', 'r') as f:
        data = f.read()
        if 'Check:' in data:
            print('early-testing,5')
            grade += 5
        else:
            print('early-testing,0')
except:
    print('early-testing,0')

testlog_path = build_dir.joinpath('meson-logs/testlog.json')
if not testlog_path.exists():
    print(f'grade,{grade}')
    # code doesn't compile?
    exit(1)

test_weights = {
    'main-thread-is-0': 4,
    'first-thread-is-1': 4,
    'main-thread-yields': 4,
    'first-thread-exits-explicitly': 4,
    'first-thread-exits-implicitly': 4,
    'first-thread-runs': 4,
    'main-thread-joins': 4,
    'first-thread-cancelled': 4,
    'thread-in-thread': 5,
    'two-threads': 5,
    'reuse-thread-0': 5,
    'error-thread-join-self': 5,
    'error-thread-yield-none': 5,
    'rishabh2050': 5,
    'rosiepie-1': 5,
    'rosiepie-2': 5,
    'lots-of-threads': 5,
    'even-more-threads': 5,
    'fifo-order': 5,
    'student-a': 4,
    'join-cancelled-thread': 4,
}

with open(testlog_path, 'r') as f:
    for line in f:
        test = json.loads(line)
        weight = test_weights[test['name']]
        if test['result'] == 'OK':
            print(test['name'], weight, sep=',')
            grade += weight
        elif test['result'] == 'FAIL':
            if weight == 0:
                grade = 0
                break
            print(test['name'], 0, sep=',')
        else:
            print(test['name'], 0, sep=',')
print(f'grade,{grade}')
