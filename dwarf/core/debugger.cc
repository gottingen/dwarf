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


#include <collie/nlohmann/json.hpp>

#include <dwarf/core/debugger.h>

namespace nl = nlohmann;

namespace dwarf {
    Debugger::Debugger()
            : p_messenger(nullptr) {
    }

    Debugger::~Debugger() {
        p_messenger = nullptr;
    }

    void Debugger::register_control_messenger(ControlMessenger &messenger) {
        p_messenger = &messenger;
    }

    nl::json Debugger::process_request(const nl::json &header,
                                        const nl::json &message) {
        return process_request_impl(header, message);
    }

    ControlMessenger &Debugger::get_control_messenger() {
        return *p_messenger;
    }

    std::unique_ptr<Debugger> make_null_debugger(Context &,
                                                  const Configuration &,
                                                  const std::string &,
                                                  const std::string &,
                                                  const nl::json &) {
        return nullptr;
    }
}

