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
