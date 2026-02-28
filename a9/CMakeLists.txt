cmake_minimum_required(VERSION 3.14)
project(
  heap_allocator 
  VERSION 1.0
  LANGUAGES C CXX)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "-g")

include_directories(include)


add_executable(main src/main.c src/alloc.c )



# remove using list() list(REMOVE_ITEM TEST_SOURCES
# "${CMAKE_CURRENT_SOURCE_DIR}/src/main_shell.c") list(REMOVE_ITEM TEST_SOURCES
#"${CMAKE_CURRENT_SOURCE_DIR}/src/shell.c")

#FetchContent_Declare(
#  googletest
#  DOWNLOAD_EXTRACT_TIMESTAMP false
#  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
#FetchContent_MakeAvailable(googletest)



#target_link_libraries(shell_test GTest::gtest_main)
#include(GoogleTest)
#gtest_discover_tests(shell_test)
