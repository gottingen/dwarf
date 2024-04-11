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

#include <collie/nlohmann/json.hpp>

#include <dwarf/core/config.h>
#include <dwarf/core/context.h>
#include <dwarf/core/kernel_configuration.h>
#include <dwarf/core/control_messenger.h>

namespace nl = nlohmann;

namespace dwarf {
    class DWARF_API Debugger {
    public:

        virtual ~Debugger();

        Debugger(const Debugger &) = delete;

        Debugger &operator=(const Debugger &) = delete;

        Debugger(Debugger &&) = delete;

        Debugger &operator=(Debugger &&) = delete;

        void register_control_messenger(ControlMessenger &messenger);

        nl::json process_request(const nl::json &header,
                                 const nl::json &message);

    protected:

        Debugger();

        ControlMessenger &get_control_messenger();

    private:

        virtual nl::json process_request_impl(const nl::json &header,
                                              const nl::json &message) = 0;

        ControlMessenger *p_messenger;
    };

    DWARF_API
    std::unique_ptr<Debugger> make_null_debugger(Context &context,
                                                 const Configuration &kernel_configuration,
                                                 const std::string &user_name,
                                                 const std::string &session_id,
                                                 const nl::json &debugger_config);

}
