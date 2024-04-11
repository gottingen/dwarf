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


#include <memory>

#include <dwarf/core/kernel.h>
#include <dwarf/core/kernel_configuration.h>
#include <dwarf/dmq/server_zmq.h>

#include "custom_interpreter.hc"

int main(int argc, char* argv[])
{
    // Load configuration file
    std::string file_name = (argc == 1) ? "connection.json" : argv[2];
    dwarf::Configuration config = dwarf::load_configuration(file_name);

    auto context = dwarf::make_context<zmq::context_t>();

    // Create interpreter instance
    using interpreter_ptr = std::unique_ptr<custom::custom_interpreter>;
    interpreter_ptr interpreter = interpreter_ptr(new custom::custom_interpreter());

    // Create kernel instance and start it
    dwarf::Kernel kernel(config, dwarf::get_user_name(), std::move(context), std::move(interpreter), dwarf::make_xserver_zmq);
    kernel.start();

    return 0;
}
