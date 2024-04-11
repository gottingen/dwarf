# Copyright 2024 The EA Authors.
# part of Elastic AI Search
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

enable_testing()
set(DWARF_ZMQ_TEST_DIR ${dwarf_SOURCE_DIR}/tests/mq)
include_directories(${dwarf_SOURCE_DIR})
message(STATUS "Forcing tests build type to Release")
set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)

include(CheckCXXCompilerFlag)

string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

if(nlohmann_json_FOUND)
  add_definitions(-DHAVE_NLOHMANN_JSON)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES Clang OR CMAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Intel)
    add_compile_options(-Wunused-parameter -Wextra -Wreorder -Wconversion -Wsign-conversion)

    CHECK_CXX_COMPILER_FLAG(-march=native HAS_MARCH_NATIVE)
    if (HAS_MARCH_NATIVE)
        add_compile_options(-march=native)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    add_compile_options(/EHsc /MP /bigobj)
    set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
endif()


find_package(Threads)

find_program(PYTEST  
    NAMES pytest-3 pytest py.test-3 py.test REQUIRED)

# Test_kernel tests
# =================

set(TEST_KERNEL_SOURCES
        mock_interpreter.cc
        mock_interpreter.h
        main.cc)

configure_file(
    "${DWARF_ZMQ_TEST_DIR}/test_kernel/kernel.json.in"
    "kernels/test_kernel/kernel.json"
)

add_executable(test_kernel ${TEST_KERNEL_SOURCES})
target_link_libraries(test_kernel PRIVATE  dwarf::dwarf Threads::Threads)
target_compile_features(test_kernel PRIVATE cxx_std_11)

set(CONNECTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/connection.json)

configure_file(
    "${DWARF_ZMQ_TEST_DIR}/test_kernel.py"
    "${CMAKE_CURRENT_BINARY_DIR}/"
    COPYONLY)

add_test(NAME test_kernel
    COMMAND ${PYTEST} test_kernel.py)
set_tests_properties(test_kernel
    PROPERTIES
    ENVIRONMENT "JUPYTER_PATH=${CMAKE_CURRENT_BINARY_DIR}")

# Test_kernel_control tests
# =========================

set(TEST_KERNEL_SPLIT_SOURCES
        mock_interpreter.cc
        mock_interpreter.h
        main_control.cc)

configure_file(
    "${DWARF_ZMQ_TEST_DIR}/test_kernel_control/kernel.json.in"
    "kernels/test_kernel_control/kernel.json"
)

add_executable(test_kernel_control ${TEST_KERNEL_SPLIT_SOURCES})
target_link_libraries(test_kernel_control PRIVATE dwarf::dwarf Threads::Threads)
target_compile_features(test_kernel_control PRIVATE cxx_std_11)

set(CONNECTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/connection.json)

configure_file(
    "${DWARF_ZMQ_TEST_DIR}/test_kernel_control.py"
    "${CMAKE_CURRENT_BINARY_DIR}/"
    COPYONLY)

add_test(NAME test_kernel_control
    COMMAND ${PYTEST} test_kernel_control.py)
set_tests_properties(test_kernel_control
    PROPERTIES
    ENVIRONMENT "JUPYTER_PATH=${CMAKE_CURRENT_BINARY_DIR}")

# Test_kernel_shell tests
# =======================

set(TEST_KERNEL_SPLIT_SOURCES
        mock_interpreter.cc
        mock_interpreter.h
        main_shell.cc)

configure_file(
    "${DWARF_ZMQ_TEST_DIR}/test_kernel_shell/kernel.json.in"
    "kernels/test_kernel_shell/kernel.json"
)

add_executable(test_kernel_shell ${TEST_KERNEL_SPLIT_SOURCES})
target_link_libraries(test_kernel_shell PRIVATE dwarf::dwarf  Threads::Threads)
target_compile_features(test_kernel_shell PRIVATE cxx_std_11)

set(CONNECTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/connection.json)

configure_file(
    "${DWARF_ZMQ_TEST_DIR}/test_kernel_shell.py"
    "${CMAKE_CURRENT_BINARY_DIR}/"
    COPYONLY)

add_test(NAME test_kernel_shell
    COMMAND ${PYTEST} test_kernel_shell.py)
set_tests_properties(test_kernel_shell
    PROPERTIES
    ENVIRONMENT "JUPYTER_PATH=${CMAKE_CURRENT_BINARY_DIR}")

