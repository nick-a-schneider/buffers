#include "stack.h"
#include "allocator.h"
#include "test_utils.h"
#include <string.h>

#define MEMORY_SIZE 2048
uint8_t testMemory[MEMORY_SIZE];
static Allocator testAllocator;

void test_stackAllocate() {
    TEST_CASE("Allocates and initializes stack correctly") {
        Stack* stack = stackAllocate(&testAllocator, 8, sizeof(uint16_t));
        ASSERT_NOT_NULL(stack, "Buffer should not be NULL");
        ASSERT_NOT_NULL(stack->raw, "raw pointer should not be NULL");
        ASSERT_EQUAL_INT(stack->size, 8, "size should be 8 on init");
        ASSERT_EQUAL_INT(stack->top, 0, "top should be 0 on init");
        ASSERT_EQUAL_INT(stack->type_size, sizeof(uint16_t), "type size should be 2 on init");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;

    TEST_CASE("zero length") {
        Stack* stack = stackAllocate(&testAllocator, 0, sizeof(uint16_t));
        ASSERT_NULL(stack, "Buffer should be NULL");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;

    TEST_CASE("zero size") {
        Stack* stack = stackAllocate(&testAllocator, 8, 0);
        ASSERT_NULL(stack, "Buffer should be NULL");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;
}

void test_stackDeallocate() {
    TEST_CASE("Deallocates and nullifies stack pointer") {
        Stack* stack = stackAllocate(&testAllocator, 8, sizeof(uint16_t));
        bool res = stackDeallocate(&testAllocator, &stack);
        ASSERT_TRUE(res, "Buffer deallocation failed");
        ASSERT_NULL(stack, "Buffer pointer should be NULL after free");
    } CASE_COMPLETE;

    TEST_CASE("invalid allocator") {
        Allocator* invalidAllocator = NULL;
        Stack* stack = stackAllocate(&testAllocator, 8, sizeof(uint16_t));
        bool res = stackDeallocate(invalidAllocator, &stack);
        ASSERT_FALSE(res, "Deallocating NULL stack should fail");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;

    TEST_CASE("Null stack pointer") {
        Stack* stack = NULL;
        bool res = stackDeallocate(&testAllocator, &stack);
        ASSERT_FALSE(res, "Deallocating NULL stack should fail");
    } CASE_COMPLETE;
}

void test_stackClear() {
    TEST_CASE("Clears stack") {
        Stack* stack = stackAllocate(&testAllocator, 8, sizeof(uint16_t));
        uint8_t* raw = (uint8_t*)stack->raw;
        stackClear(stack);
        ASSERT_EQUAL_INT(stack->top, 0, "top should be 0 after clear");
        ASSERT_EQUAL_PTR(raw, (uint8_t*)stack->raw, "raw pointer should not change on clear");
        ASSERT_EQUAL_INT(stack->size, 8, "size should not change on clear");
        ASSERT_EQUAL_INT(stack->type_size, sizeof(uint16_t), "type size should not change on clear");
        stackDeallocate(&testAllocator, &stack);
    } CASE_COMPLETE;
}

void test_stackPush() {
    TEST_CASE("Pushes data to stack") {
        Stack* stack = stackAllocate(&testAllocator, 8, sizeof(uint16_t));
        uint16_t data = 0x1234;
        bool res = stackPush(stack, (void*)&data);
        ASSERT_TRUE(res, "Pushing data to stack failed");
        ASSERT_EQUAL_INT(stack->top, 1, "top should be 1 after push");
        uint16_t top = *(uint16_t*)stack->raw;
        ASSERT_EQUAL_INT(top, data, "raw pointer should not change on push");
        ASSERT_EQUAL_INT(stack->size, 8, "size should not change on push");
        ASSERT_EQUAL_INT(stack->type_size, sizeof(uint16_t), "type size should not change on push");
        stackDeallocate(&testAllocator, &stack);
    } CASE_COMPLETE;

    TEST_CASE("Pushes complex structure to stack") {
        Stack* stack = stackAllocate(&testAllocator, 8, sizeof(TestStruct));
        uint32_t data = 0x1234;
        TestStruct input = {.flag = true, .data = data, .ptr = &data};
        bool res = stackPush(stack, (void*)&input);
        ASSERT_TRUE(res, "Pushing data to stack failed");
        ASSERT_EQUAL_INT(stack->top, 1, "top should be 1 after push");
        TestStruct top = *(TestStruct*)stack->raw;
        ASSERT_TRUE(top.flag, "flag should not change on push");
        ASSERT_EQUAL_INT(top.data, input.data, "data should not change on push");
        ASSERT_EQUAL_PTR(top.ptr, input.ptr, "pointer should not change on push");
        ASSERT_EQUAL_INT(stack->size, 8, "size should not change on push");
        ASSERT_EQUAL_INT(stack->type_size, sizeof(TestStruct), "type size should not change on push");
        stackDeallocate(&testAllocator, &stack);
    } CASE_COMPLETE;

    TEST_CASE("Invalid data") {
        Stack* stack = stackAllocate(&testAllocator, 8, sizeof(uint16_t));
        uint16_t* data = NULL;
        bool res = stackPush(stack, (void*)data);
        ASSERT_FALSE(res, "Pushing data to NULL stack should fail");
        stackDeallocate(&testAllocator, &stack);
    } CASE_COMPLETE;

    TEST_CASE("Null stack pointer") {
        Stack* stack = NULL;
        uint16_t data = 0x1234;
        bool res = stackPush(stack, (void*)&data);
        ASSERT_FALSE(res, "Pushing data to NULL stack should fail");
    } CASE_COMPLETE;
}

void test_stackPop() {
    TEST_CASE("Pop data from stack") {
        Stack* stack = stackAllocate(&testAllocator, 8, sizeof(uint16_t));
        uint16_t data = 0x1234;
        (void)stackPush(stack, (void*)&data);
        uint16_t popped;
        uint16_t* raw = (uint16_t*)stack->raw;
        bool res = stackPop(stack, (void*)&popped);
        ASSERT_TRUE(res, "Popping data from stack failed");
        ASSERT_EQUAL_INT(popped, 0x1234, "Popped data should be 0x1234");
        ASSERT_EQUAL_INT(stack->top, 0, "top should be 0 after pop");
        ASSERT_EQUAL_PTR(raw, (uint16_t*)stack->raw, "raw pointer should not change on pop");
        ASSERT_EQUAL_INT(stack->size, 8, "size should not change on pop");
        ASSERT_EQUAL_INT(stack->type_size, sizeof(uint16_t), "type size should not change on pop");
        stackDeallocate(&testAllocator, &stack);
    } CASE_COMPLETE;

    TEST_CASE("Pop complex structure from stack") {
        Stack* stack = stackAllocate(&testAllocator, 8, sizeof(TestStruct));
        uint32_t data = 0x1234;
        TestStruct input = {.flag = true, .data = data, .ptr = &data};
        (void)stackPush(stack, (void*)&input);
        TestStruct popped;
        TestStruct* raw = (TestStruct*)stack->raw;
        bool res = stackPop(stack, (void*)&popped);
        ASSERT_TRUE(res, "Popping data from stack failed");
        ASSERT_TRUE(popped.flag, "Popped flag should be true");
        ASSERT_EQUAL_INT(popped.data, input.data, "Popped data should be 0x1234");
        ASSERT_EQUAL_PTR(popped.ptr, input.ptr, "Popped pointer should have the same value");
        ASSERT_EQUAL_INT(stack->top, 0, "top should be 0 after pop");
        ASSERT_EQUAL_PTR(raw, (TestStruct*)stack->raw, "raw pointer should not change on pop");
        ASSERT_EQUAL_INT(stack->size, 8, "size should not change on pop");
        ASSERT_EQUAL_INT(stack->type_size, sizeof(TestStruct), "type size should not change on pop");
        stackDeallocate(&testAllocator, &stack);
    } CASE_COMPLETE;

    TEST_CASE("Empty stack") {
        Stack* stack = stackAllocate(&testAllocator, 8, sizeof(uint16_t));
        uint16_t data = 0x1234;
        bool res = stackPop(stack, (void*)&data);
        ASSERT_FALSE(res, "Popping data from empty stack should fail");
        ASSERT_EQUAL_INT(data, 0x1234, "Popped data should not be overwritten");
        stackDeallocate(&testAllocator, &stack);
    } CASE_COMPLETE;

    TEST_CASE("Invalid data") {
        Stack* stack = stackAllocate(&testAllocator, 8, sizeof(uint16_t));
        uint16_t* data = NULL;
        bool res = stackPop(stack, (void*)data);
        ASSERT_FALSE(res, "Popping data from NULL stack should fail");
        stackDeallocate(&testAllocator, &stack);
    } CASE_COMPLETE;

    TEST_CASE("Null stack pointer") {
        Stack* stack = NULL;
        uint16_t data = 0x1234;
        bool res = stackPop(stack, (void*)&data);
        ASSERT_FALSE(res, "Popping data from NULL stack should fail");
    } CASE_COMPLETE;
}

void test_stackFilled() {
    Stack* stack = stackAllocate(&testAllocator, 2, sizeof(uint16_t));
    uint16_t data1 = 0x1234;
    uint16_t data2 = 0x5678;
    uint16_t data3 = 0x9abc;
    TEST_CASE("Pushing data to filled stack") {
        (void)stackPush(stack, (void*)&data1);
        (void)stackPush(stack, (void*)&data2);
        bool res = stackPush(stack, (void*)&data3);
        ASSERT_FALSE(res, "Pushing data to filled stack should fail");
        uint16_t top = ((uint16_t*)stack->raw)[stack->top - 1];
        ASSERT_EQUAL_INT(top, data2, "top should still be data2 after push");
        uint16_t _ ;
        (void)stackPop(stack, (void*)&_);
        res = stackPush(stack, (void*)&data3);
        ASSERT_TRUE(res, "Pushing data should succeed after pop");
    } CASE_COMPLETE;
    
    stackDeallocate(&testAllocator, &stack);
}

int main() {
    LOG_INFO("STACK TESTS\n");
    initAllocator(&testAllocator, 4, testMemory, MEMORY_SIZE);
    TEST_EVAL(test_stackAllocate);
    TEST_EVAL(test_stackDeallocate);
    TEST_EVAL(test_stackClear);
    TEST_EVAL(test_stackPush);
    TEST_EVAL(test_stackPop);
    TEST_EVAL(test_stackFilled);
    return testGetStatus();
}
