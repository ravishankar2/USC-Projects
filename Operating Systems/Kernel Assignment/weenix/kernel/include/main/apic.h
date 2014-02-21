#pragma once

#include "types.h"

/* Initializes the APIC using data from the ACPI tables.
 * ACPI handlers must be initialized before calling this
 * function. */
void apic_init();

/* Maps the given IRQ to the given interrupt number. */
void apic_setredir(uint32_t irq, uint8_t intr);

/* Initializes the APIC timer to count down from 'count' and
 * trigger interrupt 'intr' when completed. Counter is (effectively)
 * decremented by 1/'div' every bus clock tick. If 'periodic' is set
 * repeat periodically, otherwise just do it once. This function
 * should be accessed via wrappers in the interrupt subsystem.
 * Returns 0 on success, nonzero on failure.
 * Note that 'div' must be a power of 2 less than 2^8. */
int apic_starttimer(uint32_t count, unsigned int div, uint8_t intr, int periodic);

/* Get the current value of the timer. */
uint32_t apic_gettimer(void);

/* Set the current value of the timer. */
void apic_settimer(uint32_t count);

/* Sets the interrupt to raise when a spurious
 * interrupt occurs. */
void apic_setspur(uint8_t intr);

/* Sets the interrupt priority level. This function should
 * be accessed via wrappers in the interrupt subsystem. */
void apic_setipl(uint8_t ipl);

/* Gets the interrupt priority level. This function should
 * be accessed via wrappers in the interrupt subsystem. */
uint8_t apic_getipl();

/* Writes to the APIC's memory mapped end-of-interrupt
 * register to indicate that the handling of an interrupt
 * originating from the APIC has been finished. This function
 * should only be called from the interrupt subsystem. */
void apic_eoi();
