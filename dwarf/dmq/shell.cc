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
#include <dwarf/dmq/shell.h>

namespace dwarf {
    Shell::Shell(zmq::context_t &context,
                 const std::string &transport,
                 const std::string &ip,
                 const std::string &shell_port,
                 const std::string &stdin_port,
                 ServerZmqSplit *server)
            : m_shell(context, zmq::socket_type::router), m_stdin(context, zmq::socket_type::router),
              m_publisher_pub(context, zmq::socket_type::pub), m_controller(context, zmq::socket_type::rep),
              p_server(server) {
        init_socket(m_shell, transport, ip, shell_port);
        init_socket(m_stdin, transport, ip, stdin_port);
        m_publisher_pub.set(zmq::sockopt::linger, get_socket_linger());
        m_publisher_pub.connect(get_publisher_end_point());

        m_controller.set(zmq::sockopt::linger, get_socket_linger());
        m_controller.bind(get_controller_end_point("shell"));
    }

    Shell::~Shell() {
    }

    std::string Shell::get_shell_port() const {
        return get_socket_port(m_shell);
    }

    std::string Shell::get_stdin_port() const {
        return get_socket_port(m_stdin);
    }

    void Shell::run() {
        zmq::pollitem_t items[] = {
                {m_shell,      0, ZMQ_POLLIN, 0},
                {m_controller, 0, ZMQ_POLLIN, 0}
        };

        while (true) {
            zmq::poll(&items[0], 2, std::chrono::milliseconds(-1));

            if (items[0].revents & ZMQ_POLLIN) {
                zmq::multipart_t wire_msg;
                wire_msg.recv(m_shell);
                try {
                    Message msg = p_server->deserialize(wire_msg);
                    p_server->notify_shell_listener(std::move(msg));
                }
                catch (std::exception &e) {
                    std::cerr << e.what() << std::endl;
                }
            }

            if (items[1].revents & ZMQ_POLLIN) {
                // stop message
                zmq::multipart_t wire_msg;
                wire_msg.recv(m_controller);
                std::string msg = wire_msg.peekstr(0);
                if (msg == "stop") {
                    wire_msg.send(m_controller);
                    break;
                } else {
                    zmq::multipart_t wire_reply = p_server->notify_internal_listener(wire_msg);
                    wire_reply.send(m_controller);
                }
            }
        }
    }

    void Shell::send_shell(zmq::multipart_t &message) {
        message.send(m_shell);
    }

    void Shell::send_stdin(zmq::multipart_t &message) {
        message.send(m_stdin);
        zmq::multipart_t wire_msg;
        wire_msg.recv(m_stdin);
        try {
            Message msg = p_server->deserialize(wire_msg);
            p_server->notify_stdin_listener(std::move(msg));
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void Shell::publish(zmq::multipart_t &message) {
        message.send(m_publisher_pub);
    }

    void Shell::abort_queue(const listener &l, long polling_interval) {
        while (true) {
            zmq::multipart_t wire_msg;
            bool received = wire_msg.recv(m_shell, ZMQ_NOBLOCK);
            if (!received) {
                return;
            }

            try {
                Message msg = p_server->deserialize(wire_msg);
                l(std::move(msg));
            }
            catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(polling_interval));
        }
    }

    void Shell::reply_to_controller(zmq::multipart_t &message) {
        message.send(m_controller);
    }
}

