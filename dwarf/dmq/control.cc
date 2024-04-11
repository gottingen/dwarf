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


#include <thread>
#include <chrono>
#include <iostream>

#include <dwarf/dmq/middleware.h>
#include <dwarf/dmq/server_zmq_split.h>
#include <dwarf/dmq/control.h>

namespace dwarf {
    Control::Control(zmq::context_t &context,
                     const std::string &transport,
                     const std::string &ip,
                     const std::string &control_port,
                     ServerZmqSplit *server)
            : m_control(context, zmq::socket_type::router), m_publisher_pub(context, zmq::socket_type::pub),
              m_messenger(context), p_server(server), m_request_stop(false) {
        init_socket(m_control, transport, ip, control_port);
        m_publisher_pub.set(zmq::sockopt::linger, get_socket_linger());
        m_publisher_pub.connect(get_publisher_end_point());
    }

    Control::~Control() {
    }

    std::string Control::get_port() const {
        return get_socket_port(m_control);
    }

    void Control::connect_messenger() {
        m_messenger.connect();
    }

    ControlMessenger &Control::get_messenger() {
        return m_messenger;
    }

    void Control::run() {
        m_request_stop = false;

        while (!m_request_stop) {
            zmq::multipart_t wire_msg;
            wire_msg.recv(m_control);
            try {
                Message msg = p_server->deserialize(wire_msg);
                p_server->notify_control_listener(std::move(msg));
            }
            catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
        }

        m_messenger.stop_channels();
        p_server->notify_control_stopped();
    }

    void Control::stop() {
        m_request_stop = true;
    }

    void Control::send_control(zmq::multipart_t &message) {
        message.send(m_control);
    }

    void Control::publish(zmq::multipart_t &message) {
        message.send(m_publisher_pub);
    }
}

