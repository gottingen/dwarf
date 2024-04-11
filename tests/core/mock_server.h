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

#include <cstddef>
#include <memory>
#include <queue>

#include <dwarf/core/control_messenger.h>
#include <dwarf/core/context.h>
#include <dwarf/core/server.h>

namespace dwarf
{
    class xmock_server;

    class xmock_messenger : public ControlMessenger
    {
    public:

        xmock_messenger(xmock_server* server);
        virtual ~xmock_messenger() = default;

    private:

        nl::json send_to_shell_impl(const nl::json& message) override;

        xmock_server* p_server;
    };

    class xmock_server : public Server
    {
    public:

        xmock_server();
        virtual ~xmock_server() = default;
        
        xmock_server(const xmock_server&) = delete;
        xmock_server& operator=(const xmock_server&) = delete;
        
        xmock_server(xmock_server&&) = delete;
        xmock_server& operator=(xmock_server&&) = delete;

        void receive_shell(Message message);
        void receive_control(Message message);
        void receive_stdin(Message message);

        std::size_t shell_size() const;
        Message read_shell();

        std::size_t control_size() const;
        Message read_control();
        
        std::size_t stdin_size() const;
        Message read_stdin();

        std::size_t iopub_size() const;
        PubMessage read_iopub();

        using Server::notify_internal_listener;

    private:

        ControlMessenger& get_control_messenger_impl() override;

        void send_shell_impl(Message message) override;
        void send_control_impl(Message message) override;
        void send_stdin_impl(Message message) override;
        void publish_impl(PubMessage message, channel c) override;

        void start_impl(PubMessage message) override;
        void abort_queue_impl(const listener& l, long polling_interval) override;
        void stop_impl() override;
        void update_config_impl(Configuration& config) const override;

        using message_queue = std::queue<Message>;
        Message read_impl(message_queue& q);

        xmock_messenger m_messenger;

        message_queue m_shell_messages;
        message_queue m_control_messages;
        message_queue m_stdin_messages;
        std::queue<PubMessage> m_iopub_messages;
    };

    std::unique_ptr<Server> make_mock_server(Context& context,
                                              const Configuration& config,
                                              nl::json::error_handler_t eh);
}

