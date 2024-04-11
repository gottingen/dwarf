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

#include <dwarf/core/interpreter.h>

#include <collie/nlohmann/json.hpp>

using dwarf::Interpreter;

namespace nl = nlohmann;

namespace custom {
    class custom_interpreter : public Interpreter {
    public:

        custom_interpreter() = default;

        virtual ~custom_interpreter() = default;

    private:

        void configure_impl() override;

        nl::json execute_request_impl(int execution_counter,
                                      const std::string &code,
                                      bool silent,
                                      bool store_history,
                                      nl::json user_expressions,
                                      bool allow_stdin) override;

        nl::json complete_request_impl(const std::string &code,
                                       int cursor_pos) override;

        nl::json inspect_request_impl(const std::string &code,
                                      int cursor_pos,
                                      int detail_level) override;

        nl::json is_complete_request_impl(const std::string &code) override;

        nl::json kernel_info_request_impl() override;

        void shutdown_request_impl() override;
    };
}
