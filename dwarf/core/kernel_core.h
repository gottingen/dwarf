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

#include <map>
#include <string>

#include <collie/nlohmann/json.hpp>

#include <dwarf/core/comm.h>
#include <dwarf/core/server.h>
#include <dwarf/core/interpreter.h>
#include <dwarf/core/history_manager.h>
#include <dwarf/core/debugger.h>
#include <dwarf/core/message.h>
#include <dwarf/core/logger.h>

namespace nl = nlohmann;

namespace dwarf {
    class KernelCore {
    public:

        using logger_ptr = Logger *;
        using server_ptr = Server *;
        using interpreter_ptr = Interpreter *;
        using history_manager_ptr = HistoryManager *;
        using debugger_ptr = Debugger *;

        KernelCore(const std::string &kernel_id,
                   const std::string &user_name,
                   const std::string &session_id,
                   logger_ptr logger,
                   server_ptr server,
                   interpreter_ptr p_interpreter,
                   history_manager_ptr p_history_manager,
                   debugger_ptr debugger,
                   nl::json::error_handler_t eh);

        ~KernelCore();

        PubMessage build_start_msg() const;

        void dispatch_shell(Message msg);

        void dispatch_control(Message msg);

        void dispatch_stdin(Message msg);

        nl::json dispatch_internal(nl::json msg);

        void publish_message(const std::string &msg_type,
                             nl::json metadata,
                             nl::json content,
                             buffer_sequence buffers,
                             channel origin);

        void send_stdin(const std::string &msg_type, nl::json metadata, nl::json content);

        CommManager &comm_manager() & noexcept;

        const CommManager &comm_manager() const & noexcept;

        CommManager comm_manager() const && noexcept;

        const nl::json &parent_header(channel c) const noexcept;

    private:

        using handler_type = void (KernelCore::*)(Message, channel);
        using guid_list = Message::guid_list;

        void dispatch(Message msg, channel c);

        handler_type get_handler(const std::string &msg_type);

        void execute_request(Message request, channel c);

        void complete_request(Message request, channel c);

        void inspect_request(Message request, channel c);

        void history_request(Message request, channel c);

        void is_complete_request(Message request, channel c);

        void comm_info_request(Message request, channel c);

        void comm_open(Message request, channel c);

        void comm_close(Message request, channel c);

        void comm_msg(Message request, channel c);

        void kernel_info_request(Message request, channel c);

        void shutdown_request(Message request, channel c);

        void interrupt_request(Message request, channel c);

        void debug_request(Message request, channel c);

        void publish_status(const std::string &status, channel c);

        void publish_execute_input(const std::string &code, int execution_count);

        void send_reply(const std::string &reply_type,
                        nl::json metadata,
                        nl::json reply_content,
                        channel c);

        void send_reply(const guid_list &id_list,
                        const std::string &reply_type,
                        nl::json parent_header,
                        nl::json metadata,
                        nl::json reply_content,
                        channel c);

        void abort_request(Message msg);

        std::string get_topic(const std::string &msg_type) const;

        nl::json get_metadata() const;

        void set_parent(const guid_list &list, const nl::json &parent, channel c);

        const guid_list &get_parent_id(channel c) const;

        nl::json get_parent_header(channel c) const;

        std::string m_kernel_id;
        std::string m_user_name;
        std::string m_session_id;

        std::map<std::string, handler_type> m_handler;
        CommManager m_comm_manager;
        logger_ptr p_logger;
        server_ptr p_server;
        interpreter_ptr p_interpreter;
        history_manager_ptr p_history_manager;
        debugger_ptr p_debugger;

        std::array<guid_list, 2> m_parent_id;
        std::array<nl::json, 2> m_parent_header;

        nl::json::error_handler_t m_error_handler;
    };
}