cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(BUILD_TESTS "Build unit tests" OFF)

project(buffers C)

include(FetchContent)
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

add_library(buffers STATIC 
    src/queue.c
    src/buffer.c
)

target_link_libraries(buffers PUBLIC
    bitmap_allocator
)

target_include_directories(buffers PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


install(TARGETS buffers bitmap_allocator
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
