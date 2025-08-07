#include "queue.h"
#include "buffer.h"
#include "block_allocator.h"
#include <errno.h>

/**
 * @details
 * Allocates memory for a Queue structure using the provided BlockAllocator.
 * Internally, it allocates:
 * - A circular buffer (`slot_buffer`) with `size` slots, each large enough to hold `slot_len` bytes.
 * - A parallel array (`msg_len`) to track the actual length of each stored message.
 *
 * If any allocation fails, all previously allocated structures are cleaned up.
 */
Queue* queueAllocate(BlockAllocator* allocator, uint16_t slot_len, uint16_t size) {
    if (!allocator) return NULL;
    if (slot_len == 0 || size == 0) return NULL;
    Queue* queue = (Queue*)blockAllocate(allocator, sizeof(Queue));
    if (!queue) return NULL;
    queue->slot_buffer = bufferAllocate(allocator, size, slot_len * sizeof(uint8_t));
    if (!(queue->slot_buffer)) {
        (void)blockDeallocate(allocator, queue);
        return NULL;
    }
    queue->msg_len = (uint16_t*)blockAllocate(allocator, size * sizeof(uint16_t));
    if (!(queue->msg_len)) {
        (void)bufferDeallocate(allocator, &queue->slot_buffer);
        (void)blockDeallocate(allocator, queue);
        return NULL;
    }
    queue->slot_len = slot_len;
    return queue;
}

/**
 * @details
 * Frees all memory associated with the queue using the original allocator.
 * This includes the slot buffer, the message length array, and the queue struct itself.
 * On success, sets the queue pointer to NULL.
 */
int queueDeallocate(BlockAllocator* allocator, Queue** queue) {
    if (!allocator ||!queue || !(*queue)) return -EINVAL;
    int res1, res2, res3;
    res1 = bufferDeallocate(allocator, (&(*queue)->slot_buffer));
    res2 = blockDeallocate(allocator, (*queue)->msg_len);
    res3 = blockDeallocate(allocator, *queue);
    if (res1 != BLOCK_ALLOCATOR_OK) return res1;
    if (res2 != BLOCK_ALLOCATOR_OK) return res2;
    if (res3 != BLOCK_ALLOCATOR_OK) return res3;
    *queue = NULL;
    return QUEUE_OK;
}


/**
 * @details
 * Resets the internal state of the queue, effectively clearing all messages.
 * - All stored message lengths are set to zero.
 * - The underlying circular buffer is cleared.
 * 
 * @note
 * Does not free or reallocate memory.
 */
void queueClear(Queue* queue) {
    if (!queue) return;
    for (uint16_t i = 0; i < queue->slot_buffer->size; i++) {
        queue->msg_len[i] = 0;
    }
    bufferClear(queue->slot_buffer);
}

/**
 * @details
 * Checks if the queue currently holds no messages.
 * 
 * Internally calls `bufferIsEmpty` on the underlying buffer.
 */
bool queueIsEmpty(const Queue* queue) {
    return bufferIsEmpty(queue->slot_buffer);
}

/**
 * @details
 * Checks if the queue has reached capacity and cannot accept more messages.
 * 
 * Internally calls `bufferIsFull` on the underlying buffer.
 */
bool queueIsFull(const Queue* queue) {
    return bufferIsFull(queue->slot_buffer);
}

/**
 * @details
 * Writes a new message to the queue.
 * - If the message length is less than `slot_len`, only the required bytes are written.
 * - If the message length exceeds `slot_len`, it is truncated.
 * - The original `type_size` of the buffer is temporarily modified to match the message length,
 *   allowing variable-length writes.
 * - The actual length written is stored in the `msg_len` array at the head index.
 * 
 * Returns the number of bytes written, or a negative error code on failure.
 */
int queueWrite(Queue* queue, const uint8_t* data, uint16_t len) {
    if (!queue || !data || len == 0) return -EINVAL;
    if (queueIsFull(queue)) return -ENOSPC;
    uint16_t orig_size  = queue->slot_buffer->type_size;

    if (len < queue->slot_len){
        queue->slot_buffer->type_size = len * sizeof(uint8_t);
    }
    if (len > queue->slot_len) len = queue->slot_len;
    uint16_t head = queue->slot_buffer->head;
    int res = bufferWrite(queue->slot_buffer, (void*)data);
    queue->slot_buffer->type_size = orig_size;
    if (res < BUFFER_OK) return res;
    queue->msg_len[head] = len;
    return len;
}

/**
 * @details
 * Reads the next message from the queue into the provided buffer.
 * - The actual length of the message is retrieved from the `msg_len` array.
 * - Only up to `len` bytes are copied into the output buffer; remaining bytes are discarded.
 * - As in `queueWrite`, the buffer's `type_size` is temporarily adjusted to match the read size.
 * - After reading, the slot is cleared and marked available.
 * 
 * Returns the number of bytes read, or a negative error code if the queue is empty or input is invalid.
 */
int queueRead(Queue* queue, uint8_t* data, uint16_t len) {
    if (!queue || !data || len == 0) return -EINVAL;
    if (queueIsEmpty(queue)) return -EAGAIN;
    uint16_t tail_len = queue->msg_len[queue->slot_buffer->tail];
    len = (len < tail_len) ? len : tail_len;
    uint16_t orig_size  = queue->slot_buffer->type_size;
    if (len < queue->slot_len) {
        queue->slot_buffer->type_size = len * sizeof(uint8_t);
    }
    uint16_t tail = queue->slot_buffer->tail;
    int res = bufferRead(queue->slot_buffer, (void*)data);
    queue->slot_buffer->type_size = orig_size;
    queue->msg_len[tail] = 0;
    if (res < BUFFER_OK) return res;
    return len;
}
