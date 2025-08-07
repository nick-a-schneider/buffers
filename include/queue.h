#pragma once

#include "block_allocator.h"
#include "buffer.h"
#include <stdbool.h>
#include <stdint.h>

#define QUEUE_OK 0

/**
 * @brief Fixed-size message queue with variable-length messages.
 *
 * A `Queue` stores messages up to a maximum length per slot using a circular
 * buffer as its storage backend. It tracks the length of each message independently.
 */
typedef struct {
    Buffer* slot_buffer;   ///< Circular buffer storing message data
    uint16_t* msg_len;     ///< Array of lengths for each stored message
    uint16_t slot_len;     ///< Maximum message length per slot (in bytes)
} Queue;

/**
 * @brief Allocates and initializes a new message queue.
 *
 * @param allocator Pointer to a pre-initialized BlockAllocator.
 * @param slot_len  Maximum length (in bytes) of a single message.
 * @param size      Number of message slots to support.
 *
 * @return Pointer to a new Queue instance, or NULL on failure.
 */
Queue* queueAllocate(BlockAllocator* allocator, uint16_t slot_len, uint16_t size);

/**
 * @brief Deallocates a queue and all associated memory.
 *
 * @param allocator The allocator used for the original allocation.
 * @param queue Pointer to the Queue pointer; will be set to NULL on success.
 *
 * @return errno: [EINVAL, EFAULT, QUEUE_OK]
 */
int queueDeallocate(BlockAllocator* allocator, Queue** queue);

/**
 * @brief Clears the queue, resetting message lengths and positions.
 *
 * @param queue Pointer to the queue to clear.
 */
void queueClear(Queue* queue);

/**
 * @brief Returns true if the queue contains no messages.
 *
 * @param queue Pointer to the queue.
 * @return true if empty, false otherwise.
 */
bool queueIsEmpty(const Queue* queue);

/**
 * @brief Returns true if the queue is full and cannot accept new messages.
 *
 * @param queue Pointer to the queue.
 * @return true if full, false otherwise.
 */
bool queueIsFull(const Queue* queue);

/**
 * @brief Enqueues a message into the queue.
 *
 * @param queue Pointer to the queue.
 * @param data Pointer to the message to enqueue.
 * @param len  Length of the message in bytes.
 *
 * @return Number of bytes written on success, or a negative error code on failure.
 *
 * @note Messages longer than `slot_len` are truncated. Messages shorter than
 *       `slot_len` are stored as-is, and the exact length is tracked.
 */
int queueWrite(Queue* queue, const uint8_t* data, uint16_t len);

/**
 * @brief Reads (dequeues) the next message from the queue.
 *
 * @param queue Pointer to the queue.
 * @param data  Output buffer to store the message.
 * @param len   Maximum number of bytes to read.
 *
 * @return Number of bytes read on success, or a negative error code on failure.
 *
 * @note Only up to the actual message length (or `len`, whichever is smaller)
 *       is copied. The stored message is removed after reading.
 */
int queueRead(Queue* queue, uint8_t* data, uint16_t len);
