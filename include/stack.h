#include "allocator.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool full;
    uint16_t size;
    uint16_t type_size;
    uint16_t top;
    void* raw;
} Stack;

Stack* stackAllocate(Allocator* allocator, uint16_t size, uint16_t type_size);

bool stackDeallocate(Allocator* allocator, Stack** stack);

void stackClear(Stack* stack);

bool stackPush(Stack* stack, const void* data);

bool stackPop(Stack* stack, void* data);