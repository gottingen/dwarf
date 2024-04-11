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

#include <dwarf/zmq/zmq.hpp>

#include <dwarf/core/context.h>
#include <dwarf/core/kernel_configuration.h>
#include <dwarf/core/server.h>

#include <dwarf/core/config.h>
#include <dwarf/dmq/authentication.h>
#include <dwarf/dmq/thread.h>

namespace dwarf {
    class Publisher;

    class Heartbeat;

    class TrivialMessenger;

    class DWARF_API ServerZmq : public Server {
    public:

        using publisher_ptr = std::unique_ptr<Publisher>;
        using heartbeat_ptr = std::unique_ptr<Heartbeat>;

        ServerZmq(zmq::context_t &context,
                    const Configuration &config,
                    nl::json::error_handler_t eh);

        ~ServerZmq() override;

        using Server::notify_internal_listener;

    protected:

        ControlMessenger &get_control_messenger_impl() override;

        void send_shell_impl(Message msg) override;

        void send_control_impl(Message msg) override;

        void send_stdin_impl(Message msg) override;

        void publish_impl(PubMessage msg, channel c) override;

        void start_impl(PubMessage msg) override;

        void abort_queue_impl(const listener &l, long polling_interval) override;

        void stop_impl() override;

        void update_config_impl(Configuration &config) const override;

        void poll(long timeout);

        void start_publisher_thread();

        void start_heartbeat_thread();

        void stop_channels();

        zmq::socket_t m_shell;
        zmq::socket_t m_controller;
        zmq::socket_t m_stdin;
        zmq::socket_t m_publisher_pub;
        zmq::socket_t m_publisher_controller;
        zmq::socket_t m_heartbeat_controller;

        publisher_ptr p_publisher;
        heartbeat_ptr p_heartbeat;

        ZmqThread m_iopub_thread;
        ZmqThread m_hb_thread;

        using trivial_messenger_ptr = std::unique_ptr<TrivialMessenger>;
        trivial_messenger_ptr p_messenger;

        using authentication_ptr = std::unique_ptr<Authentication>;
        authentication_ptr p_auth;
        nl::json::error_handler_t m_error_handler;

        bool m_request_stop;
    };

    DWARF_API
    std::unique_ptr<Server> make_xserver_zmq(Context &context,
                                             const Configuration &config,
                                             nl::json::error_handler_t eh = nl::json::error_handler_t::strict);
}
