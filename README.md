# zephyr-fw-update

A Zephyr module for firmware update over SMP (mcumgr) through MCUboot: the new image is received on the
same serial port as the product's own commands, test-run, and then confirmed or reverted. The module knows
no product; a product uses it fully, partly or not at all, through Kconfig options and a small C API.

**Status:** skeleton. The C API and the functions are being designed; nothing is usable yet.

## Adding the module to a product

The module brings no Zephyr of its own: it is built with the Zephyr the product already uses. The
module's `west.yml`, `tools/` and `deps/` are only for developing the module (see below); a product never
runs `tools/setup.sh`.

Choose one of the two ways, then enable the module in the product's `prj.conf`:

```
CONFIG_FW_UPDATE=y
```

### As a git submodule

```bash
git submodule add <url of this repository> modules/fw_update
```

Zephyr only looks for modules among the west projects, so tell it where the submodule is, in the
application's `CMakeLists.txt` **before** `find_package(Zephyr)`:

```cmake
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/../modules/fw_update)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
```

The path is relative to the application's folder. Check out the submodule wherever the product is built,
for example `submodules: recursive` in GitHub Actions' checkout.

### As a west project

If the product is a west workspace, add the module to its `west.yml`. Zephyr then finds it through
`zephyr/module.yml`, with nothing in `CMakeLists.txt`:

```yaml
manifest:
  remotes:
    - name: fw-update
      url-base: <url of the owner, without the repository name>
  projects:
    - name: zephyr-fw-update
      remote: fw-update
      revision: <tag or commit>
      path: modules/fw_update
```

Then run `west update`.

### What the product's Zephyr needs

* Zephyr v4.4.2 (the version the module is tested with).
* The modules `mcuboot` and `zcbor` in the product's workspace.

If the build stops with `attempt to assign the value 'y' to the undefined symbol FW_UPDATE`, Zephyr did not
find the module: check the path in `ZEPHYR_EXTRA_MODULES`, or that `west update` has fetched it.

## Developing the module

The repository is its own west workspace, so the module can be built and tested without a product. Zephyr
is pinned in `west.yml` and fetched into `deps/`, which is not committed:

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
| `west.yml`, `tools/` | The workspace for developing the module; not used by a product |
