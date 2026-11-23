# Activate the module's west workspace in this terminal: source tools/env.sh
#
# Uses the host's gcc (ZEPHYR_TOOLCHAIN_VARIANT=host), which is all native_sim needs.

if [ -n "${ZSH_VERSION:-}" ]; then
    FUZ_ROOT="$(cd "$(dirname "${(%):-%x}")/.." && pwd)"
else
    FUZ_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
fi
export FUZ_ROOT

if [ ! -f "$FUZ_ROOT/.venv/bin/activate" ] || [ ! -d "$FUZ_ROOT/deps/zephyr" ]; then
    echo "error: the workspace is not set up. Run tools/setup.sh first." >&2
    return 1 2>/dev/null || exit 1
fi

source "$FUZ_ROOT/.venv/bin/activate"
export ZEPHYR_BASE="$FUZ_ROOT/deps/zephyr"
export ZEPHYR_TOOLCHAIN_VARIANT=host
