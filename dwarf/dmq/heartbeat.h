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

namespace dwarf {
    class Heartbeat {
    public:

        Heartbeat(zmq::context_t &context,
                  const std::string &transport,
                  const std::string &ip,
                  const std::string &port);

        ~Heartbeat();

        std::string get_port() const;

        void run();

    private:

        zmq::socket_t m_heartbeat;
        zmq::socket_t m_controller;
    };
}
