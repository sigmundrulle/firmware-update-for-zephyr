/**
 * @file Ztest: reading the image versions from MCUboot's image headers.
 */
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include <bootutil/image.h>

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

/* Slot 1 as the flash map fakes give it: a buffer, erased (0xff) before every slot 1 test. */
#define SLOT1_SIZE 1024U
static uint8_t slot1[SLOT1_SIZE];
static const struct flash_area slot1_area = {.fa_size = SLOT1_SIZE};

/* An image as the tests write it: a header, CODE_SIZE bytes of code, then TLVs. */
#define CODE_SIZE 100U

static int open_slot1(uint8_t id, const struct flash_area **fa)
{
	ARG_UNUSED(id);

	*fa = &slot1_area;
	return 0;
}

static int read_slot1(const struct flash_area *fa, off_t off, void *dst, size_t len)
{
	ARG_UNUSED(fa);

	if (off < 0 || (size_t)off + len > SLOT1_SIZE) {
		return -EINVAL;
	}

	memcpy(dst, &slot1[off], len);
	return 0;
}

/* Write a header at off with version 2.1.3+4 and CODE_SIZE bytes of code; return the TLV offset. */
static size_t put_header(size_t off)
{
	sys_put_le32(IMAGE_MAGIC, &slot1[off]);
	sys_put_le16(IMAGE_HEADER_SIZE, &slot1[off + 8]);
	sys_put_le32(CODE_SIZE, &slot1[off + 12]);
	slot1[off + 20] = 2U;
	slot1[off + 21] = 1U;
	sys_put_le16(3U, &slot1[off + 22]);
	sys_put_le32(4U, &slot1[off + 24]);
	memset(&slot1[off + IMAGE_HEADER_SIZE], 0, CODE_SIZE);

	return off + IMAGE_HEADER_SIZE + CODE_SIZE;
}

/* Write a TLV area at off with the given number of SHA-256 TLVs; return the offset after it. */
static size_t put_tlvs(size_t off, uint16_t magic, int hashes)
{
	size_t tot = 4U + (size_t)hashes * (4U + 32U);

	sys_put_le16(magic, &slot1[off]);
	sys_put_le16((uint16_t)tot, &slot1[off + 2]);
	for (int i = 0; i < hashes; i++) {
		size_t tlv = off + 4U + (size_t)i * (4U + 32U);

		sys_put_le16(IMAGE_TLV_SHA256, &slot1[tlv]);
		sys_put_le16(32U, &slot1[tlv + 2]);
		memset(&slot1[tlv + 4], 0xab, 32);
	}

	return off + tot;
}

static void slot1_before(void *fixture)
{
	ARG_UNUSED(fixture);

	memset(slot1, 0xff, sizeof(slot1));
	flash_area_open_fake.custom_fake = open_slot1;
	flash_area_read_fake.custom_fake = read_slot1;
}

ZTEST_SUITE(fuz_secondary_version, NULL, NULL, slot1_before, NULL, NULL);

ZTEST(fuz_secondary_version, test_refuses_null)
{
	zassert_equal(fuz_get_secondary_version(NULL), -EINVAL);
	zassert_equal(flash_area_open_fake.call_count, 0, "nothing opened");
}

ZTEST(fuz_secondary_version, test_reads_a_complete_image)
{
	struct mcuboot_img_sem_ver version = {0};

	put_tlvs(put_header(0), IMAGE_TLV_INFO_MAGIC, 1);

	zassert_equal(fuz_get_secondary_version(&version), 0);
	zassert_equal(flash_area_open_fake.arg0_val, PARTITION_ID(slot1_partition), "slot 1");
	zassert_equal(flash_area_close_fake.call_count, 1, "closed again");

	zassert_equal(version.major, 2);
	zassert_equal(version.minor, 1);
	zassert_equal(version.revision, 3);
	zassert_equal(version.build_num, 4);
}

ZTEST(fuz_secondary_version, test_reads_past_a_protected_tlv_area)
{
	struct mcuboot_img_sem_ver version = {0};

	put_tlvs(put_tlvs(put_header(0), IMAGE_TLV_PROT_INFO_MAGIC, 0), IMAGE_TLV_INFO_MAGIC, 1);

	zassert_equal(fuz_get_secondary_version(&version), 0);
	zassert_equal(version.major, 2);
}

ZTEST(fuz_secondary_version, test_erased_slot_holds_no_image)
{
	struct mcuboot_img_sem_ver version = {.major = 7};

	zassert_equal(fuz_get_secondary_version(&version), -ENOENT);
	zassert_equal(version.major, 7, "version left unchanged on failure");
	zassert_equal(flash_area_close_fake.call_count, 1, "closed again");
}

ZTEST(fuz_secondary_version, test_unfinished_upload_holds_no_image)
{
	struct mcuboot_img_sem_ver version = {.major = 7};

	/* The header and code are written, the TLVs (last in an upload) are still erased. */
	put_header(0);

	zassert_equal(fuz_get_secondary_version(&version), -ENOENT);
	zassert_equal(version.major, 7, "version left unchanged on failure");
}

ZTEST(fuz_secondary_version, test_image_without_a_hash_is_not_valid)
{
	struct mcuboot_img_sem_ver version = {.major = 7};

	put_tlvs(put_header(0), IMAGE_TLV_INFO_MAGIC, 0);

	zassert_equal(fuz_get_secondary_version(&version), -ENOENT);
}

ZTEST(fuz_secondary_version, test_image_with_two_hashes_is_not_valid)
{
	struct mcuboot_img_sem_ver version = {.major = 7};

	put_tlvs(put_header(0), IMAGE_TLV_INFO_MAGIC, 2);

	zassert_equal(fuz_get_secondary_version(&version), -ENOENT);
}

ZTEST(fuz_secondary_version, test_passes_on_a_read_error)
{
	struct mcuboot_img_sem_ver version = {.major = 7};

	flash_area_read_fake.custom_fake = NULL;
	flash_area_read_fake.return_val = -EIO;

	zassert_equal(fuz_get_secondary_version(&version), -EIO, "the flash error, unchanged");
	zassert_equal(version.major, 7, "version left unchanged on failure");
	zassert_equal(flash_area_close_fake.call_count, 1, "closed again");
}

ZTEST(fuz_secondary_version, test_passes_on_an_open_error)
{
	struct mcuboot_img_sem_ver version = {.major = 7};

	flash_area_open_fake.custom_fake = NULL;
	flash_area_open_fake.return_val = -ENODEV;

	zassert_equal(fuz_get_secondary_version(&version), -ENODEV);
	zassert_equal(flash_area_close_fake.call_count, 0, "nothing to close");
}
