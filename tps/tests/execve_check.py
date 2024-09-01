#!/usr/bin/env python3

import subprocess
import sys

strace_args = ['strace', '-e', 'execve', sys.argv[1]]
strace = subprocess.run(
    strace_args,
    stdout=subprocess.DEVNULL,
    stderr=subprocess.PIPE,
    text=True
)
strace_lines = strace.stderr.splitlines()

has_execve = False
for line in strace_lines:
    if line.startswith('+++'):
        continue
    if line.startswith('---'):
        continue
    if not has_execve:
        has_execve = True
        continue
    try:
        syscall, ret = line.split(' = ')
    except ValueError:
        # Assume this comes from an error message in the process
        continue
    print('Illegal syscall:', syscall.strip(), file=sys.stderr)
    exit(1)

print('execve Not Used')
