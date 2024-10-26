#!/usr/bin/env python3

import argparse
import json
import pathlib
import subprocess

BASE_DIR = pathlib.Path(__file__).resolve().parent
BUILD_DIR = BASE_DIR.joinpath('build')
TESTLOG_PATH = BUILD_DIR.joinpath('meson-logs/testlog.json')
TEST_WEIGHTS = {
    'main-thread-is-0': 4,
    'first-thread-is-1': 4,
    'main-thread-yields': 4,
    'first-thread-exits-explicitly': 4,
    'first-thread-exits-implicitly': 4,
    'first-thread-runs': 5,
    'main-thread-joins': 5,
    'first-thread-cancelled': 5,
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
    'student-a': 5,
    'join-cancelled-thread': 5,
}

def run_tests():
    if BUILD_DIR.exists():
        subprocess.run(
            ['rm', '-rf', 'build'],
            cwd=BASE_DIR,
            stdout=subprocess.DEVNULL
        )
    subprocess.run(
        ['meson', 'setup', 'build'],
        cwd=BASE_DIR,
        stdout=subprocess.DEVNULL
    )
    p = subprocess.run(
        ['meson', 'compile'],
        cwd=BUILD_DIR,
        stdout=subprocess.DEVNULL
    )
    if p.returncode != 0:
        print(json.dumps({'error': 'compile'}, indent=4))
        exit(1)
    subprocess.run(
        ['meson', 'test'],
        cwd=BUILD_DIR,
        stdout=subprocess.DEVNULL
    )

def get_weighted_tests():
    tests = []
    with open(TESTLOG_PATH, 'r') as f:
        for line in f:
            test = json.loads(line)
            weight = TEST_WEIGHTS[test['name']]
            test['weight'] = weight
            test.pop('command', None)
            test.pop('env', None)
            test.pop('starttime', None)
            test.pop('stdout', None)
            stderr = test.pop('stderr', None)
            if stderr:
               lines = stderr.splitlines(True)
               num_lines = len(lines)
               if num_lines > 80:
                  lines = lines[:80]
                  lines.append(f'<{num_lines-80} lines omitted>')
               test['stderr'] = ''.join(lines)
            tests.append(test)
    return tests

def get_grade(tests):
    grade = 0
    for test in tests:
        weight = test['weight']
        if test['result'] == 'OK':
            grade += weight
        else:
            # Other results: 'TIMEOUT' 'INTERRUPT' 'SKIP' 'FAIL' 'EXPECTEDFAIL'
            #                'UNEXPECTEDPASS' 'ERROR'
            if weight == 0:
                grade = 0
                break
    return grade

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--json', action='store_true')
    args = parser.parse_args()

    run_tests()
    tests = get_weighted_tests()
    grade = get_grade(tests)

    if args.json:
        print(json.dumps({'grade': grade, 'tests': tests}, indent=4))
    else:
        print(grade)

if __name__ == '__main__':
    main()
