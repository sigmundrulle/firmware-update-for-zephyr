# Developing the module

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

## Formatting

The C code follows Zephyr's style (`.clang-format`). The scripts install clang-format at the version in
`tools/clang-format/version.txt` into `.venv`, so everyone formats the same way:

```bash
tools/clang-format/fix.sh      # formats every C file git knows about
tools/clang-format/check.sh    # only checks, as CI does
```
