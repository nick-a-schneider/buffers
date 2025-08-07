#include "stack.h"
#include "block_allocator.h"
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

/**
 * @details
 * Allocates a stack structure and backing storage from the provided BlockAllocator.
 * - Allocates memory for the stack structure.
 * - Allocates a raw buffer of `size * type_size` bytes to store values.
 * - Initializes internal fields such as `type_size`, `size`, `top`, and `full`.
 *
 * If any allocation fails, all intermediate allocations are cleaned up to avoid leaks.
 */
Stack* stackAllocate(BlockAllocator* allocator, uint16_t size, uint16_t type_size) {
    if (!allocator) return NULL;
    if (size == 0 || type_size == 0) return NULL;
    Stack* stack = (Stack*)blockAllocate(allocator, sizeof(Stack));
    if (!stack) return NULL;
    stack->raw = blockAllocate(allocator, size * type_size);
    if (!stack->raw) {
        (void)blockDeallocate(allocator, stack);
        return NULL;
    }
    stack->type_size = type_size;
    stack->size = size;
    stack->top = 0;
    stack->full = false;
    return stack;
}

/**
 * @details
 * Frees all memory associated with the stack using the provided BlockAllocator.
 * - Frees the underlying data buffer first.
 * - Then frees the stack structure itself.
 * - On success, the caller's stack pointer is set to NULL.
 *
 * If any deallocation fails, the corresponding error code is returned.
 */
int stackDeallocate(BlockAllocator* allocator, Stack** stack) {
    if (!allocator || !stack || !(*stack)) return -EINVAL;
    int res1, res2;
    res1 = blockDeallocate(allocator, (*stack)->raw);
    res2 = blockDeallocate(allocator, *stack);
    if (res1 != BLOCK_ALLOCATOR_OK) return res1;
    if (res2 != BLOCK_ALLOCATOR_OK) return res2;
    *stack = NULL;
    return STACK_OK;
}


/**
 * @details
 * Clears the contents of the stack without releasing memory.
 * - Resets the `top` index to 0.
 * - Clears the `full` flag.
 *
 * This effectively resets the stack to an empty state.
 */
void stackClear(Stack* stack) {
    if (!stack) return;
    stack->top = 0;
    stack->full = false;
}

/**
 * @details
 * Pushes a new item onto the top of the stack.
 * - Fails with `-ENOSPC` if the stack is already full (`top == size`).
 * - Writes `type_size` bytes from `data` into the appropriate offset in the raw buffer.
 * - Increments the `top` index.
 *
 * This function assumes a fixed-size item and does not manage dynamic types or metadata.
 */
int stackPush(Stack* stack, const void* data) {
    if (!stack || !data) return -EINVAL;
    if (stack->top == stack->size) return -ENOSPC;
    uint8_t* head_addr = (uint8_t*)stack->raw + (stack->top * stack->type_size);
    memcpy((void*)head_addr, data, stack->type_size);
    stack->top += 1;
    return STACK_OK;
}

/**
 * @details
 * Pops the item at the top of the stack into the provided `data` pointer.
 * - Fails with `-EAGAIN` if the stack is empty (`top == 0`).
 * - Copies `type_size` bytes from the current top index (minus 1) to the output buffer.
 * - Decrements the `top` index after copying.
 *
 * Caller must ensure `data` points to a buffer of sufficient size.
 */
int stackPop(Stack* stack, void* data) {
    if (!stack || !data) return -EINVAL;
    if (stack->top == 0) return -EAGAIN;
    uint8_t* tail_addr = (uint8_t*)stack->raw + ((stack->top - 1) * stack->type_size);
    memcpy(data, (void*)tail_addr, stack->type_size);
    stack->top--;
    return STACK_OK;
}
