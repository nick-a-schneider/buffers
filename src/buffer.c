#include "buffer.h"
#ifdef USE_BITMAP_ALLOCATOR
#include "block_allocator.h"
#endif
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
/* -- Private Functions --------------------------------------------------- */

/**
 * @brief Copies `size` bytes from `src` to `dest`.
 *
 * @param dest Destination buffer.
 * @param src Source buffer.
 * @param size Number of bytes to copy.
 *
 * @note
 * This function provides a minimal inline implementation of `memcpy` to avoid
 * relying on a standard library.
 */
static inline void memcpy(void *dest, const void *src, uint16_t size) {
    for (uint16_t i = 0; i < size; ++i) {
        ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
    }
}

/* -- Public Functions ----------------------------------------------------- */

#ifdef USE_BITMAP_ALLOCATOR
/**
 * @details
 * Allocates and initializes a circular buffer using the provided BlockAllocator.
 * Two allocations are performed:
 * - A `Buffer` structure to hold metadata
 * - A raw data array sized to `size * type_size`
 *
 * If allocation of the raw buffer fails, the previously allocated Buffer
 * structure is automatically deallocated.
 * 
 * @note
 * This function is only available if `USE_BITMAP_ALLOCATOR` is defined.
 */
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

/**
 * @details
 * Deallocates a Buffer and its associated raw data from the given BlockAllocator.
 * The pointer to the Buffer is set to NULL upon successful deallocation.
 * 
 * @note
 * This function is only available if `USE_BITMAP_ALLOCATOR` is defined.
 */
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
#endif

/**
 * @details
 * Resets the buffer state by setting head and tail indices to 0 and clearing
 * the `full` flag. The raw memory contents are not modified.
 */
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

/**
 * @details
 * Writes one element into the buffer at the current head index.
 * The element is copied from `data` into the buffer's raw memory region.
 *
 * After writing, the head index is advanced modulo `size`.
 * If the head wraps around and equals the tail, the buffer is marked as full.
 */
int bufferWrite(Buffer* buffer, const void* data) {
    if (!buffer || !data) return -EINVAL;
    if (buffer->full) return -ENOSPC;
    uint8_t* head_addr = (uint8_t*)buffer->raw + (buffer->head * buffer->type_size);
    memcpy((void*)head_addr, data, buffer->type_size);
    buffer->head = (uint16_t)((buffer->head + 1) % buffer->size);
    if (buffer->head == buffer->tail) {
        buffer->full = true;
    }
    return BUFFER_OK;
}

/**
 * @details
 * Reads one element from the buffer at the current tail index.
 * The element is copied into the memory pointed to by `data`.
 *
 * After reading, the tail index is advanced modulo `size`, and the buffer is
 * marked as not full.
 */
int bufferRead(Buffer* buffer, void* data) {
    if (!buffer  || !data) return -EINVAL;
    if (bufferIsEmpty(buffer)) return -EAGAIN;
    uint8_t* tail_addr = (uint8_t*)buffer->raw + (buffer->tail * buffer->type_size);
    memcpy(data, (void*)tail_addr, buffer->type_size);
    buffer->tail = (uint16_t)((buffer->tail + 1) % buffer->size);
    buffer->full = false;
    return BUFFER_OK;
}
