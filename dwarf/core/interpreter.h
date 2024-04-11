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
#include <string>
#include <vector>

#include <dwarf/core/comm.h>
#include <dwarf/core/config.h>
#include <dwarf/core/control_messenger.h>
#include <dwarf/core/history_manager.h>

namespace dwarf {
    class Interpreter;

    DWARF_API bool register_interpreter(Interpreter *interpreter);

    DWARF_API Interpreter &get_interpreter();

    class DWARF_API Interpreter {
    public:

        Interpreter();

        virtual ~Interpreter() = default;

        Interpreter(const Interpreter &) = delete;

        Interpreter &operator=(const Interpreter &) = delete;

        Interpreter(Interpreter &&) = delete;

        Interpreter &operator=(Interpreter &&) = delete;

        void configure();

        nl::json execute_request(const std::string &code,
                                 bool silent,
                                 bool store_history,
                                 nl::json user_expressions,
                                 bool allow_stdin);

        nl::json complete_request(const std::string &code, int cursor_pos);

        nl::json inspect_request(const std::string &code, int cursor_pos, int detail_level);

        nl::json is_complete_request(const std::string &code);

        nl::json kernel_info_request();

        void shutdown_request();

        nl::json internal_request(const nl::json &message);

        // publish(msg_type, metadata, content)
        using publisher_type = std::function<void(const std::string &, nl::json, nl::json, buffer_sequence)>;

        void register_publisher(const publisher_type &publisher);

        void publish_stream(const std::string &name, const std::string &text);

        void display_data(nl::json data, nl::json metadata, nl::json transient);

        void update_display_data(nl::json data, nl::json metadata, nl::json transient);

        void publish_execution_input(const std::string &code, int execution_count);

        void publish_execution_result(int execution_count, nl::json data, nl::json metadata);

        void publish_execution_error(const std::string &ename,
                                     const std::string &evalue,
                                     const std::vector<std::string> &trace_back);

        void clear_output(bool wait);

        // send_stdin(msg_type, metadata, content)
        using stdin_sender_type = std::function<void(const std::string &, nl::json, nl::json)>;

        void register_stdin_sender(const stdin_sender_type &sender);

        using input_reply_handler_type = std::function<void(const std::string &)>;

        void register_input_handler(const input_reply_handler_type &handler);

        void input_request(const std::string &prompt, bool pwd);

        void input_reply(const std::string &value);

        void register_comm_manager(CommManager *manager);

        CommManager &comm_manager() noexcept;

        const CommManager &comm_manager() const noexcept;

        using parent_header_type = std::function<const nl::json &()>;

        void register_parent_header(const parent_header_type &);

        const nl::json &parent_header() const noexcept;

        void register_control_messenger(ControlMessenger &messenger);

        void register_history_manager(const HistoryManager &history);

        const HistoryManager &get_history_manager() const noexcept;

    protected:

        ControlMessenger &get_control_messenger();

    private:

        virtual void configure_impl() = 0;

        virtual nl::json execute_request_impl(int execution_counter,
                                              const std::string &code,
                                              bool silent,
                                              bool store_history,
                                              nl::json user_expressions,
                                              bool allow_stdin) = 0;

        virtual nl::json complete_request_impl(const std::string &code,
                                               int cursor_pos) = 0;

        virtual nl::json inspect_request_impl(const std::string &code,
                                              int cursor_pos,
                                              int detail_level) = 0;

        virtual nl::json is_complete_request_impl(const std::string &code) = 0;

        virtual nl::json kernel_info_request_impl() = 0;

        virtual void shutdown_request_impl() = 0;

        virtual nl::json internal_request_impl(const nl::json &message);

        nl::json build_display_content(nl::json data, nl::json metadata, nl::json transient);

        publisher_type m_publisher;
        stdin_sender_type m_stdin;
        int m_execution_count;
        CommManager *p_comm_manager;
        parent_header_type m_parent_header;
        input_reply_handler_type m_input_reply_handler;
        ControlMessenger *p_messenger;
        const HistoryManager *p_history;
    };

    inline CommManager &Interpreter::comm_manager() noexcept {
        return *p_comm_manager;
    }

    inline const CommManager &Interpreter::comm_manager() const noexcept {
        return *p_comm_manager;
    }
}
