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


#include <array>
#include <mutex>
#include <string>

#include <collie/nlohmann/json.hpp>

#include <dwarf/core/logger.h>

namespace nl = nlohmann;

namespace dwarf
{

    /*****************
     * LoggerNolog *
     *****************/

    class LoggerNolog : public Logger
    {
    public:

        LoggerNolog() = default;
        virtual ~LoggerNolog() = default;

    private:

        void log_received_message_impl(const Message& message, Logger::channel c) const override;
        void log_sent_message_impl(const Message& message, Logger::channel c) const override;
        void log_iopub_message_impl(const PubMessage& message) const override;
        
        void log_message_impl(const std::string& socket_info,
                              const nl::json& header,
                              const nl::json& parent_header,
                              const nl::json& metadata,
                              const nl::json& content) const override;
    };

    /******************
     * LoggerCommon *
     ******************/

    class LoggerCommon : public Logger
    {
    public:

        virtual ~LoggerCommon();

    protected:

        using xlogger_ptr = std::unique_ptr<Logger>;
        LoggerCommon(Logger::level l, xlogger_ptr next_logger = nullptr);

    private:
        
        void log_received_message_impl(const Message& message, Logger::channel c) const override;
        void log_sent_message_impl(const Message& message, Logger::channel c) const override;
        void log_iopub_message_impl(const PubMessage& message) const override;
        
        void log_message_impl(const std::string& socket_info,
                              const nl::json& header,
                              const nl::json& parent_header,
                              const nl::json& metadata,
                              const nl::json& content) const override;

        virtual void log_message_impl(const std::string& socket_info,
                                      const nl::json& json_message) const = 0;

        xlogger_ptr p_next_logger;
        Logger::level m_level;
    };

    /*******************
     * LoggerConsole *
     *******************/

    class LoggerConsole : public LoggerCommon
    {
    public:

        using xlogger_ptr = LoggerCommon::xlogger_ptr;

        LoggerConsole(Logger::level l, xlogger_ptr next_logger = nullptr);
        virtual ~LoggerConsole() = default;

    private:

        void log_message_impl(const std::string& socket_info,
                              const nl::json& json_message) const override;

        mutable std::mutex m_mutex;
    };

    /****************
     * LoggerFile *
     ****************/

    class LoggerFile : public LoggerCommon
    {
    public:

        using xlogger_ptr = LoggerCommon::xlogger_ptr;

        LoggerFile(Logger::level l,
                     const std::string& file_name,
                     xlogger_ptr next_logger = nullptr);
        virtual ~LoggerFile() = default;

    private:

        void log_message_impl(const std::string& socket_info,
                              const nl::json& json_message) const override;

        std::string m_file_name;
        mutable std::mutex m_mutex;
    };
}

