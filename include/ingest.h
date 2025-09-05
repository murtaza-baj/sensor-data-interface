#ifndef INGEST_H_
#define INGEST_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "defs.h"

void ingest_init(void);
void process_incoming_sample(uint8_t sensor_id, const void *data, uint8_t size);

// Peek/pop for packetizer
bool ingest_peek_oldest(SampleSlot *out);
bool ingest_pop_oldest(SampleSlot *out);

size_t   ingest_get_ring_occupancy(void);

// Dynamic sensor connect/disconnect tracking functions
bool ingest_register_sensor(uint8_t sensor_id);
bool ingest_unregister_sensor(uint8_t sensor_id);

#endif /* INGEST_H_ */
