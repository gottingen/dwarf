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


#include <iostream>

#include <dwarf/core/server.h>

namespace dwarf {
    ControlMessenger &Server::get_control_messenger() {
        return get_control_messenger_impl();
    }

    void Server::send_shell(Message message) {
        send_shell_impl(std::move(message));
    }

    void Server::send_control(Message message) {
        send_control_impl(std::move(message));
    }

    void Server::send_stdin(Message message) {
        send_stdin_impl(std::move(message));
    }

    void Server::publish(PubMessage message, channel c) {
        publish_impl(std::move(message), c);
    }

    void Server::start(PubMessage message) {
        std::clog << "Run with DWARF " << DWARF_VERSION_MAJOR << "."
                  << DWARF_VERSION_MINOR << "."
                  << DWARF_VERSION_PATCH << std::endl;
        start_impl(std::move(message));
    }

    void Server::abort_queue(const listener &l, long polling_interval) {
        abort_queue_impl(l, polling_interval);
    }

    void Server::stop() {
        stop_impl();
    }

    void Server::update_config(Configuration &config) const {
        update_config_impl(config);
    }

    void Server::register_shell_listener(const listener &l) {
        m_shell_listener = l;
    }

    void Server::register_control_listener(const listener &l) {
        m_control_listener = l;
    }

    void Server::register_stdin_listener(const listener &l) {
        m_stdin_listener = l;
    }

    void Server::register_internal_listener(const internal_listener &l) {
        m_internal_listener = l;
    }

    void Server::notify_shell_listener(Message msg) {
        m_shell_listener(std::move(msg));
    }

    void Server::notify_control_listener(Message msg) {
        m_control_listener(std::move(msg));
    }

    void Server::notify_stdin_listener(Message msg) {
        m_stdin_listener(std::move(msg));
    }

    nl::json Server::notify_internal_listener(nl::json msg) {
        return m_internal_listener(std::move(msg));
    }
}
