# zephyr-fw-update

A Zephyr module for firmware update over SMP (mcumgr) through MCUboot. It knows no product: a product adds
it to its build and enables it with `CONFIG_FW_UPDATE=y`.

## Goals

* One firmware update function that several products can use, delivered to customers as a module.
* Product-neutral: nothing in the module depends on a particular product, board or customer. Non-secret
  products may be named as examples in the documentation.
* Easy to add to a product: a git submodule or a west project, and one Kconfig option.
* A product uses it fully, partly or not at all, through Kconfig options and a small C API.

## Status

**Skeleton.** The build integration, a smoke test, CI and the documentation exist. The C API is being
designed, so `include/fw_update/fw_update.h` declares no functions and `src/fw_update.c` only registers the
log module. Nothing is usable by a product yet.

## Architecture

```
zephyr-fw-update/
├── zephyr/module.yml    tells Zephyr where the module's CMake and Kconfig are
├── Kconfig              CONFIG_FW_UPDATE and the module's options
├── CMakeLists.txt       builds src/ into the application
├── include/fw_update/   the C API
├── src/                 the implementation
├── tests/               Ztest apps, run by Twister on native_sim
├── docs/                setup, development, contributing and decisions
└── west.yml, tools/     workspace for developing the module; not used by a product
```

The module is built with the product's own Zephyr: the product points Zephyr at it, as a git submodule or a
west project, and `zephyr/module.yml` does the rest.

## Documentation

* [Setup](docs/setup.md): adding the module to a product
* [Development](docs/development.md): building and testing the module on its own
* [Contributing](docs/contributing.md): branches, pull requests and review
* [Decisions](docs/decisions.md): decisions so far, planned scope and open decisions
