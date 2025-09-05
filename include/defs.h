#ifndef DEFS_H_
#define DEFS_H_

#include <stdint.h>

#define MAX_SENSORS 10
#define SAMPLE_PAYLOAD_MAX 6
#define RING_CAPACITY 1024
#define MAX_PACKET_BYTES 200
#define PACKET_HEADER_BYTES 4
#define MIN_TX_INTERVAL_MS 10
#define DEFAULT_AGE_MS 20

typedef struct SampleSlot {
    uint32_t timestamp_ms;
    uint8_t sensor_id;
    uint8_t size;
    uint8_t payload[SAMPLE_PAYLOAD_MAX];
} SampleSlot;

#endif /* DEFS_H_ */
