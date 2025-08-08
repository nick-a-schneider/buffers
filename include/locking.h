#pragma once
/**
 * @file lock.h
 * @brief Locking and slot state management utilities for thread-safe data structures.
 *
 * This header provides a unified locking mechanism for concurrent data structures such as
 * ring buffers, queues, and stacks. It supports both atomic (thread-safe) and non-atomic
 * modes via the `USE_ATOMIC` macro, and optionally integrates with a bitmap-based block
 * allocator via `USE_BITMAP_ALLOCATOR`.
 *
 * ## Features
 * - Atomic read/write locks for multi-threaded safety (if `USE_ATOMIC` is defined).
 * - Per-slot state tracking for structures storing multiple elements.
 * - Macros to create and manipulate lock objects with minimal boilerplate.
 * - Optional integration with a `BlockAllocator` for dynamic lock allocation.
 *
 * ## Usage
 * - Use `CREATE_LOCK()` to define a lock for a fixed number of slots.
 * - Acquire and release locks with `TAKE_READ_LOCK()` / `CLEAR_READ_LOCK()`
 *   and `TAKE_WRITE_LOCK()` / `CLEAR_WRITE_LOCK()`.
 * - For each slot in a data structure, use `SET_SLOT_STATE()` or
 *   `EXPECT_SLOT_STATE()` to manage availability and state transitions.
 *
 * Thread safety depends on whether `USE_ATOMIC` is defined. Without it,
 * all macros become no-ops for single-threaded use.
 */
#ifdef USE_BITMAP_ALLOCATOR
#include "block_allocator.h"
#endif
#include <stddef.h>
#include <stdbool.h>
#include <errno.h>

#define LOCK_OK 0 // success

/**
 * @enum BufferState
 * @brief Represents the per-slot state in a lock-managed container.
 *
 * These values are typically stored in the `slot_state` array of a `Lock_t`
 * structure to track whether an element is available, claimed for writing,
 * ready to read, or currently being read.
 */
typedef enum {
    BUFFER_FREE = 0,   ///< Slot is unused and available. */
    BUFFER_CLAIMED,    ///< Slot has been claimed by a writer but not yet filled. */
    BUFFER_READY,      ///< Slot contains valid data ready to be read. */
    BUFFER_READING,    ///< Slot is currently being accessed by a reader. */
} BufferState;

#ifdef USE_ATOMIC
#include <stdatomic.h>

/**
 * @brief [internal] Clear a lock variable (set to `false`).
 * @param lock Pointer to the lock variable (`atomic_bool*`).
 */
#define CLEAR_LOCK(lock) atomic_store_explicit(lock, false, memory_order_release)

/**
 * @brief [internal] Set a lock or slot state to a specific value.
 * @param lock Pointer to the atomic variable.
 * @param val  Value to set.
 */
#define SET_LOCK_VAL(lock, val) atomic_store_explicit(lock, val, memory_order_relaxed)

/**
 * @brief [internal] Compare and set a lock value atomically.
 *
 * Attempts to update the given atomic variable from `*expected` to `val`.
 * If the current value equals `*expected`, it is replaced with `val`
 * and the operation succeeds.
 *
 * @param lock     Pointer to the atomic variable.
 * @param expected Pointer to the expected value.
 * @param val      Value to store if the comparison succeeds.
 * @return `true` if the value was updated, `false` otherwise.
 */
#define COMPARE_SET_LOCK(lock, expected, val)       \
    atomic_compare_exchange_strong_explicit(        \
        lock,                                       \
        expected,                                   \
        val,                                        \
        memory_order_acquire,                       \
        memory_order_relaxed)

/** @brief [internal] helper for lock acquisition macros. */
static bool __lock_expect_false__ = false;

/**
 * @brief Attempt to take the read lock.
 * @param lock Pointer to a `Lock_t` structure.
 * @return `true` if the lock was acquired, `false` if already locked.
 */
#define TAKE_READ_LOCK(lock) COMPARE_SET_LOCK(&lock->read, &__lock_expect_false__, true)

/**
 * @brief Attempt to take the write lock.
 * @param lock Pointer to a `Lock_t` structure.
 * @return `true` if the lock was acquired, `false` if already locked.
 */
#define TAKE_WRITE_LOCK(lock) COMPARE_SET_LOCK(&lock->write, &__lock_expect_false__, true)

