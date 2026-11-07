#!/bin/sh
# ABR-03: runs the README's end-to-end block, unchanged, in a clean copy of
# the working tree (the `git clone` line is replaced by that copy).
set -eu
root=${1:-.}
dir=$(mktemp -d)
trap 'rm -rf "$dir"' EXIT
mkdir "$dir/Child"
(cd "$root" && git ls-files -co --exclude-standard | grep -v '^\.centaur/' | tar -cf - -T -) | (cd "$dir/Child" && tar -xf -)
block=$(awk '/^```bash$/ { on = 1; next } /^```$/ { if (on) exit } on' "$root/README.md" | grep -v '^git clone')
cd "$dir/Child"
printf '%s\n' "$block" | sh -eu > "$dir/out.txt" 2>&1 || { cat "$dir/out.txt"; exit 1; }
grep -q '^name     meu$' "$dir/out.txt"
grep -q '^name     root$' "$dir/out.txt"
grep -q '^learned  0 bytes$' "$dir/out.txt"
echo "readme example ok"
