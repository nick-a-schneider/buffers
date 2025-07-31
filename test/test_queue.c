#include "queue.h"
#include "allocator.h"
#include "test_utils.h"
#include <string.h>

#define MEMORY_SIZE 2048
uint8_t testMemory[MEMORY_SIZE];
static Allocator testAllocator;

void test_queueAllocate() {
    TEST_CASE("Allocates and initializes buffer correctly") {
        Queue* buf = queueAllocate(&testAllocator, 8, 4);
        ASSERT_NOT_NULL(buf, "Buffer should not be NULL");
        ASSERT_NOT_NULL(buf->slots, "slots pointer should not be NULL");
        ASSERT_NOT_NULL(buf->msg_len, "msg_len pointer should not be NULL");
        ASSERT_EQUAL_INT(buf->slot_len, 8, "slot_len mismatch");
        ASSERT_EQUAL_INT(buf->slot_cnt, 4, "slot_cnt mismatch");
        ASSERT_FALSE(buf->full, "should not be full on init");
        for (int i = 0; i < 4; ++i) ASSERT_EQUAL_INT(buf->msg_len[i], 0, "msg_len[i] should be 0 on init");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;

    TEST_CASE("insufficient memory") {
        Queue* buf = queueAllocate(&testAllocator, 512, 512);
        ASSERT_NULL(buf, "Buffer should be NULL");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;

    TEST_CASE("invalid allocator") {
        Allocator* invalidAllocator = NULL;
        Queue* buf = queueAllocate(invalidAllocator, 2, 2);
        ASSERT_NULL(buf, "should return NULL on invalid allocator");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;

    TEST_CASE("zero dimensions") {
        Queue* buf = queueAllocate(&testAllocator, 0, 2);
        ASSERT_NULL(buf, "should return NULL if `slot_len` is zero");
        buf = queueAllocate(&testAllocator, 2, 0);
        ASSERT_NULL(buf, "should return NULL if `size` is zero");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;
}

void test_queueDeallocate() {
    TEST_CASE("Deallocates and nullifies buffer pointer") {
        Queue* buf = queueAllocate(&testAllocator, 8, 4);
        bool success = queueDeallocate(&testAllocator, &buf);
        ASSERT_TRUE(success, "Buffer deallocation failed");
        ASSERT_NULL(buf, "Buffer pointer should be NULL after free");
    } CASE_COMPLETE;

    TEST_CASE("invalid allocator") {
        Allocator* invalidAllocator = NULL;
        Queue* buf = queueAllocate(&testAllocator, 8, 4);
        bool success = queueDeallocate(invalidAllocator, &buf);
        ASSERT_FALSE(success, "Deallocating NULL buffer should fail");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;

    TEST_CASE("Null buffer pointer") {
        Queue* buf = NULL;
        bool success = queueDeallocate(&testAllocator, &buf);
        ASSERT_FALSE(success, "Deallocating NULL buffer should fail");
    } CASE_COMPLETE;

}

void test_queueClear() {
    TEST_CASE("Clears head/tail/full and msg_len array") {
        Queue* buf = queueAllocate(&testAllocator, 8, 4);
        buf->head = 2; buf->tail = 1; buf->full = true;
        for (int i = 0; i < 4; ++i) buf->msg_len[i] = 4;
        queueClear(buf);
        ASSERT_EQUAL_INT(buf->head, 0, "head not reset");
        ASSERT_EQUAL_INT(buf->tail, 0, "tail not reset");
        ASSERT_FALSE(buf->full, "full not reset");
        for (int i = 0; i < 4; ++i)
            ASSERT_EQUAL_INT(buf->msg_len[i], 0, "msg_len[i] not cleared");
        queueDeallocate(&testAllocator, &buf);
    } CASE_COMPLETE;
}

void test_queueWrite() {
    TEST_CASE("Writes correctly") {
        Queue* buf = queueAllocate(&testAllocator, 8, 2);
        uint8_t* input = "SwampWho";
        uint16_t written = queueWrite(buf, input, 8);
        ASSERT_EQUAL_INT(written, 8, "Expected to write 8 bytes");
        ASSERT_FALSE(queueIsEmpty(buf), "Buffer shouldn't be empty after write");
        ASSERT_FALSE(queueIsFull(buf), "Buffer shouldn't be full yet");
        ASSERT_EQUAL_INT(buf->head, 1, "Head did not move after write");
        ASSERT_EQUAL_INT(buf->tail, 0, "Tail moved after write");
        ASSERT_EQUAL_INT(buf->msg_len[0], 8, "Expected to write 8 bytes");
        ASSERT_EQUAL_STR(buf->slots[0], input, 8, "Written data mismatch");
        queueDeallocate(&testAllocator, &buf);
    } CASE_COMPLETE;

    TEST_CASE("Write oversided message") {
        memset(testMemory, 0, sizeof(testMemory));
        Queue* buf = queueAllocate(&testAllocator, 4, 2);
        uint8_t* input = "SwampWho";
        uint16_t written = queueWrite(buf, input, 8);
        ASSERT_EQUAL_INT(written, 4, "Expected to write 4 bytes");
        ASSERT_EQUAL_STR(buf->slots[0], input, 4, "Written data mismatch");
        ASSERT_NOT_EQUAL_STR(buf->slots[0], input, 8, "written data overflowed");
        queueDeallocate(&testAllocator, &buf);
    }  CASE_COMPLETE;

    TEST_CASE("Invalid buffer") {
        Queue* buf = NULL;
        uint8_t* input = "SwampWho";
        uint16_t written = queueWrite(buf, input, 8);
        ASSERT_EQUAL_INT(written, 0, "Expected to write 0 bytes");
    }  CASE_COMPLETE;

    TEST_CASE("Invalid data") {
        Queue* buf = queueAllocate(&testAllocator, 8, 2);
        uint8_t* input = NULL;
        uint16_t written = queueWrite(buf, input, 8);
        ASSERT_EQUAL_INT(written, 0, "Expected to write 0 bytes");
        queueDeallocate(&testAllocator, &buf);
    }  CASE_COMPLETE;

    TEST_CASE("zero slot_length") {
        Queue* buf = queueAllocate(&testAllocator, 8, 2);
        uint8_t* input = "SwampWho";
        uint16_t written = queueWrite(buf, input, 0);
        ASSERT_EQUAL_INT(written, 0, "Expected to write 0 bytes");
        queueDeallocate(&testAllocator, &buf);
    }  CASE_COMPLETE;
}

void test_queueRead() {
    TEST_CASE("Reads correctly") {
        Queue* buf = queueAllocate(&testAllocator, 8, 2);
        uint8_t* input = "SwampWho";
        uint8_t output[10];
        (void)queueWrite(buf, input, 8);
        uint16_t read = queueRead(buf, output, 10);
        ASSERT_EQUAL_INT(read, 8, "Expected to read 8 bytes");
        ASSERT_EQUAL_STR(output, input, 8, "Read data mismatch");
        ASSERT_TRUE(queueIsEmpty(buf), "Buffer should be empty after read");
        ASSERT_FALSE(queueIsFull(buf), "Buffer shouldn't be full yet");
        ASSERT_EQUAL_INT(buf->head, 1, "Head moved after read");
        ASSERT_EQUAL_INT(buf->tail, 1, "Tail did not move after read");
        ASSERT_EQUAL_INT(buf->msg_len[0], 0, "msg_len was not reset after read");
        
        queueDeallocate(&testAllocator, &buf);
    } CASE_COMPLETE;

    TEST_CASE("read oversided message") {
        memset(testMemory, 0, sizeof(testMemory));
        Queue* buf = queueAllocate(&testAllocator, 15, 2);
        uint8_t* input = "Hello World!\n";
        uint8_t output[15];
        (void)queueWrite(buf,input, 13);
        uint16_t read = queueRead(buf, output, 8);
        ASSERT_EQUAL_INT(read, 8, "Expected to read 8 bytes");
        ASSERT_EQUAL_STR(output, input, 8, "Read data mismatch");
        ASSERT_NOT_EQUAL_STR(output, input, 13, "read data overflowed");
        queueDeallocate(&testAllocator, &buf);
    }  CASE_COMPLETE;

    TEST_CASE("Invalid buffer") {
        Queue* buf = NULL;
        uint8_t output[8];
        uint16_t read = queueRead(buf, output, 8);
        ASSERT_EQUAL_INT(read, 0, "Cannot read from NULL buffer");
    }  CASE_COMPLETE;

    TEST_CASE("Invalid output") {
        Queue* buf = queueAllocate(&testAllocator, 8, 2);
        uint8_t* input = "SwampWho";
        (void)queueWrite(buf, input, 8);
        uint16_t read = queueRead(buf, NULL, 8);
        ASSERT_EQUAL_INT(read, 0, "Cannot read to NULL buffer");
        queueDeallocate(&testAllocator, &buf);
    }  CASE_COMPLETE;

    TEST_CASE("zero slot_length") {
        Queue* buf = queueAllocate(&testAllocator, 8, 2);
        uint8_t output[8];
        uint16_t read = queueRead(buf, output, 0);
        ASSERT_EQUAL_INT(read, 0, "Expected to read 0 bytes");
        queueDeallocate(&testAllocator, &buf);
    }  CASE_COMPLETE;
}

void test_QueueFill() {
    Queue* buf = queueAllocate(&testAllocator, 8, 2);
    uint8_t* input1 = "swamp";
    uint8_t* input2 = "moose";
    uint8_t* input3 = "lagoon";

    TEST_CASE( "test bufferIsFull") {
        (void)queueWrite(buf, input1, 5);
        (void)queueWrite(buf, input2, 5);
        ASSERT_TRUE(queueIsFull(buf), "Buffer should be full");
    } CASE_COMPLETE;

    TEST_CASE( "test write to full buffer") {
        uint16_t written3 = queueWrite(buf, input3, 6);
        ASSERT_EQUAL_INT(written3, 0, "Expected to not write anything");
        ASSERT_EQUAL_STR(buf->slots[0], input1, 5, "write to full buffer should not overwrite previous data");
    } CASE_COMPLETE;

    TEST_CASE("test read from full buffer") {
        uint8_t output[10];
        uint16_t read = queueRead(buf, output, 10);
        ASSERT_EQUAL_INT(read, 5, "Expected to read 5 bytes");
        ASSERT_EQUAL_STR(output, input1, 5, "Read data mismatch");
    } CASE_COMPLETE;

    TEST_CASE( "test write over after full") {
        uint16_t written3 = queueWrite(buf, input3, 6);
        ASSERT_EQUAL_INT(written3, 6, "Expected to not write anything");
        ASSERT_EQUAL_STR(buf->slots[0], input3, 6, "write to full buffer should not overwrite previous data");
    } CASE_COMPLETE;

    queueDeallocate(&testAllocator, &buf);
}

int main() {
    LOG_INFO("QUEUE TESTS\n");
    initAllocator(&testAllocator, 4, testMemory, MEMORY_SIZE);
    TEST_EVAL(test_queueAllocate);
    TEST_EVAL(test_queueDeallocate);
    TEST_EVAL(test_queueClear);
    TEST_EVAL(test_queueWrite);
    TEST_EVAL(test_queueRead);
    TEST_EVAL(test_QueueFill);
    return testGetStatus();
}