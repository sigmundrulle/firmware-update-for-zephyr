# zephyr-fw-update

A Zephyr module for firmware update over SMP (mcumgr) through MCUboot: the new image is received on the
same serial port as the product's own commands, test-run, and then confirmed or reverted. The module knows
no product; a product uses it fully, partly or not at all, through Kconfig options and a small C API.

**Status:** skeleton. The C API and the functions are being designed; nothing is usable yet.

## Using the module in an application

The module is a Zephyr module (`zephyr/module.yml`). Add it to the application's build, either as a
project in the application's `west.yml` or with `ZEPHYR_EXTRA_MODULES`, and enable it:

```
CONFIG_FW_UPDATE=y
```

## Building and testing the module on its own

The repository is its own west workspace, with Zephyr pinned in `west.yml` and fetched into `deps/`:

```bash
tools/setup.sh              # first time only
source tools/env.sh         # every new terminal
west twister -T tests -p native_sim
```

The tests run on `native_sim` (Zephyr built as a Linux program) with the host's gcc, so no board or Zephyr
SDK is needed. `native_sim` is 32-bit like the boards and needs `gcc-multilib` (`sudo apt install
gcc-multilib`).

## Layout

| Path | Content |
|---|---|
| `zephyr/module.yml` | Tells Zephyr where the module's CMake and Kconfig are |
| `Kconfig`, `CMakeLists.txt` | `CONFIG_FW_UPDATE` and the module's sources |
| `include/fw_update/` | The C API |
| `src/` | The implementation |
| `tests/` | Ztest apps, run by Twister on `native_sim` |
| `west.yml`, `tools/` | The workspace for building the module on its own |
