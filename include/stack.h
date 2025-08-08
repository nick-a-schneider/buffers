#pragma once
/**
 * @file stack.h
 * @brief Lightweight, type-agnostic stack implementation with optional allocator support.
 *
 * This module provides a fixed-size stack abstraction for arbitrary data types, using
 * preallocated memory or an optional `BlockAllocator` for dynamic allocation. It is
 * designed for embedded systems or environments where memory management needs to be explicit.
 */

#include "locking.h"
#ifdef USE_BITMAP_ALLOCATOR
#include "block_allocator.h"
#endif
#include <stdint.h>
#include <stdbool.h>

#define STACK_OK 0 // success

// Stack lock macros, reuse lock->write as single lock
#define TAKE_STACK_LOCK(lock) TAKE_WRITE_LOCK(lock)
#define CLEAR_STACK_LOCK(lock) CLEAR_WRITE_LOCK(lock)

/**
 * @brief Creates a statically allocated stack instance.
 *
 * @param id         The identifier for the stack instance.
 * @param type_size_  The size in bytes of the data type to be stored.
 * @param count      The number of elements the stack can hold.
 *
 * This macro creates a `Stack` variable and backing storage using static memory.
 */
#define CREATE_STACK(id, count, type_size_)                        \
    uint8_t __##id##_raw[(count) * (type_size_)] = {0};            \
    CREATE_LOCK(id##_lock, count);                                 \
    Stack id = {                                                   \
        .raw = __##id##_raw,                                       \
        .type_size = (type_size_),                                 \
        .size = (count),                                           \
        .top = 0,                                                  \
        .full = false,                                             \
        .lock = &id##_lock                                         \
    }
    
/**
 * @struct Stack
 * @brief Data structure representing a generic fixed-size stack.
 *
 * The stack tracks its current position (`top`), capacity (`size`),
 * element size (`type_size`), and whether it is full.
 */
typedef struct {
    bool full;          /**< True if the stack is full. */
    uint16_t size;      /**< Maximum number of elements. */
    uint16_t type_size; /**< Size of each element in bytes. */
    uint16_t top;       /**< Current top index. */
    void* raw;          /**< Pointer to backing storage. */
    Lock_t* lock;       /**< Pointer to the lock structure. */
} Stack;

#ifdef USE_BITMAP_ALLOCATOR
/**
 * @brief Allocate a stack dynamically using a BlockAllocator.
 *
 * @param allocator   Pointer to a valid BlockAllocator instance.
 * @param size        Maximum number of elements the stack should hold.
 * @param type_size   Size in bytes of the element type to store.
 * @return Pointer to the newly allocated stack, or NULL on failure.
 */
Stack* stackAllocate(BlockAllocator* allocator, uint16_t size, uint16_t type_size);

/**
 * @brief Deallocate a stack allocated via `stackAllocate`.
 *
 * @param allocator   Pointer to the same BlockAllocator used to allocate the stack.
 * @param stack       Address of the pointer to the stack to deallocate.
 *                    The pointer will be set to NULL on success.
 * @return 0 on success, or an error code on failure.
 */
int stackDeallocate(BlockAllocator* allocator, Stack** stack);
#endif

/**
 * @brief Clear the contents of a stack without freeing memory.
 *
 * @param stack   Pointer to the stack to clear.
 */
void stackClear(Stack* stack);

/**
 * @brief Push an element onto the stack.
 *
 * @param stack   Pointer to the stack.
 * @param data    Pointer to the data to push. Must be `type_size` bytes long.
 * @return stack index, or a negative error code (e.g. if the stack is full).
 */
int stackPush(Stack* stack, const void* data);

/**
 * @brief Pop the top element from the stack.
 *
 * @param stack   Pointer to the stack.
 * @param data    Pointer to the memory where the popped data will be written.
 *                Must be `type_size` bytes long.
 * @return stack index, or a negative error code (e.g. if the stack is empty).
 */
int stackPop(Stack* stack, void* data);