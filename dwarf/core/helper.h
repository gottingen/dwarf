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

#include <iostream>
#include <string>
#include <vector>

#include <dwarf/core/config.h>
#include <dwarf/core/kernel_configuration.h>

#include <collie/nlohmann/json.hpp>

namespace nl = nlohmann;

namespace dwarf {
    DWARF_API std::string print_starting_message(const Configuration &config);

    DWARF_API std::string extract_filename(int argc, char *argv[]);

    DWARF_API bool should_print_version(int argc, char *argv[]);

    DWARF_API
    nl::json create_error_reply(const std::string &ename = std::string(),
                                const std::string &evalue = std::string(),
                                const nl::json &trace_back = nl::json::array());

    DWARF_API
    nl::json create_successful_reply(const nl::json &payload = nl::json::array(),
                                     const nl::json &user_expressions = nl::json::object());

    DWARF_API
    nl::json create_complete_reply(const nl::json &matches,
                                   const int &cursor_start,
                                   const int &cursor_end,
                                   const nl::json &metadata = nl::json::object());

    DWARF_API
    nl::json create_inspect_reply(const bool found = false,
                                  const nl::json &data = nl::json::object(),
                                  const nl::json &metadata = nl::json::object());

    DWARF_API
    nl::json create_is_complete_reply(const std::string &status = std::string(),
                                      const std::string &indent = std::string(""));

    DWARF_API
    nl::json create_info_reply(const std::string &protocol_version = std::string(),
                               const std::string &implementation = std::string(),
                               const std::string &implementation_version = std::string(),
                               const std::string &language_name = std::string(),
                               const std::string &language_version = std::string(),
                               const std::string &language_mimetype = std::string(),
                               const std::string &language_file_extension = std::string(),
                               const std::string &pygments_lexer = std::string(),
                               const std::string &language_codemirror_mode = std::string(),
                               const std::string &language_nbconvert_exporter = std::string(),
                               const std::string &banner = std::string(),
                               const bool debugger = false,
                               const nl::json &help_links = nl::json::array());
}
