#!/usr/bin/env python3

import itertools
import subprocess
import shlex
import sys

ps_args = ['ps', '-eo', 'pid:5,ucmd']
ps = subprocess.run(ps_args, capture_output=True, text=True)
ps_lines = ps.stdout.splitlines()

tps_args = [sys.argv[1]]
tps = subprocess.run(tps_args, capture_output=True, text=True)
tps_lines = tps.stdout.splitlines()

for ps_line, tps_line in itertools.zip_longest(ps_lines, tps_lines):

    if ps_line is None:
        print("Expected: no output", file=sys.stderr)
        print(f"     Got: '{tps_line}'", file=sys.stderr)
        exit(1)

    if tps_line is None:
        print(f"Expected:{ps_line}", file=sys.stderr)
        print("     Got: no output", file=sys.stderr)
        exit(1)

    if ps_line == tps_line:
        continue

    ps_pid, ps_name = ps_line.split(maxsplit=1)
    tps_pid, tps_name = tps_line.split(maxsplit=1)

    if ps_name == 'ps' and tps_name == 'tps':
        continue
    
    if ps_pid == tps_pid \
       and ps_name.startswith('kworker/') and tps_name.startswith('kworker/'):
        continue

    print(f"Expected: '{ps_line}'", file=sys.stderr)
    print(f"     Got: '{tps_line}'", file=sys.stderr)
    exit(1)

ps_cmd = shlex.join(ps_args)
tps_cmd = shlex.join(tps_args)
print(f"All lines match between '{ps_cmd}' and '{tps_cmd}'")
