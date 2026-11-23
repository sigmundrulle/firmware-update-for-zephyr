/**
 * @file The fuz module's C API.
 *
 *       Firmware update over SMP (mcumgr) through MCUboot. The functions are not written yet; the
 *       header exists so that applications and tests can include it from the start.
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/dfu/mcuboot.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef fuz_write_fn
 * @brief Callback that writes fuz's SMP responses to the product's serial port.
 *
 * Called from mcumgr's thread. @p data is only valid during the call.
 *
 * @param data Bytes to write.
 * @param len Number of bytes in @p data.
 * @param user Pointer given to fuz_init().
 *
 * @return 0 on success, negative errno on failure.
 */
typedef int (*fuz_write_fn)(const uint8_t *data, size_t len, void *user);

/**
 * @brief Give fuz the function that writes its SMP responses.
 *
 * Call once at startup, before the first frame is passed to fuz_receive_smp_frame().
 *
 * @param write Function that writes the responses, see fuz_write_fn.
 * @param user Pointer passed unchanged to @p write.
 *
 * @retval 0 Success.
 * @retval -EINVAL @p write is NULL.
 * @retval -EALREADY fuz_init() has already been called.
 */
int fuz_init(fuz_write_fn write, void *user);

#ifdef __cplusplus
} // extern "C"
#endif
