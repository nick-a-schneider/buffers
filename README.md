# Circular Buffer (Generic FIFO)

Provides a **circular FIFO buffer** implementation. Supports arbitrary element types and sizes with efficient memory use and predictable behavior.

## Features

- Generic: stores any data type via `element_size`
- Circular: FIFO behavior with automatic wrapping
- Efficient: fixed memory footprint, no dynamic resizing
- Safe: bounds-checked reads/writes and error codes
- Allocator-backed: memory is managed via [`BlockAllocator`](https://github.com/nick-a-schneider/bitmap_allocator?tab=readme-ov-file#block-allocator)

## Quick Start

```c
#include "buffer.h"
#include "block_allocator.h"

// Allocate memory with BlockAllocator as per:
// https://github.com/nick-a-schneider/bitmap_allocator/blob/master/README.md

// Create a circular buffer for 12 integers
Buffer* int_buf = bufferAllocate(&allocator, 12, sizeof(int));

int int_val = 42;
int int_res = bufferWrite(int_buf, (void*)&int_val);
int_res = bufferRead(int_buf, (void*)&int_val);
// ...
int_res = bufferDeallocate(&int_buf);
// ...
typedef struct {
    int x;
    char* y;
} Data_t;

Buffer* data_buf = bufferAllocate(&allocator, 8, sizeof(Data_t));

Data_t data_val = {
    .x = 42,
    .y = "val"
};
int data_res = bufferWrite(data_buf, (void*)&data_val);
data_res = bufferRead(data_buf, (void*)&data_val);
// ...
data_res = bufferDeallocate(&data_buf);
// ...
```
