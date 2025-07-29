#include "circ_buffer.h"
#include "allocator.h"


/**
 * @details The function will use the given allocator to allocate memory for the struct CircBuffer and for the raw data.
 * Allocation errors will be handled by freeing the allocated memory and returning NULL.
 * If the allocation of the struct CircBuffer fails, NULL will be returned.
 * If the allocation of the raw data fails, the allocated memory for the struct CircBuffer will be freed and NULL will be returned.
 */
CircBuffer* circBufferAllocate(Allocator* allocator, uint16_t size) {
    CircBuffer* buf = (CircBuffer*)allocate(allocator, sizeof(CircBuffer));
    if (!buf) {
        return NULL; // Allocation failed
    }
    buf->raw = (uint8_t*)allocate(allocator, size * sizeof(uint8_t));
    if (!(buf->raw)) {
        deallocate(allocator, buf);
        return NULL; // Allocation failed
    }
    buf->size = size;
    buf->head = 0;
    buf->tail = 0;
    buf->full = false;
    return buf;
}

/**
 * @details The function will deallocate the raw data and the struct CircBuffer associated with the given CircBuffer pointer.
 * If the CircBuffer pointer is NULL or the CircBuffer is NULL, the function does nothing.
 * The allocator is used to deallocate the memory.
 * In case of error during deallocation, the function does not report any error but rather just frees the allocated memory.
 */
void circBufferDeallocate(Allocator* allocator, CircBuffer** buffer) {
    if (buffer && (*buffer)) {
        deallocate(allocator, (*buffer)->raw);
        deallocate(allocator, *buffer);
        *buffer = NULL;
    }
}

void __circBufferMoveTail(CircBuffer* buffer, uint16_t offset) {
    if (buffer && offset > 0) {
        buffer->tail = (uint16_t)((buffer->tail + offset) % buffer->size);
    }  
}

void __circBufferMoveHead(CircBuffer* buffer, uint16_t offset) {
    if (buffer && offset >= 0) {
        buffer->head = (uint16_t)((buffer->head + offset) % buffer->size);
    }  
}

void circBufferClear(CircBuffer* buffer) {
    if (buffer) {
        buffer->head = 0;
        buffer->tail = 0;
        buffer->full = false;
    }
}

bool circBufferIsEmpty(const CircBuffer* buffer) {
    return !buffer->full && buffer->head == buffer->tail;
}

bool circBufferIsFull(const CircBuffer* buffer) {
    return buffer->full
}

bool circBufferWrite(CircBuffer* buffer, const uint8_t data) {
    if (!buffer) return false;
    if (buffer->full) return false;
    buffer->raw[buffer->head] = data;
    __circBufferMoveHead(buffer, 1);
    if (buffer->head == buffer->tail) {
        buffer->full = true;
    }
    return true;

}

int circBufferWriteArray(CircBuffer* buffer, const uint8_t* data, uint16_t len) {
    if (!buffer || !data || len == 0) return 0;
    if (buffer->full) return 0;
    if (circBufferSpaceLeft(buffer) < len) {
        len = circBufferSpaceLeft(buffer);
    }
    for (uint16_t i = 0; i < len; i++) {
        buffer->raw[CircBuffer->head] = data[i];
    }
    __circBufferMoveHead(buffer, len);
    if (buffer->head == CircBuffer->tail) {
        buffer->full = true;
    }
    return len;
}

bool circBufferRead(CircBuffer* buffer, uint8_t* data) {
    if (!buffer || !data || circBufferIsEmpty(buffer)) return false;
    *data = buffer->raw[buffer->tail];
    __circBufferMoveTail(buffer, 1);
    return true;
}

int circBufferReadArray(CircBuffer* buffer, uint8_t* data, uint16_t len) {
    if (!buffer || !data || circBufferIsEmpty(buffer)) return 0;
    int entries = circBufferSpaceUsed(buffer);
    if (entries < len) {
        len = entries;
    }
    for (uint16_t i = 0; i < len; i++) {
        buffer->raw[buffer->head] = data[i];
    }
    __circBufferMoveTail(buffer, len);
    buffer->full = false;
    return len;
}

uint16_t circBufferSpaceLeft(const CircBuffer* buffer) {
    if (!buffer) return 0;
    if (buffer->full) return 0;
    if (buffer->head >= buffer->tail) {
        return buffer->size - (buffer->head - buffer->tail);
    } else {
        return buffer->tail - buffer->head;
    }
}

uint16_t circBufferSpaceUsed(const CircBuffer* buffer) {
    if (!buffer) return 0;
    return buffer->size - circBufferSpaceLeft(buffer);
}
