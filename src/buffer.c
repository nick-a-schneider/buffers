#include "buffer.h"
#include "block_allocator.h"
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

static inline void memcpy(void *dest, const void *src, uint16_t size) {
    for (uint16_t i = 0; i < size; ++i) {
        ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
    }
}

Buffer* bufferAllocate(BlockAllocator* allocator, uint16_t size, uint16_t type_size) {
    if (!allocator) return NULL;
    if (size == 0 || type_size == 0) return NULL;
    Buffer* buf = (Buffer*)blockAllocate(allocator, sizeof(Buffer));
    if (!buf) {
        return NULL; // Allocation failed
    }
    buf->raw = blockAllocate(allocator, size * type_size);
    if (!(buf->raw)) {
        blockDeallocate(allocator, buf);
        return NULL; // Allocation failed
    }
    buf->size = size;
    buf->type_size = type_size;
    buf->head = 0;
    buf->tail = 0;
    buf->full = false;
    return buf;
}

int bufferDeallocate(BlockAllocator* allocator, Buffer** buffer) {
    if (!allocator || !buffer || !(*buffer)) return -EINVAL;
    int res1, res2;
    res1 = blockDeallocate(allocator, (*buffer)->raw);
    res2 = blockDeallocate(allocator, *buffer);
    if (res1 != BLOCK_ALLOCATOR_OK) return res1;
    if (res2 != BLOCK_ALLOCATOR_OK) return res2;
    *buffer = NULL;
    return BUFFER_OK;
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

int bufferWrite(Buffer* buffer, const void* data) {
    if (!buffer || !data) return -EINVAL;
    if (buffer->full) return -ENOSPC;
    uint8_t* head_addr = (uint8_t*)buffer->raw + (buffer->head * buffer->type_size);
    memcpy((void*)head_addr, data, buffer->type_size);
    __bufferMoveHead(buffer, 1);
    if (buffer->head == buffer->tail) {
        buffer->full = true;
    }
    return BUFFER_OK;
}

int bufferRead(Buffer* buffer, void* data) {
    if (!buffer  || !data) return -EINVAL;
    if (bufferIsEmpty(buffer)) return -EAGAIN;
    uint8_t* tail_addr = (uint8_t*)buffer->raw + (buffer->tail * buffer->type_size);
    memcpy(data, (void*)tail_addr, buffer->type_size);
    __bufferMoveTail(buffer, 1);
    buffer->full = false;
    return BUFFER_OK;
}
