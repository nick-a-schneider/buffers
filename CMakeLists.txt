cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "Build unit tests" OFF)
option(USE_BITMAP_ALLOCATOR "Enable bitmap_allocator as custom dynamic allocator" OFF)
option(USE_ATOMIC "Enable locking for thread safety" ON)

project(buffers C)

add_library(buffers STATIC 
    src/queue.c
    src/buffer.c
    src/stack.c
    src/locking.c
)

if (USE_ATOMIC)
    message(STATUS "Enabling atomic locking")
    target_compile_definitions(buffers PUBLIC USE_ATOMIC)
endif()

set(BUFFERS_INSTALL_TARGETS buffers)

include(FetchContent)
if (USE_BITMAP_ALLOCATOR)
    if(NOT TARGET bitmap_allocator)
        message(STATUS "Fetching bitmap_allocator...")
        FetchContent_Declare(
            bitmap_allocator
            GIT_REPOSITORY https://github.com/nick-a-schneider/bitmap_allocator.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(bitmap_allocator)
    endif()

    if(NOT TARGET bitmap_allocator)
        message(FATAL_ERROR "Failed to fetch bitmap_allocator")
    endif()

    target_compile_definitions(buffers PUBLIC USE_BITMAP_ALLOCATOR)
    target_link_libraries(buffers PUBLIC
        bitmap_allocator
    )

    list(APPEND BUFFERS_INSTALL_TARGETS bitmap_allocator)

endif()


target_include_directories(buffers PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS ${BUFFERS_INSTALL_TARGETS}
    EXPORT buffersTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(EXPORT buffersTargets
    FILE buffersTargets.cmake
    NAMESPACE buffers::
    DESTINATION lib/cmake/buffers
)

install(DIRECTORY include/ DESTINATION include)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()
