#include "types.h"

#include "main/io.h"
#include "main/acpi.h"

#include "mm/page.h"
#include "mm/pagetable.h"

#include "util/debug.h"

#define APIC_SIGNATURE (*(uint32_t*)"APIC")

#define TYPE_LAPIC (0)
#define TYPE_IOAPIC (1)

#define PORT_PIC1 0x20
#define PORT_PIC2 0xa0

#define IOREGSEL(table) (*(volatile uint32_t*)((table)->at_addr))
#define IOWIN(table) (*(volatile uint32_t*)((table)->at_addr+0x10))

#define IOAPICID(table) ((table)->at_inti)
#define IOAPICVER(table) ((table)->at_inti+0x01)
#define IOAPICARB(table) ((table)->at_inti+0x02)
#define IOREDTBL(table,index,part) ((table)->at_inti+0x10+(2*(index))+(part))

#define LAPICID (*(volatile uint32_t*)(apic->at_addr + 0x20))
#define LAPICVER (*(volatile uint32_t*)(apic->at_addr + 0x30))
#define LAPICEOI (*(volatile uint32_t*)(apic->at_addr + 0xb0))
#define LAPICSPUR (*(volatile uint32_t*)(apic->at_addr + 0xf0))
#define LAPICTPR (*(volatile uint32_t*)(apic->at_addr + 0x80))
#define LAPICERR (*(volatile uint32_t*)(apic->at_addr + 0x280))

#define LAPICTIMER (*(volatile uint32_t*)(apic->at_addr + 0x320))
#define LAPICINITCNT (*(volatile uint32_t*)(apic->at_addr + 0x380))
#define LAPICCURCNT (*(volatile uint32_t*)(apic->at_addr + 0x390))
#define LAPICDIVCONF (*(volatile uint32_t*)(apic->at_addr + 0x3e0))

#define BIT_SET(data,bit) do { (data) = ((data)|(0x1<<(bit))); } while(0);
#define BIT_UNSET(data,bit) do { (data) = ((data)&~(0x1<<(bit))); } while(0);

struct apic_table {
        struct acpi_header at_header;
        uint32_t at_addr;
        uint32_t at_flags;
};

struct lapic_table {
        uint8_t at_type;
        uint8_t at_size;
        uint8_t at_procid;
        uint8_t at_apicid;
        uint32_t at_flags;
};

struct ioapic_table {
        uint8_t at_type;
        uint8_t at_size;
        uint8_t at_apicid;
        uint8_t at_reserved;
        uint32_t at_addr;
        uint32_t at_inti;
};

static struct apic_table *apic = NULL;
static struct lapic_table *lapic = NULL;
static struct ioapic_table *ioapic = NULL;

static uint32_t __ioapic_getid(void)
{
        IOREGSEL(ioapic) = IOAPICID(ioapic);
        uint32_t id = IOWIN(ioapic);
        return (id >> 24) & 0x0f;
}

static uint32_t __lapic_getid(void)
{
        uint32_t id = LAPICID;
        return (id >> 24) & 0x0f;
}

static uint32_t __lapic_getver(void)
{
        uint32_t ver = LAPICVER;
        return ver & 0xff;
}

static void __lapic_setspur(uint8_t intr)
{
        uint32_t data = LAPICSPUR;
        ((uint8_t *)&data)[0] = intr;
        LAPICSPUR = data;
}

static uint32_t __ioapic_getver()
{
        IOREGSEL(ioapic) = IOAPICVER(ioapic);
        uint32_t ver = IOWIN(ioapic);
        return ver & 0xff;
}

static uint32_t __ioapic_getmaxredir()
{
        IOREGSEL(ioapic) = IOAPICVER(ioapic);
        uint32_t ver = IOWIN(ioapic);
        return (ver >> 16) & 0xff;
}

static void __ioapic_setredir(uint32_t irq, uint8_t intr)
{
        IOREGSEL(ioapic) = IOREDTBL(ioapic, irq, 0);
        uint32_t data = IOWIN(ioapic);
        ((uint8_t *)&data)[0] = intr;
        BIT_UNSET(data, 8);
        BIT_UNSET(data, 9);
        BIT_UNSET(data, 10);
        BIT_UNSET(data, 11);
        BIT_UNSET(data, 13);
        BIT_UNSET(data, 15);
        IOWIN(ioapic) = data;

        IOREGSEL(ioapic) = IOREDTBL(ioapic, irq, 1);
        data = IOWIN(ioapic);
        ((uint8_t *)&data)[3] = lapic->at_apicid;
        IOWIN(ioapic) = data;
}

static void __ioapic_setmask(uint32_t irq, int mask)
{
        IOREGSEL(ioapic) = IOREDTBL(ioapic, irq, 0);
        uint32_t data = IOWIN(ioapic);
        if (mask) {
                BIT_SET(data, 16);
        } else {
                BIT_UNSET(data, 16);
        }
        IOWIN(ioapic) = data;
}

uint8_t apic_getipl()
{
        return LAPICTPR & 0xff;
}

void apic_setipl(uint8_t ipl)
{
        LAPICTPR = ipl;
}

