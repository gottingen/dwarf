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


#include <collie/nlohmann/json.hpp>
#include <dwarf/dmq/middleware.h>
#include <dwarf/dmq/zmq_messenger.h>

namespace nl = nlohmann;

namespace dwarf {
    ZmqMessenger::ZmqMessenger(zmq::context_t &context)
            : m_shell_controller(context, zmq::socket_type::req),
              m_publisher_controller(context, zmq::socket_type::req),
              m_heartbeat_controller(context, zmq::socket_type::req) {
    }

    ZmqMessenger::~ZmqMessenger() {
    }

    void ZmqMessenger::connect() {
        m_shell_controller.set(zmq::sockopt::linger, get_socket_linger());
        m_shell_controller.connect(get_controller_end_point("shell"));
        m_publisher_controller.set(zmq::sockopt::linger, get_socket_linger());
        m_publisher_controller.connect(get_controller_end_point("publisher"));
        m_heartbeat_controller.set(zmq::sockopt::linger, get_socket_linger());
        m_heartbeat_controller.connect(get_controller_end_point("heartbeat"));
    }

    void ZmqMessenger::stop_channels() {
        zmq::message_t stop_msg("stop", 4);
        zmq::message_t response;

        // Wait for shell answer
        m_shell_controller.send(stop_msg, zmq::send_flags::none);
        (void) m_shell_controller.recv(response);

        // Wait for publisher answer
        m_publisher_controller.send(stop_msg, zmq::send_flags::none);
        (void) m_publisher_controller.recv(response);

        // Wait for heartbeat answer
        m_heartbeat_controller.send(stop_msg, zmq::send_flags::none);
        (void) m_heartbeat_controller.recv(response);
    }

    nl::json ZmqMessenger::send_to_shell_impl(const nl::json &message) {
        zmq::multipart_t wire_msg(message.dump());
        wire_msg.send(m_shell_controller);
        zmq::multipart_t wire_reply;
        wire_reply.recv(m_shell_controller);
        return nl::json::parse(wire_reply.popstr());
    }
}

