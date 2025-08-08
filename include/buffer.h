#pragma once 
/**
 * @file buffer.h
 * @brief Generic circular buffer structure.
 *
 * This header defines a generic, type-agnostic circular buffer structure,
 * designed to store arbitrary fixed-size elements in a contiguous memory block.
 *
 * The buffer operates in-place on user-provided memory and does not allocate
 * internally, although it may use a BlockAllocator for memory management. 
 */
#include "locking.h"
#ifdef USE_BITMAP_ALLOCATOR
#include "block_allocator.h"
#endif
#include <stdbool.h>
#include <stdint.h>

#define BUFFER_OK 0 // success

/**
 * @brief Creates a statically allocated circular buffer instance.
 *
 * @param name  The identifier for the buffer instance.
 * @param S     Number of elements the buffer can hold.
 * @param T     The data type of each element in the buffer.
 *
 * This macro defines a `Buffer` and its backing storage using static memory.
 */
#define CREATE_BUFFER(name, S, T)           \
    uint8_t name##_raw[(S) * (T)];          \
    CREATE_LOCK(__##name##_lock, S);        \
    Buffer name = {                         \
        .full = false,                      \
        .head = 0,                          \
        .tail = 0,                          \
        .size = (S),                        \
        .type_size = (T),                   \
        .raw = name##_raw,                  \
        .lock = &__##name##_lock,           \
    };                                      \

/**
 * @brief Circular FIFO buffer for fixed-size elements.
 */
typedef struct {
    bool full;                          ///< Indicates whether the buffer is full
    uint16_t head;                      ///< Index for next write
    uint16_t tail;                      ///< Index for next read
    uint16_t size;                      ///< Total number of elements the buffer can hold
    uint16_t type_size;                 ///< Size of each element in bytes
    void* raw;                          ///< Pointer to the raw memory backing the buffer
    Lock_t* lock;                       ///< Pointer to the lock structure
} Buffer;

#ifdef USE_BITMAP_ALLOCATOR
/**
 * @brief Allocates and initializes a new buffer.
 * 
 * @param allocator The BlockAllocator used to obtain memory for the Buffer and its storage.
 * @param size The number of elements the buffer can hold.
 * @param type_size The size of each element in bytes.
 *
 * @return A pointer to the initialized Buffer on success, or `NULL` if allocation fails
 * or if invalid parameters are provided.
 */
Buffer* bufferAllocate(BlockAllocator* allocator, uint16_t size, uint16_t type_size);

/**
 * @brief Deallocates a buffer and its storage.
 * 
 * @param allocator The BlockAllocator used for buffer memory management.
 * @param buffer Double pointer to the Buffer to deallocate.
 *
 * @return `BUFFER_OK` on success, or a negative errno value if:
 * - `-EINVAL` if any argument is NULL
 * - Any error propagated from `blockDeallocate`
 */
int bufferDeallocate(BlockAllocator* allocator, Buffer** buffer);
#endif

/**
 * @brief Resets the buffer without clearing contents.
 * 
 * @param buffer The Buffer to clear. No action is taken if NULL.
 */
void bufferClear(Buffer* buffer);

/**
 * @brief Checks if the buffer is empty.
 * 
 * @param buffer The Buffer to check.
 * @return `true` if the buffer is empty, `false` otherwise.
 */
bool bufferIsEmpty(const Buffer* buffer);

/**
 * @brief Checks if the buffer is full.
 * 
 * @param buffer The Buffer to check.
 * @return `true` if the buffer is full, `false` otherwise.
 */
bool bufferIsFull(const Buffer* buffer);

/**
 * @brief Writes one element to the buffer.
 * 
 * @param buffer The Buffer to write into.
 * @param data Pointer to the element to be written. Must have a size of `type_size`.
 *
 * @return slot index on success, or a negative errno value if:
 * - `-EINVAL` if arguments are invalid
 * - `-ENOSPC` if the buffer is full
 */
int bufferWrite(Buffer* buffer, const void* data);

/**
 * @brief Writes one element to the buffer, using a custom size.
 * 
 * @param buffer The Buffer to write into.
 * @param data Pointer to the element to be written.
 * @param size The size of the element to be written.
 *
 * @return slot index on success, or a negative errno value if:
 * - `-EINVAL` if arguments are invalid
 * - `-ENOSPC` if the buffer is full
 */
int bufferWriteRaw(Buffer* buffer, const void* data, uint16_t size);

/**
 * @brief Reads one element from the buffer.
 * 
 * @param buffer The Buffer to read from.
 * @param data Pointer to destination memory where the element will be copied.
 *
 * @return slot index on success, or a negative errno value if:
 * - `-EINVAL` if arguments are invalid
 * - `-EAGAIN` if the buffer is empty
 */
int bufferRead(Buffer* buffer, void* data);

/**
 * @brief Reads one element from the buffer with specified size.
 * 
 * @param buffer The Buffer to read from.
 * @param data Pointer to destination memory where the element will be copied.
 * @param size The size of the element to read.
 *
 * @return slot index on success, or a negative errno value if:
 * - `-EINVAL` if arguments are invalid
 * - `-EAGAIN` if the buffer is empty
 */
int bufferReadRaw(Buffer* buffer, void* data, uint16_t size);
