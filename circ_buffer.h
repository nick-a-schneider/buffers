#pragma once

#include "allocator.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool full;
    uint8_t* raw;
    uint16_t size;
    uint16_t head;
    uint16_t tail;
} CircBuffer;

#define CREATE_CIRC_BUFFER(...)  \
{                           \
    .raw = NULL,            \
    .size = 0,              \
    .head = 0,              \
    .tail = 0,              \
    .full = false,          \
    __VA_ARGS__             \
}

CircBuffer* circBufferAllocate(Allocator* allocator, uint16_t size);
void circBufferDeallocate(Allocator* allocator, CircBuffer** buffer);

void __circBufferMoveTail(CircBuffer* buffer, uint16_t offset);
void __circBufferMoveHead(CircBuffer* buffer, uint16_t offset);

void circBufferClear(CircBuffer* buffer);

bool circBufferIsEmpty(const CircBuffer* buffer);
bool circBufferIsFull(const CircBuffer* buffer);

bool circBufferWrite(CircBuffer* buffer, const uint8_t data);
bool circBufferRead(CircBuffer* buffer, uint8_t* data);
