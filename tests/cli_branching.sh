#!/bin/sh
# End-to-end check of branching through the CLI: only the fed branch learns.
set -eu
bin=${1:-./build/child}
src=${2:-src/rng.hpp}
dir=$(mktemp -d)
trap 'rm -rf "$dir"' EXIT

learned() { "$bin" info "$1" | sed -n 's/^learned  \([0-9]*\) bytes$/\1/p'; }

"$bin" init "$dir/root.child" --name root >/dev/null
"$bin" fork "$dir/root.child" "$dir/a.child" --name a >/dev/null
"$bin" fork "$dir/root.child" "$dir/b.child" --name b >/dev/null
captured=$("$bin" feed "$dir/a.child" "$src" --epochs 2 | sed -n 's/^captured \([0-9]*\) bytes$/\1/p')

[ "$captured" -gt 0 ]
[ "$(learned "$dir/a.child")" -eq $((captured * 2)) ]
[ "$(learned "$dir/root.child")" -eq 0 ]
[ "$(learned "$dir/b.child")" -eq 0 ]
echo "cli branching ok"
