#include "sensor_interface.h"
#include "ingest.h"

void sensor_data_callback(uint8_t sensor_id, const void* data, uint8_t size)
{
    process_incoming_sample(sensor_id, data, size);
}
