# Sensor Data Interface (MCU1 to MCU2 Firmware)

## Overview
This project implements firmware for **MCU1** to ingest sensor data, buffer it safely, and transmit it to **MCU2** under strict real-time constraints.

The design demonstrates:
- Low-latency packetization  
- Bounded memory usage (static allocation only)  
- Robust handling of dynamic sensor connections  
- Deterministic throughput under timing constraints  

---

## Key Constraints
- Max sensors: **10**  
- Sensor payload: **2–6 bytes**  
- Sample intervals: **5–1000 ms**  
- Max packet size: **200 bytes**  
- Min TX interval: **10 ms**  
- Data must include sensor ID and timestamp  

---

## Architecture
- **Ingest (ISR-safe):** Static ring buffer with drop-oldest policy if full  
- **Packetizer:** Base timestamp plus delta encoding (0–255 ms) for efficient packing  
- **Comm wrapper:** Enforces 200-byte max, retries with backoff  
- **Sensor manager:** Dynamic connect and disconnect tracking  
- **Main loop:** Runs packetizer and transmission while applying power-saving idle policy  

---

## Packet Format
Header (4 bytes):
  - Sequence (2 bytes)
  - Sample Count (1 byte)
  - Flags (1 byte)

Payload:
  - Base Timestamp (4 bytes)
  - For each sample:
      - Sensor ID (1 byte)
      - Delta ms (1 byte)
      - Size (1 byte)
      - Payload (2–6 bytes)
Delta encoding minimizes overhead while preserving timing accuracy.

---

## Design Choices
- **Static allocation** → deterministic RAM use, no fragmentation  
- **Drop-oldest policy** → prioritizes newest sensor data in real-time monitoring  
- **Delta timestamps** → reduce bandwidth while retaining timing fidelity  
- **Configurable knobs** → buffer size, age threshold, retry policy  

---

