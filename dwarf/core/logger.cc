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


#include <dwarf/core/logger.h>

#include <collie/nlohmann/json.hpp>

namespace nl = nlohmann;

namespace dwarf {
    void Logger::log_received_message(const Message &message, channel c) const {
        log_received_message_impl(message, c);
    }

    void Logger::log_sent_message(const Message &message, channel c) const {
        log_sent_message_impl(message, c);
    }

    void Logger::log_iopub_message(const PubMessage &message) const {
        log_iopub_message_impl(message);
    }

    void Logger::log_message(const std::string &socket_info,
                              const nl::json &header,
                              const nl::json &parent_header,
                              const nl::json &metadata,
                              const nl::json &content) const {
        log_message_impl(socket_info,
                         header,
                         parent_header,
                         metadata,
                         content);
    }
}

