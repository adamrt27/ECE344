#!/usr/bin/env python3

from base import test

expected = [
    '    PID CMD   STATUS\n',
    '        true  0\n',
    '        false 1\n',
]

test(expected)
