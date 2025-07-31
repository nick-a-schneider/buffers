#pragma once

#include "allocator.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool full;
    uint8_t** raw;
    uint16_t arr_size;
    uint16_t len;
    uint16_t* used;
    uint16_t head;
    uint16_t tail;
} ArrBuffer;

#define CREATE_ARR_BUFFER(...) {    \
    .raw = NULL,                    \
    .arr_size = 0,                  \
    .len = 0,                       \
    .used = NULL,                   \
    .head = 0,                      \
    .tail = 0,                      \
    .full = false,                  \
    __VA_ARGS__                     \
}

ArrBuffer* arrBufferAllocate(Allocator* allocator, uint16_t len, uint16_t size);
bool arrBufferDeallocate(Allocator* allocator, ArrBuffer** buffer);

void __arrBufferMoveTail(ArrBuffer* buffer, uint16_t offset);
void __arrBufferMoveHead(ArrBuffer* buffer, uint16_t offset);

void arrBufferClear(ArrBuffer* buffer);

bool arrBufferIsEmpty(const ArrBuffer* buffer);
bool arrBufferIsFull(const ArrBuffer* buffer);
uint16_t arrBufferWrite(ArrBuffer* buffer, const uint8_t* data, uint16_t len);
uint16_t arrBufferRead(ArrBuffer* buffer, uint8_t* data, uint16_t len);
