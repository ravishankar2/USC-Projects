#include "main/io.h"
#include "main/interrupt.h"
#include "util/delay.h"

/* IRQ */
#define PIT_IRQ 0

/* I/O ports */
#define PIT_DATA0 0x40
#define PIT_DATA1 0x41
#define PIT_DATA2 0x42
#define PIT_CMD   0x43

#define CLOCK_TICK_RATE 1193182
#undef HZ
#define HZ 1000

#define LATCH (CLOCK_TICK_RATE / HZ)

void pit_starttimer(uint8_t intr)
{
        intr_map(PIT_IRQ, intr);

        /* Shamelessly cribbed from "Understanding the Linux Kernel", pp 230 */
        outb(0x34, PIT_CMD);
        udelay(10);
        outb(LATCH & 0xff, PIT_DATA0);
        udelay(10);
        outb(LATCH >> 8, PIT_DATA0);
}
