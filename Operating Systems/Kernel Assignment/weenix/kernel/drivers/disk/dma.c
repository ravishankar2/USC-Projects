#include "main/io.h"

#include "util/debug.h"
#include "util/string.h"
#include "util/delay.h"

#include "drivers/disk/dma.h"

#include "mm/pagetable.h"
#include "mm/page.h"

/* Linux kernel: drivers/ata/libata-sff.c */

#define DMA_BASE_PRIMARY   0xc000
#define DMA_BASE_SECONDARY (DMA_BASE_PRIMARY + 0x08)

#define DMA_COMMAND 0x00
#define DMA_STATUS  0x02
#define DMA_PRD     0x04 /* dword register */

/* Can clear interrupt and err by writing 1 to them */
#define DMA_SR_ACTIVE 0x01
#define DMA_SR_ERR    0x02
#define DMA_SR_INTR   0x04

#define DMA_CMD_START 0x01
#define DMA_CMD_READ  0x08
#define DMA_CMD_WRITE 0x00

#define dma_inb_reg(channel, reg) inb(DMA_BASES[(channel)] + (reg))
#define dma_inw_reg(channel, reg) inw(DMA_BASES[(channel)] + (reg))
#define dma_inl_reg(channel, reg) inl(DMA_BASES[(channel)] + (reg))
#define dma_outb_reg(channel, reg, data) \
        outb(DMA_BASES[(channel)] + (reg), (data))
#define dma_outw_reg(channel, reg, data) \
        outw(DMA_BASES[(channel)] + (reg), (data))
#define dma_outl_reg(channel, reg, data) \
        outl(DMA_BASES[(channel)] + (reg), (data))

/* Index by channel */
static uint16_t DMA_BASES[2] = { DMA_BASE_PRIMARY, DMA_BASE_SECONDARY };

typedef struct {
        uint32_t prd_addr;
        uint16_t prd_count;
        uint16_t prd_last;
} prd_t;

static prd_t prd_table[2] __attribute__((aligned(32)));

static prd_t *DMA_PRDS[2];

void
dma_init()
{
        /* Clear the table */
        memset(prd_table, 0, sizeof(prd_t) * 2);
        /* Set pointers to it; note each channel only needs one PRD entry */
        DMA_PRDS[0] = prd_table;
        DMA_PRDS[1] = prd_table + 1;

}

void
dma_load(uint8_t channel, void *start, int count, int write)
{
        KASSERT(PAGE_ALIGNED(start));
        memset(DMA_PRDS[channel], 0, 8);
        dma_reset(channel);
        DMA_PRDS[channel]->prd_addr = pt_virt_to_phys((uintptr_t) start);
        DMA_PRDS[channel]->prd_count = (uint16_t) count;
        DMA_PRDS[channel]->prd_last = (1 << 15);
        dma_outl_reg(channel, DMA_PRD,
                     pt_virt_to_phys((uintptr_t) DMA_PRDS[channel]));
        /* Write out the command's read/write code */
        dma_outb_reg(channel, DMA_COMMAND,
                     (write ? DMA_CMD_WRITE : DMA_CMD_READ));
}

uint8_t
dma_status(uint8_t channel)
{
        return inb(DMA_BASES[channel] + DMA_STATUS);
}

void
dma_reset(uint8_t channel)
{
        inb(DMA_BASES[channel] + DMA_STATUS);
        outb(DMA_BASES[channel] + DMA_STATUS, DMA_SR_ERR | DMA_SR_INTR);
        outb(DMA_BASES[channel] + DMA_COMMAND, 0);
}

void
dma_start(uint8_t channel)
{
        uint8_t cmd = dma_inb_reg(channel, DMA_COMMAND);
        dma_outb_reg(channel, DMA_COMMAND, cmd | DMA_CMD_START);
}

void
dma_stop(uint8_t channel)
{
        uint8_t cmd = dma_inb_reg(channel, DMA_COMMAND);
        dma_outb_reg(channel, DMA_COMMAND, cmd & ~DMA_CMD_START);
}
