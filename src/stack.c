#include "stack.h"
#include "allocator.h"
#include <stdint.h>
#include <stdbool.h>

Stack* stackAllocate(Allocator* allocator, uint16_t size, uint16_t type_size) {
    if (!allocator) return NULL;
    if (size == 0 || type_size == 0) return NULL;
    Stack* stack = (Stack*)allocate(allocator, sizeof(Stack));
    if (!stack) return NULL;
    stack->raw = allocate(allocator, size * type_size);
    if (!stack->raw) {
        deallocate(allocator, stack);
        return NULL;
    }
    stack->type_size = type_size;
    stack->size = size;
    stack->top = 0;
    stack->full = false;
    return stack;
}

bool stackDeallocate(Allocator* allocator, Stack** stack) {
    if (!allocator) return false;
    if (!stack || !(*stack)) return false;
    bool res = true;
    res &= deallocate(allocator, (*stack)->raw);
    res &= deallocate(allocator, *stack);
    if (res) *stack = NULL;
    return res;
}

void stackClear(Stack* stack) {
    if (!stack) return;
    stack->top = 0;
    stack->full = false;
}

bool stackPush(Stack* stack, const void* data) {
    if (!stack || !data) return false;
    if (stack->top == stack->size) return false;
    uint8_t* head_addr = (uint8_t*)stack->raw + (stack->top * stack->type_size);
    memcpy((void*)head_addr, data, stack->type_size);
    stack->top += 1;
    return true;
}

bool stackPop(Stack* stack, void* data) {
    if (!stack || !data) return false;
    if (stack->top == 0) return false;
    uint8_t* tail_addr = (uint8_t*)stack->raw + ((stack->top - 1) * stack->type_size);
    memcpy(data, (void*)tail_addr, stack->type_size);
    stack->top--;
    return true;
}
