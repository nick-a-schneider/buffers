#include "stdio.h"
#include "stdint.h"
#include "arr_buffer.h"

#define ARR_SIZE 5
#define LEN 32

void printArrBuffer(ArrBuffer* buffer) {
    uint16_t len = 0;
    for (uint16_t i = 0; i < buffer->arr_size; i++) {
        len = buffer->used[i];
        if (len) {
            printf("%.*s\n", buffer->used[i], buffer->raw[i]);
        }
        else {
            printf("NULL\n");
        }
    }
    printf("\n\n");
}

int main(void) { 
    uint8_t raw[ARR_SIZE][LEN];
    uint16_t used[ARR_SIZE];
    ArrBuffer buffer = CREATE_ARR_BUFFER({
        .raw = raw,
        .used = used,
        .arr_size = ARR_SIZE,
        .len = LEN
    });
    (void)arrBufferWrite(&buffer, "Hello World", 11);
    printArrBuffer(&buffer);
    return 0;
}