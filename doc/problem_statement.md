# Sensor Data Interface — Problem Statement

## Overview
Design and implement firmware for **MCU1** to efficiently handle and transmit data from multiple sensors to **MCU2**. The system must adhere to strict constraints and optimize performance.

## System Description
- **MCU1** collects data from up to **10 sensors**, which may connect or disconnect dynamically.  
- Sensor data is sent at varying intervals (**5 ms to 1000 ms**) and sizes (**2 to 6 bytes**).  
- **MCU1** transmits the collected data to **MCU2** over a wireless communication protocol with constraints:
  - **Packet Size Limit:** 200 bytes  
  - **Transmission Interval:** Minimum 10 ms between packets  

## Requirements
1. Receive sensor data through callback functions.  
2. Store the sensor data and prepare packets for transmission.  
3. Transmit data to MCU2 using a provided function.  
4. Ensure:
   - Each sample includes sensor ID + timestamp  
   - Optimized latency and throughput  
   - Minimal power consumption and memory usage  

## Assumptions
- Sensor data arrives through a callback function (`sensor_data_callback`) — implementation not required.  
- A function for sending data to MCU2 (`send_data_to_mcu2`) is provided — implementation not required.  

## Deliverables
- Firmware implementation for MCU1 meeting all requirements.  
- Clean and maintainable code using only static allocation.  
- Documentation of design tradeoffs, packet format, and testing strategy.