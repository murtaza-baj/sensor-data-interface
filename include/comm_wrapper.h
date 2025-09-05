#ifndef COMM_WRAPPER_H_
#define COMM_WRAPPER_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "communication_interface.h"

// Thin wrapper around send_data_to_mcu2; returns the CommStatus directly.
CommStatus comm_send_once(const uint8_t *packet, size_t len);

/* Send wrapper with bounded retries and small backoff.
 * Returns true on success, false on permanent failure.
 * out_attempts is set to the number of attempts performed.
 */
bool send_wrapper(const uint8_t *packet, size_t len, uint32_t *out_attempts);

#endif /* COMM_WRAPPER_H_ */
