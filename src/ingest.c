/* Small, efficient implementation designed for ISR-safe callbacks.
 * - static ring buffer, no dynamic allocation
 * - drop-oldest policy when full
 * - simple registration table for up to MAX_SENSORS sensors
 *
 * Note: platform_enter_critical/platform_exit_critical must be implemented
 * on the target to provide brief interrupt protection.
 */

#include "ingest.h"
#include "platform.h"

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <stdbool.h>

static SampleSlot ring_buf[RING_CAPACITY];
static size_t ring_capacity = RING_CAPACITY;

static uint64_t prod_index = 0;
static uint64_t cons_index = 0;

static uint32_t samples_received = 0;
static uint32_t samples_dropped = 0;
static uint32_t ring_high_water = 0;
static uint32_t invalid_size_counter = 0;

static bool sensor_registered[MAX_SENSORS] = {0};
static size_t registered_count = 0;

void ingest_init(void) {
    prod_index = 0;
    cons_index = 0;
    samples_received = 0;
    samples_dropped = 0;
    ring_high_water = 0;
    invalid_size_counter = 0;
    memset(sensor_registered, 0, sizeof(sensor_registered));
    registered_count = 0;
}

// Called from sensor_data_callback
void process_incoming_sample(uint8_t sensor_id, const void *data, uint8_t size)
{
    if (size < 2 || size > SAMPLE_PAYLOAD_MAX) {
        platform_enter_critical();
        invalid_size_counter++;
        platform_exit_critical();
        return;
    }

    if (sensor_id >= MAX_SENSORS) {
        platform_enter_critical();
        invalid_size_counter++;
        platform_exit_critical();
        return;
    }

    uint32_t ts = platform_get_tick_ms();

    platform_enter_critical();

    if (!sensor_registered[sensor_id]) {
        invalid_size_counter++;
        platform_exit_critical();
        return;
    }

    uint64_t occupancy = prod_index - cons_index;
    if (occupancy >= ring_capacity) {
        // Drop-oldest policy
        cons_index += 1;
        samples_dropped += 1;
    }

    size_t write_idx = (size_t)(prod_index % ring_capacity);

    ring_buf[write_idx].timestamp_ms = ts;
    ring_buf[write_idx].sensor_id    = sensor_id;
    ring_buf[write_idx].size         = size;
    memcpy(ring_buf[write_idx].payload, data, size);

    prod_index += 1;

    uint64_t cur_occ = prod_index - cons_index;
    if (cur_occ > ring_high_water) {
        ring_high_water = (cur_occ > UINT32_MAX) ? UINT32_MAX : (uint32_t)cur_occ;
    }

    samples_received += 1;

    platform_exit_critical();
}

// Peek oldest sample without removing it.
bool ingest_peek_oldest(SampleSlot *out)
{
    bool ret = false;
    platform_enter_critical();
    uint64_t occ = prod_index - cons_index;
    if (occ == 0) {
        ret = false;
    } else {
        size_t idx = (size_t)(cons_index % ring_capacity);
        if (out) {
            *out = ring_buf[idx];
        }
        ret = true;
    }
    platform_exit_critical();
    return ret;
}

// Pop oldest sample.
bool ingest_pop_oldest(SampleSlot *out)
{
    bool ret = false;
    platform_enter_critical();
    uint64_t occ = prod_index - cons_index;
    if (occ == 0) {
        ret = false;
    } else {
        size_t idx = (size_t)(cons_index % ring_capacity);
        if (out) {
            *out = ring_buf[idx];
        }
        cons_index += 1;
        ret = true;
    }
    platform_exit_critical();
    return ret;
}

size_t ingest_get_ring_occupancy(void)
{
    size_t occ;
    platform_enter_critical();
    occ = (size_t)(prod_index - cons_index);
    platform_exit_critical();
    return occ;
}

size_t ingest_get_ring_capacity(void)
{
    return ring_capacity;
}

uint32_t ingest_get_ring_high_water(void)
{
    uint32_t val;
    platform_enter_critical();
    val = ring_high_water;
    platform_exit_critical();
    return val;
}

uint32_t ingest_get_invalid_size_counter(void)
{
    uint32_t v;
    platform_enter_critical();
    v = invalid_size_counter;
    platform_exit_critical();
    return v;
}

// Registration APIs: sensor_id must be 0..(MAX_SENSORS-1)
bool ingest_register_sensor(uint8_t sensor_id)
{
    if (sensor_id >= MAX_SENSORS) return false;
    platform_enter_critical();
    if (!sensor_registered[sensor_id] && registered_count >= MAX_SENSORS) {
        platform_exit_critical();
        return false;
    }
    if (!sensor_registered[sensor_id]) {
        sensor_registered[sensor_id] = true;
        registered_count++;
    }
    platform_exit_critical();
    return true;
}

bool ingest_unregister_sensor(uint8_t sensor_id)
{
    if (sensor_id >= MAX_SENSORS) return false;
    platform_enter_critical();
    if (sensor_registered[sensor_id]) {
        sensor_registered[sensor_id] = false;
        if (registered_count > 0) registered_count--;
    }
    platform_exit_critical();
    return true;
}
