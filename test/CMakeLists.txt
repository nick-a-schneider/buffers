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

set(TEST_SOURCES
    queue.c
    buffer.c
)

foreach(test_src IN LISTS TEST_SOURCES)
    # Strip extension and prefix with "test_"
    get_filename_component(test_name ${test_src} NAME_WE)  
    set(exe_name "test_${test_name}")

    add_executable(${exe_name} ${exe_name}.c)
    target_link_libraries(${exe_name} PRIVATE
        buffers
        bitmap_allocator
        test_utils
    )

    add_test(NAME ${test_name} COMMAND ${exe_name})
endforeach()
