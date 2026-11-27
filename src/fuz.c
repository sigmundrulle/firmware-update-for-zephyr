/**
 * @file The fuz module's implementation.
 */
#include <errno.h>
#include <stdbool.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>

#include <bootutil/image.h>

#include "fuz/fuz.h"

/* The hash TLV MCUboot checks, as mcumgr's image management chooses it. */
#if defined(CONFIG_MCUBOOT_BOOTLOADER_USES_SHA512)
#define FUZ_TLV_SHA     IMAGE_TLV_SHA512
#define FUZ_TLV_SHA_LEN 64U
#else
#define FUZ_TLV_SHA     IMAGE_TLV_SHA256
#define FUZ_TLV_SHA_LEN 32U
#endif

/* A TLV type in erased flash: the TLV area ends here. */
#define FUZ_TLV_ERASED 0xffffU

/* The flash area of slot 1. */
#define FUZ_SLOT1_ID PARTITION_ID(slot1_partition)

LOG_MODULE_REGISTER(fuz, CONFIG_FUZ_LOG_LEVEL);

/* The client must not confirm slot 1 before it has run (see docs/decisions.md and Kconfig). */
BUILD_ASSERT(
	!IS_ENABLED(CONFIG_MCUMGR_GRP_IMG_ALLOW_CONFIRM_NON_ACTIVE_SLOT),
	"fuz: CONFIG_MCUMGR_GRP_IMG_ALLOW_CONFIRM_NON_ACTIVE_SLOT must be n, so that the client "
	"cannot confirm an update before it has run");

/* The product's write function and its pointer, from fuz_init(); NULL until then. */
static fuz_write_fn fuz_write;
static void *fuz_user;

int fuz_init(fuz_write_fn write, void *user)
{
	if (write == NULL) {
		return -EINVAL;
	}

	if (fuz_write != NULL) {
		return -EALREADY;
	}

	fuz_write = write;
	fuz_user = user;

	return 0;
}

int fuz_receive_smp_frame(const uint8_t *data, size_t len)
{
	/* Not implemented yet: SMP transport. */
	ARG_UNUSED(data);
	ARG_UNUSED(len);

	return -ENOTSUP;
}

bool fuz_is_restart_requested(void)
{
	/* Not implemented yet: nothing receives SMP, so no client can request a restart. */
	return false;
}

int fuz_set_ready_for_restart(void)
{
	/* Not implemented yet: no restart can have been requested. */
	return -ENOENT;
}

bool fuz_is_confirmed(void)
{
	return boot_is_img_confirmed();
}

#if defined(CONFIG_FUZ_CONFIRM_AT_BOOT)

static int fuz_confirm_at_boot(void)
{
	int rc;

	if (fuz_is_confirmed()) {
		return 0;
	}

	rc = boot_write_img_confirmed();
	if (rc != 0) {
		LOG_ERR("Could not confirm the running image: %d", rc);
		return rc;
	}

	LOG_INF("Running image confirmed");

	return 0;
}