void apic_init()
{
        uint8_t *ptr = acpi_table(APIC_SIGNATURE, 0);
        apic = (struct apic_table *)ptr;
        KASSERT(NULL != apic && "APIC table not found in ACPI."
                "If you are using Bochs make sure you configured --enable-apic.");

        dbgq(DBG_CORE, "--- APIC INIT ---\n");
        dbgq(DBG_CORE, "local apic paddr:     0x%x\n", apic->at_addr);
        dbgq(DBG_CORE, "PC-AT compatible:    %i\n", apic->at_flags & 0x1);
        KASSERT(PAGE_ALIGNED(apic->at_addr));
        apic->at_addr = pt_phys_perm_map(apic->at_addr, 1);

        /* Get the tables for the local APIC and IO APICS,
         * Weenix currently only supports one of each, in order
         * to enforce this a KASSERT will fail this if more than one
         * of each type is found */
        uint8_t off = sizeof(*apic);
        while (off < apic->at_header.ah_size) {
                uint8_t type = *(ptr + off);
                uint8_t size = *(ptr + off + 1);
                if (TYPE_LAPIC == type) {
                        KASSERT(sizeof(struct lapic_table) == size);
                        KASSERT(NULL == lapic && "Weenix only supports a single local APIC");
                        lapic = (struct lapic_table *)(ptr + off);
                        dbgq(DBG_CORE, "LAPIC:\n");
                        dbgq(DBG_CORE, "   id:         0x%.2x\n", (uint32_t)lapic->at_apicid);
                        dbgq(DBG_CORE, "   processor:  0x%.3x\n", (uint32_t)lapic->at_procid);
                        dbgq(DBG_CORE, "   enabled:    %i\n", lapic->at_flags & 0x1);
                        KASSERT(lapic->at_flags & 0x1 && "The local APIC is disabled");
                } else if (TYPE_IOAPIC == type) {
                        KASSERT(sizeof(struct ioapic_table) == size);
                        KASSERT(NULL == ioapic && "Weenix only supports a single IO APIC");
                        ioapic = (struct ioapic_table *)(ptr + off);
                        dbgq(DBG_CORE, "IOAPIC:\n");
                        dbgq(DBG_CORE, "   id:         0x%.2x\n", (uint32_t)ioapic->at_apicid);
                        dbgq(DBG_CORE, "   base paddr:  0x%.8x\n", ioapic->at_addr);
                        dbgq(DBG_CORE, "   inti addr:   0x%.8x\n", ioapic->at_inti);
                        KASSERT(PAGE_ALIGNED(ioapic->at_addr));
                        ioapic->at_addr = pt_phys_perm_map(ioapic->at_addr, 1);
                } else {
                        dbgq(DBG_CORE, "Unknown APIC type:  0x%x\n", (uint32_t)type);
                }
                off += size;
        }
        KASSERT(NULL != lapic && "Could not find a local APIC device");
        KASSERT(NULL != ioapic && "Could not find an IO APIC");

        LAPICSPUR = LAPICSPUR | 0x100;
        dbgq(DBG_CORE, "Local APIC 0x%.2x Configuration:\n", __lapic_getid());
        dbgq(DBG_CORE, "    APIC Version:         0x%.2x\n", __lapic_getver());
        dbgq(DBG_CORE, "    Spurious Vector:      0x%.8x\n", LAPICSPUR);

        dbgq(DBG_CORE, "IO APIC 0x%.2x Configuration:\n", __ioapic_getid());
        dbgq(DBG_CORE, "    APIC Version:         0x%.2x\n", __ioapic_getver());
        dbgq(DBG_CORE, "    Maximum Redirection:  0x%.2x\n", __ioapic_getmaxredir());

        /* disable 8259 PICs by initializing them and masking all interrupts */
        outb(PORT_PIC1, 0x11); /* start initialization */
        outb(PORT_PIC2, 0x11);
        outb(PORT_PIC1 + 1, 0x20); /* remap interrupt vectors */
        outb(PORT_PIC2 + 1, 0x28);
        outb(PORT_PIC1 + 1, 4); /* continue initialization */
        outb(PORT_PIC2 + 1, 2);
        outb(PORT_PIC1 + 1, 0x01); /* finish initialization */
        outb(PORT_PIC2 + 1, 0x01);
        outb(PORT_PIC1 + 1, 0xff); /* mask all IRQS on the PIC */
}

int apic_starttimer(uint32_t count, unsigned int div, uint8_t intr, int periodic)
{
        switch (div) {
                case 2:   LAPICDIVCONF = 0b0000; break;
                case 4:   LAPICDIVCONF = 0b0001; break;
                case 8:   LAPICDIVCONF = 0b0010; break;
                case 16:  LAPICDIVCONF = 0b0011; break;
                case 32:  LAPICDIVCONF = 0b1000; break;
                case 64:  LAPICDIVCONF = 0b1001; break;
                case 128: LAPICDIVCONF = 0b1010; break;
                case 1:   LAPICDIVCONF = 0b1011; break;
                default:
                        dbg(DBG_CORE, "Invalid LAPIC timer divide configuration %u\n", div);
                        return -1;
        }
        LAPICTIMER = ((uint32_t)intr) | (periodic ? (1 << 17) : 0);
        LAPICINITCNT = count;
        uint32_t err;
        if (0 != (err = LAPICERR)) {
                dbg(DBG_CORE, "APIC error: 0x%08x\n", err);
                return -err;
        }
        return 0;
}

uint32_t apic_gettimer()
{
        return LAPICCURCNT;
}

void apic_settimer(uint32_t count)
{
        LAPICINITCNT = count;
}

void apic_setredir(uint32_t irq, uint8_t intr)
{
        dbg(DBG_CORE, "redirecting irq %u to interrupt %hhu\n", irq, intr);
        __ioapic_setredir(irq, intr);
        __ioapic_setmask(irq, 0);
}

void apic_setspur(uint8_t intr)
{
        dbg(DBG_CORE, "mapping spurious interrupts to %hhu\n", intr);
        __lapic_setspur(intr);
}

void apic_eoi()
{
        LAPICEOI = 0x0;
}
