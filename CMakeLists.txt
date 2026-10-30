cmake_minimum_required(VERSION 3.20)
project(Child CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

add_library(child_core STATIC
  src/capture.cpp
  src/model.cpp
  src/store.cpp)
target_include_directories(child_core PUBLIC src)
target_compile_options(child_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(child src/main.cpp)
target_link_libraries(child PRIVATE child_core)
target_compile_options(child PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_executable(child_tests tests/tests.cpp)
target_link_libraries(child_tests PRIVATE child_core)
add_test(NAME child_tests COMMAND child_tests)
