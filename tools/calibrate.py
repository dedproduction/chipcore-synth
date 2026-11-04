#!/usr/bin/env python3
"""Loudness-matches the factory bank.

Builds the headless renderer, renders every preset, and nudges each preset's
master gain so its peak lands near TARGET. Results go to tools/trims.json,
which gen_presets.py folds into Source/Presets.h.
"""
import json
import math
import os
import subprocess
import sys

TARGET_PEAK = 0.62
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TRIMS = os.path.join(ROOT, "tools", "trims.json")
BIN = os.path.join(ROOT, "build-test", "render_test")
LEVELS = os.path.join(ROOT, "build-test", "levels.tsv")


def run(cmd):
    subprocess.run(cmd, check=True, cwd=ROOT)


def main(iterations=3):
    trims = {}
    if os.path.exists(TRIMS):
        with open(TRIMS) as f:
            trims = json.load(f)
    os.makedirs(os.path.dirname(BIN), exist_ok=True)
    for it in range(iterations):
        run([sys.executable, "tools/gen_presets.py"])
        run(["g++", "-std=c++17", "-O2", "-o", BIN, "tests/render_test.cpp"])
        subprocess.run([BIN, "-", LEVELS], cwd=ROOT, stdout=subprocess.DEVNULL)
        worst = 0.0
        with open(LEVELS) as f:
            for line in f:
                name, peak, _rms = line.rstrip("\n").split("\t")
                peak = max(float(peak), 1e-4)
                delta = 20.0 * math.log10(TARGET_PEAK / peak)
                delta = max(-12.0, min(12.0, delta))
                trims[name] = round(trims.get(name, 0.0) + delta, 2)
                worst = max(worst, abs(delta))
        with open(TRIMS, "w") as f:
            json.dump(trims, f, indent=1, sort_keys=True)
        print("iteration %d: largest correction %.2f dB" % (it + 1, worst))
    run([sys.executable, "tools/gen_presets.py"])


if __name__ == "__main__":
    main()
