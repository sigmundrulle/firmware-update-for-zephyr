# Adding the module to a product

The module brings no Zephyr of its own: it is built with the Zephyr the product already uses. The
module's `west.yml`, `tools/` and `deps/` are only for
[developing the module](../contributor/development.md); a product never runs `tools/setup.sh`.

Choose one of the two ways, then enable the module in the product's `prj.conf`:

```
CONFIG_FUZ=y
```

## As a git submodule

```bash
git submodule add <url of this repository> modules/fuz
```

Zephyr only looks for modules among the west projects, so tell it where the submodule is, in the
application's `CMakeLists.txt` **before** `find_package(Zephyr)`:

```cmake
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/../modules/fuz)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
```

The path is relative to the application's folder. Check out the submodule wherever the product is built,
for example `submodules: recursive` in GitHub Actions' checkout.

## As a west project

If the product is a west workspace, add the module to its `west.yml`. Zephyr then finds it through
`zephyr/module.yml`, with nothing in `CMakeLists.txt`:

```yaml
manifest:
  remotes:
    - name: fuz
      url-base: <url of the owner, without the repository name>
  projects:
    - name: firmware-update-for-zephyr
      remote: fuz
      revision: <tag or commit>
      path: modules/fuz
```

Then run `west update`.

## What the product's Zephyr needs

* Zephyr v4.4.2 (the version the module is tested with).
* The modules `mcuboot` and `zcbor` in the product's workspace.
* An application built with MCUboot (`west build --sysbuild`). MCUboot's RAM load mode is not
  supported: it has no test run, confirm or revert.
* `CONFIG_MCUMGR_GRP_IMG_ALLOW_CONFIRM_NON_ACTIVE_SLOT` left at fuz's default, `n`, so that a client
  cannot confirm an update before it has run. The build stops if the product sets it to `y`.

## Log level

fuz logs through Zephyr's logging, as the log module `fuz`. Its level, `CONFIG_FUZ_LOG_LEVEL`, is
the most detailed level built into the image; messages above it are left out and cost no flash.

By default (*Default* in menuconfig), it follows Zephyr's `CONFIG_LOG_DEFAULT_LEVEL`, which is 3
(Info) unless the product sets it: `LOG_ERR`, `LOG_WRN` and `LOG_INF` are built in, `LOG_DBG` is
not. To set fuz alone, choose a level in the product's `prj.conf`, for example:

```
CONFIG_FUZ_LOG_LEVEL_DBG=y
```

The other choices are `_OFF`, `_ERR`, `_WRN` and `_INF`. In menuconfig, the option is *Max
compiled-in log level for fuz* under the module. Kconfig files and menuconfig name the symbols
without the `CONFIG_` prefix (`LOG_DEFAULT_LEVEL`); `prj.conf`, `.config` and C code use it.

## If the build stops

If the build stops with `attempt to assign the value 'y' to the undefined symbol FUZ`, Zephyr did
not find the module: check the path in `ZEPHYR_EXTRA_MODULES`, or that `west update` has fetched it.
