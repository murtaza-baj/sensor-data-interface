# Sensor Data Interface — Design & Assumptions

## Overview
MCU1 collects data from up to 10 sensors and forwards aggregated packets to MCU2 over a wireless link. Sensors may connect/disconnect dynamically. The system prioritizes low latency and predictable memory usage while respecting packet size and transmission-interval constraints.

## Key requirements
- Max sensors concurrently: **10**
- Sensor payload size: **2–6 bytes**
- Sensor sample rates: **5 ms to 1000 ms**
- Max packet size: **200 bytes**
- Min delay between transmissions: **10 ms**
- Include: sensor ID + timestamp
- No dynamic heap allocation; minimal power & memory usage

## File map
- `platform.h/.c` — target-specific abstractions (tick, delays, critical sections)
- `defs.h` — compile-time constants and sample struct
- `ingest.*` — ISR-safe sample ingestion, static ring buffer, registration API
- `packetizer.*` — packet assembly and transmission control
- `comm_wrapper.*` — wrapper around `send_data_to_mcu2` with bounded retries
- `sensor_wrapper.c` — sensor callback adapter
- `main.c` — initialization and main loop
- `design.md` — this document

## Packet format
- Header (4 bytes):
  - seq (2 bytes, little-endian)
  - sample_count (1 byte)
  - flags/reserved (1 byte)
- Payload:
  - base_timestamp (4 bytes, little-endian) — timestamp of first sample in packet
  - For each sample:
    - sensor_id (1 byte)
    - delta_ms (1 byte) — sample.timestamp - base_timestamp (0..255)
    - size (1 byte)
    - payload (size bytes)

Notes:
- Delta encoding reduces repeated full timestamps.
- If a sample's delta would be >255, packetizer will break packet; if the first sample itself would exceed timing rules it is still accepted with base timestamp.
- Packets are capped to **200 bytes** (including header).

## Timestamps
- 32-bit monotonic milliseconds since boot (`platform_get_tick_ms()`).
- Wrap-around occurs at ~49 days — acceptable for many embedded use cases. If longer monotonic range is required, adopt 64-bit timestamps or epoch time.

## Ingest & buffer policy
- Static ring buffer with `RING_CAPACITY` slots (compile-time constant).
- Producer/consumer indices are 64-bit to avoid ambiguity on overflows.
- Drop-oldest policy: when buffer full, increment `cons_index` (discard oldest) and record `samples_dropped`.
- Registration API (`ingest_register_sensor` / `ingest_unregister_sensor`) guards which sensor IDs are accepted. `main.c` pre-registers IDs 0..MAX_SENSORS-1 by default.
- Incoming sample validation:
  - `size` must be between 2 and 6 bytes.
  - `sensor_id` must be < MAX_SENSORS.
  - Invalid samples increment `invalid_size_counter` (diagnostic).

## Concurrency / critical sections
- `process_incoming_sample` is designed to be callable from ISR or driver callback.
- Critical section (platform_enter_critical/platform_exit_critical) is used only for:
  - Checking registration.
  - Reserving slot and publishing by advancing `prod_index`.
  - Updating shared stats.
- Small payload sizes (≤6 bytes) justify a brief memcpy inside the critical region. If your platform requires shorter IRQ-disabled times, implement a per-slot valid flag or double-buffer strategy.

## Packetization strategy
- `packetizer_run_once()` aggregates oldest samples while:
  - Packet size remains ≤200 bytes.
  - Sample timestamp deltas fit in one byte (0..255 ms).
  - Age threshold not exceeded (default tuned to 10 ms in main to prioritize latency).
- If no samples are present, packetizer returns immediately.
- Enforces minimum inter-transmit spacing of 10 ms using `platform_get_tick_ms()` / `platform_wait_ms()`.

## Communication
- `comm_wrapper` calls `send_data_to_mcu2()` and does up to N retries (bounded) with small backoff.
- If the comm layer returns `COMM_STATUS_INVALID_PACKET_*`, the wrapper treats it as permanent failure and does not retry.

## Power & latency tradeoffs
- Idle policy:
  - Sleep `MIN_TX_INTERVAL_MS` (10 ms) when ring empty to conserve power.
  - Sleep 1 ms while ring non-empty to reduce latency.
- Age threshold controls how long packetizer waits to aggregate more samples (lower => lower latency, higher => better aggregation).
- Ring capacity should be chosen based on worst-case burst size and available RAM.

## Configuration knobs
- `RING_CAPACITY` (defs.h): adjust for memory budget and expected bursts.
- `DEFAULT_AGE_MS` / `packetizer_set_age_threshold_ms()`: tune aggregation vs latency.
- `COMM_WRAPPER_MAX_RETRIES` / `COMM_WRAPPER_BACKOFF_MS`: tune retry behavior.
- `MIN_TX_INTERVAL_MS`: minimum inter-transmit spacing.

## Porting notes
- Implement `platform_*` functions:
  - `platform_init()` must start a 1 ms tick counter or configure the RTOS tick.
  - `platform_get_tick_ms()` should be lock-free and cheap (prefer a volatile ms counter updated in SysTick).
  - Use the proper interrupts disable/enable for `platform_enter_critical()` / `platform_exit_critical()` on your MCU.
- Ensure `send_data_to_mcu2()` implements the wireless protocol and enforces the 200-byte limit.