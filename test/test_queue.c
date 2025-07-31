#include "arr_buffer.h"
#include "allocator.h"
#include "test_utils.h"
#include <string.h>

#define MEMORY_SIZE 2048
uint8_t testMemory[MEMORY_SIZE];
static Allocator testAllocator;

void test_arrBufferAllocate() {
    TEST_CASE("Allocates and initializes buffer correctly") {
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 8, 4);
        ASSERT_NOT_NULL(buf, "Buffer should not be NULL");
        ASSERT_NOT_NULL(buf->raw, "raw pointer should not be NULL");
        ASSERT_NOT_NULL(buf->used, "used pointer should not be NULL");
        ASSERT_EQUAL_INT(buf->len, 8, "len mismatch");
        ASSERT_EQUAL_INT(buf->arr_size, 4, "arr_size mismatch");
        ASSERT_FALSE(buf->full, "should not be full on init");
        for (int i = 0; i < 4; ++i) ASSERT_EQUAL_INT(buf->used[i], 0, "used[i] should be 0 on init");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;

    TEST_CASE("insufficient memory") {
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 512, 512);
        ASSERT_NULL(buf, "Buffer should be NULL");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;

    TEST_CASE("invalid allocator") {
        Allocator* invalidAllocator = NULL;
        ArrBuffer* buf = arrBufferAllocate(invalidAllocator, 2, 2);
        ASSERT_NULL(buf, "should return NULL on invalid allocator");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;

    TEST_CASE("zero dimensions") {
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 0, 2);
        ASSERT_NULL(buf, "should return NULL if `len` is zero");
        buf = arrBufferAllocate(&testAllocator, 2, 0);
        ASSERT_NULL(buf, "should return NULL if `size` is zero");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;
}

void test_arrBufferDeallocate() {
    TEST_CASE("Deallocates and nullifies buffer pointer") {
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 8, 4);
        bool success = arrBufferDeallocate(&testAllocator, &buf);
        ASSERT_TRUE(success, "Buffer deallocation failed");
        ASSERT_NULL(buf, "Buffer pointer should be NULL after free");
    } CASE_COMPLETE;

    TEST_CASE("invalid allocator") {
        Allocator* invalidAllocator = NULL;
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 8, 4);
        bool success = arrBufferDeallocate(invalidAllocator, &buf);
        ASSERT_FALSE(success, "Deallocating NULL buffer should fail");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;

    TEST_CASE("Null buffer pointer") {
        ArrBuffer* buf = NULL;
        bool success = arrBufferDeallocate(&testAllocator, &buf);
        ASSERT_FALSE(success, "Deallocating NULL buffer should fail");
    } CASE_COMPLETE;

}

void test_arrBufferClear() {
    TEST_CASE("Clears head/tail/full and used array") {
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 8, 4);
        buf->head = 2; buf->tail = 1; buf->full = true;
        for (int i = 0; i < 4; ++i) buf->used[i] = 4;
        arrBufferClear(buf);
        ASSERT_EQUAL_INT(buf->head, 0, "head not reset");
        ASSERT_EQUAL_INT(buf->tail, 0, "tail not reset");
        ASSERT_FALSE(buf->full, "full not reset");
        for (int i = 0; i < 4; ++i)
            ASSERT_EQUAL_INT(buf->used[i], 0, "used[i] not cleared");
        arrBufferDeallocate(&testAllocator, &buf);
    } CASE_COMPLETE;
}

void test_arrBufferWrite() {
    TEST_CASE("Writes correctly") {
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 8, 2);
        uint8_t* input = "SwampWho";
        uint16_t written = arrBufferWrite(buf, input, 8);
        ASSERT_EQUAL_INT(written, 8, "Expected to write 8 bytes");
        ASSERT_FALSE(arrBufferIsEmpty(buf), "Buffer shouldn't be empty after write");
        ASSERT_FALSE(arrBufferIsFull(buf), "Buffer shouldn't be full yet");
        ASSERT_EQUAL_INT(buf->head, 1, "Head did not move after write");
        ASSERT_EQUAL_INT(buf->tail, 0, "Tail moved after write");
        ASSERT_EQUAL_INT(buf->used[0], 8, "Expected to write 8 bytes");
        ASSERT_EQUAL_STR(buf->raw[0], input, 8, "Written data mismatch");
        arrBufferDeallocate(&testAllocator, &buf);
    } CASE_COMPLETE;

    TEST_CASE("Write oversided message") {
        memset(testMemory, 0, sizeof(testMemory));
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 4, 2);
        uint8_t* input = "SwampWho";
        uint16_t written = arrBufferWrite(buf, input, 8);
        ASSERT_EQUAL_INT(written, 4, "Expected to write 4 bytes");
        ASSERT_EQUAL_STR(buf->raw[0], input, 4, "Written data mismatch");
        ASSERT_NOT_EQUAL_STR(buf->raw[0], input, 8, "written data overflowed");
        arrBufferDeallocate(&testAllocator, &buf);
    }  CASE_COMPLETE;

    TEST_CASE("Invalid buffer") {
        ArrBuffer* buf = NULL;
        uint8_t* input = "SwampWho";
        uint16_t written = arrBufferWrite(buf, input, 8);
        ASSERT_EQUAL_INT(written, 0, "Expected to write 0 bytes");
    }  CASE_COMPLETE;

    TEST_CASE("Invalid data") {
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 8, 2);
        uint8_t* input = NULL;
        uint16_t written = arrBufferWrite(buf, input, 8);
        ASSERT_EQUAL_INT(written, 0, "Expected to write 0 bytes");
        arrBufferDeallocate(&testAllocator, &buf);
    }  CASE_COMPLETE;

    TEST_CASE("zero length") {
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 8, 2);
        uint8_t* input = "SwampWho";
        uint16_t written = arrBufferWrite(buf, input, 0);
        ASSERT_EQUAL_INT(written, 0, "Expected to write 0 bytes");
        arrBufferDeallocate(&testAllocator, &buf);
    }  CASE_COMPLETE;
}

