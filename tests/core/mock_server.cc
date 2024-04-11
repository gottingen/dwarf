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


#include "mock_server.h"

namespace dwarf
{

xmock_messenger::xmock_messenger(xmock_server* server)
    : p_server(server)
{
}

nl::json xmock_messenger::send_to_shell_impl(const nl::json& message)
{
    return p_server->notify_internal_listener(message);
}

xmock_server::xmock_server()
    : m_messenger(this)
{
}

void xmock_server::receive_shell(Message message)
{
    m_shell_messages.push(std::move(message));
}

void xmock_server::receive_control(Message message)
{
    m_control_messages.push(std::move(message));
}

void xmock_server::receive_stdin(Message message)
{
    m_stdin_messages.push(std::move(message));
}

std::size_t xmock_server::shell_size() const
{
    return m_shell_messages.size();
}

Message xmock_server::read_shell()
{
    return read_impl(m_shell_messages);
}

std::size_t xmock_server::control_size() const
{
    return m_control_messages.size();
}

Message xmock_server::read_control()
{
    return read_impl(m_control_messages);
}

std::size_t xmock_server::stdin_size() const
{
    return m_stdin_messages.size();
}

Message xmock_server::read_stdin()
{
    return read_impl(m_stdin_messages);
}

std::size_t xmock_server::iopub_size() const
{
    return m_iopub_messages.size();
}

PubMessage xmock_server::read_iopub()
{
    PubMessage res = std::move(m_iopub_messages.back());
    m_iopub_messages.pop();
    return res;
}

Message xmock_server::read_impl(message_queue& q)
{
    Message res = std::move(q.back());
    q.pop();
    return res;
}

ControlMessenger& xmock_server::get_control_messenger_impl()
{
    return m_messenger;
}

void xmock_server::send_shell_impl(Message message)
{
    m_shell_messages.push(std::move(message));
}

void xmock_server::send_control_impl(Message message)
{
    m_control_messages.push(std::move(message));
}

void xmock_server::send_stdin_impl(Message message)
{
    m_stdin_messages.push(std::move(message));
}

void xmock_server::publish_impl(PubMessage message, channel)
{
    m_iopub_messages.push(std::move(message));
}

void xmock_server::start_impl(PubMessage message)
{
    m_iopub_messages.push(std::move(message));
}

void xmock_server::abort_queue_impl(const listener&, long)
{
}

void xmock_server::stop_impl()
{
}

void xmock_server::update_config_impl(Configuration&) const
{
}

std::unique_ptr<Server> make_mock_server(Context&,
                                          const Configuration&,
                                          nl::json::error_handler_t)
{
    return std::make_unique<xmock_server>();
}

}

