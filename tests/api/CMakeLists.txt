# Ztest: fuz's C API on native_sim, with FFF fakes of the MCUboot functions it calls (run with
# Twister, see docs/development.md). One file per area of the API, one test suite per file.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(fuz_test_api)

target_sources(app PRIVATE src/fakes.c src/test_api.c src/test_confirm.c)
