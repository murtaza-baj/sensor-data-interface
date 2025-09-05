#ifndef PACKETIZER_H_
#define PACKETIZER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Initialize packetizer with a starting sequence number.
void packetizer_init(uint16_t start_seq);

// Set the age threshold (ms) used to force send old samples (DEFAULT_AGE_MS).
void packetizer_set_age_threshold_ms(uint32_t age_ms);

// Run packetizer once — called frequently from main loop.
void packetizer_run_once(void);

void write_u16_le(uint8_t *p, uint16_t v);
void write_u32_le(uint8_t *p, uint32_t v);

#endif /* PACKETIZER_H_ */
