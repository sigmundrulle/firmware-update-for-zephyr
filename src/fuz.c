/**
 * @file The fuz module's implementation.
 */
#include <errno.h>
#include <stdbool.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>

#include "fuz/fuz.h"

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

	sys_reboot(SYS_REBOOT_WARM);

	/* sys_reboot() does not return on a board; only reached if the restart did not happen. */
	return -EIO;
}

int fuz_get_running_version(struct mcuboot_img_sem_ver *version)
{
	/* Not implemented yet: read the image header in slot 0. */
	ARG_UNUSED(version);

	return -ENOTSUP;
}

int fuz_get_secondary_version(struct mcuboot_img_sem_ver *version)
{
	/* Not implemented yet: read the image header in slot 1, one sector in or at its start. */
	ARG_UNUSED(version);

	return -ENOTSUP;
}

int fuz_is_secondary_pending(void)
{
	/* Not implemented yet: MCUboot's swap type for slot 1. */
	return -ENOTSUP;
}
