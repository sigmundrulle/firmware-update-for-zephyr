/**
 * @file Ztest: the CONFIG_FUZ_CONFIRM_TIMEOUT_MS time limit, one case per build (CONFIG_TEST_CASE_*
 * in Kconfig, set in testcase.yaml). native_sim's time is simulated in tests, so the sleeps take no
 * real time.
 */
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>
#include <zephyr/ztest.h>

#include "fakes.h"
#include "fuz/fuz.h"

#define LIMIT_MS CONFIG_FUZ_CONFIRM_TIMEOUT_MS

BUILD_ASSERT(IS_ENABLED(CONFIG_TEST_CASE_NO_LIMIT) == (LIMIT_MS == 0),
	     "CONFIG_TEST_CASE_NO_LIMIT goes with CONFIG_FUZ_CONFIRM_TIMEOUT_MS=0 only");

/* Long enough after the limit for the work queue to have run. */
#define PAST_THE_LIMIT K_MSEC(LIMIT_MS + 1000)

#if defined(CONFIG_TEST_CASE_CONFIRMED_AT_STARTUP)

static int confirmed_at_startup(void)
{
	boot_is_img_confirmed_fake.return_val = true;

	return 0;
}

/* Before fuz's own SYS_INIT: the same level, an earlier priority. */
BUILD_ASSERT(CONFIG_APPLICATION_INIT_PRIORITY > 0,
	     "confirmed_at_startup() must run before fuz_start_confirm_timeout()");
SYS_INIT(confirmed_at_startup, APPLICATION, 0);

#endif /* CONFIG_TEST_CASE_CONFIRMED_AT_STARTUP */

/*
 * The sys_reboot() fake's body here. fuz restarts from the system work queue, not from the test's
 * thread, so the longjmp() back to the test cannot be used: the work queue's thread stops instead,
 * and never returns, like sys_reboot().
 */
static void sys_reboot_stops_the_thread(int type)
{
	ARG_UNUSED(type);

	k_sleep(K_FOREVER);
}

/* After the ZTEST_RULE in fakes.c, which resets the fakes. */
static void before(void *fixture)
{
	ARG_UNUSED(fixture);

	sys_reboot_fake.custom_fake = sys_reboot_stops_the_thread;
}

ZTEST_SUITE(fuz_confirm_timeout, NULL, NULL, before, NULL, NULL);

ZTEST(fuz_confirm_timeout, test_restarts_when_the_limit_passes)
{
	Z_TEST_SKIP_IFNDEF(CONFIG_TEST_CASE_EXPIRES);

	k_sleep(K_MSEC(LIMIT_MS / 2));
	zassert_equal(sys_reboot_fake.call_count, 0, "not before the limit");

	k_sleep(PAST_THE_LIMIT);
	zassert_equal(sys_reboot_fake.call_count, 1, "restarted once");
	zassert_equal(sys_reboot_fake.arg0_val, SYS_REBOOT_WARM);
	zassert_equal(boot_write_img_confirmed_fake.call_count, 0, "not confirmed");
}

ZTEST(fuz_confirm_timeout, test_product_confirms_in_time)
{
	Z_TEST_SKIP_IFNDEF(CONFIG_TEST_CASE_PRODUCT_CONFIRMS);

	k_sleep(K_MSEC(LIMIT_MS / 2));
	zassert_equal(fuz_confirm(), 0);

	/* The fake still answers "not confirmed": only stopping the limit prevents the restart. */
	k_sleep(PAST_THE_LIMIT);
	zassert_equal(sys_reboot_fake.call_count, 0, "the limit was stopped");
}

ZTEST(fuz_confirm_timeout, test_client_confirms_in_time)
{
	Z_TEST_SKIP_IFNDEF(CONFIG_TEST_CASE_CLIENT_CONFIRMS);

	k_sleep(K_MSEC(LIMIT_MS / 2));
	/* What SMP image confirm leaves behind: MCUboot's confirmed flag set. */
	boot_is_img_confirmed_fake.return_val = true;

	k_sleep(PAST_THE_LIMIT);
	zassert_equal(sys_reboot_fake.call_count, 0, "confirmed, so no restart");
}

ZTEST(fuz_confirm_timeout, test_confirmed_at_startup_starts_no_limit)
{
	Z_TEST_SKIP_IFNDEF(CONFIG_TEST_CASE_CONFIRMED_AT_STARTUP);

	zassert_equal(startup_is_confirmed_calls, 1, "fuz asked at startup");

	/* The fake now answers "not confirmed" (reset), so a running limit would restart. */
	k_sleep(PAST_THE_LIMIT);
	zassert_equal(sys_reboot_fake.call_count, 0, "no limit was started");
}

ZTEST(fuz_confirm_timeout, test_no_limit)
{
	Z_TEST_SKIP_IFNDEF(CONFIG_TEST_CASE_NO_LIMIT);

	zassert_equal(startup_is_confirmed_calls, 0, "nothing to start, nothing asked");

	k_sleep(PAST_THE_LIMIT);
	zassert_equal(sys_reboot_fake.call_count, 0, "no restart");
}
