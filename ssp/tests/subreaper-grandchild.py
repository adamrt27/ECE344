#!/usr/bin/env python3

from base import test

expected = [
    '    PID CMD       STATUS\n',
    '        <unknown> 64\n',
    '    PID CMD       STATUS\n',
    '        <unknown> 64\n',
    '        <unknown> 65\n',
]

test(expected)
