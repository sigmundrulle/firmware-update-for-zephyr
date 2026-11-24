# Decisions

What is decided about the module, what is planned and what is still open. Update this file in the same pull
request as the change that makes or changes a decision.

## Decisions so far

| Decision | Choice |
|---|---|
| Where the module lives | Its own repository from the start, not inside a product, so it can be delivered on its own |
| Name | Repository `firmware-update-for-zephyr`; module `fuz` (Firmware Update for Zephyr), the prefix of its Kconfig options (`CONFIG_FUZ_*`), headers (`include/fuz/`), C API (`fuz_*`) and log module. Short, since it is typed in every symbol; the README says what it stands for, so it is not read as fuzz testing |
| C API | `include/fuz/fuz.h`, in C with `extern "C"` guards: setup and transport (`fuz_init()` with the product's write function, `fuz_receive_smp_frame()`), the restart handshake, confirm and revert, and MCUboot's state (confirmed, versions as Zephyr's `struct mcuboot_img_sem_ver`, a pending swap). Functions return 0 or a negative errno; `bool` for answers that cannot fail, `int` with 1, 0 or a negative errno for reads that can |
| How a product gets it | A git submodule (with `ZEPHYR_EXTRA_MODULES`) or a west project; both are documented in [setup](setup.md) |
| Zephyr | The module brings no Zephyr; it is built with the product's. Development uses Zephyr v4.4.2 |
| Tests | Ztest on `native_sim` (32-bit, like the boards) with the host's gcc, run by Twister |
| Language of the tracked files in the repository | English, since the module goes to other customers |
| Visibility and review | Public repository; `main` only through approved pull requests |
| Who confirms a new image | A Kconfig choice (#10): `CONFIG_FUZ_CONFIRM_BY_PRODUCT` (default: the product with `fuz_confirm()` after its self-test; the client's image state writes are rejected until then), `CONFIG_FUZ_CONFIRM_BY_CLIENT` (SMP image confirm; `fuz_confirm()` returns `-EPERM`), `CONFIG_FUZ_CONFIRM_BY_ANY` (whichever comes first) or `CONFIG_FUZ_CONFIRM_AT_BOOT` (fuz confirms at startup, so an update is never reverted). The product can reject an image that is not confirmed yet with `fuz_revert()`, in every mode; with `CONFIG_FUZ_CONFIRM_AT_BOOT` the image is confirmed at startup, so `fuz_revert()` returns `-EALREADY`. The client never confirms slot 1 without running it first. Both the product and the client: not yet, it needs fuz to remember who has confirmed |
| Time limits | `CONFIG_FUZ_CONFIRM_TIMEOUT_MS` (default 0, no limit): an image not confirmed in time is not kept, fuz restarts and MCUboot reverts; not with `CONFIG_FUZ_CONFIRM_AT_BOOT`. `CONFIG_FUZ_RESET_READY_TIMEOUT_MS` (default 5000): the product's time to call `fuz_set_ready_for_restart()` after an SMP os reset, otherwise the client gets busy and there is no restart |

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
| Confirm or revert as chosen in Kconfig (see *Who confirms a new image*): after the self-test result the product reports, by the client, or at startup; rejecting the client's confirm while the product's self-test runs; restarting without confirm after a time limit | The self-test, and reporting its result |
| Reading MCUboot's state: running version, confirmed or not, the image in the second slot | Showing it, if the product wants to |

MCUboot runs in **swap using offset** on products where the image fits in the internal flash. The new image
is written one sector into the second slot; after a swap, the previous image is at the start of that slot,
so whatever reads the header there must handle both places. RAM-load products are not part of the first
version.

## Open decisions

- Code style and `.clang-format`: Zephyr's style or another one.
- The license (there is none yet).
- Versions and tags: when a product-facing version is tagged.
- The word *test* (MCUboot's and mcumgr's) or *trial* for the state between swap and confirm.
- The size of the buffer for SMP responses, and that mcumgr finds the previous image at the start of the
  second slot after a swap.
