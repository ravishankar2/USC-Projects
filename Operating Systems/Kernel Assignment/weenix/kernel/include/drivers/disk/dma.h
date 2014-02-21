#pragma once

/**
 * Initializes the DMA subsystem.
 */
void dma_init(void);

/**
 * Return the DMA status.
 *
 * @param channel the disk channel
 */
uint8_t dma_status(uint8_t channel);

/**
 * Resets DMA for its next operation by acknowledging an interrupt,
 * clearing all interrupts and errors.
 *
 * @param channel the disk channel
 */
void dma_reset(uint8_t channel);

/**
 * Initialize DMA for an operation
 *
 * @param channel the channel on which to perform the operation
 * @param start the beginning of the buffer in memory
 * @param count the number of bytes to read/write
 * @param write true if writing, false if reading
 */
void dma_load(uint8_t channel, void *start, int count, int write);

/**
 * Cancel the current DMA operation.
 *
 * @param channel the disk channel
 */
void dma_stop(uint8_t channel);

/**
 * Execute a DMA operation.
 *
 * @param channel the disk channel
 */
void dma_start(uint8_t channel);
