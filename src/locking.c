#ifdef USE_BITMAP_ALLOCATOR
#include "locking.h"
#include "block_allocator.h"
#include <stddef.h>
#include <stdbool.h>
#include <errno.h>
#ifdef USE_ATOMIC
#include <stdatomic.h>

/**
 * @details 
 * This function creates a `Lock_t` instance and its associated per-slot state array.
 * When `USE_ATOMIC` is defined, both the read and write locks are initialized to 0
 * and each slot state is set to `BUFFER_FREE`. If allocation of either the lock
 * structure or the slot state array fails, any partially allocated memory is released
 * before returning.

 * @note The lock and slot state memory are allocated separately and must both be
 *       released with `lockDeallocate()` when no longer needed.
 */
Lock_t* lockAllocate(BlockAllocator* allocator, uint16_t size) {
    if (!allocator || size == 0) return NULL;
    Lock_t* lock = blockAllocate(allocator, sizeof(Lock_t));
    if (!lock) return NULL;
    lock->slot_state = blockAllocate(allocator, sizeof(atomic_uint_least8_t) * size);
    if (!lock->slot_state) {
        blockDeallocate(allocator, lock);
        return NULL;
    }
    for (int i = 0; i < size; i++) {
        atomic_store(lock->slot_state + i, BUFFER_FREE);          
    }
    atomic_store(&lock->read, false);
    atomic_store(&lock->write, false);
    return lock;
}

/**
 * @details
 * This function frees the memory associated with a `Lock_t` instance previously
 * allocated by `lockAllocate()`. It first releases the per-slot state array, then
 * the lock structure itself.

 * @note Both the lock structure and its slot state array must have been allocated
 *       using the same `BlockAllocator`. Freeing in a different order or with a
 *       different allocator will cause undefined behavior.
 */
int lockDeallocate(BlockAllocator* allocator, Lock_t** lock) {
    if (!allocator || !lock || !(*lock)) return -EINVAL;
    int res1, res2;
    res1 = blockDeallocate(allocator, (*lock)->slot_state);
    res2 = blockDeallocate(allocator, (*lock));
    if (res1 != BLOCK_ALLOCATOR_OK) return res1;
    if (res2 != BLOCK_ALLOCATOR_OK) return res2;
    *lock = NULL;
    return LOCK_OK;
}
#else // USE_ATOMIC
Lock_t* lockAllocate(BlockAllocator* allocator, uint16_t size) { return (Lock_t*)allocator->block_size; } // garbage non-null pointer
int lockDeallocate(BlockAllocator* allocator, Lock_t** lock) {return LOCK_OK; }
#endif // USE_ATOMIC

#endif // USE_BITMAP_ALLOCATOR