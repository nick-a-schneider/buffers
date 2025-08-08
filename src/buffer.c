#include "buffer.h"
#include "locking.h"
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
    buf->lock = lockAllocate(allocator, size);
    if (!(buf->lock)) {
        blockDeallocate(allocator, buf->raw);
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
    int res1, res2, res3;
    res1 = lockDeallocate(allocator, (&(*buffer)->lock));
    res2 = blockDeallocate(allocator, (*buffer)->raw);
    res3 = blockDeallocate(allocator, *buffer);
    if (res1 != LOCK_OK) return res1;
    if (res2 != BLOCK_ALLOCATOR_OK) return res2;
    if (res3 != BLOCK_ALLOCATOR_OK) return res3;
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

int bufferWriteRaw(Buffer* buffer, const void* data, uint16_t size) {
    // ensure arguments are valid
    if (!buffer || !data) return -EINVAL;
    if (size == 0 || size > buffer->type_size) return -EINVAL;
    // claim the write lock or exit if busy
    if (!TAKE_WRITE_LOCK(buffer->lock)) {
        return -EBUSY;
    }
    // check if the buffer is full, release the lock if so
    if (buffer->full) {
        CLEAR_WRITE_LOCK(buffer->lock);
        return -ENOSPC;
    }
    // check if the current slot has already been claimed
    uint16_t cur_head = buffer->head;
    uint8_t slot_expected = BUFFER_FREE;
    if (!EXPECT_SLOT_STATE(buffer->lock, cur_head, &slot_expected, BUFFER_CLAIMED)) {
        CLEAR_WRITE_LOCK(buffer->lock);
        // another process has claimed the slot
        return -EBUSY;
    }
    // With all checks down, we can claim the slot
    uint8_t* head_addr = (uint8_t*)buffer->raw + (cur_head * buffer->type_size);
    // advance the head and mark if the buffer is now full
    buffer->head = (uint16_t)((cur_head + 1) % buffer->size);
    if (buffer->head == buffer->tail) {
        buffer->full = true;
    }
    // release the write lock so concurrent writers can claim a slot
    CLEAR_WRITE_LOCK(buffer->lock);
    // copy the data into the slot
    memcpy((void*)head_addr, data, size);
    // mark the slot as ready to be read
    SET_SLOT_STATE(buffer->lock, cur_head, BUFFER_READY);
    // exit
    return cur_head;
}

int bufferWrite(Buffer* buffer, const void* data) {
    if (!buffer || !data) return -EINVAL;
    return bufferWriteRaw(buffer, data, buffer->type_size);
}

/**
 * @details
 * Reads one element from the buffer at the current tail index.
 * The element is copied into the memory pointed to by `data`.
 *
 * After reading, the tail index is advanced modulo `size`, and the buffer is
 * marked as not full.
 */
int bufferReadRaw(Buffer* buffer, void* data, uint16_t size) {
    // ensure arguments are valid
    if (!buffer  || !data) return -EINVAL;
    if (size == 0 || size > buffer->type_size) return -EINVAL;
    // claim the read lock or exit if busy
    if (!TAKE_READ_LOCK(buffer->lock)) {
        return -EBUSY;
    }
    // check if the buffer is empty, release the lock if so
    if (bufferIsEmpty(buffer)) {
        CLEAR_READ_LOCK(buffer->lock);
        return -EAGAIN;
    }
    // check if the current slot has already been claimed
    uint16_t cur_tail = buffer->tail;
    uint8_t slot_expected = BUFFER_READY;
    if (!EXPECT_SLOT_STATE(buffer->lock, cur_tail, &slot_expected, BUFFER_READING)) {
        CLEAR_READ_LOCK(buffer->lock);
        // another process has claimed the slot
        return -EBUSY;
    }
    // With all checks down, we can claim the slot
    uint8_t* tail_addr = (uint8_t*)buffer->raw + (buffer->tail * buffer->type_size);
    buffer->tail = (uint16_t)((buffer->tail + 1) % buffer->size);
    buffer->full = false;
    // release the read lock so concurrent readers can claim a slot
    CLEAR_READ_LOCK(buffer->lock);
    // copy the data from the slot
    memcpy(data, (void*)tail_addr, size);
    // mark the slot as free
    SET_SLOT_STATE(buffer->lock, cur_tail, BUFFER_FREE);
    // exit
    return cur_tail;
}

int bufferRead(Buffer* buffer, void* data) {
    if (!buffer || !data) return -EINVAL;
    return bufferReadRaw(buffer, data, buffer->type_size);
}
