#include "stack.h"
#include "block_allocator.h"
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

static inline void memcpy(void *dest, const void *src, uint16_t size) {
    for (uint16_t i = 0; i < size; ++i) {
        ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
    }
}

Stack* stackAllocate(BlockAllocator* allocator, uint16_t size, uint16_t type_size) {
    if (!allocator) return NULL;
    if (size == 0 || type_size == 0) return NULL;
    Stack* stack = (Stack*)blockAllocate(allocator, sizeof(Stack));
    if (!stack) return NULL;
    stack->raw = blockAllocate(allocator, size * type_size);
    if (!stack->raw) {
        blockDeallocate(allocator, stack);
        return NULL;
    }
    stack->type_size = type_size;
    stack->size = size;
    stack->top = 0;
    stack->full = false;
    return stack;
}

int stackDeallocate(BlockAllocator* allocator, Stack** stack) {
    if (!allocator || !stack || !(*stack)) return -EINVAL;
    int res1, res2 = STACK_OK;
    res1 = blockDeallocate(allocator, (*stack)->raw);
    res2 = blockDeallocate(allocator, *stack);
    if (res1 != STACK_OK) return res1;
    if (res2 != STACK_OK) return res2;
    *stack = NULL;
    return STACK_OK;
}

void stackClear(Stack* stack) {
    if (!stack) return;
    stack->top = 0;
    stack->full = false;
}

int stackPush(Stack* stack, const void* data) {
    if (!stack || !data) return -EINVAL;
    if (stack->top == stack->size) return -ENOSPC;
    uint8_t* head_addr = (uint8_t*)stack->raw + (stack->top * stack->type_size);
    memcpy((void*)head_addr, data, stack->type_size);
    stack->top += 1;
    return STACK_OK;
}

int stackPop(Stack* stack, void* data) {
    if (!stack || !data) return -EINVAL;
    if (stack->top == 0) return -EAGAIN;
    uint8_t* tail_addr = (uint8_t*)stack->raw + ((stack->top - 1) * stack->type_size);
    memcpy(data, (void*)tail_addr, stack->type_size);
    stack->top--;
    return STACK_OK;
}
