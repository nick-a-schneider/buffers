#include "buffer.h"
#include "allocator.h"
#include <stdint.h>
#include <stdbool.h>

static inline void memcpy(void *dest, const void *src, uint16_t size) {
    for (uint16_t i = 0; i < size; ++i) {
        ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
    }
}

Buffer* bufferAllocate(Allocator* allocator, uint16_t size, uint16_t type_size) {
    if (!allocator) return NULL;
    if (size == 0 || type_size == 0) return NULL;
    Buffer* buf = (Buffer*)allocate(allocator, sizeof(Buffer));
    if (!buf) {
        return NULL; // Allocation failed
    }
    buf->raw = allocate(allocator, size * type_size);
    if (!(buf->raw)) {
        deallocate(allocator, buf);
        return NULL; // Allocation failed
    }
    buf->size = size;
    buf->type_size = type_size;
    buf->head = 0;
    buf->tail = 0;
    buf->full = false;
    return buf;
}

bool bufferDeallocate(Allocator* allocator, Buffer** buffer) {
    if (!allocator) return false;
    if (!buffer || !(*buffer)) return false;
    bool res = true;
    res &= deallocate(allocator, (*buffer)->raw);
    res &= deallocate(allocator, *buffer);
     if (res) *buffer = NULL;
    return res;
}

void __bufferMoveTail(Buffer* buffer, uint16_t offset) {
    if (!buffer) return;
    buffer->tail = (uint16_t)((buffer->tail + offset) % buffer->size);
}

void __bufferMoveHead(Buffer* buffer, uint16_t offset) {
    if (!buffer) return;
    buffer->head = (uint16_t)((buffer->head + offset) % buffer->size);
}

void bufferClear(Buffer* buffer) {
    if (buffer) {
        buffer->head = 0;
        buffer->tail = 0;
        buffer->full = false;
    }
}

bool bufferIsEmpty(const Buffer* buffer) {
    return !buffer->full && buffer->head == buffer->tail;
}

bool bufferIsFull(const Buffer* buffer) {
    return buffer->full;
}

bool bufferWrite(Buffer* buffer, const void* data) {
    if (!buffer) return false;
    if (!data) return false;
    if (buffer->full) return false;
    uint8_t* head_addr = (uint8_t*)buffer->raw + (buffer->head * buffer->type_size);
    memcpy((void*)head_addr, data, buffer->type_size);
    __bufferMoveHead(buffer, 1);
    if (buffer->head == buffer->tail) {
        buffer->full = true;
    }
    return true;
}

bool bufferRead(Buffer* buffer, void* data) {
    if (!buffer) return false;
    if (!data) return false;
    if (bufferIsEmpty(buffer)) return false;
    uint8_t* tail_addr = (uint8_t*)buffer->raw + (buffer->tail * buffer->type_size);
    memcpy(data, (void*)tail_addr, buffer->type_size);
    __bufferMoveTail(buffer, 1);
    buffer->full = false;
    return true;
}