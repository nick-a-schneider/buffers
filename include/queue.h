#pragma once

#include "block_allocator.h"
#include "buffer.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    Buffer* slot_buffer;
    uint16_t* msg_len;
    uint16_t slot_len;
} Queue;

Queue* queueAllocate(BlockAllocator* allocator, uint16_t slot_len, uint16_t size);
bool queueDeallocate(BlockAllocator* allocator, Queue** queue);

void queueClear(Queue* queue);

bool queueIsEmpty(const Queue* queue);
bool queueIsFull(const Queue* queue);
uint16_t queueWrite(Queue* queue, const uint8_t* data, uint16_t len);
uint16_t queueRead(Queue* queue, uint8_t* data, uint16_t len);
