#include "queue.h"

#include "allocator.h"
#include <stdio.h>

Queue* queueAllocate(Allocator* allocator, uint16_t slot_len, uint16_t size) {
    if (!allocator) return NULL;
    if (slot_len == 0 || size == 0) return NULL;
    Queue* queue = (Queue*)allocate(allocator, sizeof(Queue));
    if (!queue) return NULL;
    queue->slot_buffer = bufferAllocate(allocator, size, slot_len * sizeof(uint8_t));
    if (!(queue->slot_buffer)) {
        (void)deallocate(allocator, queue);
        return NULL;
    }
    queue->msg_len = (uint16_t*)allocate(allocator, size * sizeof(uint16_t));
    if (!(queue->msg_len)) {
        (void)bufferDeallocate(allocator, queue->slot_buffer);
        (void)deallocate(allocator, queue);
        return NULL;
    }
    queue->slot_len = slot_len;
    return queue;
}

bool queueDeallocate(Allocator* allocator, Queue** queue) {
    if (!allocator) return false;
    if (!queue || !(*queue)) return false;
    bool res = true;
    res &= bufferDeallocate(allocator, (&(*queue)->slot_buffer));
    res &= deallocate(allocator, (*queue)->msg_len);
    res &= deallocate(allocator, *queue);
    if (res) *queue = NULL;
    return res;
}

void queueClear(Queue* queue) {
    if (!queue) return;
    for (uint16_t i = 0; i < queue->slot_buffer->size; i++) {
        queue->msg_len[i] = 0;
    }
    bufferClear(queue->slot_buffer);
}

bool queueIsEmpty(const Queue* queue) {
    return bufferIsEmpty(queue->slot_buffer);
}

bool queueIsFull(const Queue* queue) {
    return bufferIsFull(queue->slot_buffer);
}

uint16_t queueWrite(Queue* queue, const uint8_t* data, uint16_t len) {
    if (!queue || !data || len == 0) return 0;
    if (queueIsFull(queue)) return 0;
    uint16_t orig_size  = queue->slot_buffer->type_size;

    if (len < queue->slot_len){
        queue->slot_buffer->type_size = len * sizeof(uint8_t);
    }
    if (len > queue->slot_len) len = queue->slot_len;
    uint16_t head = queue->slot_buffer->head;
    bool res = bufferWrite(queue->slot_buffer, (void*)data);
    queue->slot_buffer->type_size = orig_size;
    if (!res) return 0;
    queue->msg_len[head] = len;
    return len;
}

uint16_t queueRead(Queue* queue, uint8_t* data, uint16_t len) {
    if (!queue || !data || queueIsEmpty(queue)) return 0;
    uint16_t tail_len = queue->msg_len[queue->slot_buffer->tail];
    len = (len < tail_len) ? len : tail_len;
    uint16_t orig_size  = queue->slot_buffer->type_size;
    if (len < queue->slot_len) {
        queue->slot_buffer->type_size = len * sizeof(uint8_t);
    }
    uint16_t tail = queue->slot_buffer->tail;
    bool res = bufferRead(queue->slot_buffer, (void*)data);
    queue->slot_buffer->type_size = orig_size;
    queue->msg_len[tail] = 0;
    if (!res) return 0;
    return len;
}
