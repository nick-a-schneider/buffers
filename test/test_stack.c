#include "stack.h"
#ifdef USE_BITMAP_ALLOCATOR
#include "block_allocator.h"
#endif
#include "test_utils.h"
#include <string.h>
#include <errno.h>

#ifdef USE_BITMAP_ALLOCATOR
#define MEMORY_SIZE 2048
uint8_t testMemory[MEMORY_SIZE];
static BlockAllocator testAllocator;

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
        int res = stackDeallocate(&testAllocator, &stack);
        ASSERT_EQUAL_INT(res, STACK_OK, "Buffer deallocation failed");
        ASSERT_NULL(stack, "Buffer pointer should be NULL after free");
    } CASE_COMPLETE;

    TEST_CASE("invalid allocator") {
        BlockAllocator* invalidAllocator = NULL;
        Stack* stack = stackAllocate(&testAllocator, 8, sizeof(uint16_t));
        int res = stackDeallocate(invalidAllocator, &stack);
        ASSERT_EQUAL_INT(res, -EINVAL, "Deallocating NULL stack should fail");
        memset(testMemory, 0, sizeof(testMemory));
    } CASE_COMPLETE;

    TEST_CASE("Null stack pointer") {
        Stack* stack = NULL;
        int res = stackDeallocate(&testAllocator, &stack);
        ASSERT_EQUAL_INT(res, -EINVAL, "Deallocating NULL stack should fail");
    } CASE_COMPLETE;
}
#endif

void test_createStackMacro() {
    TEST_CASE("Creates stack correctly") {
        CREATE_STACK(stack, 8, sizeof(uint16_t));
        ASSERT_NOT_NULL(stack.raw, "raw pointer should not be NULL");
        ASSERT_EQUAL_INT(stack.size, 8, "size should be 8 on init");
        ASSERT_EQUAL_INT(stack.top, 0, "top should be 0 on init");
        ASSERT_EQUAL_INT(stack.type_size, sizeof(uint16_t), "type size should be 2 on init");
    } CASE_COMPLETE;
}

void test_stackClear() {
    TEST_CASE("Clears stack") {
        CREATE_STACK(stack, 8, sizeof(uint16_t));
        uint8_t* raw = (uint8_t*)stack.raw;
        stackClear(&stack);
        ASSERT_EQUAL_INT(stack.top, 0, "top should be 0 after clear");
        ASSERT_EQUAL_PTR(raw, (uint8_t*)stack.raw, "raw pointer should not change on clear");
        ASSERT_EQUAL_INT(stack.size, 8, "size should not change on clear");
        ASSERT_EQUAL_INT(stack.type_size, sizeof(uint16_t), "type size should not change on clear");
        
    } CASE_COMPLETE;
}

void test_stackPush() {
    TEST_CASE("Pushes data to stack") {
        CREATE_STACK(stack, 8, sizeof(uint16_t));
        uint16_t data = 0x1234;
        int res = stackPush(&stack, (void*)&data);
        ASSERT_EQUAL_INT(res, STACK_OK, "Pushing data to stack failed");
        ASSERT_EQUAL_INT(stack.top, 1, "top should be 1 after push");
        uint16_t top = *(uint16_t*)stack.raw;
        ASSERT_EQUAL_INT(top, data, "raw pointer should not change on push");
        ASSERT_EQUAL_INT(stack.size, 8, "size should not change on push");
        ASSERT_EQUAL_INT(stack.type_size, sizeof(uint16_t), "type size should not change on push");
        
    } CASE_COMPLETE;

    TEST_CASE("Pushes complex structure to stack") {
        CREATE_STACK(stack, 8, sizeof(TestStruct));
        uint32_t data = 0x1234;
        TestStruct input = {.flag = true, .data = data, .ptr = &data};
        int res = stackPush(&stack, (void*)&input);
        ASSERT_EQUAL_INT(res, STACK_OK, "Pushing data to stack failed");
        ASSERT_EQUAL_INT(stack.top, 1, "top should be 1 after push");
        TestStruct top = *(TestStruct*)stack.raw;
        ASSERT_TRUE(top.flag, "flag should not change on push");
        ASSERT_EQUAL_INT(top.data, input.data, "data should not change on push");
        ASSERT_EQUAL_PTR(top.ptr, input.ptr, "pointer should not change on push");
        ASSERT_EQUAL_INT(stack.size, 8, "size should not change on push");
        ASSERT_EQUAL_INT(stack.type_size, sizeof(TestStruct), "type size should not change on push");
        
    } CASE_COMPLETE;

    TEST_CASE("Invalid data") {
        CREATE_STACK(stack, 8, sizeof(uint16_t));
        uint16_t* data = NULL;
        int res = stackPush(&stack, (void*)data);
        ASSERT_EQUAL_INT(res, -EINVAL, "Pushing data to NULL stack should fail");
        
    } CASE_COMPLETE;

    TEST_CASE("Null stack pointer") {
        Stack* stack = NULL;
        uint16_t data = 0x1234;
        int res = stackPush(stack, (void*)&data);
        ASSERT_EQUAL_INT(res, -EINVAL, "Pushing data to NULL stack should fail");
    } CASE_COMPLETE;
}

