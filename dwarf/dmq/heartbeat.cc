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


#include <iterator>
#include <string>

#include <dwarf/zmq/zmq_addon.hpp>
#include <dwarf/dmq/middleware.h>
#include <dwarf/dmq/heartbeat.h>

namespace dwarf {
    Heartbeat::Heartbeat(zmq::context_t &context,
                         const std::string &transport,
                         const std::string &ip,
                         const std::string &port)
            : m_heartbeat(context, zmq::socket_type::router), m_controller(context, zmq::socket_type::rep) {
        init_socket(m_heartbeat, transport, ip, port);
        init_socket(m_controller, get_controller_end_point("heartbeat"));
    }

    Heartbeat::~Heartbeat() {
    }

    std::string Heartbeat::get_port() const {
        return get_socket_port(m_heartbeat);
    }

    void Heartbeat::run() {
        zmq::pollitem_t items[] = {
                {m_heartbeat,  0, ZMQ_POLLIN, 0},
                {m_controller, 0, ZMQ_POLLIN, 0}
        };

        while (true) {
            zmq::poll(&items[0], 2, std::chrono::milliseconds(-1));

            if (items[0].revents & ZMQ_POLLIN) {
                zmq::multipart_t wire_msg;
                wire_msg.recv(m_heartbeat);
                wire_msg.send(m_heartbeat);
            }

            if (items[1].revents & ZMQ_POLLIN) {
                // stop message
                zmq::multipart_t wire_msg;
                wire_msg.recv(m_controller);
                wire_msg.send(m_controller);
                break;
            }
        }
    }
}
