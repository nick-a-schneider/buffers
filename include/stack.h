#include "block_allocator.h"
#include <stdint.h>
#include <stdbool.h>

#define STACK_OK 0

typedef struct {
    bool full;
    uint16_t size;
    uint16_t type_size;
    uint16_t top;
    void* raw;
} Stack;

Stack* stackAllocate(BlockAllocator* allocator, uint16_t size, uint16_t type_size);

int stackDeallocate(BlockAllocator* allocator, Stack** stack);

void stackClear(Stack* stack);

int stackPush(Stack* stack, const void* data);

int stackPop(Stack* stack, void* data);