/**
 * @brief Release the read lock.
 * @param lock Pointer to a `Lock_t` structure.
 */
#define CLEAR_READ_LOCK(lock) CLEAR_LOCK(&lock->read)

/**
 * @brief Release the write lock.
 * @param lock Pointer to a `Lock_t` structure.
 */
#define CLEAR_WRITE_LOCK(lock) CLEAR_LOCK(&lock->write)

/**
 * @brief Set the state of a slot in the lock.
 * @param lock  Pointer to the `Lock_t` structure.
 * @param index Index of the slot to update.
 * @param val   New state value.
 */
#define SET_SLOT_STATE(lock, index, val) SET_LOCK_VAL(lock->slot_state + index, val)

/**
 * @brief Atomically check and update a slot's state.
 *
 * Similar to `COMPARE_SET_LOCK()` but operates on a slot's state.
 *
 * @param lock     Pointer to the `Lock_t` structure.
 * @param index    Index of the slot to update.
 * @param expected Pointer to the expected state value.
 * @param val      New state value if comparison succeeds.
 * @return `true` if the slot state was updated, `false` otherwise.
 */
#define EXPECT_SLOT_STATE(lock, index, expected, val) COMPARE_SET_LOCK(lock->slot_state + index, expected, val)

/**
 * @brief Create and initialize a `Lock_t` object for a fixed number of slots.
 *
 * This macro creates a `Lock_t` instance named `name`, initializes its
 * per-slot state array to `BUFFER_FREE`, and sets both `read` and `write`
 * locks to `false`.
 *
 * @param name Name of the lock variable to create.
 * @param len  Number of slots to manage.
 */
#define CREATE_LOCK(name, len)                      \
    atomic_uint_least8_t name##_lock_state[len];    \
    for (int i = 0; i < (len); i++) {               \
        SET_LOCK_VAL(                               \
            name##_lock_state + i,                  \ 
            BUFFER_FREE);                           \
    }                                               \
    Lock_t name = {                                 \
        .slot_state = name##_lock_state             \
    };                                              \
    atomic_store(&name.read, false);                \
    atomic_store(&name.write, false)                \

/**
 * @struct Lock_t
 * @brief Represents the locking state for a concurrent container.
 *
 * Contains separate atomic read and write locks, plus a per-slot state
 * array for tracking individual elements.
 */
typedef struct {
    atomic_bool read;                  ///< Read lock flag. */
    atomic_bool write;                 ///< Write lock flag. */
    atomic_uint_least8_t* slot_state;  ///< Pointer to per-slot state array. */
} Lock_t;

#else /* Non-atomic mode: macros become no-ops */

/** @brief Always succeeds in single-threaded mode. */
#define TAKE_READ_LOCK(lock) true
/** @brief Always succeeds in single-threaded mode. */
#define TAKE_WRITE_LOCK(lock) true
/** @brief No-op in single-threaded mode. */
#define CLEAR_READ_LOCK(lock) {}
/** @brief No-op in single-threaded mode. */
#define CLEAR_WRITE_LOCK(lock) {}
/** @brief No-op in single-threaded mode. */
#define SET_SLOT_STATE(lock, index, val) {}
/** @brief Always returns true in single-threaded mode. */
#define EXPECT_SLOT_STATE(lock, index, expected, val) true
/** @brief Declares a dummy lock variable in single-threaded mode. */
#define CREATE_LOCK(name, len) Lock_t name = 0

/** @brief Dummy lock type for non-atomic builds. */
typedef uint8_t Lock_t;

#endif /* USE_ATOMIC */

#ifdef USE_BITMAP_ALLOCATOR

/**
 * @brief Allocate a new `Lock_t` object from a block allocator.
 *
 * @param allocator Pointer to a `BlockAllocator` instance.
 * @param size      Number of slots to allocate state for.
 * @return Pointer to the allocated `Lock_t`, or `NULL` on failure.
 */
Lock_t* lockAllocate(BlockAllocator* allocator, uint16_t size);

/**
 * @brief Deallocate a previously allocated `Lock_t` object.
 *
 * @param allocator Pointer to a `BlockAllocator` instance.
 * @param lock      Pointer to the `Lock_t*` to free. Will be set to `NULL` on success.
 * @return `LOCK_OK` (0) on success, or a negative error code.
 */
int lockDeallocate(BlockAllocator* allocator, Lock_t** lock);
#endif
