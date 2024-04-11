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
#include <dwarf/dmq/server_zmq.h>
#include <dwarf/dmq/middleware.h>
#include <dwarf/dmq/zmq_serializer.h>
#include <dwarf/dmq/publisher.h>
#include <dwarf/dmq/heartbeat.h>
#include <dwarf/dmq/trivial_messenger.h>

namespace dwarf
{

    ServerZmq::ServerZmq(zmq::context_t& context,
                             const Configuration& config,
                             nl::json::error_handler_t eh)
        : m_shell(context, zmq::socket_type::router)
        , m_controller(context, zmq::socket_type::router)
        , m_stdin(context, zmq::socket_type::router)
        , m_publisher_pub(context, zmq::socket_type::pub)
        , m_publisher_controller(context, zmq::socket_type::req)
        , m_heartbeat_controller(context, zmq::socket_type::req)
        , p_publisher(new Publisher(context, config.m_transport, config.m_ip, config.m_iopub_port))
        , p_heartbeat(new Heartbeat(context, config.m_transport, config.m_ip, config.m_hb_port))
        , m_iopub_thread()
        , m_hb_thread()
        , p_messenger(new TrivialMessenger(this))
        , p_auth(make_authentication(config.m_signature_scheme, config.m_key))
        , m_error_handler(eh)
        , m_request_stop(false)
    {
        init_socket(m_shell, config.m_transport, config.m_ip, config.m_shell_port);
        init_socket(m_controller, config.m_transport, config.m_ip, config.m_control_port);
        init_socket(m_stdin, config.m_transport, config.m_ip, config.m_stdin_port);
        m_publisher_pub.set(zmq::sockopt::linger, get_socket_linger());
        m_publisher_pub.connect(get_publisher_end_point());

        m_publisher_controller.set(zmq::sockopt::linger, get_socket_linger());
        m_publisher_controller.connect(get_controller_end_point("publisher"));
        m_heartbeat_controller.set(zmq::sockopt::linger, get_socket_linger());
        m_heartbeat_controller.connect(get_controller_end_point("heartbeat"));
    }

    // Has to be in the cpp because incomplete
    // types are used in unique_ptr in the header
    ServerZmq::~ServerZmq() = default;

    ControlMessenger& ServerZmq::get_control_messenger_impl()
    {
        return *p_messenger;
    }

    void ServerZmq::send_shell_impl(Message msg)
    {
        zmq::multipart_t wire_msg = xzmq_serializer::serialize(std::move(msg), *p_auth, m_error_handler);
        wire_msg.send(m_shell);
    }

    void ServerZmq::send_control_impl(Message msg)
    {
        zmq::multipart_t wire_msg = xzmq_serializer::serialize(std::move(msg), *p_auth, m_error_handler);
        wire_msg.send(m_controller);
    }

    void ServerZmq::send_stdin_impl(Message msg)
    {
        zmq::multipart_t wire_msg = xzmq_serializer::serialize(std::move(msg), *p_auth, m_error_handler);
        wire_msg.send(m_stdin);
        zmq::multipart_t wire_reply;
	// Block until a response to the input request is received.
        wire_reply.recv(m_stdin);
        try
        {
            Message reply = xzmq_serializer::deserialize(wire_reply, *p_auth);
            Server::notify_stdin_listener(std::move(reply));
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

    void ServerZmq::publish_impl(PubMessage msg, channel)
    {
        zmq::multipart_t wire_msg = xzmq_serializer::serialize_iopub(std::move(msg), *p_auth, m_error_handler);
        wire_msg.send(m_publisher_pub);
    }

    void ServerZmq::start_impl(PubMessage message)
    {
        start_publisher_thread();
        start_heartbeat_thread();

        m_request_stop = false;

        publish(std::move(message), channel::SHELL);

        while (!m_request_stop)
        {
            poll(-1);
        }

        stop_channels();
    }

    void ServerZmq::start_publisher_thread()
    {
        m_iopub_thread = std::move(ZmqThread(&Publisher::run, p_publisher.get()));
    }

    void ServerZmq::start_heartbeat_thread()
    {
        m_hb_thread = std::move(ZmqThread(&Heartbeat::run, p_heartbeat.get()));
    }

    void ServerZmq::poll(long timeout)
    {
        zmq::pollitem_t items[]
            = { { m_controller, 0, ZMQ_POLLIN, 0 }, { m_shell, 0, ZMQ_POLLIN, 0 } };

        zmq::poll(&items[0], 2, std::chrono::milliseconds(timeout));

        try
        {
            if (items[0].revents & ZMQ_POLLIN)
            {
                zmq::multipart_t wire_msg;
                wire_msg.recv(m_controller);
                Message msg = xzmq_serializer::deserialize(wire_msg, *p_auth);
                Server::notify_control_listener(std::move(msg));
            }

            if (!m_request_stop && (items[1].revents & ZMQ_POLLIN))
            {
                zmq::multipart_t wire_msg;
                wire_msg.recv(m_shell);
                Message msg = xzmq_serializer::deserialize(wire_msg, *p_auth);
                Server::notify_shell_listener(std::move(msg));
            }
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

    void ServerZmq::abort_queue_impl(const listener& l, long polling_interval)
    {
        while (true)
        {
            zmq::multipart_t wire_msg;
            bool msg = wire_msg.recv(m_shell, ZMQ_NOBLOCK);
            if (!msg)
            {
                return;
            }

            try
            {
                Message msg = xzmq_serializer::deserialize(wire_msg, *p_auth);
                l(std::move(msg));
            }
            catch (std::exception& e)
            {
                std::cerr << e.what() << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(polling_interval));
        }
    }

    void ServerZmq::stop_impl()
    {
        m_request_stop = true;
    }

    void ServerZmq::update_config_impl(Configuration& config) const
    {
        config.m_control_port = get_socket_port(m_controller);
        config.m_shell_port = get_socket_port(m_shell);
        config.m_stdin_port = get_socket_port(m_stdin);
        config.m_iopub_port = p_publisher->get_port();
        config.m_hb_port = p_heartbeat->get_port();
    }

    void ServerZmq::stop_channels()
    {
        zmq::message_t stop_msg("stop", 4);
        zmq::message_t response;

        // Wait for publisher answer
        m_publisher_controller.send(stop_msg, zmq::send_flags::none);
        (void)m_publisher_controller.recv(response);

        // Wait for heartbeat answer
        m_heartbeat_controller.send(stop_msg, zmq::send_flags::none);
        (void)m_heartbeat_controller.recv(response);
    }

    std::unique_ptr<Server> make_xserver_zmq(Context& context,
                                              const Configuration& config,
                                              nl::json::error_handler_t eh)
    {
        return std::make_unique<ServerZmq>(context.get_wrapped_context<zmq::context_t>(), config, eh);
    }
}