SYS_INIT(fuz_confirm_at_boot, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

#endif /* CONFIG_FUZ_CONFIRM_AT_BOOT */

int fuz_confirm(void)
{
	int rc;

	if (IS_ENABLED(CONFIG_FUZ_CONFIRM_BY_CLIENT)) {
		return -EPERM;
	}

	/*
	 * With CONFIG_FUZ_CONFIRM_AT_BOOT, fuz_confirm_at_boot() has confirmed already, so this
	 * returns -EALREADY; if that write failed, this tries again.
	 */
	if (fuz_is_confirmed()) {
		return -EALREADY;
	}

	rc = boot_write_img_confirmed();
	if (rc != 0) {
		LOG_ERR("Could not confirm the running image: %d", rc);
		return rc;
	}

	/*
	 * Not implemented yet: stopping the CONFIG_FUZ_CONFIRM_TIMEOUT_MS time limit. Only after a
	 * successful write, so that MCUboot still goes back if the write failed.
	 */
	LOG_INF("Running image confirmed");

	return 0;
}

int fuz_revert(void)
{
	if (fuz_is_confirmed()) {
		return -EALREADY;
	}

	LOG_INF("Restarting without confirming, MCUboot goes back to the previous image");
	/* Deferred logging would lose the line above at the restart: write it out now. */
	LOG_PANIC();

	/* Never returns (FUNC_NORETURN): the board restarts, or hangs if it cannot. */
	sys_reboot(SYS_REBOOT_WARM);
}

int fuz_get_running_version(struct mcuboot_img_sem_ver *version)
{
	struct mcuboot_img_header header;
	int rc;

	if (version == NULL) {
		return -EINVAL;
	}

	rc = boot_read_bank_header(boot_fetch_active_slot(), &header, sizeof(header));
	if (rc != 0) {
		LOG_ERR("Could not read the running image's header: %d", rc);
		return rc;
	}

	*version = header.h.v1.sem_ver;

	return 0;
}

/*
 * Read the version of the image that starts at off in fa, if the image is complete: a header, the
 * TLV area after the code and exactly one hash TLV, the same check as mcumgr's
 * img_mgmt_read_info(). A half-written upload has no TLVs yet, since they come last.
 *
 * Returns 0, -ENOENT if no complete image starts at off, or the flash read's error. version is
 * written only on success.
 */
static int read_version_at(const struct flash_area *fa, off_t off,
			   struct mcuboot_img_sem_ver *version)
{
	struct image_header header;
	struct image_tlv_info info;
	struct image_tlv tlv;
	off_t tlv_off;
	off_t tlv_end;
	int hashes = 0;
	int rc;

	rc = flash_area_read(fa, off, &header, sizeof(header));
	if (rc != 0) {
		return rc;
	}

	if (sys_le32_to_cpu(header.ih_magic) != IMAGE_MAGIC) {
		return -ENOENT;
	}

	/* The TLV areas follow the code: the protected one, if there is one, then the hash's. */
	tlv_off = off + sys_le16_to_cpu(header.ih_hdr_size) + sys_le32_to_cpu(header.ih_img_size);
	if (tlv_off + (off_t)sizeof(info) > (off_t)fa->fa_size) {
		return -ENOENT;
	}

	rc = flash_area_read(fa, tlv_off, &info, sizeof(info));
	if (rc != 0) {
		return rc;
	}

	if (sys_le16_to_cpu(info.it_magic) == IMAGE_TLV_PROT_INFO_MAGIC) {
		/* it_tlv_tot includes this header. */
		tlv_off += sys_le16_to_cpu(info.it_tlv_tot);
		if (tlv_off + (off_t)sizeof(info) > (off_t)fa->fa_size) {
			return -ENOENT;
		}

		rc = flash_area_read(fa, tlv_off, &info, sizeof(info));
		if (rc != 0) {
			return rc;
		}
	}

	if (sys_le16_to_cpu(info.it_magic) != IMAGE_TLV_INFO_MAGIC) {
		return -ENOENT;
	}

	tlv_end = MIN(tlv_off + (off_t)sys_le16_to_cpu(info.it_tlv_tot), (off_t)fa->fa_size);
	tlv_off += sizeof(info);

	while (tlv_off + (off_t)sizeof(tlv) <= tlv_end) {
		rc = flash_area_read(fa, tlv_off, &tlv, sizeof(tlv));
		if (rc != 0) {
			return rc;
		}

		if (sys_le16_to_cpu(tlv.it_type) == FUZ_TLV_ERASED) {
			return -ENOENT;
		}

		if (sys_le16_to_cpu(tlv.it_type) == FUZ_TLV_SHA &&
		    sys_le16_to_cpu(tlv.it_len) == FUZ_TLV_SHA_LEN) {
			hashes++;
		}

		tlv_off += sizeof(tlv) + sys_le16_to_cpu(tlv.it_len);
	}

	if (hashes != 1) {
		return -ENOENT;
	}

	version->major = header.ih_ver.iv_major;
	version->minor = header.ih_ver.iv_minor;
	version->revision = sys_le16_to_cpu(header.ih_ver.iv_revision);
	version->build_num = sys_le32_to_cpu(header.ih_ver.iv_build_num);

	return 0;
}

int fuz_get_secondary_version(struct mcuboot_img_sem_ver *version)
{
	const struct flash_area *fa;
	off_t start;
	int rc;

	if (version == NULL) {
		return -EINVAL;
	}

	rc = flash_area_open(FUZ_SLOT1_ID, &fa);
	if (rc != 0) {
		LOG_ERR("Could not open slot 1: %d", rc);
		return rc;
	}

	/*
	 * Where Zephyr places the image: with swap using offset, one sector in before a swap and at
	 * the start while the previous image waits to be swapped back; elsewhere at the start.
	 */
	start = (off_t)boot_get_image_start_offset(FUZ_SLOT1_ID);
	rc = read_version_at(fa, start, version);

	/* After a confirmed swap, the previous image is at the start, where Zephyr doesn't look. */
	if (rc == -ENOENT && start != 0) {
		rc = read_version_at(fa, 0, version);
	}

	flash_area_close(fa);

	if (rc != 0 && rc != -ENOENT) {
		LOG_ERR("Could not read slot 1: %d", rc);
	}

	return rc;
}

int fuz_is_secondary_pending(void)
{
	int swap_type = mcuboot_swap_type();

	switch (swap_type) {
	case BOOT_SWAP_TYPE_TEST:
	case BOOT_SWAP_TYPE_PERM:
		return 1;
	case BOOT_SWAP_TYPE_NONE:
	/*
	 * The previous image comes back, nothing new waits: smpmgr image state-read does not
	 * show slot 1 as pending either.
	 */
	case BOOT_SWAP_TYPE_REVERT:
		return 0;
	default:
		/* BOOT_SWAP_TYPE_PANIC (0xff, not an errno): a trailer could not be read. */
		LOG_ERR("Could not read MCUboot's swap state: %d", swap_type);
		return -EIO;
	}
}
