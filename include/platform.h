#ifndef PLATFORM_H_
#define PLATFORM_H_

/* Platform abstraction header (production stubs).
 *
 * NOTE:
 *  - This submission is hardware-agnostic. The platform_* functions below
 *    are declared here; minimal production stubs are provided in platform.c.
 */

#include <stdint.h>

// Initialize timekeeping. Call once at startup.
void platform_init(void);

// Return a monotonic millisecond tick.
uint32_t platform_get_tick_ms(void);

// Blocking wait for approximately 'ms' milliseconds for low power.

void platform_wait_ms(uint32_t ms);

// Enter/exit small critical section where you don't want interrupts to go off.
void platform_enter_critical(void);
void platform_exit_critical(void);

#endif /* PLATFORM_H_ */
