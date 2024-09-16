#!/usr/bin/env python3

from base import test

expected = [
    '    PID CMD  STATUS\n',
    '        echo 0\n',
    '        cat  0\n',
    '        cat  0\n',
]

test(expected)
