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


#include <thread>
#include <chrono>
#include <iostream>

#include <dwarf/zmq/zmq_addon.hpp>
#include <dwarf/core/guid.h>
#include <dwarf/dmq/server_control_main.h>
#include <dwarf/dmq/middleware.h>
#include <dwarf/dmq/control.h>

namespace dwarf
{
    ServerControlMain::ServerControlMain(zmq::context_t& context,
                                               const Configuration& config,
                                               nl::json::error_handler_t eh)
        : ServerZmqSplit(context, config, eh)
    {
    }

    ServerControlMain::~ServerControlMain()
    {
    }

    void ServerControlMain::start_server(zmq::multipart_t& wire_msg)
    {
        ServerZmqSplit::start_publisher_thread();
        ServerZmqSplit::start_heartbeat_thread();
        ServerZmqSplit::start_shell_thread();

        ServerZmqSplit::get_controller().publish(wire_msg);
        ServerZmqSplit::get_controller().run();
    }

    std::unique_ptr<Server> make_xserver_control_main(Context& context,
                                                       const Configuration& config,
                                                       nl::json::error_handler_t eh)
    {
        return std::make_unique<ServerControlMain>(context.get_wrapped_context<zmq::context_t>(), config, eh);
    }
}

