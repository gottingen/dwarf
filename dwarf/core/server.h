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

#include <functional>

#include <dwarf/core/config.h>
#include <dwarf/core/kernel_configuration.h>
#include <dwarf/core/control_messenger.h>
#include <dwarf/core/message.h>

namespace dwarf {
    enum class channel {
        SHELL,
        CONTROL
    };

    class DWARF_API Server {
    public:

        using listener = std::function<void(Message)>;
        using internal_listener = std::function<nl::json(nl::json)>;

        virtual ~Server() = default;

        Server(const Server &) = delete;

        Server &operator=(const Server &) = delete;

        Server(Server &&) = delete;

        Server &operator=(Server &&) = delete;

        ControlMessenger &get_control_messenger();

        void send_shell(Message message);

        void send_control(Message message);

        void send_stdin(Message message);

        void publish(PubMessage message, channel c);

        void start(PubMessage message);

        void abort_queue(const listener &l, long polling_interval);

        void stop();

        void update_config(Configuration &config) const;

        void register_shell_listener(const listener &l);

        void register_control_listener(const listener &l);

        void register_stdin_listener(const listener &l);

        void register_internal_listener(const internal_listener &l);

    protected:

        Server() = default;

        void notify_shell_listener(Message msg);

        void notify_control_listener(Message msg);

        void notify_stdin_listener(Message msg);

        nl::json notify_internal_listener(nl::json msg);

    private:

        virtual ControlMessenger &get_control_messenger_impl() = 0;

        virtual void send_shell_impl(Message message) = 0;

        virtual void send_control_impl(Message message) = 0;

        virtual void send_stdin_impl(Message message) = 0;

        virtual void publish_impl(PubMessage message, channel c) = 0;

        virtual void start_impl(PubMessage message) = 0;

        virtual void abort_queue_impl(const listener &l, long polling_interval) = 0;

        virtual void stop_impl() = 0;

        virtual void update_config_impl(Configuration &config) const = 0;

        listener m_shell_listener;
        listener m_control_listener;
        listener m_stdin_listener;
        internal_listener m_internal_listener;
    };
}
