/* Packet format:
 *  header (4 bytes):
 *    seq (2 bytes LE)
 *    sample_count (1 byte)
 *    flags/reserved (1 byte)
 *  payload:
 *    base_timestamp (4 bytes LE) - timestamp of first sample
 *    sample entries:
 *      sensor_id (1)
 *      delta_ms (1)   - timestamp difference from base_timestamp, 0..255
 *      size (1)
 *      payload (size bytes)
 */

#include "packetizer.h"
#include "defs.h"
#include "platform.h"
#include "ingest.h"
#include "comm_wrapper.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define PACKET_MAX_PAYLOAD_BYTES (MAX_PACKET_BYTES - PACKET_HEADER_BYTES)

static uint16_t seq_num = 0;
static uint32_t last_send_time_ms = 0;
static uint32_t age_threshold_ms = DEFAULT_AGE_MS;

static uint32_t packets_sent = 0;
static uint32_t packets_failed = 0;

void write_u16_le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
}

void write_u32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
    p[2] = (uint8_t)((v >> 16) & 0xFF);
    p[3] = (uint8_t)((v >> 24) & 0xFF);
}

// Encode a sample using compact layout: sensor_id(1)+delta(1)+size(1)+payload
static size_t encode_sample_compact(uint8_t *dst, const SampleSlot *s, uint8_t delta)
{
    size_t enc_len = 1 + 1 + 1 + (size_t)s->size;
    dst[0] = s->sensor_id;
    dst[1] = delta;
    dst[2] = s->size;
    memcpy(dst + 3, s->payload, s->size);
    return enc_len;
}

void packetizer_init(uint16_t start_seq)
{
    seq_num = start_seq;
    last_send_time_ms = 0;
    age_threshold_ms = DEFAULT_AGE_MS;
    packets_sent = 0;
    packets_failed = 0;
}

void packetizer_set_age_threshold_ms(uint32_t age_ms)
{
    age_threshold_ms = age_ms;
}

void packetizer_run_once(void)
{
    size_t occ = ingest_get_ring_occupancy();
    if (occ == 0) {
        return;
    }

    uint8_t packet[MAX_PACKET_BYTES];
    size_t packet_len = PACKET_HEADER_BYTES;
    uint8_t sample_count = 0;
    uint32_t base_ts = 0;

    while (true) {
        SampleSlot tmp;
        bool has = ingest_peek_oldest(&tmp);
        if (!has) break;

        if (sample_count == 0) {
            base_ts = tmp.timestamp_ms;
        }

        uint32_t delta = (uint32_t)(tmp.timestamp_ms - base_ts);
        if (delta > 255) {
            if (sample_count == 0) {
                // First sample can't have delta==0
            }
            break;
        }

        size_t enc_len = 1 + 1 + 1 + (size_t)tmp.size; // sensor + delta + size + payload

        size_t effective_len = packet_len + enc_len;
        if (sample_count == 0) effective_len += 4; // account for base timestamp

        if (effective_len > MAX_PACKET_BYTES) {
            if (sample_count == 0) {
                // Drop malformed/oversized sample by consuming it
                SampleSlot drop;
                if (ingest_pop_oldest(&drop)) {
                    // sample dropped
                }
                continue;
            }
            break;
        }

        if (sample_count > 0) {
            uint32_t now = platform_get_tick_ms();
            if ((uint32_t)(now - base_ts) >= age_threshold_ms) {
                break;
            }
        }

        SampleSlot s;
        if (!ingest_pop_oldest(&s)) {
            break;
        }

        if (sample_count == 0) {
            write_u32_le(packet + packet_len, base_ts);
            packet_len += 4;
        }

        uint8_t delta8 = (uint8_t)((uint32_t)(s.timestamp_ms - base_ts) & 0xFFu);
        size_t written = encode_sample_compact(packet + packet_len, &s, delta8);
        packet_len += written;
        sample_count += 1;
    }

    if (sample_count == 0) {
        return;
    }

    // Fill header
    write_u16_le(packet, seq_num);
    packet[2] = sample_count;
    packet[3] = 0;  // Reserved

    // Enforce minimum transmit interval
    uint32_t now = platform_get_tick_ms();
    if ((uint32_t)(now - last_send_time_ms) < MIN_TX_INTERVAL_MS) {
        platform_wait_ms((uint32_t)(MIN_TX_INTERVAL_MS - (now - last_send_time_ms)));
    }

    uint32_t attempts = 0;
    bool ok = send_wrapper(packet, packet_len, &attempts);
    uint32_t t_end = platform_get_tick_ms();
    last_send_time_ms = t_end;

    if (ok) {
        packets_sent += 1;
        seq_num++;
    } else {
        packets_failed += 1;
    }
}
