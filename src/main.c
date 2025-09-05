#include "platform.h"
#include "ingest.h"
#include "packetizer.h"
#include "defs.h"
#include <stdint.h>

void sensor_connected(uint8_t sensor_id)
{
    (void)ingest_register_sensor(sensor_id);
}

void sensor_disconnected(uint8_t sensor_id)
{
    (void)ingest_unregister_sensor(sensor_id);
}

int main(void)
{
    platform_init();
    ingest_init(RING_CAPACITY);
    packetizer_init(0);

    // Accept samples from all possible sensor IDs by default
    for (uint8_t id = 0; id < MAX_SENSORS; ++id) {
        (void)ingest_register_sensor(id);
    }

    packetizer_set_age_threshold_ms(10);

    while(1) {
        packetizer_run_once();

        if (ingest_get_ring_occupancy() == 0) {
            platform_wait_ms(MIN_TX_INTERVAL_MS);
        } else {
            platform_wait_ms(1);
        }
    }

    return 0;
}
