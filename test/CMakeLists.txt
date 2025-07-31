include(FetchContent)
message(STATUS "Fetching test_utils...")
FetchContent_Declare(
    test_utils
    GIT_REPOSITORY https://github.com/nick-a-schneider/test_utils.git
    GIT_TAG master
)
FetchContent_MakeAvailable(test_utils)

add_executable(test_arr_buffer test_arr_buffer.c)
target_link_libraries(test_arr_buffer PRIVATE 
    buffers
    bitmap_allocator
    test_utils
)

add_test(NAME arr_buffer_test COMMAND test_buffers)
