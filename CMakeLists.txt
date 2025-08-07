cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(BUILD_TESTS "Build unit tests" OFF)
option(USE_BITMAP_ALLOCATOR "Enable bitmap_allocator as custom dynamic allocator" OFF)

project(buffers C)
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
endif()

add_library(buffers STATIC 
    src/queue.c
    src/buffer.c
    src/stack.c
)

if (USE_BITMAP_ALLOCATOR)
target_compile_definitions(buffers PUBLIC USE_BITMAP_ALLOCATOR)
target_link_libraries(buffers PUBLIC
    bitmap_allocator
)
endif()

target_include_directories(buffers PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set(BUFFERS_INSTALL_TARGETS buffers)

if(USE_BITMAP_ALLOCATOR)
    list(APPEND BUFFERS_INSTALL_TARGETS bitmap_allocator)
endif()

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
