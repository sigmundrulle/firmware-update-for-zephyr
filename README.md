# Firmware update for Zephyr (fuz)

**fuz** stands for **F**irmware **U**pdate for **Z**ephyr: the module's name in Zephyr, and the
prefix of its Kconfig options (`CONFIG_FUZ_*`) and C API (`fuz_*`). Not related to fuzz testing.

A Zephyr module for firmware update over SMP (mcumgr) through MCUboot. It knows no product: a product adds
it to its build and enables it with `CONFIG_FUZ=y`.

## Goals

* One firmware update function that several products can use, delivered to customers as a module.
* Product-neutral: nothing in the module depends on a particular product, board or customer. Non-secret
  products may be named as examples in the documentation.
* Easy to add to a product: a git submodule or a west project, and one Kconfig option.
* A product uses it fully, partly or not at all, through Kconfig options and a small C API.

## Status

**Early.** The C API is declared in `include/fuz/fuz.h`, and the Kconfig options for who confirms a
new image and the time limits exist. Implemented: `fuz_init()`, `fuz_is_confirmed()`,
`fuz_confirm()`, `fuz_revert()`, `fuz_get_running_version()` and `fuz_is_secondary_pending()`,
confirming at startup with `CONFIG_FUZ_CONFIRM_AT_BOOT`, and the client cannot confirm an update
before it has run. The other functions return "not implemented", so the module cannot update a
product yet.

## Architecture

```
firmware-update-for-zephyr/
├── zephyr/module.yml    tells Zephyr where the module's CMake and Kconfig are
├── Kconfig              CONFIG_FUZ and the module's options
├── CMakeLists.txt       builds src/ into the application
├── include/fuz/         the C API
├── src/                 the implementation
├── tests/               Ztest apps, run by Twister on native_sim
├── docs/                user/ (for products), contributor/ (for work on the module), decisions
└── west.yml, tools/     workspace for developing the module; not used by a product
```

The module is built with the product's own Zephyr: the product points Zephyr at it, as a git submodule or a
west project, and `zephyr/module.yml` does the rest.

## Documentation

For products (`docs/user/`):

* [Setup](docs/user/setup.md): adding the module to a product
* [SMP buffer size](docs/user/smp-buffer.md): how the buffer size affects upload time, RAM and the
  serial port

For work on the module (`docs/contributor/`):

* [Development](docs/contributor/development.md): building and testing the module on its own
* [Contributing](docs/contributor/contributing.md): branches, pull requests, review, commits and
  tests

For both:

* [Decisions](docs/decisions.md): decisions so far, planned scope and open decisions
