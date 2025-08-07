# Circular Buffer (Generic FIFO)

Provides a **circular FIFO buffer** implementation. Supports arbitrary element types and sizes with efficient memory use and predictable behavior.

## Example

```c
#include "buffer.h"

// Create a circular buffer for 12 integers
CREATE_BUFFER(int_buf, 12, int);

int int_val = 42;
int int_res = bufferWrite(&int_buf, (void*)&int_val);
int_res = bufferRead(&int_buf, (void*)&int_val);

// ...
// Create a circular buffer of any datatype
typedef struct {
    int x;
    char* y;
} Data_t;

CREATE_BUFFER(data_buf, 12, Data_t);

Data_t data_val = {
    .x = 42,
    .y = "val"
};
int data_res = bufferWrite(&data_buf, (void*)&data_val);
data_res = bufferRead(&data_buf, (void*)&data_val);

// ...
// Alternatively, if a buffer needs to be created dynamically:

#include "block_allocator.h"

// Initialize a BlockAllocator as per:
// https://github.com/nick-a-schneider/bitmap_allocator/blob/master/README.md
// note: Make sure USE_BITMAP_ALLOCATOR is defined at compile time
Buffer* data_buf = bufferAllocate(&allocator, 8, sizeof(Data_t));
// ...
data_res = bufferDeallocate(&data_buf);
// ...
```

# Queue 
The Queue type is built upon the circular buffer, using fixed length char arrays as the underlying data type. 
Functions as a FIFO buffer for full messages.

## Example
```c
#include "queue.h"

CREATE_QUEUE(queue, 16, 4);
char msg[] = "Hello World!";
int res = queueWrite(queue, msg, 12);
res = queueRead(queue, msg, 12);
// ...
// Alternatively, if a buffer needs to be created dynamically:

#include "block_allocator.h"

// Initialize a BlockAllocator as per:
// https://github.com/nick-a-schneider/bitmap_allocator/blob/master/README.md
// note: Make sure USE_BITMAP_ALLOCATOR is defined at compile time
Queue* queue = queueAllocate(&allocator, 16, 4);
// ...
res = bufferDeallocate(&queue);
// ...
```

# Stack

Provides a fixed-size stack implementation for arbitrary data types.

## Example

```c
#include "stack.h"

// Create a stack for 10 integers
CREATE_STACK(int_stack, 10, int);

int int_val = 42;
int int_res = stackPush(&int_stack, (void*)&int_val);
int_res = stackPop(&int_stack, (void*)&int_val);
// ...
typedef struct {
    int x;
    char* y;
} Data_t;

CREATE_STACK(int_stack, 12, Data_t);

Data_t data_val = {
    .x = 42,
    .y = "val"
};
int data_res = stackPush(&int_stack, (void*)&data_val);
data_res = stackPop(&int_stack, (void*)&data_val);
// ...
// Alternatively, if a stack needs to be created dynamically:

#include "block_allocator.h"

// Initialize a BlockAllocator as per:
// https://github.com/nick-a-schneider/bitmap_allocator/blob/master/README.md
// note: Make sure USE_BITMAP_ALLOCATOR is defined at compile time
Stack* int_stack = stackAllocate(&allocator, 10, sizeof(int));
// ...
res = stackDeallocate(&allocator, &int_stack);
// ...
```
