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


#include <chrono>
#include <iostream>

#include <dwarf/zmq/zmq_addon.hpp>
#include <dwarf/core/guid.h>
#include <dwarf/dmq/server_zmq_split.h>
#include <dwarf/dmq/middleware.h>
#include <dwarf/dmq/zmq_serializer.h>
#include <dwarf/dmq/control.h>
#include <dwarf/dmq/heartbeat.h>
#include <dwarf/dmq/publisher.h>
#include <dwarf/dmq/shell.h>
#include <dwarf/dmq/zmq_messenger.h>

namespace dwarf {
    ServerZmqSplit::ServerZmqSplit(zmq::context_t &context,
                                   const Configuration &config,
                                   nl::json::error_handler_t eh)
            : p_controller(new Control(context, config.m_transport, config.m_ip, config.m_control_port, this)),
              p_heartbeat(new Heartbeat(context, config.m_transport, config.m_ip, config.m_hb_port)),
              p_publisher(new Publisher(context, config.m_transport, config.m_ip, config.m_iopub_port)),
              p_shell(new Shell(context, config.m_transport, config.m_ip, config.m_shell_port, config.m_stdin_port,
                                 this)), m_control_thread(), m_hb_thread(), m_iopub_thread(), m_shell_thread(),
              p_auth(make_authentication(config.m_signature_scheme, config.m_key)), m_error_handler(eh),
              m_control_stopped(false) {
        p_controller->connect_messenger();
    }

    // Has to be in the cpp because incomplete
    // types are used in unique_ptr in the header
    ServerZmqSplit::~ServerZmqSplit() = default;

    zmq::multipart_t ServerZmqSplit::notify_internal_listener(zmq::multipart_t &wire_msg) {
        nl::json msg = nl::json::parse(wire_msg.popstr());
        nl::json reply = Server::notify_internal_listener(msg);
        return zmq::multipart_t(reply.dump(-1, ' ', false, m_error_handler));
    }

    void ServerZmqSplit::notify_control_stopped() {
        m_control_stopped = true;
    }

    Message ServerZmqSplit::deserialize(zmq::multipart_t &wire_msg) const {
        return xzmq_serializer::deserialize(wire_msg, *p_auth);
    }

    ControlMessenger &ServerZmqSplit::get_control_messenger_impl() {
        return p_controller->get_messenger();
    }

    void ServerZmqSplit::send_shell_impl(Message msg) {
        zmq::multipart_t wire_msg = xzmq_serializer::serialize(std::move(msg), *p_auth, m_error_handler);
        p_shell->send_shell(wire_msg);
    }

    void ServerZmqSplit::send_control_impl(Message msg) {
        zmq::multipart_t wire_msg = xzmq_serializer::serialize(std::move(msg), *p_auth, m_error_handler);
        p_controller->send_control(wire_msg);
    }

    void ServerZmqSplit::send_stdin_impl(Message msg) {
        zmq::multipart_t wire_msg = xzmq_serializer::serialize(std::move(msg), *p_auth, m_error_handler);
        p_shell->send_stdin(wire_msg);
    }

    void ServerZmqSplit::publish_impl(PubMessage msg, channel c) {
        zmq::multipart_t wire_msg = xzmq_serializer::serialize_iopub(std::move(msg), *p_auth, m_error_handler);
        if (c == channel::SHELL) {
            p_shell->publish(wire_msg);
        } else {
            p_controller->publish(wire_msg);
        }
    }

    void ServerZmqSplit::start_impl(PubMessage msg) {
        zmq::multipart_t wire_msg = xzmq_serializer::serialize_iopub(std::move(msg), *p_auth, m_error_handler);
        start_server(wire_msg);
    }

    void ServerZmqSplit::abort_queue_impl(const listener &l, long polling_interval) {
        p_shell->abort_queue(l, polling_interval);
    }

    void ServerZmqSplit::stop_impl() {
        p_controller->stop();
    }

    void ServerZmqSplit::update_config_impl(Configuration &config) const {
        config.m_control_port = p_controller->get_port();
        config.m_shell_port = p_shell->get_shell_port();
        config.m_stdin_port = p_shell->get_stdin_port();
        config.m_iopub_port = p_publisher->get_port();
        config.m_hb_port = p_heartbeat->get_port();
    }

    void ServerZmqSplit::start_control_thread() {
        m_control_thread = std::move(ZmqThread(&Control::run, p_controller.get()));
    }

    void ServerZmqSplit::start_heartbeat_thread() {
        m_hb_thread = std::move(ZmqThread(&Heartbeat::run, p_heartbeat.get()));
    }

    void ServerZmqSplit::start_publisher_thread() {
        m_iopub_thread = std::move(ZmqThread(&Publisher::run, p_publisher.get()));
    }

    void ServerZmqSplit::start_shell_thread() {
        m_shell_thread = std::move(ZmqThread(&Shell::run, p_shell.get()));
    }

    Control &ServerZmqSplit::get_controller() {
        return *p_controller;
    }

    Shell &ServerZmqSplit::get_shell() {
        return *p_shell;
    }

    bool ServerZmqSplit::is_control_stopped() const {
        return m_control_stopped;
    }
}

