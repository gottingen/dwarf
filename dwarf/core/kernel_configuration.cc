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
#include <string>

#include <collie/nlohmann/json.hpp>

#include <dwarf/core/kernel_configuration.h>

namespace nl = nlohmann;

namespace dwarf {
    Configuration load_configuration(const std::string &file_name) {
        std::ifstream ifs(file_name);

        nl::json doc;
        ifs >> doc;

        Configuration res;
        res.m_transport = doc["transport"].get<std::string>();
        res.m_ip = doc["ip"].get<std::string>();
        res.m_control_port = std::to_string(doc["control_port"].get<int>());
        res.m_shell_port = std::to_string(doc["shell_port"].get<int>());
        res.m_stdin_port = std::to_string(doc["stdin_port"].get<int>());
        res.m_iopub_port = std::to_string(doc["iopub_port"].get<int>());
        res.m_hb_port = std::to_string(doc["hb_port"].get<int>());
        res.m_signature_scheme = doc.value("signature_scheme", "");
        if (res.m_signature_scheme != "") {
            res.m_key = doc["key"].get<std::string>();
        } else {
            res.m_key = "";
        }

        return res;
    }
}
