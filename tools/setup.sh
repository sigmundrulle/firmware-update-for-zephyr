#!/usr/bin/env bash
#
# Set up the west workspace for building and testing the module on its own, inside the repository:
#
#   .venv/          Python environment with west and Zephyr's Python tools.
#   .west/config    Makes the repository the west workspace, with west.yml as its manifest.
#   deps/           The Zephyr release and modules that west.yml pins.
#
# The tests run on native_sim with the host's gcc, so no Zephyr SDK is needed.
#
# Running the script again is safe: it brings the workspace up to date with west.yml.
#
# Usage (from anywhere):
#   tools/setup.sh
# Then, in every new terminal:
#   source tools/env.sh
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

missing=()
for tool in git cmake python3 gcc; do
    command -v "$tool" &> /dev/null || missing+=("$tool")
done
if ! python3 -c "import venv, ensurepip" &> /dev/null; then
    missing+=("python3-venv")
fi
if [ ${#missing[@]} -ne 0 ]; then
    echo "error: missing ${missing[*]}." >&2
    exit 1
fi

if [ ! -x .venv/bin/python3 ]; then
    echo "==== Creating the Python environment in .venv"
    python3 -m venv .venv
fi
.venv/bin/pip install --quiet --upgrade pip
.venv/bin/pip install --quiet west ninja

# `west init -l` would put the workspace in the parent directory instead, so the configuration is
# written directly.
if [ ! -f .west/config ]; then
    echo "==== Creating the west workspace"
    mkdir -p .west
    printf '[manifest]\npath = .\nfile = west.yml\n' > .west/config
fi
.venv/bin/west config zephyr.base deps/zephyr

echo "==== Fetching Zephyr and its modules (the first time takes a few minutes)"
.venv/bin/west update --narrow -o=--depth=1

echo "==== Installing Zephyr's Python requirements"
ZEPHYR_BASE="$ROOT/deps/zephyr" .venv/bin/west packages pip --install

echo
echo "Done. In every new terminal: source tools/env.sh"
