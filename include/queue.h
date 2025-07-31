#pragma once

#include "allocator.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool full;
    uint8_t** slots;
    uint16_t slot_cnt;
    uint16_t slot_len;
    uint16_t* msg_len;
    uint16_t head;
    uint16_t tail;
} Queue;

#define CREATE_QUEUE(...) { \
    .slots = NULL,          \
    .slot_cnt = 0,          \
    .slot_len = 0,          \
    .msg_len = NULL,        \
    .head = 0,              \
    .tail = 0,              \
    .full = false,          \
    __VA_ARGS__             \
}

Queue* queueAllocate(Allocator* allocator, uint16_t slot_len, uint16_t size);
bool queueDeallocate(Allocator* allocator, Queue** queue);

void queueClear(Queue* queue);

bool queueIsEmpty(const Queue* queue);
bool queueIsFull(const Queue* queue);
uint16_t queueWrite(Queue* queue, const uint8_t* data, uint16_t slot_len);
uint16_t queueRead(Queue* queue, uint8_t* data, uint16_t slot_len);
