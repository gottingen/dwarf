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

#include <collie/nlohmann/json.hpp>

#include <dwarf/core/message.h>
#include <dwarf/core/logger_impl.h>

namespace nl = nlohmann;

namespace dwarf
{

    /********************************
     * LoggerNolog implementation *
     ********************************/

    void LoggerNolog::log_received_message_impl(const Message&, Logger::channel) const
    {
    }

    void LoggerNolog::log_sent_message_impl(const Message&, Logger::channel) const
    {
    }

    void LoggerNolog::log_iopub_message_impl(const PubMessage&) const
    {
    }
    
    void LoggerNolog::log_message_impl(const std::string&,
                                         const nl::json&,
                                         const nl::json&,
                                         const nl::json&,
                                         const nl::json&) const
    {
    }

    /*********************************
     * LoggerCommon implementation *
     *********************************/

    namespace
    {
        const std::array<std::string, Logger::CHANNEL_SIZE> channel_str = { "shell", "control", "stdin", "heartbeat" };


    
        /**
         * @copyright Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
         * @sa http://bjoern.hoehrmann.de/utf-8/decoder/dfa/
         */
        std::uint8_t decode_utf8(std::uint8_t& state, std::uint32_t& codep, const std::uint8_t byte) noexcept
        {
            static const uint8_t UTF8_ACCEPT = 0;
            static const std::array<std::uint8_t, 400> utf8d =
            {
                {
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 00..1F
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 20..3F
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 40..5F
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 60..7F
                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, // 80..9F
                    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, // A0..BF
                    8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // C0..DF
                    0xA, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x3, 0x3, // E0..EF
                    0xB, 0x6, 0x6, 0x6, 0x5, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, // F0..FF
                    0x0, 0x1, 0x2, 0x3, 0x5, 0x8, 0x7, 0x1, 0x1, 0x1, 0x4, 0x6, 0x1, 0x1, 0x1, 0x1, // s0..s0
                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, // s1..s2
                    1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, // s3..s4
                    1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, // s5..s6
                    1, 3, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 // s7..s8
                }
            };

            const std::uint8_t type = utf8d[byte];

            codep = (state != UTF8_ACCEPT)
                    ? (byte & 0x3fu) | (codep << 6u)
                    : (0xFFu >> type) & (byte);

            state = utf8d[256u + state * 16u + type];
            return state;
        }

        bool is_utf8_valid(const std::string& str)
        {
            std::uint32_t codepoint(0);
            std::uint8_t state(0);

            for (std::size_t i = 0; i < str.size(); ++i)
            {
                auto byte = static_cast<uint8_t>(str[i]);
                decode_utf8(state, codepoint, byte);
            }
            return !state;
        }
    }

    LoggerCommon::LoggerCommon(Logger::level l, xlogger_ptr next_logger)
        : p_next_logger(next_logger != nullptr ? std::move(next_logger) : std::make_unique<LoggerNolog>())
        , m_level(l)
    {
    }

    LoggerCommon::~LoggerCommon()
    {
    }
    
    void LoggerCommon::log_received_message_impl(const Message& message, Logger::channel c) const
    {
        std::string id = message.identities()[0];
        std::string socket_info = "DWARF: received message on "
                                + channel_str[c] + " - "
                                + (is_utf8_valid(id) ? id : "invalid UTF8");
        Logger::log_message(socket_info,
                             message.header(),
                             message.parent_header(),
                             message.metadata(),
                             message.content());
    }

    void LoggerCommon::log_sent_message_impl(const Message& message, Logger::channel c) const
    {
        std::string id = message.identities()[0];
        std::string socket_info = "DWARF: sent message on "
                                + channel_str[c] + " - "
                                + (is_utf8_valid(id) ? id : "invalid UTF8");
        Logger::log_message(socket_info,
                             message.header(),
                             message.parent_header(),
                             message.metadata(),
                             message.content());
    }

    void LoggerCommon::log_iopub_message_impl(const PubMessage& message) const
    {
        std::string socket_info = "DWARF: sent message on iopub - "
                                + message.topic();
        Logger::log_message(socket_info,
                             message.header(),
                             message.parent_header(),
                             message.metadata(),
                             message.content());
    }
    
    void LoggerCommon::log_message_impl(const std::string& socket_info,
                                          const nl::json& header,
                                          const nl::json& parent_header,
                                          const nl::json& metadata,
                                          const nl::json& json_content) const
    {
        nl::json message;
        message["msg_type"] = header.value("msg_type", "");
        switch(m_level)
        {
        case msg_type:
            break;
        case content:
            message["content"] = json_content;
            break;
        case full:
        default:
            {
                message["header"] = header;
                message["parent_header"] = parent_header;
                message["metadata"] = metadata;
                message["content"] = json_content;
            }
            break;
        }

        log_message_impl(socket_info, message);
        p_next_logger->log_message(socket_info, header, parent_header, metadata, json_content);
    }

    /**********************************
     * LoggerConsole implementation *
     **********************************/

    LoggerConsole::LoggerConsole(Logger::level l, xlogger_ptr next_logger)
        : LoggerCommon(l, std::move(next_logger))
    {
    }

    void LoggerConsole::log_message_impl(const std::string& socket_info,
                                           const nl::json& json_message) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::cout << socket_info << '\n' << json_message.dump(4) << std::endl;
    }

    /*******************************
     * LoggerFile implementation *
     *******************************/

    LoggerFile::LoggerFile(Logger::level l,
                               const std::string& file_name,
                               xlogger_ptr next_logger)
        : LoggerCommon(l, std::move(next_logger))
        , m_file_name(file_name)
    {
    }

    void LoggerFile::log_message_impl(const std::string& socket_info,
                                        const nl::json& json_message) const
    {
        nl::json log;
        log["info"] = socket_info;
        log["message"] = json_message;
        std::lock_guard<std::mutex> lock(m_mutex);
        std::ofstream out(m_file_name, std::ios_base::app);
        out << log.dump(4) << std::endl;
    }

    /************************************
     * Builder functions implementation *
     ************************************/

    std::unique_ptr<Logger> make_console_logger(Logger::level log_level,
                                                 std::unique_ptr<Logger> next_logger)
    {
        return std::make_unique<LoggerConsole>(log_level, std::move(next_logger));
    }

    std::unique_ptr<Logger> make_file_logger(Logger::level log_level,
                                              const std::string& file_name,
                                              std::unique_ptr<Logger> next_logger)
    {
        return std::make_unique<LoggerFile>(log_level, file_name, std::move(next_logger));
    }
}

