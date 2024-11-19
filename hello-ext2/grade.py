#!/usr/bin/env python3

from test_lab4 import Lab4TestCase

import os
import pathlib
import unittest

if __name__ == '__main__':
    base_dir = pathlib.Path(__file__).resolve().parent
    os.chdir(base_dir)

    loader = unittest.TestLoader()
    suite = loader.loadTestsFromTestCase(Lab4TestCase)
    with open('/dev/null', 'w') as f:
        runner = unittest.TextTestRunner(stream=f)
        result = runner.run(suite)

    max_grade = 85
    assert(4 * 8 + result.testsRun == max_grade)
    assert(len(result.skipped) == 0)
    assert(len(result.expectedFailures) == 0)
    assert(len(result.unexpectedSuccesses) == 0)

    grade = max_grade
    for test_case, string in result.failures + result.errors:
        method_name = test_case.id().rsplit('.', 1)[-1]
        if method_name.startswith('test_fsck'):
            grade -= 4
        elif method_name == 'test_hello':
            grade -= 4
        elif method_name == 'test_hello_world':
            grade -= 4
        grade -= 1
    grade += 15

    print(grade)
