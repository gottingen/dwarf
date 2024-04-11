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

#include <fstream>
#include <iostream>

#include <dwarf/dmq/dap_tcp_client.h>
#include <dwarf/dmq/debugger_base.h>
#include <dwarf/dmq/middleware.h>

using namespace std::placeholders;

namespace dwarf
{
    DebuggerInfo::DebuggerInfo(std::size_t hash_seed,
                                   const std::string& tmp_file_prefix,
                                   const std::string& tmp_file_suffix,
                                   bool rich_rendering,
                                   std::vector<std::string> exception_paths)
        : m_hash_seed(hash_seed)
        , m_tmp_file_prefix(tmp_file_prefix)
        , m_tmp_file_suffix(tmp_file_suffix)
        , m_rich_rendering(rich_rendering)
        , m_exception_paths(exception_paths)
    {
    }

    DebuggerBase::DebuggerBase(zmq::context_t& context)
        : m_header_socket(context, zmq::socket_type::req)
        , m_request_socket(context, zmq::socket_type::req)
        , m_is_started(false)
    {
        m_header_socket.set(zmq::sockopt::linger, dwarf::get_socket_linger());
        m_request_socket.set(zmq::sockopt::linger, dwarf::get_socket_linger());

        register_request_handler("debugInfo", std::bind(&DebuggerBase::debug_info_request, this, _1), false);
        register_request_handler("dumpCell", std::bind(&DebuggerBase::dump_cell_request, this, _1), true);
        register_request_handler("setBreakpoints", std::bind(&DebuggerBase::set_breakpoints_request, this, _1), true);
        register_request_handler("source", std::bind(&DebuggerBase::source_request, this, _1), true);
        register_request_handler("stackTrace", std::bind(&DebuggerBase::stack_trace_request, this, _1), true);
        register_request_handler("variables", std::bind(&DebuggerBase::variables_request, this, _1), true);

        register_event_handler("continued", std::bind(&DebuggerBase::continued_event, this, _1));
        register_event_handler("stopped", std::bind(&DebuggerBase::stopped_event, this, _1));
    }

    bool DebuggerBase::is_started() const
    {
        return m_is_started;
    }

    std::function<void (const nl::json&)> DebuggerBase::get_event_callback()
    {
        return std::bind(&DebuggerBase::handle_event, this, _1);
    }

    /*********************
     * Requests handling *
     *********************/

    void DebuggerBase::register_request_handler(const std::string& command,
                                                  const request_handler_t& handler,
                                                  bool require_started)
    {
        request_handler_map_t& m = require_started ? m_started_handler : m_request_handler;
        m[command] = handler;
    }

    nl::json DebuggerBase::debug_info_request(const nl::json& message)
    {
        nl::json breakpoint_list = nl::json::array();

        if(m_is_started)
        {
            for(auto it = m_breakpoint_list.cbegin(); it != m_breakpoint_list.cend(); ++it)
            {
                breakpoint_list.push_back({{"source", it->first},
                                           {"breakpoints", it->second}});
            }
        }

        DebuggerInfo info = get_debugger_info();
        std::lock_guard<std::mutex> lock(m_stopped_mutex);
        nl::json reply = {
            {"type", "response"},
            {"request_seq", message["seq"]},
            {"success", true},
            {"command", message["command"]},
            {"body", {
                {"isStarted", m_is_started},
                {"hashMethod", "Murmur2"},
                {"hashSeed", info.m_hash_seed},
                {"tmpFilePrefix", info.m_tmp_file_prefix},
                {"tmpFileSuffix", info.m_tmp_file_suffix},
                {"breakpoints", breakpoint_list},
                {"stoppedThreads", m_stopped_threads},
                {"richRendering", info.m_rich_rendering},
                {"exceptionPaths", info.m_exception_paths}
            }}
        };
        return reply;
    }

