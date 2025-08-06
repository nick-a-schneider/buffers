#include "buffer.h"
#include "block_allocator.h"
#include "test_utils.h"
#include <string.h>

#define MEMORY_SIZE 2048

uint8_t testMemory[MEMORY_SIZE];
static BlockAllocator testAllocator;

void test_bufferAllocate() {
    TEST_CASE("Allocates and initializes buffer correctly") {
        Buffer* buf = bufferAllocate(&testAllocator, 8, sizeof(uint8_t));
        ASSERT_NOT_NULL(buf, "Buffer should not be NULL");
        ASSERT_NOT_NULL(buf->raw, "raw pointer should not be NULL");
        ASSERT_EQUAL_INT(buf->size, 8, "size should be 8 on init");
        ASSERT_EQUAL_INT(buf->type_size, sizeof(uint8_t), "type size should be 1 on init");
        ASSERT_FALSE(buf->full, "should not be full on init");
        ASSERT_EQUAL_INT(buf->head, 0, "head should be 0 on init");
        ASSERT_EQUAL_INT(buf->tail, 0, "tail should be 0 on init");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;

    TEST_CASE("zero length") {
        Buffer* buf = bufferAllocate(&testAllocator, 0, sizeof(uint8_t));
        ASSERT_NULL(buf, "Buffer should be NULL");
    } CASE_COMPLETE;

    TEST_CASE("zero size") {
        Buffer* buf = bufferAllocate(&testAllocator, 8, 0);
        ASSERT_NULL(buf, "Buffer should be NULL");
    } CASE_COMPLETE;

    TEST_CASE("insufficient memory") {
        Buffer* buf = bufferAllocate(&testAllocator, 512, 512);
        ASSERT_NULL(buf, "Buffer should be NULL");
    } CASE_COMPLETE;

    TEST_CASE("invalid allocator") {
        BlockAllocator* invalidAllocator = NULL;
        Buffer* buf = bufferAllocate(invalidAllocator, 8, sizeof(uint8_t));
        ASSERT_NULL(buf, "should return NULL on invalid allocator");
    } CASE_COMPLETE;
    
    memset(testMemory, 0, sizeof(testMemory));
}

void test_bufferDeallocate() {
    TEST_CASE("Deallocates and nullifies buffer pointer") {
        Buffer* buf = bufferAllocate(&testAllocator, 8, sizeof(uint8_t));
        bool success = bufferDeallocate(&testAllocator, &buf);
        ASSERT_TRUE(success, "Buffer deallocation failed");
        ASSERT_NULL(buf, "Buffer pointer should be NULL after free");
    } CASE_COMPLETE;

    TEST_CASE("invalid allocator") {
        BlockAllocator* invalidAllocator = NULL;
        Buffer* buf = bufferAllocate(&testAllocator, 8, sizeof(uint8_t));
        bool success = bufferDeallocate(invalidAllocator, &buf);
        ASSERT_FALSE(success, "Deallocating NULL buffer should fail");
    } CASE_COMPLETE;

    TEST_CASE("Null buffer pointer") {
        Buffer* buf = NULL;
        bool success = bufferDeallocate(&testAllocator, &buf);
        ASSERT_FALSE(success, "Deallocating NULL buffer should fail");
    } CASE_COMPLETE;
    
    memset(testMemory, 0, sizeof(testMemory));
}

void test_bufferClear() {
    TEST_CASE("Clears head/tail/full") {
        Buffer* buf = bufferAllocate(&testAllocator, 8, sizeof(uint8_t));
        buf->head = 2; buf->tail = 1; buf->full = true;
        uint8_t* raw = (uint8_t*)buf->raw;
        bufferClear(buf);
        ASSERT_EQUAL_INT(buf->head, 0, "head not reset");
        ASSERT_EQUAL_INT(buf->tail, 0, "tail not reset");
        ASSERT_FALSE(buf->full, "full not reset");
        ASSERT_EQUAL_PTR(raw, (uint8_t*)buf->raw, "raw pointer reset on clear");
        ASSERT_EQUAL_INT(buf->size, 8, "size reset on clear");
        ASSERT_EQUAL_INT(buf->type_size, sizeof(uint8_t), "size reset on clear");
        bufferDeallocate(&testAllocator, &buf);
    } CASE_COMPLETE;
}

void test_bufferWrite() {
    TEST_CASE("Writes correctly") {
        Buffer* buf = bufferAllocate(&testAllocator, 8, sizeof(uint8_t));
        uint8_t input = 68;
        bool res = bufferWrite(buf, (void*)&input);
        ASSERT_TRUE(res, "Write failed");
        ASSERT_FALSE(bufferIsEmpty(buf), "Buffer shouldn't be empty after write");
        ASSERT_FALSE(bufferIsFull(buf), "Buffer shouldn't be full yet");
        ASSERT_EQUAL_INT(buf->head, 1, "Head did not move after write");
        ASSERT_EQUAL_INT(buf->tail, 0, "Tail moved after write");
        ASSERT_EQUAL_INT(*(uint8_t*)buf->raw, input, "Written data mismatch");
        bufferDeallocate(&testAllocator, &buf);
    } CASE_COMPLETE;

    TEST_CASE("Writes complex structure") {
        Buffer* buf = bufferAllocate(&testAllocator, 8, sizeof(TestStruct));
        uint32_t data = 68;
        TestStruct input = {.data = data, .flag = true, .ptr = &data};
        bool res = bufferWrite(buf, (void*)&input);
        ASSERT_TRUE(res, "Write failed");
        ASSERT_FALSE(bufferIsEmpty(buf), "Buffer shouldn't be empty after write");
        ASSERT_FALSE(bufferIsFull(buf), "Buffer shouldn't be full yet");
        ASSERT_EQUAL_INT(buf->head, 1, "Head did not move after write");
        ASSERT_EQUAL_INT(buf->tail, 0, "Tail moved after write");
        TestStruct output = *(TestStruct*)buf->raw;
        ASSERT_EQUAL_INT(output.data, data, "Written data mismatch");
        ASSERT_TRUE(output.flag, "Written data mismatch");
        ASSERT_EQUAL_PTR(output.ptr, &data, "Written data mismatch");
        bufferDeallocate(&testAllocator, &buf);
    } CASE_COMPLETE;

    TEST_CASE("Invalid buffer") {
        Buffer* buf = NULL;
        uint8_t input = 68;
        bool res = bufferWrite(buf, (void*)&input);
        ASSERT_FALSE(res, "Expected write to fail");
    }  CASE_COMPLETE;

    TEST_CASE("Invalid data") {
        Buffer* buf = bufferAllocate(&testAllocator, 8, sizeof(uint8_t));
        uint8_t input = NULL;
        bool res = bufferWrite(buf, input);
        ASSERT_FALSE(res, "Expected write to fail");
        bufferDeallocate(&testAllocator, &buf);
    }  CASE_COMPLETE;
}

void test_bufferRead() {
    TEST_CASE("Reads correctly") {
        Buffer* buf = bufferAllocate(&testAllocator, 8, sizeof(uint8_t));
        uint8_t input = 68;
        (void)bufferWrite(buf, (void*)&input);
        uint8_t output;
        bool res = bufferRead(buf, (void*)&output);
        ASSERT_TRUE(res, "Read failed");
        ASSERT_TRUE(bufferIsEmpty(buf), "Buffer should be empty after read");
        ASSERT_FALSE(bufferIsFull(buf), "Buffer shouldn't be full yet");
        ASSERT_EQUAL_INT(buf->head, 1, "Head moved after read");
        ASSERT_EQUAL_INT(buf->tail, 1, "Tail did not move after read");
        ASSERT_EQUAL_INT(output, input, "Read data mismatch");
        bufferDeallocate(&testAllocator, &buf);
    } CASE_COMPLETE;

    TEST_CASE("read complex structure") {
        Buffer* buf = bufferAllocate(&testAllocator, 8, sizeof(TestStruct));
        uint32_t data = 68;
        TestStruct input = {.data = data, .flag = true, .ptr = &data};
        (void)bufferWrite(buf, (void*)&input);
        TestStruct output;
        bool res = bufferRead(buf, (void*)&output);
        ASSERT_TRUE(res, "Read failed");
        ASSERT_TRUE(bufferIsEmpty(buf), "Buffer should be empty after read");
        ASSERT_FALSE(bufferIsFull(buf), "Buffer shouldn't be full yet");
        ASSERT_EQUAL_INT(buf->head, 1, "Head moved after read");
        ASSERT_EQUAL_INT(buf->tail, 1, "Tail did not move after read");
        ASSERT_EQUAL_INT(output.data, input.data, "Read data mismatch");
        ASSERT_TRUE(output.flag, "Read data mismatch");
        ASSERT_EQUAL_PTR(output.ptr, input.ptr, "Read data mismatch");
        bufferDeallocate(&testAllocator, &buf);
    } CASE_COMPLETE;

    TEST_CASE("Invalid buffer") {
        Buffer* buf = NULL;
        uint8_t output;
        bool res = bufferRead(buf, (void*)&output);
        ASSERT_FALSE(res, "Cannot read from NULL buffer");
    } CASE_COMPLETE;

    TEST_CASE("Invalid output") {
        Buffer* buf = bufferAllocate(&testAllocator, 8, sizeof(uint8_t));
        uint8_t input = 68;
        (void)bufferWrite(buf, (void*)&input);
        bool res = bufferRead(buf, NULL);
        ASSERT_FALSE(res, "Cannot read to NULL buffer");
        bufferDeallocate(&testAllocator, &buf);
    } CASE_COMPLETE;
}

void test_BufferFill() {
    Buffer* buf = bufferAllocate(&testAllocator, 2, sizeof(uint8_t));
    uint8_t input1 = 68;
    uint8_t input2 = 24;
    uint8_t input3 = 47;

    TEST_CASE( "test bufferIsFull") {
        (void)bufferWrite(buf, (void*)&input1);
        (void)bufferWrite(buf, (void*)&input2);
        ASSERT_TRUE(bufferIsFull(buf), "Buffer should be full");
    } CASE_COMPLETE;

    TEST_CASE( "test write to full buffer") {
        bool res = bufferWrite(buf, (void*)&input3);
        ASSERT_FALSE(res, "Expected to not write anything");
        ASSERT_EQUAL_INT(*(uint8_t*)buf->raw, input1, "data was overwritten");
    } CASE_COMPLETE;

    TEST_CASE("test read from full buffer") {
        uint8_t output;
        bool res = bufferRead(buf, (void*)&output);
        ASSERT_TRUE(res, "Read failed");
        ASSERT_FALSE(bufferIsFull(buf), "Buffer shouldn't be full yet");
        ASSERT_EQUAL_INT(output, input1, "Read data mismatch");
    } CASE_COMPLETE;

    TEST_CASE( "test write over after full") {
        (void)bufferWrite(buf, (void*)&input3);
        ASSERT_EQUAL_INT(*(uint8_t*)buf->raw, input3, "data was overwritten");
    } CASE_COMPLETE;

    bufferDeallocate(&testAllocator, &buf);
}

int main() {
    LOG_INFO("BUFFER TESTS\n");
    initBlockAllocator(&testAllocator, 4, testMemory, MEMORY_SIZE);
    TEST_EVAL(test_bufferAllocate);
    TEST_EVAL(test_bufferDeallocate);
    TEST_EVAL(test_bufferClear);
    TEST_EVAL(test_bufferWrite);
    TEST_EVAL(test_bufferRead);
    TEST_EVAL(test_BufferFill);
    return testGetStatus();
}