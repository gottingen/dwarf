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

#include <string>

#include <dwarf/core/config.h>

namespace dwarf {
    struct DWARF_API Configuration {
        std::string m_transport = "tcp";
        std::string m_ip = "127.0.0.1";
        std::string m_control_port;
        std::string m_shell_port;
        std::string m_stdin_port;
        std::string m_iopub_port;
        std::string m_hb_port;
        std::string m_signature_scheme = "hmac-sha256";
        std::string m_key;
    };

    DWARF_API
    Configuration load_configuration(const std::string &file_name);
}
