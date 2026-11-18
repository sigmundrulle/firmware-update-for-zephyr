# Activate the module's west workspace in this terminal: source tools/env.sh
#
# Uses the host's gcc (ZEPHYR_TOOLCHAIN_VARIANT=host), which is all native_sim needs.

if [ -n "${ZSH_VERSION:-}" ]; then
    FW_UPDATE_ROOT="$(cd "$(dirname "${(%):-%x}")/.." && pwd)"
else
    FW_UPDATE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
fi
export FW_UPDATE_ROOT

if [ ! -f "$FW_UPDATE_ROOT/.venv/bin/activate" ] || [ ! -d "$FW_UPDATE_ROOT/deps/zephyr" ]; then
    echo "error: the workspace is not set up. Run tools/setup.sh first." >&2
    return 1 2>/dev/null || exit 1
fi

source "$FW_UPDATE_ROOT/.venv/bin/activate"
export ZEPHYR_BASE="$FW_UPDATE_ROOT/deps/zephyr"
export ZEPHYR_TOOLCHAIN_VARIANT=host
