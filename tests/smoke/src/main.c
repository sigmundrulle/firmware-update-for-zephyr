/**
 * @file Ztest: the module is found by Zephyr and built into an application.
 */
#include <zephyr/ztest.h>

#include "fuz/fuz.h"

ZTEST_SUITE(fuz_smoke, NULL, NULL, NULL, NULL, NULL);

ZTEST(fuz_smoke, test_module_is_enabled)
{
	/* Fails to build if Zephyr did not find the module: CONFIG_FUZ would not exist. */
	zassert_true(IS_ENABLED(CONFIG_FUZ));
}
