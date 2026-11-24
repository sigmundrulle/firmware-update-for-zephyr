/**
 * @file The fuz module's implementation.
 */
#include <errno.h>
#include <stdbool.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "fuz/fuz.h"

LOG_MODULE_REGISTER(fuz, CONFIG_FUZ_LOG_LEVEL);

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

int fuz_confirm(void)
{
	/* Not implemented yet: confirm according to CONFIG_FUZ_CONFIRM_*. */
	return -ENOTSUP;
}

int fuz_revert(void)
{
	/* Not implemented yet: restart without confirm. Returns, as on any failure. */
	return -ENOTSUP;
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
