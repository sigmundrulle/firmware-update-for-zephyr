/**
 * @file The fuz module's implementation.
 */
#include <errno.h>
#include <stdbool.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/logging/log.h>

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

bool fuz_is_confirmed(void)
{
	return boot_is_img_confirmed();
}
