#include "comm_wrapper.h"
#include "platform.h"
#include <stdint.h>

#define COMM_WRAPPER_MAX_RETRIES 2
#define COMM_WRAPPER_BACKOFF_MS 5

CommStatus comm_send_once(const uint8_t *packet, size_t len)
{
    return send_data_to_mcu2(packet, len);
}

bool send_wrapper(const uint8_t *packet, size_t len, uint32_t *out_attempts)
{
    if (out_attempts) *out_attempts = 0;

    for (uint32_t attempt = 1; attempt <= COMM_WRAPPER_MAX_RETRIES; ++attempt) {
        if (out_attempts) *out_attempts = attempt;

        CommStatus st = comm_send_once(packet, len);
        if (st == COMM_STATUS_SUCCESS) {
            return true;
        }

        if (st == COMM_STATUS_INVALID_PACKET_LENGTH || st == COMM_STATUS_INVALID_PACKET_LOCATION) {
            return false;
        }
        platform_wait_ms(COMM_WRAPPER_BACKOFF_MS);
    }
    return false;
}
