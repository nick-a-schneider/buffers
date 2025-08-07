/**
 * @file stack.h
 * @brief Lightweight, type-agnostic stack implementation with optional allocator support.
 *
 * This module provides a fixed-size stack abstraction for arbitrary data types, using
 * preallocated memory or an optional `BlockAllocator` for dynamic allocation. It is
 * designed for embedded systems or environments where memory management needs to be explicit.
 */

#include "block_allocator.h"
#include <stdint.h>
#include <stdbool.h>

#define STACK_OK 0

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
} Stack;

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
 * @return 0 on success, or a negative error code (e.g. if the stack is full).
 */
int stackPush(Stack* stack, const void* data);

/**
 * @brief Pop the top element from the stack.
 *
 * @param stack   Pointer to the stack.
 * @param data    Pointer to the memory where the popped data will be written.
 *                Must be `type_size` bytes long.
 * @return 0 on success, or a negative error code (e.g. if the stack is empty).
 */
int stackPop(Stack* stack, void* data);