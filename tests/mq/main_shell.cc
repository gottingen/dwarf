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


#include <iostream>
#include <memory>

#include <dwarf/core/kernel.h>
#include <dwarf/core/kernel_configuration.h>
#include <dwarf/dmq/server_shell_main.h>
#include "mock_interpreter.h"

int main(int argc, char* argv[])
{
    std::string file_name = (argc == 1) ? "connection.json" : argv[2];
    dwarf::Configuration config = dwarf::load_configuration(file_name);

    using history_manager_ptr = std::unique_ptr<dwarf::HistoryManager>;
    history_manager_ptr hist = dwarf::make_in_memory_history_manager();
    
    auto context = dwarf::make_context<zmq::context_t>();

    using interpreter_ptr = std::unique_ptr<dwarf::MockInterpreter>;
    interpreter_ptr interpreter = interpreter_ptr(new dwarf::MockInterpreter());
    
    dwarf::Kernel kernel(config,
                         dwarf::get_user_name(),
                         std::move(context),
                         std::move(interpreter),
                         dwarf::make_xserver_shell_main,
                         std::move(hist),
                         nullptr);
    std::cout << "starting kernel" << std::endl;
    kernel.start();

    return 0;
}
