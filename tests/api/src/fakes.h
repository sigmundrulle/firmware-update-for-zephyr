/**
 * @file FFF fakes of the MCUboot functions fuz calls: native_sim has no MCUboot. They are reset
 * before every test, in every suite, by a ZTEST_RULE in fakes.c, and the calls they got during
 * startup are saved first.
 */
#pragma once

#include <stdbool.h>

#include <zephyr/fff.h>

DECLARE_FAKE_VALUE_FUNC(bool, boot_is_img_confirmed);
DECLARE_FAKE_VALUE_FUNC(int, boot_write_img_confirmed);

/*
 * The calls fuz made to each fake during startup (its own SYS_INIT), saved in fakes.c after every
 * SYS_INIT at the APPLICATION level and before the ZTEST_RULE resets the fakes for the first test.
 * At startup, the fakes return their defaults: false and 0.
 */
extern unsigned int startup_is_confirmed_calls;
extern unsigned int startup_write_calls;
