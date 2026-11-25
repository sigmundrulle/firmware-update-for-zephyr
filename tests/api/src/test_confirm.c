/**
 * @file Ztest: fuz_confirm() in each confirm mode (the variants in testcase.yaml).
 */
#include <errno.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "fakes.h"
#include "fuz/fuz.h"

ZTEST_SUITE(fuz_confirm, NULL, NULL, NULL, NULL, NULL);

ZTEST(fuz_confirm, test_by_client_refuses)
{
	Z_TEST_SKIP_IFNDEF(CONFIG_FUZ_CONFIRM_BY_CLIENT);

	/* Even when the client has confirmed already: the answer does not depend on timing. */
	boot_is_img_confirmed_fake.return_val = true;

	zassert_equal(fuz_confirm(), -EPERM, "only the client may confirm");
	zassert_equal(boot_write_img_confirmed_fake.call_count, 0, "nothing written");
}

ZTEST(fuz_confirm, test_writes)
{
	Z_TEST_SKIP_IFDEF(CONFIG_FUZ_CONFIRM_BY_CLIENT);

	boot_is_img_confirmed_fake.return_val = false;
	boot_write_img_confirmed_fake.return_val = 0;

	zassert_equal(fuz_confirm(), 0);
	zassert_equal(boot_write_img_confirmed_fake.call_count, 1, "written once");
}

ZTEST(fuz_confirm, test_already_confirmed)
{
	Z_TEST_SKIP_IFDEF(CONFIG_FUZ_CONFIRM_BY_CLIENT);

	boot_is_img_confirmed_fake.return_val = true;

	zassert_equal(fuz_confirm(), -EALREADY);
	zassert_equal(boot_write_img_confirmed_fake.call_count, 0, "nothing written");
}

ZTEST(fuz_confirm, test_passes_on_write_error)
{
	Z_TEST_SKIP_IFDEF(CONFIG_FUZ_CONFIRM_BY_CLIENT);

	boot_is_img_confirmed_fake.return_val = false;
	boot_write_img_confirmed_fake.return_val = -EIO;

	zassert_equal(fuz_confirm(), -EIO, "MCUboot's error, unchanged");
}
