#include "queue.h"
#include "buffer.h"
#include "block_allocator.h"
#include <errno.h>

Queue* queueAllocate(BlockAllocator* allocator, uint16_t slot_len, uint16_t size) {
    if (!allocator) return NULL;
    if (slot_len == 0 || size == 0) return NULL;
    Queue* queue = (Queue*)blockAllocate(allocator, sizeof(Queue));
    if (!queue) return NULL;
    queue->slot_buffer = bufferAllocate(allocator, size, slot_len * sizeof(uint8_t));
    if (!(queue->slot_buffer)) {
        (void)blockDeallocate(allocator, queue);
        return NULL;
    }
    queue->msg_len = (uint16_t*)blockAllocate(allocator, size * sizeof(uint16_t));
    if (!(queue->msg_len)) {
        (void)bufferDeallocate(allocator, &queue->slot_buffer);
        (void)blockDeallocate(allocator, queue);
        return NULL;
    }
    queue->slot_len = slot_len;
    return queue;
}

int queueDeallocate(BlockAllocator* allocator, Queue** queue) {
    if (!allocator ||!queue || !(*queue)) return -EINVAL;
    int res1, res2, res3;
    res1 = bufferDeallocate(allocator, (&(*queue)->slot_buffer));
    res2 = blockDeallocate(allocator, (*queue)->msg_len);
    res3 = blockDeallocate(allocator, *queue);
    if (res1 != QUEUE_OK) return res1;
    if (res2 != QUEUE_OK) return res2;
    if (res3 != QUEUE_OK) return res3;
    *queue = NULL;
    return QUEUE_OK;
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

int queueWrite(Queue* queue, const uint8_t* data, uint16_t len) {
    if (!queue || !data || len == 0) return -EINVAL;
    if (queueIsFull(queue)) return -ENOSPC;
    uint16_t orig_size  = queue->slot_buffer->type_size;

    if (len < queue->slot_len){
        queue->slot_buffer->type_size = len * sizeof(uint8_t);
    }
    if (len > queue->slot_len) len = queue->slot_len;
    uint16_t head = queue->slot_buffer->head;
    int res = bufferWrite(queue->slot_buffer, (void*)data);
    queue->slot_buffer->type_size = orig_size;
    if (res < BUFFER_OK) return res;
    queue->msg_len[head] = len;
    return len;
}

int queueRead(Queue* queue, uint8_t* data, uint16_t len) {
    if (!queue || !data || len == 0) return -EINVAL;
    if (queueIsEmpty(queue)) return -EAGAIN;
    uint16_t tail_len = queue->msg_len[queue->slot_buffer->tail];
    len = (len < tail_len) ? len : tail_len;
    uint16_t orig_size  = queue->slot_buffer->type_size;
    if (len < queue->slot_len) {
        queue->slot_buffer->type_size = len * sizeof(uint8_t);
    }
    uint16_t tail = queue->slot_buffer->tail;
    int res = bufferRead(queue->slot_buffer, (void*)data);
    queue->slot_buffer->type_size = orig_size;
    queue->msg_len[tail] = 0;
    if (res < BUFFER_OK) return res;
    return len;
}
