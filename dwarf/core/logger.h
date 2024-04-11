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

#include <memory>

#include <collie/nlohmann/json.hpp>

#include <dwarf/core/config.h>
#include <dwarf/core/message.h>

namespace nl = nlohmann;

namespace dwarf {

    class DWARF_API Logger {
    public:

        enum channel {
            shell,
            control,
            stdinput,
            heartbeat,
            CHANNEL_SIZE
        };

        enum level {
            msg_type,
            content,
            full
        };

        virtual ~Logger() = default;

        Logger(const Logger &) = delete;

        Logger &operator=(const Logger &) = delete;

        Logger(Logger &&) = delete;

        Logger &operator=(Logger &&) = delete;

        void log_received_message(const Message &message, channel c) const;

        void log_sent_message(const Message &message, channel c) const;

        void log_iopub_message(const PubMessage &message) const;

        void log_message(const std::string &socket_info,
                         const nl::json &header,
                         const nl::json &parent_header,
                         const nl::json &metadata,
                         const nl::json &content) const;

    protected:

        Logger() = default;

    private:

        virtual void log_received_message_impl(const Message &message, channel c) const = 0;

        virtual void log_sent_message_impl(const Message &message, channel c) const = 0;

        virtual void log_iopub_message_impl(const PubMessage &message) const = 0;

        virtual void log_message_impl(const std::string &socket_info,
                                      const nl::json &header,
                                      const nl::json &parent_header,
                                      const nl::json &metadata,
                                      const nl::json &content) const = 0;
    };

    DWARF_API
    std::unique_ptr<Logger> make_console_logger(Logger::level log_level,
                                                 std::unique_ptr<Logger> next_logger = nullptr);

    DWARF_API
    std::unique_ptr<Logger> make_file_logger(Logger::level log_level,
                                              const std::string &file_name,
                                              std::unique_ptr<Logger> next_logger = nullptr);
}

