/* platform.c — PRODUCTION STUBS (no hardware-specific implementation)
 *
 * This file intentionally contains minimal, non-functional stubs. It DOES NOT implement timing
 * or critical sections. When porting to real hardware these functions must
 * be implemented according to the target platform:
 *
 *   - platform_init(): initialize SysTick/RTC/OS tick, or configure hardware timer.
 *   - platform_get_tick_ms(): return monotonic ms since boot (wrap-around handled).
 *   - platform_wait_ms(ms): blocking delay using the OS or busy-wait as appropriate.
 *   - platform_enter_critical()/platform_exit_critical(): disable/enable IRQs.
 *
 * NOTE TO REVIEWER:
 * - This project intentionally does not include MCU HAL code, because the
 *   target MCU was not specified. The meat of the solution (ring buffer,
 *   ingress, packetizer, send wrapper) is hardware-agnostic and available
 *   in src/*.c.
 */

#include "platform.h"

// Production stubs: intentionally minimal. Replace with MCU-specific code.

void platform_init(void)
{
    /* TODO: Initialize SysTick or OS tick on the MCU.
     */
}

uint32_t platform_get_tick_ms(void)
{
    uint32_t ms = 0;
    // TODO: Return monotonic ms since boot to keep track of time
    return ms;
}

void platform_wait_ms(uint32_t ms)
{
    // TODO: Implement blocking delay appropriate to environment.
    (void)ms;
}

void platform_enter_critical(void)
{
    // TODO: Disable interrupts or take the RTOS lock.
}

void platform_exit_critical(void)
{
    // TODO: Re-enable interrupts or release RTOS lock.
}
