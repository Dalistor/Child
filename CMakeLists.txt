cmake_minimum_required(VERSION 3.20)
project(Child CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
# Portable by default: the same seed must give the same weights on any x86-64
# machine, so floating point is never contracted into FMA behind our back.
option(CHILD_NATIVE "Optimize for this CPU (-march=native); weights are no longer reproducible across machines" OFF)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
add_compile_options(-ffp-contract=off)
if(CHILD_NATIVE)
  add_compile_options(-march=native)
endif()

add_library(child_core STATIC
  src/capture/capture.cpp
  src/model/model.cpp
  src/engram/engram.cpp
  src/neuron/neuron.cpp
  src/synapse/synapse.cpp
  src/synapse/homeostasis.cpp
  src/dopamine/dopamine.cpp
  src/interneuron/interneuron.cpp
  src/thalamus/thalamus.cpp)
target_include_directories(child_core PUBLIC src)
target_compile_options(child_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(child src/main.cpp)
target_link_libraries(child PRIVATE child_core)
target_compile_options(child PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_executable(child_tests tests/tests.cpp)
target_link_libraries(child_tests PRIVATE child_core)
add_test(NAME child_tests COMMAND child_tests)
add_test(NAME cli_branching
  COMMAND sh ${CMAKE_SOURCE_DIR}/tests/cli_branching.sh $<TARGET_FILE:child> ${CMAKE_SOURCE_DIR}/src/model/rng.hpp)

# REG-01: folder naming. Python is not a build dependency; without it the check is skipped.
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
  add_test(NAME brain_layout
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/brain_layout.py ${CMAKE_SOURCE_DIR}/src)
endif()
