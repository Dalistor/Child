#!/usr/bin/env python3
"""PAR-01: every parameter of the brain (`child params`) appears in
docs/parametros.md, section "Registro", with the same unit, range and default,
and the default lies inside the range."""
import re
import subprocess
import sys
from pathlib import Path


def registry(binary: str) -> dict[str, tuple[str, float, float, float]]:
    out = subprocess.run([binary, "params"], check=True, capture_output=True, text=True).stdout.splitlines()
    rows = {}
    for line in out[1:]:
        name, value, low, high, unit = line.split()[:5]
        rows[name] = (unit, float(value), float(low), float(high))
    return rows


def documented(doc: Path) -> dict[str, tuple[str, float, float, float]]:
    text = doc.read_text(encoding="utf-8")
    section = text.split("## 7. Registro", 1)[1] if "## 7. Registro" in text else ""
    rows = {}
    for m in re.finditer(r"^\| `([a-z_.]+)` \| ([^|]+) \| ([^|]+) \| ([^|]+) \| ([^|]+) \|", section, re.M):
        name, unit, value, low, high = (g.strip() for g in m.groups())
        num = lambda s: float(s.replace(",", ".").replace("−", "-"))
        rows[name] = (unit, num(value), num(low), num(high))
    return rows


def close(a: float, b: float) -> bool:
    return abs(a - b) <= 1e-6 * max(1.0, abs(a), abs(b))


if __name__ == "__main__":
    binary = sys.argv[1] if len(sys.argv) > 1 else "build/child"
    doc = Path(sys.argv[2] if len(sys.argv) > 2 else "docs/parametros.md")
    code, docs = registry(binary), documented(doc)
    problems = []
    for name, (unit, value, low, high) in code.items():
        if not low <= value <= high * (1 + 1e-6):
            problems.append(f"{name}: default {value} outside [{low}, {high}]")
        if name not in docs:
            problems.append(f"{name}: missing from {doc} (Registro)")
            continue
        d = docs[name]
        if d[0] != unit or not all(close(x, y) for x, y in zip(d[1:], (value, low, high))):
            problems.append(f"{name}: code {unit} {value} [{low}, {high}] but doc {d[0]} {d[1]} [{d[2]}, {d[3]}]")
    for name in docs.keys() - code.keys():
        problems.append(f"{name}: documented but not in the code")
    for p in problems:
        print(p)
    print("parameters documented" if not problems else f"{len(problems)} parameter problem(s)")
    sys.exit(1 if problems else 0)
