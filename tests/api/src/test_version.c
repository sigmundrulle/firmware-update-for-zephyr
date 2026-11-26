/**
 * @file Ztest: reading the image versions from MCUboot's image headers.
 */
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "fakes.h"
#include "fuz/fuz.h"

/* The flash area the boot_fetch_active_slot() fake answers, passed on to the header read. */
#define ACTIVE_SLOT 3U

/* The boot_read_bank_header() fake's body: a v1 header with version 2.1.3+4. */
static int read_header_v2_1_3_4(uint8_t area_id, struct mcuboot_img_header *header,
				size_t header_size)
{
	ARG_UNUSED(area_id);
	ARG_UNUSED(header_size);

	header->mcuboot_version = 1U;
	header->h.v1.image_size = 1024U;
	header->h.v1.sem_ver.major = 2U;
	header->h.v1.sem_ver.minor = 1U;
	header->h.v1.sem_ver.revision = 3U;
	header->h.v1.sem_ver.build_num = 4U;

	return 0;
}

ZTEST_SUITE(fuz_version, NULL, NULL, NULL, NULL, NULL);

ZTEST(fuz_version, test_running_version_refuses_null)
{
	zassert_equal(fuz_get_running_version(NULL), -EINVAL);
	zassert_equal(boot_read_bank_header_fake.call_count, 0, "nothing read");
}

ZTEST(fuz_version, test_running_version_reads_the_active_slot)
{
	struct mcuboot_img_sem_ver version = {0};

	boot_fetch_active_slot_fake.return_val = ACTIVE_SLOT;
	boot_read_bank_header_fake.custom_fake = read_header_v2_1_3_4;

	zassert_equal(fuz_get_running_version(&version), 0);
	zassert_equal(boot_read_bank_header_fake.call_count, 1, "read once");
	zassert_equal(boot_read_bank_header_fake.arg0_val, ACTIVE_SLOT, "from the active slot");
	zassert_equal(boot_read_bank_header_fake.arg2_val, sizeof(struct mcuboot_img_header));

	zassert_equal(version.major, 2);
	zassert_equal(version.minor, 1);
	zassert_equal(version.revision, 3);
	zassert_equal(version.build_num, 4);
}

ZTEST(fuz_version, test_running_version_passes_on_read_error)
{
	struct mcuboot_img_sem_ver version = {.major = 7};

	boot_read_bank_header_fake.return_val = -EIO;

	zassert_equal(fuz_get_running_version(&version), -EIO, "MCUboot's error, unchanged");
	zassert_equal(version.major, 7, "version left unchanged on failure");
}
