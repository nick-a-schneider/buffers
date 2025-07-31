#include "arr_buffer.h"

#include "allocator.h"
#include <stdio.h>

ArrBuffer* arrBufferAllocate(Allocator* allocator, uint16_t len, uint16_t size) {
    if (!allocator) return NULL;
    if (len == 0 || size == 0) return NULL;
    ArrBuffer* buf = (ArrBuffer*)allocate(allocator, sizeof(ArrBuffer));
    if (!buf) return NULL;
    buf->raw = (uint8_t**)allocate(allocator, size * sizeof(uint8_t*));
    if (!(buf->raw)) {
        deallocate(allocator, buf);
        return NULL;
    }
    buf->used = (uint16_t*)allocate(allocator, size * sizeof(uint16_t));
    if (!(buf->used)) {
        deallocate(allocator, buf->raw);
        deallocate(allocator, buf);
        return NULL;
    }
    for (uint16_t i = 0; i < size; i++) {
        uint8_t* elem = (uint8_t*)allocate(allocator, len * sizeof(uint8_t));
        if (!(elem)) {
            for (uint16_t j = 0; j < i; j++) {
                deallocate(allocator, buf->raw[j]);
            }
            deallocate(allocator, buf->used);
            deallocate(allocator, buf->raw);
            deallocate(allocator, buf);
            return NULL;
        }
        buf->raw[i] = elem;
    }
    buf->arr_size = size;
    buf->len = len;
    buf->head = 0;
    buf->tail = 0;
    buf->full = false;
    return buf;
}

bool arrBufferDeallocate(Allocator* allocator, ArrBuffer** buffer) {
    if (!allocator) return false;
    if (!buffer || !(*buffer)) return false;
    bool res = true;
    for (uint16_t i = 0; i < (*buffer)->arr_size; i++) {
        res = deallocate(allocator, (*buffer)->raw[i]);
    }
    res = deallocate(allocator, (*buffer)->used);
    res = deallocate(allocator, (*buffer)->raw);
    res = deallocate(allocator, *buffer);
    *buffer = NULL;
    return res;
}

void arrBufferClear(ArrBuffer* buffer) {
    if (!buffer) return;
    for (uint16_t i = 0; i < buffer->arr_size; i++) {
        buffer->used[i] = 0;
    }
    buffer->head = 0;
    buffer->tail = 0;
    buffer->full = false;
}

bool arrBufferIsEmpty(const ArrBuffer* buffer) {
    return !buffer->full && buffer->head == buffer->tail;
}

bool arrBufferIsFull(const ArrBuffer* buffer) {
    return buffer->full;
}

uint16_t arrBufferWrite(ArrBuffer* buffer, const uint8_t* data, uint16_t len) {
    if (!buffer || !data || len == 0) return 0;
    if (buffer->full) return 0;
    uint16_t valid_len = (len < buffer->len) ? len : buffer->len;
    for (uint16_t i = 0; i < valid_len; i++) {
        buffer->raw[buffer->head][i] = data[i];
    }
    buffer->used[buffer->head] = valid_len;
    buffer->head = (uint16_t)((buffer->head + 1) % buffer->arr_size);
    if (buffer->head == buffer->tail) {
        buffer->full = true;
    }
    return valid_len;
}

uint16_t arrBufferRead(ArrBuffer* buffer, uint8_t* data, uint16_t len) {
    if (!buffer || !data || arrBufferIsEmpty(buffer)) return 0;
    if (buffer->used[buffer->tail] < len) {
        len = buffer->used[buffer->tail];
    }
    for (uint16_t i = 0; i < len; i++) {
        data[i] = buffer->raw[buffer->tail][i];
    }
    buffer->used[buffer->tail] = 0;
    buffer->tail = (uint16_t)((buffer->tail + 1) % buffer->arr_size);

    buffer->full = false;
    return len;
}
