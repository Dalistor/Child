#!/usr/bin/env python3
"""REG-01: every non-legacy folder in src/ is named after a brain structure and
has a main file with the same name (src/neuron/neuron.hpp or .cpp)."""
import sys
from pathlib import Path

# Pre-brain folders, kept until their replacements arrive.
LEGACY = {"capture", "model"}


def violations(src: Path) -> list[str]:
    return [
        f"{folder}: missing {folder.name}.hpp or {folder.name}.cpp"
        for folder in sorted(p for p in src.iterdir() if p.is_dir() and p.name not in LEGACY)
        if not any((folder / f"{folder.name}{ext}").is_file() for ext in (".hpp", ".cpp"))
    ]


if __name__ == "__main__":
    src = Path(sys.argv[1] if len(sys.argv) > 1 else "src")
    problems = violations(src)
    for problem in problems:
        print(problem)
    print("brain layout ok" if not problems else f"{len(problems)} folder(s) break REG-01")
    sys.exit(1 if problems else 0)
