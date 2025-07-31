#include "queue.h"

#include "allocator.h"
#include <stdio.h>

Queue* queueAllocate(Allocator* allocator, uint16_t slot_len, uint16_t size) {
    if (!allocator) return NULL;
    if (slot_len == 0 || size == 0) return NULL;
    Queue* queue = (Queue*)allocate(allocator, sizeof(Queue));
    if (!queue) return NULL;
    queue->slots = (uint8_t**)allocate(allocator, size * sizeof(uint8_t*));
    if (!(queue->slots)) {
        deallocate(allocator, queue);
        return NULL;
    }
    queue->msg_len = (uint16_t*)allocate(allocator, size * sizeof(uint16_t));
    if (!(queue->msg_len)) {
        deallocate(allocator, queue->slots);
        deallocate(allocator, queue);
        return NULL;
    }
    for (uint16_t i = 0; i < size; i++) {
        uint8_t* elem = (uint8_t*)allocate(allocator, slot_len * sizeof(uint8_t));
        if (!(elem)) {
            for (uint16_t j = 0; j < i; j++) {
                deallocate(allocator, queue->slots[j]);
            }
            deallocate(allocator, queue->msg_len);
            deallocate(allocator, queue->slots);
            deallocate(allocator, queue);
            return NULL;
        }
        queue->slots[i] = elem;
    }
    queue->slot_cnt = size;
    queue->slot_len = slot_len;
    queue->head = 0;
    queue->tail = 0;
    queue->full = false;
    return queue;
}

bool queueDeallocate(Allocator* allocator, Queue** queue) {
    if (!allocator) return false;
    if (!queue || !(*queue)) return false;
    bool res = true;
    for (uint16_t i = 0; i < (*queue)->slot_cnt; i++) {
        res = deallocate(allocator, (*queue)->slots[i]);
    }
    res = deallocate(allocator, (*queue)->msg_len);
    res = deallocate(allocator, (*queue)->slots);
    res = deallocate(allocator, *queue);
    *queue = NULL;
    return res;
}

void queueClear(Queue* queue) {
    if (!queue) return;
    for (uint16_t i = 0; i < queue->slot_cnt; i++) {
        queue->msg_len[i] = 0;
    }
    queue->head = 0;
    queue->tail = 0;
    queue->full = false;
}

bool queueIsEmpty(const Queue* queue) {
    return !queue->full && queue->head == queue->tail;
}

bool queueIsFull(const Queue* queue) {
    return queue->full;
}

uint16_t queueWrite(Queue* queue, const uint8_t* data, uint16_t slot_len) {
    if (!queue || !data || slot_len == 0) return 0;
    if (queue->full) return 0;
    uint16_t valid_slot_len = (slot_len < queue->slot_len) ? slot_len : queue->slot_len;
    for (uint16_t i = 0; i < valid_slot_len; i++) {
        queue->slots[queue->head][i] = data[i];
    }
    queue->msg_len[queue->head] = valid_slot_len;
    queue->head = (uint16_t)((queue->head + 1) % queue->slot_cnt);
    if (queue->head == queue->tail) {
        queue->full = true;
    }
    return valid_slot_len;
}

uint16_t queueRead(Queue* queue, uint8_t* data, uint16_t slot_len) {
    if (!queue || !data || queueIsEmpty(queue)) return 0;
    if (queue->msg_len[queue->tail] < slot_len) {
        slot_len = queue->msg_len[queue->tail];
    }
    for (uint16_t i = 0; i < slot_len; i++) {
        data[i] = queue->slots[queue->tail][i];
    }
    queue->msg_len[queue->tail] = 0;
    queue->tail = (uint16_t)((queue->tail + 1) % queue->slot_cnt);

    queue->full = false;
    return slot_len;
}
