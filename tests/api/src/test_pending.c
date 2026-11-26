/**
 * @file Ztest: fuz_is_secondary_pending() for each swap type MCUboot can report.
 */
#include <errno.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/ztest.h>

#include "fakes.h"
#include "fuz/fuz.h"

/* MCUboot's BOOT_SWAP_TYPE_PANIC (bootutil_public.h): a trailer could not be read. */
#define SWAP_TYPE_PANIC 0xff

ZTEST_SUITE(fuz_pending, NULL, NULL, NULL, NULL, NULL);

ZTEST(fuz_pending, test_test_is_pending)
{
	mcuboot_swap_type_fake.return_val = BOOT_SWAP_TYPE_TEST;

	zassert_equal(fuz_is_secondary_pending(), 1, "marked with image test");
}

ZTEST(fuz_pending, test_perm_is_pending)
{
	mcuboot_swap_type_fake.return_val = BOOT_SWAP_TYPE_PERM;

	zassert_equal(fuz_is_secondary_pending(), 1, "marked with image confirm");
}

ZTEST(fuz_pending, test_none_is_not_pending)
{
	mcuboot_swap_type_fake.return_val = BOOT_SWAP_TYPE_NONE;

	zassert_equal(fuz_is_secondary_pending(), 0);
}

ZTEST(fuz_pending, test_revert_is_not_pending)
{
	mcuboot_swap_type_fake.return_val = BOOT_SWAP_TYPE_REVERT;

	zassert_equal(fuz_is_secondary_pending(), 0, "the previous image comes back, nothing new");
}

ZTEST(fuz_pending, test_panic_is_an_error)
{
	mcuboot_swap_type_fake.return_val = SWAP_TYPE_PANIC;

	zassert_equal(fuz_is_secondary_pending(), -EIO, "a positive 0xff, not a pending swap");
}

ZTEST(fuz_pending, test_unknown_swap_type_is_an_error)
{
	mcuboot_swap_type_fake.return_val = BOOT_SWAP_TYPE_FAIL;

	zassert_equal(fuz_is_secondary_pending(), -EIO);
}
