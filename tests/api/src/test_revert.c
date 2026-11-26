/**
 * @file Ztest: fuz_revert() in each confirm mode (the variants in testcase.yaml); it is allowed in
 * every mode.
 */
#include <errno.h>
#include <setjmp.h>

#include <zephyr/sys/reboot.h>
#include <zephyr/ztest.h>

#include "fakes.h"
#include "fuz/fuz.h"

ZTEST_SUITE(fuz_revert, NULL, NULL, NULL, NULL, NULL);

ZTEST(fuz_revert, test_restarts_without_confirming)
{
	boot_is_img_confirmed_fake.return_val = false;

	/* The sys_reboot() fake jumps back here, with setjmp() returning 1. */
	if (setjmp(sys_reboot_jump) == 0) {
		(void)fuz_revert();
		zassert_unreachable("fuz_revert() returned instead of restarting");
	}

	zassert_equal(sys_reboot_fake.call_count, 1, "restarted once");
	zassert_equal(sys_reboot_fake.arg0_val, SYS_REBOOT_WARM);
	zassert_equal(boot_write_img_confirmed_fake.call_count, 0, "nothing written");
}

ZTEST(fuz_revert, test_already_confirmed)
{
	boot_is_img_confirmed_fake.return_val = true;

	zassert_equal(fuz_revert(), -EALREADY);
	zassert_equal(sys_reboot_fake.call_count, 0, "no restart");
}
