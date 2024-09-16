#!/usr/bin/env python3

from base import test

expected = [
    '    PID CMD STATUS\n',
    '        cat -1\n',
    '    PID CMD STATUS\n',
    '        cat 0\n',
]

test(expected)
