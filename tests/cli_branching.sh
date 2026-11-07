#!/bin/sh
# End-to-end check of branching through the CLI: only the fed branch learns,
# for the brain (the default engine) and for the mlp oracle.
set -eu
bin=${1:-./build/child}
src=${2:-src/model/rng.hpp}
dir=$(mktemp -d)
trap 'rm -rf "$dir"' EXIT

learned() { "$bin" info "$1" | sed -n 's/^learned  \([0-9]*\) bytes$/\1/p'; }

for engine in brain mlp; do
    "$bin" init "$dir/root.child" --name root --engine "$engine" >/dev/null
    "$bin" fork "$dir/root.child" "$dir/a.child" --name a >/dev/null
    "$bin" fork "$dir/root.child" "$dir/b.child" --name b >/dev/null
    cp "$dir/root.child" "$dir/before.child"
    captured=$("$bin" feed "$dir/a.child" "$src" --epochs 2 | sed -n 's/^captured \([0-9]*\) bytes$/\1/p')

    [ "$captured" -gt 0 ]
    [ "$(learned "$dir/a.child")" -eq $((captured * 2)) ]
    [ "$(learned "$dir/root.child")" -eq 0 ]
    [ "$(learned "$dir/b.child")" -eq 0 ]
    cmp -s "$dir/root.child" "$dir/before.child"  # the root is untouched, byte for byte
    "$bin" info "$dir/a.child" | grep -q "^engine   $engine$"
done
echo "cli branching ok"
