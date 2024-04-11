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


#include <string>
#include <utility>
#include <vector>

#include <collie/nlohmann/json.hpp>

#include <dwarf/core/interpreter.h>

namespace nl = nlohmann;

namespace dwarf {
    Interpreter::Interpreter()
            : m_execution_count(0) {
    }

    void Interpreter::configure() {
        configure_impl();
    }

    nl::json Interpreter::execute_request(const std::string &code,
                                          bool silent,
                                          bool store_history,
                                          nl::json user_expressions,
                                          bool allow_stdin) {
        if (!silent) {
            ++m_execution_count;
            publish_execution_input(code, m_execution_count);
        }

        nl::json reply = execute_request_impl(
                m_execution_count, code, silent,
                store_history, user_expressions, allow_stdin
        );

        reply["execution_count"] = m_execution_count;
        return reply;
    }

    nl::json Interpreter::complete_request(const std::string &code, int cursor_pos) {
        return complete_request_impl(code, cursor_pos);
    }

    nl::json Interpreter::inspect_request(const std::string &code, int cursor_pos, int detail_level) {
        return inspect_request_impl(code, cursor_pos, detail_level);
    }

    nl::json Interpreter::is_complete_request(const std::string &code) {
        return is_complete_request_impl(code);
    }

    nl::json Interpreter::kernel_info_request() {
        return kernel_info_request_impl();
    }

    void Interpreter::shutdown_request() {
        shutdown_request_impl();
    }

    nl::json Interpreter::internal_request(const nl::json &message) {
        return internal_request_impl(message);
    }

    void Interpreter::register_publisher(const publisher_type &publisher) {
        m_publisher = publisher;
    }

    void Interpreter::publish_stream(const std::string &name, const std::string &text) {
        if (m_publisher) {
            nl::json content;
            content["name"] = name;
            content["text"] = text;
            m_publisher("stream", nl::json::object(), std::move(content), buffer_sequence());
        }
    }

    void Interpreter::display_data(nl::json data, nl::json metadata, nl::json transient) {
        if (m_publisher) {
            m_publisher(
                    "display_data",
                    nl::json::object(),
                    build_display_content(std::move(data), std::move(metadata), std::move(transient)),
                    buffer_sequence());
        }
    }

    void Interpreter::update_display_data(nl::json data, nl::json metadata, nl::json transient) {
        if (m_publisher) {
            m_publisher(
                    "update_display_data",
                    nl::json::object(),
                    build_display_content(std::move(data), std::move(metadata), std::move(transient)),
                    buffer_sequence());
        }
    }

    void Interpreter::publish_execution_input(const std::string &code, int execution_count) {
        if (m_publisher) {
            nl::json content;
            content["code"] = code;
            content["execution_count"] = execution_count;
            m_publisher("execute_input", nl::json::object(), std::move(content), buffer_sequence());
        }
    }

    void Interpreter::publish_execution_result(int execution_count, nl::json data, nl::json metadata) {
        if (m_publisher) {
            nl::json content;
            content["execution_count"] = execution_count;
            content["data"] = std::move(data);
            content["metadata"] = std::move(metadata);
            m_publisher("execute_result", nl::json::object(), std::move(content), buffer_sequence());
        }
    }

    void Interpreter::publish_execution_error(const std::string &ename,
                                              const std::string &evalue,
                                              const std::vector<std::string> &trace_back) {
        if (m_publisher) {
            nl::json content;
            content["ename"] = ename;
            content["evalue"] = evalue;
            content["traceback"] = trace_back;
            m_publisher("error", nl::json::object(), std::move(content), buffer_sequence());
        }
    }

    void Interpreter::clear_output(bool wait) {
        if (m_publisher) {
            nl::json content;
            content["wait"] = wait;
            m_publisher("clear_output", nl::json::object(), std::move(content), buffer_sequence());
        }
    }

    void Interpreter::register_stdin_sender(const stdin_sender_type &sender) {
        m_stdin = sender;
    }

    void Interpreter::register_input_handler(const input_reply_handler_type &handler) {
        m_input_reply_handler = handler;
    }

    void Interpreter::register_comm_manager(CommManager *manager) {
        p_comm_manager = manager;
    }

    void Interpreter::register_parent_header(const parent_header_type &parent_header) {
        m_parent_header = parent_header;
    }

    const nl::json &Interpreter::parent_header() const noexcept {
        static const auto dummy = nl::json::object();
        if (m_parent_header) {
            return m_parent_header();
        } else {
            return dummy;
        }
    }

    void Interpreter::register_control_messenger(ControlMessenger &messenger) {
        p_messenger = &messenger;
    }

    void Interpreter::register_history_manager(const HistoryManager &history) {
        p_history = &history;
    }

    const HistoryManager &Interpreter::get_history_manager() const noexcept {
        return *p_history;
    }

    ControlMessenger &Interpreter::get_control_messenger() {
        return *p_messenger;
    }

    void Interpreter::input_request(const std::string &prompt, bool pwd) {
        if (m_stdin) {
            nl::json content;
            content["prompt"] = prompt;
            content["pwd"] = pwd;
            m_stdin("input_request", nl::json::object(), std::move(content));
        }
    }

    void Interpreter::input_reply(const std::string &value) {
        if (m_input_reply_handler) {
            m_input_reply_handler(value);
        }
    }

    nl::json Interpreter::internal_request_impl(const nl::json &) {
        nl::json res;
        res["status"] = "error";
        res["what"] = "internal request not supported";
        return res;
    }

    nl::json Interpreter::build_display_content(nl::json data, nl::json metadata, nl::json transient) {
        nl::json res;
        res["data"] = std::move(data);
        res["metadata"] = std::move(metadata);
        res["transient"] = std::move(transient);
        return res;
    }
}
