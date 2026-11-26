/**
 * @file FFF fakes of the MCUboot and reboot functions fuz calls, defined once for every test file,
 * the calls they got during startup, and the rule that resets them before every test.
 */
#include <setjmp.h>

#include <zephyr/init.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>
#include <zephyr/ztest.h>

#include "fakes.h"

DEFINE_FFF_GLOBALS;

DEFINE_FAKE_VALUE_FUNC(bool, boot_is_img_confirmed);
DEFINE_FAKE_VALUE_FUNC(int, boot_write_img_confirmed);
DEFINE_FAKE_VALUE_FUNC(uint8_t, boot_fetch_active_slot);
DEFINE_FAKE_VALUE_FUNC(int, boot_read_bank_header, uint8_t, struct mcuboot_img_header *, size_t);
DEFINE_FAKE_VOID_FUNC(sys_reboot, int);

jmp_buf sys_reboot_jump;

/* The sys_reboot() fake's body: back to the test's setjmp(), never back to the caller. */
static void sys_reboot_jumps_back(int type)
{
	ARG_UNUSED(type);

	longjmp(sys_reboot_jump, 1);
}

unsigned int startup_is_confirmed_calls;
unsigned int startup_write_calls;

static int save_startup_calls(void)
{
	startup_is_confirmed_calls = boot_is_img_confirmed_fake.call_count;
	startup_write_calls = boot_write_img_confirmed_fake.call_count;

	return 0;
}

/* After fuz's own SYS_INIT: the same level, a later priority. */
BUILD_ASSERT(CONFIG_APPLICATION_INIT_PRIORITY < 99,
	     "save_startup_calls() must run after fuz_confirm_at_boot()");
SYS_INIT(save_startup_calls, APPLICATION, 99);

static void fakes_reset_before(const struct ztest_unit_test *test, void *fixture)
{
	ARG_UNUSED(test);
	ARG_UNUSED(fixture);

	RESET_FAKE(boot_is_img_confirmed);
	RESET_FAKE(boot_write_img_confirmed);
	RESET_FAKE(boot_fetch_active_slot);
	RESET_FAKE(boot_read_bank_header);
	RESET_FAKE(sys_reboot);
	FFF_RESET_HISTORY();

	sys_reboot_fake.custom_fake = sys_reboot_jumps_back;
}

/* Before every test in every suite, so that no test file can forget it. */
ZTEST_RULE(fakes_reset, fakes_reset_before, NULL);
