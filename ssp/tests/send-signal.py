#!/usr/bin/env python3

from base import test

expected = [
    '    PID CMD   STATUS\n',
    '        sleep -1\n',
    '    PID CMD   STATUS\n',
    '        sleep 137\n',
]

test(expected)
