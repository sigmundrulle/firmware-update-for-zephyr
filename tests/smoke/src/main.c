/**
 * @file Ztest: the module is found by Zephyr and built into an application.
 */
#include <zephyr/ztest.h>

#include "fw_update/fw_update.h"

ZTEST_SUITE(fw_update_smoke, NULL, NULL, NULL, NULL, NULL);

ZTEST(fw_update_smoke, test_module_is_enabled)
{
	/* Fails to build if Zephyr did not find the module: CONFIG_FW_UPDATE would not exist. */
	zassert_true(IS_ENABLED(CONFIG_FW_UPDATE));
}
