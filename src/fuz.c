/**
 * @file The fuz module's implementation.
 */
#include <stdbool.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/logging/log.h>

#include "fuz/fuz.h"

LOG_MODULE_REGISTER(fuz, CONFIG_FUZ_LOG_LEVEL);

bool fuz_is_confirmed(void)
{
	return boot_is_img_confirmed();
}