void test_arrBufferRead() {
    TEST_CASE("Reads correctly") {
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 8, 2);
        uint8_t* input = "SwampWho";
        uint8_t output[10];
        (void)arrBufferWrite(buf, input, 8);
        uint16_t read = arrBufferRead(buf, output, 10);
        ASSERT_EQUAL_INT(read, 8, "Expected to read 8 bytes");
        ASSERT_EQUAL_STR(output, input, 8, "Read data mismatch");
        ASSERT_TRUE(arrBufferIsEmpty(buf), "Buffer should be empty after read");
        ASSERT_FALSE(arrBufferIsFull(buf), "Buffer shouldn't be full yet");
        ASSERT_EQUAL_INT(buf->head, 1, "Head moved after read");
        ASSERT_EQUAL_INT(buf->tail, 1, "Tail did not move after read");
        ASSERT_EQUAL_INT(buf->used[0], 0, "used was not reset after read");
        
        arrBufferDeallocate(&testAllocator, &buf);
    } CASE_COMPLETE;

    TEST_CASE("read oversided message") {
        memset(testMemory, 0, sizeof(testMemory));
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 15, 2);
        uint8_t* input = "Hello World!\n";
        uint8_t output[15];
        (void)arrBufferWrite(buf,input, 13);
        uint16_t read = arrBufferRead(buf, output, 8);
        ASSERT_EQUAL_INT(read, 8, "Expected to read 8 bytes");
        ASSERT_EQUAL_STR(output, input, 8, "Read data mismatch");
        ASSERT_NOT_EQUAL_STR(output, input, 13, "read data overflowed");
        arrBufferDeallocate(&testAllocator, &buf);
    }  CASE_COMPLETE;

    TEST_CASE("Invalid buffer") {
        ArrBuffer* buf = NULL;
        uint8_t output[8];
        uint16_t read = arrBufferRead(buf, output, 8);
        ASSERT_EQUAL_INT(read, 0, "Cannot read from NULL buffer");
    }  CASE_COMPLETE;

    TEST_CASE("Invalid output") {
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 8, 2);
        uint8_t* input = "SwampWho";
        (void)arrBufferWrite(buf, input, 8);
        uint16_t read = arrBufferRead(buf, NULL, 8);
        ASSERT_EQUAL_INT(read, 0, "Cannot read to NULL buffer");
        arrBufferDeallocate(&testAllocator, &buf);
    }  CASE_COMPLETE;

    TEST_CASE("zero length") {
        ArrBuffer* buf = arrBufferAllocate(&testAllocator, 8, 2);
        uint8_t output[8];
        uint16_t read = arrBufferRead(buf, output, 0);
        ASSERT_EQUAL_INT(read, 0, "Expected to read 0 bytes");
        arrBufferDeallocate(&testAllocator, &buf);
    }  CASE_COMPLETE;
}

void test_arrBufferFill() {
    ArrBuffer* buf = arrBufferAllocate(&testAllocator, 8, 2);
    uint8_t* input1 = "swamp";
    uint8_t* input2 = "moose";
    uint8_t* input3 = "lagoon";

    TEST_CASE( "test bufferIsFull") {
        (void)arrBufferWrite(buf, input1, 5);
        (void)arrBufferWrite(buf, input2, 5);
        ASSERT_TRUE(arrBufferIsFull(buf), "Buffer should be full");
    } CASE_COMPLETE;

    TEST_CASE( "test write to full buffer") {
        uint16_t written3 = arrBufferWrite(buf, input3, 6);
        ASSERT_EQUAL_INT(written3, 0, "Expected to not write anything");
        ASSERT_EQUAL_STR(buf->raw[0], input1, 5, "write to full buffer should not overwrite previous data");
    } CASE_COMPLETE;

    TEST_CASE("test read from full buffer") {
        uint8_t output[10];
        uint16_t read = arrBufferRead(buf, output, 10);
        ASSERT_EQUAL_INT(read, 5, "Expected to read 5 bytes");
        ASSERT_EQUAL_STR(output, input1, 5, "Read data mismatch");
    } CASE_COMPLETE;

    TEST_CASE( "test write over after full") {
        uint16_t written3 = arrBufferWrite(buf, input3, 6);
        ASSERT_EQUAL_INT(written3, 6, "Expected to not write anything");
        ASSERT_EQUAL_STR(buf->raw[0], input3, 6, "write to full buffer should not overwrite previous data");
    } CASE_COMPLETE;

    arrBufferDeallocate(&testAllocator, &buf);
}

int main() {
    LOG_INFO("ARRAY BUFFER TESTS\n");
    initAllocator(&testAllocator, 4, testMemory, MEMORY_SIZE);
    TEST_EVAL(test_arrBufferAllocate);
    TEST_EVAL(test_arrBufferDeallocate);
    TEST_EVAL(test_arrBufferClear);
    TEST_EVAL(test_arrBufferWrite);
    TEST_EVAL(test_arrBufferRead);
    TEST_EVAL(test_arrBufferFill);
    return testGetStatus();
}