    nl::json DebuggerBase::dump_cell_request(const nl::json& message)
    {
        std::string code;
        try
        {
            code = message["arguments"]["code"].get<std::string>();
        }
        catch(nl::json::type_error& e)
        {
            std::clog << e.what() << std::endl;
        }
        catch(...)
        {
            std::clog << "XDEBUGGER: Unknown issue" << std::endl;
        }

        std::string next_file_name = get_cell_temporary_file(code);
        std::clog << "XDEBUGGER: dumped " << next_file_name << std::endl;

        std::fstream fs(next_file_name, std::ios::in);
        if(!fs.is_open())
        {
            fs.clear();
            fs.open(next_file_name, std::ios::out);
            fs << code;
        }

        nl::json reply = {
            {"type", "response"},
            {"request_seq", message["seq"]},
            {"success", true},
            {"command", message["command"]},
            {"body", {
                {"sourcePath", next_file_name}
            }}
        };
        return reply;
    }

    nl::json DebuggerBase::set_breakpoints_request(const nl::json& message)
    {
        std::string source = message["arguments"]["source"]["path"].get<std::string>();
        m_breakpoint_list.erase(source);
        nl::json bp_json = message["arguments"]["breakpoints"];
        std::vector<nl::json> bp_list(bp_json.begin(), bp_json.end());
        m_breakpoint_list.insert(std::make_pair(std::move(source), std::move(bp_list)));
        nl::json breakpoint_reply = forward_message(message);
        return breakpoint_reply;
    }

    nl::json DebuggerBase::source_request(const nl::json& message)
    {
        std::string sourcePath;
        try
        {
            sourcePath = message["arguments"]["source"]["path"].get<std::string>();
        }
        catch(nl::json::type_error& e)
        {
            std::clog << e.what() << std::endl;
        }
        catch(...)
        {
            std::clog << "XDEBUGGER: Unknown issue" << std::endl;
        }

        std::ifstream ifs(sourcePath, std::ios::in);
        if(!ifs.is_open())
        {
            nl::json reply = {
                {"type", "response"},
                {"request_seq", message["seq"]},
                {"success", false},
                {"command", message["command"]},
                {"message", "source unavailable"},
                {"body", {{}}}
            };
            return reply;
        }

        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        nl::json reply = {
            {"type", "response"},
            {"request_seq", message["seq"]},
            {"success", true},
            {"command", message["command"]},
            {"body", {
                {"content", content}
            }}
        };
        return reply;
    }

    nl::json DebuggerBase::stack_trace_request(const nl::json& message)
    {
        nl::json reply = forward_message(message);
        size_t size = reply["body"]["stackFrames"].size();
        for(size_t i = 0; i < size; ++i)
        {
            if(reply["body"]["stackFrames"][i]["source"]["path"] == "<string>")
            {
                reply["body"]["stackFrames"].erase(i);
                break;
            }
        }
#ifdef WIN32
        size = reply["body"]["stackFrames"].size();
        for(size_t i = 0; i < size; ++i)
        {
            std::string path = reply["body"]["stackFrames"][i]["source"]["path"];
            std::replace(path.begin(), path.end(), '\\', '/');
            reply["body"]["stackFrames"][i]["source"]["path"] = path;
        }
#endif
        return reply;
    }

    nl::json DebuggerBase::variables_request(const nl::json& message)
    {
        return variables_request_impl(message);
    }

    nl::json DebuggerBase::forward_message(const nl::json& message)
    {
        std::string content = message.dump();
        size_t content_length = content.length();
        std::string buffer = DapTcpClient::HEADER
                           + std::to_string(content_length)
                           + DapTcpClient::SEPARATOR
                           + content;
        zmq::message_t raw_message(buffer.c_str(), buffer.length());
        m_request_socket.send(raw_message, zmq::send_flags::none);

        zmq::message_t raw_reply;
        (void)m_request_socket.recv(raw_reply);

        return nl::json::parse(std::string(raw_reply.data<const char>(), raw_reply.size()));
    }

    /*******************
     * Events handling *
     *******************/

