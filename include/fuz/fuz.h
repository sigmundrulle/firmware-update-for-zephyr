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

/**
 * @brief Pass one SMP frame from the product's serial port to fuz.
 *
 * Call it for each received frame, that is a line starting with 0x06 0x09 (first frame of an SMP
 * packet) or 0x04 0x14 (continuation). fuz copies the frame, so @p data is only needed during the
 * call. Does not block. The response, once the whole SMP packet has arrived, is written later
 * through the fuz_write_fn given to fuz_init().
 *
 * @param data The frame, from its start bytes.
 * @param len Number of bytes in @p data.
 *
 * @retval 0 The frame was accepted.
 * @retval -EPERM fuz_init() has not been called.
 * @retval -EINVAL @p data is not an SMP frame.
 * @retval -ENOMEM No room for the frame; fuz is still handling earlier packets.
 */
int fuz_receive_smp_frame(const uint8_t *data, size_t len);

/**
 * @brief Check whether the client has requested a restart (SMP os reset).
 *
 * Call it often, for example in every pass of the product's main loop. When it returns true, shut
 * down in an orderly way and then call fuz_set_ready_for_restart().
 *
 * @return true if a restart has been requested, false otherwise.
 */
bool fuz_is_restart_requested(void);

/**
 * @brief Tell fuz that the product has shut down and may be restarted.
 *
 * fuz then lets mcumgr answer the client's os reset and restart shortly after. Keep the main loop
 * running after this call until the restart, so that the answer can be written out through the
 * fuz_write_fn. If the product does not call this within CONFIG_FUZ_RESET_READY_TIMEOUT_MS of the
 * request, fuz answers the client with busy and does not restart.
 *
 * @retval 0 The restart may go ahead.
 * @retval -ENOENT No restart has been requested, or the time limit has already passed.
 */
int fuz_set_ready_for_restart(void);

#ifdef __cplusplus
} // extern "C"
#endif
