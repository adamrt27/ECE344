#!/usr/bin/env python3

import subprocess
import sys

strace_args = ['strace', '-e', 'openat,close,dup', sys.argv[1]]
strace = subprocess.run(
    strace_args,
    stdout=subprocess.DEVNULL,
    stderr=subprocess.PIPE,
    text=True
)
strace_lines = strace.stderr.splitlines()

opened_proc = False
open_fds = set()
for line in strace_lines:
    if line.startswith('+++'):
        continue
    if line.startswith('---'):
        continue
    try:
        syscall, ret = line.split(' = ')
    except ValueError:
        # Assume this comes from an error message in the process
        continue
    if syscall.startswith('open'):
        if not ret.isdigit():
            # open *may* return an error in rare cases where the process
            # no longer exists
            continue
        if ret == '5':
            print("close file descriptors when they're no longer needed",
                  file=sys.stderr)
            exit(1)
        if '"/proc"' in syscall or '"/proc/"' in syscall:
            opened_proc = True
        open_fds.add(ret)
    elif syscall.startswith('close'):
        if ret != '0':
            print(f"close returned an error: {ret}", file=sys.stderr)
            exit(1)
        fd = syscall.strip()[6:-1]
        if fd not in open_fds:
            print(f"closed an unknown file descriptor: {fd}", file=sys.stderr)
            exit(1)
        open_fds.remove(fd)
    # perror will dup fd 2 for some reason then close it
    elif syscall.startswith('dup'):
        if not ret.isdigit():
            # dup *may* return an error in rare cases where the process
            # no longer exists
            continue
        open_fds.add(ret)

if len(open_fds) != 0:
    print(f"file descriptors still open: {', '.join(open_fds)}",
          file=sys.stderr)
    exit(1)

if not opened_proc:
    print("No file descriptors opened in /proc", file=sys.stderr)
    exit(1)

print('All file descriptors closed')