    void DebuggerBase::register_event_handler(const std::string& event,
                                                const event_handler_t& handler)
    {
        m_event_handler[event] = handler;
    }
    
    void DebuggerBase::continued_event(const nl::json& message)
    {
        std::lock_guard<std::mutex> lock(m_stopped_mutex);
        if (message["body"]["allThreadsContinued"])
        {
            m_stopped_threads.clear();
        }
        else
        {
            int id = message["body"]["threadId"];
            m_stopped_threads.erase(id);
        }
    }

    void DebuggerBase::stopped_event(const nl::json& message)
    {
        std::lock_guard<std::mutex> lock(m_stopped_mutex);
        if (message["body"]["allThreadsStopped"])
        {
            for (auto& id: message["body"]["threadList"])
            {
                m_stopped_threads.insert(id.get<int>());
            }
        }
        else
        {
            int id = message["body"]["threadId"];
            m_stopped_threads.insert(id);
        }
    }

    const std::set<int>& DebuggerBase::get_stopped_threads() const
    {
        return m_stopped_threads;
    }

    nl::json DebuggerBase::variables_request_impl(const nl::json& message)
    {
        nl::json reply = forward_message(message);
        auto start_it =  message["arguments"].find("start");
        auto count_it = message["arguments"].find("count");
        auto end_it = message["arguments"].end();
        if(start_it != end_it || count_it != end_it)
        {
            int start = start_it != end_it ? start_it->get<int>() : 0;
            int count = count_it != end_it ? count_it->get<int>() : 0;
            if(start != 0 || count != 0)
            {
                int end = count == 0 ? reply["body"]["variables"].size() : start + count;
                nl::json old_variables_list = reply["body"]["variables"];
                reply["body"].erase("variables");
                nl::json variables_list;
                for(int i = start; i < end; ++i)
                {
                    variables_list.push_back(old_variables_list.at(i));
                }
                reply["body"]["variables"] = variables_list;
            }
        }
        return reply;
    }

    /**************************
     * Private implementation *
     **************************/

    void DebuggerBase::handle_event(const nl::json& message)
    {
        std::string event = message["event"];
        auto it = m_event_handler.find(event);
        if (it != m_event_handler.end())
        {
            (it->second)(message);
        }
    }

    nl::json DebuggerBase::process_request_impl(const nl::json& header,
                                                  const nl::json& message)
    {
        nl::json reply = nl::json::object();

        if(message["command"] == "initialize")
        {
            if(m_is_started)
            {
                std::clog << "XDEBUGGER: the debugger has already started" << std::endl;
            }
            else
            {
                m_is_started = start(m_header_socket, m_request_socket);
                if (m_is_started)
                {
                    std::clog << "XDEBUGGER: the debugger has started" << std::endl;
                }
                else
                {
                    reply = 
                    {
                        {"command", "initialize"},
                        {"request_seq", message["seq"]},
                        {"seq", 3},
                        {"success", false},
                        {"type", "response"}
                    };
                }
            }
        }

        auto it = m_request_handler.find(message["command"]);
        if (it != m_request_handler.end())
        {
            reply = (it->second)(message);
        }
        else if (m_is_started)
        {
            std::string header_buffer = header.dump();
            zmq::message_t raw_header(header_buffer.c_str(), header_buffer.length());
            m_header_socket.send(raw_header, zmq::send_flags::none);
            // client responds with ACK message
            (void)m_header_socket.recv(raw_header);

            auto it = m_started_handler.find(message["command"]);
            if (it != m_started_handler.end())
            {
                reply = (it->second)(message);
            }
            else
            {
                reply = forward_message(message);
            }
        }

        if (message["command"] == "disconnect")
        {
            stop(m_header_socket, m_request_socket);
            m_breakpoint_list.clear();
            m_stopped_threads.clear();
            m_is_started = false;
            std::clog << "XDEBUGGER: the debugger has stopped" << std::endl;
        }
        return reply;
    }
}

