/**
 * @file FFF fakes of the MCUboot functions fuz calls, defined once for every test file, and the
 * rule that resets them before every test.
 */
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "fakes.h"

DEFINE_FFF_GLOBALS;

DEFINE_FAKE_VALUE_FUNC(bool, boot_is_img_confirmed);
DEFINE_FAKE_VALUE_FUNC(int, boot_write_img_confirmed);

static void fakes_reset_before(const struct ztest_unit_test *test, void *fixture)
{
	ARG_UNUSED(test);
	ARG_UNUSED(fixture);

	RESET_FAKE(boot_is_img_confirmed);
	RESET_FAKE(boot_write_img_confirmed);
	FFF_RESET_HISTORY();
}

/* Before every test in every suite, so that no test file can forget it. */
ZTEST_RULE(fakes_reset, fakes_reset_before, NULL);
