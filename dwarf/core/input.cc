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


#include <string>

#include <dwarf/core/input.h>
#include <dwarf/core/interpreter.h>

namespace dwarf {
    std::string blocking_input_request(const std::string &prompt, bool password) {
        auto &interpreter = dwarf::get_interpreter();

        // Register the input handler
        std::string value;
        interpreter.register_input_handler([&value](const std::string &v) { value = v; });

        // Send the input request
        interpreter.input_request(prompt, password);

        // Remove input handler
        interpreter.register_input_handler(nullptr);

        return value;
    }
}
