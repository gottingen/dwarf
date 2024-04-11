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

#include <atomic>

#include <dwarf/zmq/zmq_addon.hpp>

#include <dwarf/core/server.h>
#include <dwarf/core/kernel_configuration.h>

#include <dwarf/core/config.h>
#include <dwarf/dmq/authentication.h>
#include <dwarf/dmq/thread.h>

namespace dwarf {
    class Control;

    class Heartbeat;

    class Publisher;

    class Shell;

    class DWARF_API ServerZmqSplit : public Server {
    public:

        using controller_ptr = std::unique_ptr<Control>;
        using heartbeat_ptr = std::unique_ptr<Heartbeat>;
        using publisher_ptr = std::unique_ptr<Publisher>;
        using shell_ptr = std::unique_ptr<Shell>;

        ServerZmqSplit(zmq::context_t &context,
                       const Configuration &config,
                       nl::json::error_handler_t eh);

        ~ServerZmqSplit() override;

        // The Control object needs to call this method
        using Server::notify_control_listener;
        // The Shell object needs to call these methods
        using Server::notify_shell_listener;
        using Server::notify_stdin_listener;

        zmq::multipart_t notify_internal_listener(zmq::multipart_t &wire_msg);

        void notify_control_stopped();

        Message deserialize(zmq::multipart_t &wire_msg) const;

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

        void start_control_thread();

        void start_heartbeat_thread();

        void start_publisher_thread();

        void start_shell_thread();

        Control &get_controller();

        Shell &get_shell();

        bool is_control_stopped() const;

    private:

        virtual void start_server(zmq::multipart_t &wire_msg) = 0;

        controller_ptr p_controller;
        heartbeat_ptr p_heartbeat;
        publisher_ptr p_publisher;
        shell_ptr p_shell;

        ZmqThread m_control_thread;
        ZmqThread m_hb_thread;
        ZmqThread m_iopub_thread;
        ZmqThread m_shell_thread;

        using authentication_ptr = std::unique_ptr<Authentication>;
        authentication_ptr p_auth;
        nl::json::error_handler_t m_error_handler;

        std::atomic<bool> m_control_stopped;
    };
}

