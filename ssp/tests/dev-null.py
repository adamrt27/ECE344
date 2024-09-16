#!/usr/bin/env python3

from base import test

expected = [
    '    PID CMD STATUS\n',
    '        ls  0\n',
    '        ls  0\n',
    '        ls  0\n',
    '        ls  0\n',
]

test(expected)
