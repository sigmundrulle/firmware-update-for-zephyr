# Decisions

What is decided about the module, what is planned and what is still open. Update this file in the same pull
request as the change that makes or changes a decision.

## Decisions so far

| Decision | Choice |
|---|---|
| Where the module lives | Its own repository from the start, not inside a product, so it can be delivered on its own |
| Name | Repository `firmware-update-for-zephyr`; module `fuz` (Firmware Update for Zephyr), the prefix of its Kconfig options (`CONFIG_FUZ_*`), headers (`include/fuz/`), C API (`fuz_*`) and log module. Short, since it is typed in every symbol; the README says what it stands for, so it is not read as fuzz testing |
| How a product gets it | A git submodule (with `ZEPHYR_EXTRA_MODULES`) or a west project; both are documented in [setup](setup.md) |
| Zephyr | The module brings no Zephyr; it is built with the product's. Development uses Zephyr v4.4.2 |
| Tests | Ztest on `native_sim` (32-bit, like the boards) with the host's gcc, run by Twister |
| Language of the tracked files in the repository | English, since the module goes to other customers |
| Visibility and review | Public repository; `main` only through approved pull requests |

## Planned scope (agreed, not implemented)

These responsibilities are planned for the module. The product keeps its own drivers and application logic
and talks to the module through the C API.

| In the module | In the product |
|---|---|
| The SMP transport on the product's serial port, shared with the product's text commands | Its serial driver separates SMP frames from text and passes the frames to the module |
| Writing SMP responses through a write function the product gives at startup | Writing the responses on its serial port |
| A reset hook with a handshake: "restart requested", then "app ready", with a time limit | An orderly shutdown before the restart, then "ready" |
| Checking that a received image is for the right board (board target in a TLV), erasing it if not | – |
| Kconfig defaults for mcumgr (progressive erase, hash, size checks) | – |
| Confirm or reset based on the self-test result the product reports; rejecting confirm while the self-test runs | The self-test, and reporting its result |
| Reading MCUboot's state: running version, confirmed or not, the image in the second slot | Showing it, if the product wants to |

MCUboot runs in **swap using offset** on products where the image fits in the internal flash. The new image
is written one sector into the second slot; after a swap, the previous image is at the start of that slot,
so whatever reads the header there must handle both places. RAM-load products are not part of the first
version.

## Open decisions

- The C API: functions and types (the prefix is `fuz_`).
- Code style and `.clang-format`: Zephyr's style or another one.
- The license (there is none yet).
- Versions and tags: when a product-facing version is tagged.
- The word *test* (MCUboot's and mcumgr's) or *trial* for the state between swap and confirm.
- The size of the buffer for SMP responses, and that mcumgr finds the previous image at the start of the
  second slot after a swap.
