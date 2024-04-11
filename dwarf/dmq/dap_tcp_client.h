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
#include <deque>
#include <string>

#include <dwarf/zmq/zmq.hpp>
#include <collie/nlohmann/json.hpp>
#include <dwarf/core/kernel_configuration.h>

#include <dwarf/dmq/authentication.h>
#include <dwarf/core/config.h>

namespace nl = nlohmann;

namespace dwarf {
    enum class dap_tcp_type {
        client,
        server
    };

    enum class dap_init_type {
        sequential,
        parallel
    };

    struct DWARF_API DapTcpConfiguration {
        dap_tcp_type m_dap_tcp_type;
        dap_init_type m_dap_init_type;
        std::string m_user_name;
        std::string m_session_id;

        DapTcpConfiguration(dap_tcp_type adap_tcp_type,
                            dap_init_type adap_init_type,
                            const std::string &user_name,
                            const std::string &session_id);
    };

    class DWARF_API DapTcpClient {
    public:

        static constexpr const char *HEADER = "Content-Length: ";
        static constexpr size_t HEADER_LENGTH = 16;
        static constexpr const char *SEPARATOR = "\r\n\r\n";
        static constexpr size_t SEPARATOR_LENGTH = 4;

        using event_callback = std::function<void(const nl::json &)>;

        virtual ~DapTcpClient() = default;

        DapTcpClient(const DapTcpClient &) = delete;

        DapTcpClient &operator=(const DapTcpClient &) = delete;

        DapTcpClient(DapTcpClient &&) = delete;

        DapTcpClient &operator=(DapTcpClient &&) = delete;

        void start_debugger(std::string tcp_end_point,
                            std::string publisher_end_point,
                            std::string controller_end_point,
                            std::string controller_header_end_point);

    protected:

        DapTcpClient(zmq::context_t &context,
                     const dwarf::Configuration &config,
                     int socket_linger,
                     const DapTcpConfiguration &dap_config,
                     const event_callback &cb);

        const std::string &parent_header() const;

        void forward_event(nl::json message);

        void send_dap_request(nl::json message);

        using message_condition = std::function<bool(const nl::json &)>;

        nl::json wait_for_message(const message_condition &condition);

    private:

        using queue_type = std::deque<std::string>;

        zmq::message_t get_tcp_id() const;

        void init_tcp_socket(const std::string &tcp_end_point);

        void finalize_tcp_socket(const std::string &tcp_end_point);

        void handle_header_socket();

        void handle_control_socket();

        void handle_tcp_socket(queue_type &message_queue);

        void append_tcp_message(std::string &buffer);

        void process_message_queue();

        void handle_init_sequence();

        virtual void handle_event(nl::json message) = 0;

        zmq::socket_t m_tcp_socket;
        zmq::message_t m_socket_id;

        zmq::socket_t m_publisher;
        zmq::socket_t m_controller;
        zmq::socket_t m_controller_header;

        dap_tcp_type m_dap_tcp_type;
        dap_init_type m_dap_init_type;

        std::string m_user_name;
        std::string m_session_id;

        event_callback m_event_callback;

        using authentication_ptr = std::unique_ptr<dwarf::Authentication>;
        authentication_ptr p_auth;

        // Saves parent header for future debug events
        std::string m_parent_header;

        bool m_request_stop;
        bool m_wait_attach;

        queue_type m_message_queue;
        queue_type m_stopped_queue;
    };
}

