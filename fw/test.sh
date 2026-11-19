#!/usr/bin/env bash
# Build Blood, run the unit tests, and verify every sample program.
#
# Set BLOOD=/path/to/blood to test an existing binary instead of building.

set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

tmp="$(mktemp -d "${TMPDIR:-/tmp}/blood-fw.XXXXXX")"
cleanup() {
	rm -rf "$tmp"
	rm -rf /tmp/blood-unit-*
	rm -f /tmp/blood_sample_file_io.txt
}
trap cleanup EXIT

if [[ -n "${BLOOD:-}" ]]; then
	blood_bin="$BLOOD"
else
	if ! command -v spectre >/dev/null 2>&1; then
		echo "error: spectre is not installed and BLOOD is not set" >&2
		exit 1
	fi
	echo "==> building blood (dev)"
	spectre build dev
	blood_bin="$root/blood-dev"
fi

if [[ ! -x "$blood_bin" ]]; then
	echo "error: blood binary is not executable: $blood_bin" >&2
	exit 1
fi

if command -v spectre >/dev/null 2>&1; then
	echo "==> running unit tests"
	unit_log="$tmp/unit.log"
	if ! spectre test tests >"$unit_log" 2>&1; then
		cat "$unit_log"
		echo "unit tests failed" >&2
		exit 1
	fi
	if grep -qE 'assertion failed|\[fail\]|modules? failed' "$unit_log"; then
		cat "$unit_log"
		echo "unit tests failed" >&2
		exit 1
	fi
	passes="$(grep -c '^\[pass\]' "$unit_log" || true)"
	printf '    %s unit tests passed\n' "$passes"
else
	echo "==> skipping unit tests (spectre is not installed)"
fi

echo "==> running samples"
failures=0
count=0
shopt -s nullglob

for sample in samples/*.bx; do
	name="$(basename "$sample" .bx)"
	expected="samples/expected/$name.out"
	if [[ ! -f "$expected" ]]; then
		echo "FAIL $name: missing expected output ($expected)"
		failures=$((failures + 1))
		continue
	fi

	sample_args=()
	args_file="samples/args/$name.args"
	if [[ -f "$args_file" ]]; then
		read -r -a sample_args <"$args_file" || true
	fi

	out="$tmp/$name.out"
	err="$tmp/$name.err"
	set +e
	"$blood_bin" "$sample" "${sample_args[@]}" >"$out" 2>"$err"
	status=$?
	set -e
	count=$((count + 1))

	if [[ $status -ne 0 ]]; then
		echo "FAIL $name: exited with status $status"
		sed 's/^/    /' "$err"
		failures=$((failures + 1))
		continue
	fi

	if [[ -s "$err" ]]; then
		echo "FAIL $name: wrote to stderr"
		sed 's/^/    /' "$err"
		failures=$((failures + 1))
		continue
	fi

	if ! diff -u "$expected" "$out" >"$tmp/$name.diff"; then
		echo "FAIL $name: output did not match $expected"
		sed 's/^/    /' "$tmp/$name.diff"
		failures=$((failures + 1))
		continue
	fi

	echo "PASS $name"
done

if [[ $count -eq 0 ]]; then
	echo "error: no samples found" >&2
	exit 1
fi

if [[ $failures -ne 0 ]]; then
	echo "$failures of $count samples failed" >&2
	exit 1
fi

echo "all $count samples passed"
