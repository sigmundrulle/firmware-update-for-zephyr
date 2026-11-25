/**
 * @file FFF fakes of the MCUboot functions fuz calls: native_sim has no MCUboot. They are reset
 * before every test, in every suite, by a ZTEST_RULE in fakes.c.
 */
#pragma once

#include <stdbool.h>

#include <zephyr/fff.h>

DECLARE_FAKE_VALUE_FUNC(bool, boot_is_img_confirmed);
DECLARE_FAKE_VALUE_FUNC(int, boot_write_img_confirmed);
