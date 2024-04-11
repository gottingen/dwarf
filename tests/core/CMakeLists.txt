#
# Copyright 2023 The titan-search Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
find_package(Threads REQUIRED)
set(DWARF_TESTS
    in_memory_history_manager_test.cc
    kernel_test.cc
)

set(DWARF_TEST_SRCS
    mock_interpreter.h
    mock_interpreter.cc
    mock_server.h
    mock_server.cc)

foreach(filename IN LISTS DWARF_TESTS)
    get_filename_component(targetname ${filename} NAME_WE)
    carbin_cc_test(
            NAME ${targetname}
            SOURCES test_main.cc ${filename} ${DWARF_TEST_SRCS}
            DEPS dwarf::dwarf Threads::Threads
            COPTS ${CARBIN_CXX_OPTIONS}
    )
endforeach()
