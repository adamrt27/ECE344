import difflib
import subprocess
import sys

def test(expected):
    p = subprocess.run(
        sys.argv[1],
        capture_output=True,
        text=True,
    )

    lines = p.stdout.splitlines(keepends=True)

    checked_first = False
    for i, line in enumerate(lines):
        if i >= len(expected):
            print('Unexpected output (too many lines)', file=sys.stderr)
            exit(1)
        expected_line = expected[i]
        if not expected_line.startswith('    PID'):
            if line[7] != ' ':
                print("Expected a space after pid (character index 7)",
                      file=sys.stderr)
                exit(1)
            pid = line[:7].lstrip()
            if not pid.isdigit():
                print(f"Expected a number for pid, got: '{pid}'",
                      file=sys.stderr)
                exit(1)
            pid = int(pid)
            if pid <= 1:
                print(f"Expected pids above 1, got: '{pid}'", file=sys.stderr)
                exit(1)
            line = line[8:]
            expected_line = expected_line[8:]
        if line != expected_line:
            sys.stderr.writelines(difflib.unified_diff(
                [line], [expected_line],
                fromfile='Your output', tofile='Expected output',
            ))
            exit(1)

    if not 'i' in locals():
        print('Received no output', file=sys.stderr)
        exit(1)

    if i + 1 != len(expected):
        expected_line = expected[i]
        print(f'Missing line: {expected_line}', file=sys.stderr)
        exit(1)

    if p.returncode != 0:
        print('Process did not exit with status 0', file=sys.stderr)
        print(1)
