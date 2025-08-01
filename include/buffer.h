#pragma once 

#include "allocator.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool full;
    void* raw;
    uint16_t size;
    uint16_t type_size;
    uint16_t head;
    uint16_t tail;
} Buffer;

Buffer* bufferAllocate(Allocator* allocator, uint16_t size, uint16_t type_size);

bool bufferDeallocate(Allocator* allocator, Buffer** buffer);

void __bufferMoveTail(Buffer* buffer, uint16_t offset);

void __bufferMoveHead(Buffer* buffer, uint16_t offset);

void bufferClear(Buffer* buffer);

bool bufferIsEmpty(const Buffer* buffer);

bool bufferWrite(Buffer* buffer, const void* data);

bool bufferRead(Buffer* buffer, void* data);
