# Ztest: CONFIG_FUZ_CONFIRM_TIMEOUT_MS on native_sim (run with Twister, see
# docs/contributor/development.md). The limit starts once, at startup, so each case is a build of
# its own (Kconfig, testcase.yaml). The fakes are tests/api's, so that both apps fake the same
# functions.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(fuz_test_confirm_timeout)

target_include_directories(app PRIVATE
    ../api/src
)

target_sources(app PRIVATE
    ../api/src/fakes.c
    src/test_confirm_timeout.c
)
