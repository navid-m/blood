#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="${1:-${BLOOD_OUT:-${ROOT}/blood}}"

case "$(uname -s)" in
Darwin)
	GENERATED="generated/blood_darwin.c"
	;;
Linux)
	GENERATED="generated/blood_linux.c"
	;;
*)
	echo "error: unsupported host platform: $(uname -s)" >&2
	exit 1
	;;
esac

if ! command -v cc >/dev/null 2>&1; then
	echo "error: cc is required to build Blood" >&2
	exit 1
fi

if [ ! -f "${ROOT}/${GENERATED}" ]; then
	echo "error: ${GENERATED} not found under ${ROOT}" >&2
	exit 1
fi

mkdir -p "$(dirname "$OUT")"

echo "Compiling ${ROOT}/${GENERATED}..."
cc -std=gnu17 -O2 -w -Wno-everything -Wno-error=int-conversion \
	-o "$OUT" "${ROOT}/${GENERATED}" -lpthread -lm

"$OUT" -v
echo "Blood installed at ${OUT}"
