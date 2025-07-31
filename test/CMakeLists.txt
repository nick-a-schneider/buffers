include(FetchContent)

if(NOT TARGET test_utils)
    message(STATUS "Fetching test_utils...")
    FetchContent_Declare(
        test_utils
        GIT_REPOSITORY https://github.com/nick-a-schneider/test_utils.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(test_utils)
endif()

if(NOT TARGET test_utils)
    message(FATAL_ERROR "Failed to fetch test_utils")
endif()

add_executable(test_queue test_queue.c)
target_link_libraries(test_queue PRIVATE 
    buffers
    bitmap_allocator
    test_utils
)

add_test(NAME queue_test COMMAND test_buffers)
