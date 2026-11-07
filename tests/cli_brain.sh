#!/bin/sh
# The brain through the CLI: every command, the parameter file (CFG-01) and
# the environment variables that steer an engine and its growth (NEO-02).
set -eu
bin=${1:-./build/child}
dir=$(mktemp -d)
trap 'rm -rf "$dir"' EXIT
fail() { echo "cli brain: $*" >&2; exit 1; }
neurons() { "$bin" info "$1" | sed -n 's/^cortex   \([0-9]*\) pyramidal.*/\1/p'; }
value() { "$bin" params "$1" | awk -v n="$2" '$1 == n { print $2 }'; }

# A small, quickly growing brain, from a parameter file.
cat > "$dir/small.params" <<'P'
# test brain
cortex.excitatory = 32
cortex.interneurons = 8
thalamus.channels = 64
neurogenesis.window = 64
neurogenesis.batch = 8
neurogenesis.max_neurons = 64
P
awk 'BEGIN { srand(3); for (i = 0; i < 600; i++) printf "%c", 33 + int(rand() * 90) }' > "$dir/hard.txt"

# init: the brain is the default engine; the file sets its shape (CFG-01).
"$bin" init "$dir/root.child" --params "$dir/small.params" --seed 5 | grep -q '^engine   brain$' || fail "init is not a brain"
[ "$(neurons "$dir/root.child")" -eq 32 ] || fail "parameter file not applied"
CHILD_PARAMS="$dir/small.params" "$bin" init "$dir/env.child" --seed 5 >/dev/null
"$bin" params "$dir/root.child" > "$dir/a.txt"
"$bin" params "$dir/env.child" > "$dir/b.txt"
cmp -s "$dir/a.txt" "$dir/b.txt" || fail "CHILD_PARAMS differs from --params"
[ "$(value "$dir/root.child" neuron.capacitance)" = 281 ] || fail "missing value did not keep its default"
printf 'neuron.capacitance = 1000\n' > "$dir/bad.params"
if "$bin" init "$dir/bad.child" --params "$dir/bad.params" 2> "$dir/err.txt"; then fail "out-of-range value accepted"; fi
grep -q 'neuron.capacitance' "$dir/err.txt" || fail "error does not name the parameter"
CHILD_ENGINE=mlp "$bin" init "$dir/mlp.child" | grep -q '^engine   mlp$' || fail "CHILD_ENGINE=mlp ignored"

# fork inherits the parameters; feed grows when needed, unless told not to.
"$bin" fork "$dir/root.child" "$dir/off.child" --name off >/dev/null
"$bin" fork "$dir/root.child" "$dir/cap.child" --name cap >/dev/null
"$bin" fork "$dir/root.child" "$dir/grow.child" --name grow >/dev/null
"$bin" params "$dir/grow.child" > "$dir/c.txt"
cmp -s "$dir/a.txt" "$dir/c.txt" || fail "fork did not inherit the parameters"
CHILD_NEUROGENESIS=off "$bin" feed "$dir/off.child" "$dir/hard.txt" --epochs 2 >/dev/null
CHILD_MAX_NEURONS=40 "$bin" feed "$dir/cap.child" "$dir/hard.txt" --epochs 2 >/dev/null
"$bin" feed "$dir/grow.child" "$dir/hard.txt" --epochs 2 --sleep 1 >/dev/null
[ "$(neurons "$dir/off.child")" -eq 32 ] || fail "CHILD_NEUROGENESIS=off still grew"
[ "$(neurons "$dir/cap.child")" -eq 40 ] || fail "CHILD_MAX_NEURONS=40 not respected: $(neurons "$dir/cap.child")"
[ "$(neurons "$dir/grow.child")" -eq 64 ] || fail "no growth up to neurogenesis.max_neurons: $(neurons "$dir/grow.child")"
[ "$(neurons "$dir/root.child")" -eq 32 ] || fail "feeding a fork changed the root"
if CHILD_NEUROGENESIS=maybe "$bin" feed "$dir/off.child" "$dir/hard.txt" >/dev/null 2>&1; then fail "bad CHILD_NEUROGENESIS accepted"; fi

# sleep, grow, sample, bench.
"$bin" sleep "$dir/off.child" --cycles 2 | grep -q '^slept 2 cycle' || fail "sleep"
"$bin" grow "$dir/off.child" --neurons 8 | grep -q 'now 40' || fail "grow"
CHILD_MAX_NEURONS=40 "$bin" grow "$dir/off.child" --neurons 8 | grep -q 'grew 0' || fail "grow past the ceiling"
[ "$("$bin" sample "$dir/grow.child" --prompt ab --len 20 | wc -c)" -ge 22 ] || fail "sample"
"$bin" bench --bytes 50 --neurons 32 | grep -q '^neuron-steps/s' || fail "bench"
echo "cli brain ok"
