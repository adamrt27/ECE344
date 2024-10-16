#!/usr/bin/env python3

import json
import pathlib
import shutil
import subprocess

tps_dir = pathlib.Path(__file__).resolve().parent
build_dir = tps_dir.joinpath('build')

shutil.rmtree(build_dir, ignore_errors=True)

subprocess.run(
    ['meson', 'setup', 'build'],
    check=True,
    cwd=tps_dir,
    stdout=subprocess.DEVNULL
)

subprocess.run(
    ['meson', 'compile', '-C', 'build'],
    check=True,
    cwd=tps_dir,
    stdout=subprocess.DEVNULL
)

subprocess.run(
    ['build/test/wut'],
    cwd=tps_dir,
    text=True,
)