void test_stackPop() {
    TEST_CASE("Pop data from stack") {
        CREATE_STACK(stack, 8, sizeof(uint16_t));
        uint16_t data = 0x1234;
        (void)stackPush(&stack, (void*)&data);
        uint16_t popped;
        uint16_t* raw = (uint16_t*)stack.raw;
        int res = stackPop(&stack, (void*)&popped);
        ASSERT_EQUAL_INT(res, STACK_OK, "Popping data from stack failed");
        ASSERT_EQUAL_INT(popped, 0x1234, "Popped data should be 0x1234");
        ASSERT_EQUAL_INT(stack.top, 0, "top should be 0 after pop");
        ASSERT_EQUAL_PTR(raw, (uint16_t*)stack.raw, "raw pointer should not change on pop");
        ASSERT_EQUAL_INT(stack.size, 8, "size should not change on pop");
        ASSERT_EQUAL_INT(stack.type_size, sizeof(uint16_t), "type size should not change on pop");
        
    } CASE_COMPLETE;

    TEST_CASE("Pop complex structure from stack") {
        CREATE_STACK(stack, 8, sizeof(TestStruct));
        uint32_t data = 0x1234;
        TestStruct input = {.flag = true, .data = data, .ptr = &data};
        (void)stackPush(&stack, (void*)&input);
        TestStruct popped;
        TestStruct* raw = (TestStruct*)stack.raw;
        int res = stackPop(&stack, (void*)&popped);
        ASSERT_EQUAL_INT(res, STACK_OK, "Popping data from stack failed");
        ASSERT_TRUE(popped.flag, "Popped flag should be true");
        ASSERT_EQUAL_INT(popped.data, input.data, "Popped data should be 0x1234");
        ASSERT_EQUAL_PTR(popped.ptr, input.ptr, "Popped pointer should have the same value");
        ASSERT_EQUAL_INT(stack.top, 0, "top should be 0 after pop");
        ASSERT_EQUAL_PTR(raw, (TestStruct*)stack.raw, "raw pointer should not change on pop");
        ASSERT_EQUAL_INT(stack.size, 8, "size should not change on pop");
        ASSERT_EQUAL_INT(stack.type_size, sizeof(TestStruct), "type size should not change on pop");
        
    } CASE_COMPLETE;

    TEST_CASE("Empty stack") {
        CREATE_STACK(stack, 8, sizeof(uint16_t));
        uint16_t data = 0x1234;
        int res = stackPop(&stack, (void*)&data);
        ASSERT_EQUAL_INT(res, -EAGAIN, "Popping data from empty stack should fail");
        ASSERT_EQUAL_INT(data, 0x1234, "Popped data should not be overwritten");
        
    } CASE_COMPLETE;

    TEST_CASE("Invalid data") {
        CREATE_STACK(stack, 8, sizeof(uint16_t));
        uint16_t* data = NULL;
        int res = stackPop(&stack, (void*)data);
        ASSERT_EQUAL_INT(res, -EINVAL, "Popping data from NULL stack should fail");
        
    } CASE_COMPLETE;

    TEST_CASE("Null stack pointer") {
        Stack* stack = NULL;
        uint16_t data = 0x1234;
        int res = stackPop(stack, (void*)&data);
        ASSERT_EQUAL_INT(res, -EINVAL, "Popping data from NULL stack should fail");
    } CASE_COMPLETE;
}

void test_stackFilled() {
    CREATE_STACK(stack, 2, sizeof(uint16_t));
    uint16_t data1 = 0x1234;
    uint16_t data2 = 0x5678;
    uint16_t data3 = 0x9abc;
    TEST_CASE("Pushing data to filled stack") {
        (void)stackPush(&stack, (void*)&data1);
        (void)stackPush(&stack, (void*)&data2);
        int res = stackPush(&stack, (void*)&data3);
        ASSERT_EQUAL_INT(res, -ENOSPC, "Pushing data to filled stack should fail");
        uint16_t top = ((uint16_t*)stack.raw)[stack.top - 1];
        ASSERT_EQUAL_INT(top, data2, "top should still be data2 after push");
        uint16_t _ ;
        (void)stackPop(&stack, (void*)&_);
        res = stackPush(&stack, (void*)&data3);
        ASSERT_EQUAL_INT(res, STACK_OK, "Pushing data should succeed after pop");
    } CASE_COMPLETE;
    
    
}

int main() {
    LOG_INFO("STACK TESTS\n");
#ifdef USE_BITMAP_ALLOCATOR
    initBlockAllocator(&testAllocator, 4, testMemory, MEMORY_SIZE);
    TEST_EVAL(test_stackAllocate);
    TEST_EVAL(test_stackDeallocate);
#endif
    TEST_EVAL(test_createStackMacro);
    TEST_EVAL(test_stackClear);
    TEST_EVAL(test_stackPush);
    TEST_EVAL(test_stackPop);
    TEST_EVAL(test_stackFilled);
    return testGetStatus();
}
