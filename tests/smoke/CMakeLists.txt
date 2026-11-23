# Ztest: the module is found and built into an application (run with Twister, see README.md).

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(fuz_test_smoke)

target_sources(app PRIVATE src/main.c)
