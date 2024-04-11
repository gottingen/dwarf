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
#include <dwarf/dmq/zmq_messenger.h>

namespace dwarf {
    class ServerZmqSplit;

    class Control {
    public:

        using listener = std::function<void(zmq::multipart_t &)>;

        Control(zmq::context_t &context,
                const std::string &transport,
                const std::string &ip,
                const std::string &control_port,
                ServerZmqSplit *server);

        ~Control();

        std::string get_port() const;

        void connect_messenger();

        ControlMessenger &get_messenger();

        void run();

        void stop();

        void send_control(zmq::multipart_t &message);

        void publish(zmq::multipart_t &message);

    private:

        zmq::socket_t m_control;
        zmq::socket_t m_publisher_pub;
        // Internal sockets for controlling other threads
        ZmqMessenger m_messenger;
        ServerZmqSplit *p_server;
        bool m_request_stop;
    };
}

