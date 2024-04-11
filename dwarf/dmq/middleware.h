// Copyright 2024 The EA Authors.
// part of Elastic AI Search
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//


#pragma once

#include <string>

#include <dwarf/zmq/zmq.hpp>
#include <dwarf/core/config.h>

namespace dwarf
{
    DWARF_API
    std::string get_controller_end_point(const std::string& channel);
    
    DWARF_API
    std::string get_publisher_end_point();

    DWARF_API
    std::string get_end_point(const std::string& transport,
                              const std::string& ip,
                              const std::string& port);

    DWARF_API
    int get_socket_linger();

    DWARF_API
    void init_socket(zmq::socket_t& socket,
                     const std::string& transport,
                     const std::string& ip,
                     const std::string& port);
    DWARF_API
    void init_socket(zmq::socket_t& socket, const std::string& end_point);

    DWARF_API
    std::string get_socket_port(const zmq::socket_t& socket);

    DWARF_API
    std::string find_free_port(std::size_t max_tries = 100, int start = 49152, int stop = 65536);
}
