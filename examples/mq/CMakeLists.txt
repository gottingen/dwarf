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

set(EXECUTABLE_NAME my_kernel)

# Configuration
# =============

include(GNUInstallDirs)
include_directories(${dwarf_SOURCE_DIR})
# We generate the kernel.json file, given the installation prefix and the executable name
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/my_kernel/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/my_kernel/kernel.json"
)


find_package(Threads)

# Flags
# =====

include(CheckCXXCompilerFlag)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    CHECK_CXX_COMPILER_FLAG("-std=c++14" HAS_CPP14_FLAG)

    if (HAS_CPP14_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    else()
        message(FATAL_ERROR "Unsupported compiler -- dwarf requires C++14 support!")
    endif()
endif()

# Target and link
# ===============

# my_kernel source files
set(MY_KERNEL_SRC
        src/custom_interpreter.cc
        src/custom_interpreter.hc
)

# My kernel executable
add_executable(${EXECUTABLE_NAME} src/main.cc ${MY_KERNEL_SRC} )
target_link_libraries(${EXECUTABLE_NAME} PRIVATE dwarf::dwarf Threads::Threads)

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

# Installation
# ============

# Install my_kernel
install(TARGETS ${EXECUTABLE_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Configuration and data directories for jupyter and my_kernel
set(JUPYTER_DATA_DIR "share/jupyter" CACHE STRING "Jupyter data directory")

# Install Jupyter kernelspecs
set(MY_KERNELSPEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels)
install(DIRECTORY ${MY_KERNELSPEC_DIR}
        DESTINATION ${JUPYTER_DATA_DIR}
        PATTERN "*.in" EXCLUDE)
