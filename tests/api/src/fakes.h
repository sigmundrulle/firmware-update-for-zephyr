/**
 * @file FFF fakes of the MCUboot and reboot functions fuz calls: native_sim has no MCUboot, and the
 * tests must not restart. They are reset before every test, in every suite, by a ZTEST_RULE in
 * fakes.c, and the calls they got during startup are saved first.
 */
#pragma once

#include <setjmp.h>
#include <stdbool.h>

#include <zephyr/fff.h>

DECLARE_FAKE_VALUE_FUNC(bool, boot_is_img_confirmed);
DECLARE_FAKE_VALUE_FUNC(int, boot_write_img_confirmed);
DECLARE_FAKE_VOID_FUNC(sys_reboot, int);

/*
 * Where the sys_reboot() fake jumps back to. sys_reboot() is FUNC_NORETURN, so the code after a
 * call to it may be left out by the compiler, and the fake must not return. Call
 * setjmp(sys_reboot_jump) in the test before anything that may call sys_reboot().
 */
extern jmp_buf sys_reboot_jump;

/*
 * The calls fuz made to each fake during startup (its own SYS_INIT), saved in fakes.c after every
 * SYS_INIT at the APPLICATION level and before the ZTEST_RULE resets the fakes for the first test.
 * At startup, the fakes return their defaults: false and 0.
 */
extern unsigned int startup_is_confirmed_calls;
extern unsigned int startup_write_calls;
