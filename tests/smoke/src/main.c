/**
 * @file Ztest: the module is built into an application, and every function in the C API links and
 * answers as documented. The functions not implemented yet answer "not supported".
 */
#include <errno.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/fff.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "fuz/fuz.h"

DEFINE_FFF_GLOBALS;

/* native_sim has no MCUboot: a fake of the one MCUboot function fuz calls so far. */
FAKE_VALUE_FUNC(bool, boot_is_img_confirmed);

/* A write function for fuz_init(); nothing is written yet. */
static int write_nothing(const uint8_t *data, size_t len, void *user)
{
	ARG_UNUSED(data);
	ARG_UNUSED(len);
	ARG_UNUSED(user);

	return 0;
}

static void reset_fakes(void *fixture)
{
	ARG_UNUSED(fixture);

	RESET_FAKE(boot_is_img_confirmed);
	FFF_RESET_HISTORY();
}

ZTEST_SUITE(fuz_smoke, NULL, NULL, reset_fakes, NULL, NULL);

ZTEST(fuz_smoke, test_module_is_enabled)
{
	/* Fails to build if Zephyr did not find the module: CONFIG_FUZ would not exist. */
	zassert_true(IS_ENABLED(CONFIG_FUZ));
}

/*
 * fuz_init() keeps its state for the whole program, so its three cases run in one test, in
 * order.
 */
ZTEST(fuz_smoke, test_init)
{
	zassert_equal(fuz_init(NULL, NULL), -EINVAL, "a NULL write function is refused");
	zassert_equal(fuz_init(write_nothing, NULL), 0);
	zassert_equal(fuz_init(write_nothing, NULL), -EALREADY, "only once");
}

ZTEST(fuz_smoke, test_is_confirmed_asks_mcuboot)
{
	boot_is_img_confirmed_fake.return_val = true;
	zassert_true(fuz_is_confirmed());

	boot_is_img_confirmed_fake.return_val = false;
	zassert_false(fuz_is_confirmed());

	zassert_equal(boot_is_img_confirmed_fake.call_count, 2, "asked MCUboot each time");
}

/*
 * Every function not implemented yet links and says so. Remove a line when its function is
 * implemented, and give that function a test of its own.
 */
ZTEST(fuz_smoke, test_stubs)
{
	const uint8_t frame[] = {0x06, 0x09};
	struct mcuboot_img_sem_ver version = {.major = 7};

	zassert_equal(fuz_receive_smp_frame(frame, sizeof(frame)), -ENOTSUP);

	zassert_false(fuz_is_restart_requested());

	zassert_equal(fuz_set_ready_for_restart(), -ENOENT);

	zassert_equal(fuz_confirm(), -ENOTSUP);

	zassert_equal(fuz_revert(), -ENOTSUP);

	zassert_equal(fuz_get_running_version(&version), -ENOTSUP);
	zassert_equal(version.major, 7, "version left unchanged on failure");

	zassert_equal(fuz_get_secondary_version(&version), -ENOTSUP);
	zassert_equal(version.major, 7, "version left unchanged on failure");

	zassert_equal(fuz_is_secondary_pending(), -ENOTSUP);
}
