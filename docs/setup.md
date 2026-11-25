# Adding the module to a product

The module brings no Zephyr of its own: it is built with the Zephyr the product already uses. The
module's `west.yml`, `tools/` and `deps/` are only for [developing the module](development.md); a product
never runs `tools/setup.sh`.

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

If the build stops with `attempt to assign the value 'y' to the undefined symbol FUZ`, Zephyr did
not find the module: check the path in `ZEPHYR_EXTRA_MODULES`, or that `west update` has fetched it.
