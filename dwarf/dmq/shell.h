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
#include <dwarf/zmq/zmq_addon.hpp>

#include <dwarf/core/message.h>

namespace dwarf {
    class ServerZmqSplit;

    class Shell {
    public:

        using listener = std::function<void(Message)>;

        Shell(zmq::context_t &context,
              const std::string &transport,
              const std::string &ip,
              const std::string &shell_port,
              const std::string &sdtin_port,
              ServerZmqSplit *server);

        ~Shell();

        std::string get_shell_port() const;

        std::string get_stdin_port() const;

        void run();

        void send_shell(zmq::multipart_t &message);

        void send_stdin(zmq::multipart_t &message);

        void publish(zmq::multipart_t &message);

        void abort_queue(const listener &l, long polling_interval);

        void reply_to_controller(zmq::multipart_t &message);

    private:

        zmq::socket_t m_shell;
        zmq::socket_t m_stdin;
        zmq::socket_t m_publisher_pub;
        zmq::socket_t m_controller;
        ServerZmqSplit *p_server;
    };
}
