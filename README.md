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
