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

#include <dwarf/core/context.h>
#include <dwarf/core/kernel_configuration.h>

#include <dwarf/core/config.h>
#include <dwarf/dmq/server_zmq_split.h>

namespace dwarf {
    class DWARF_API ServerShellMain : public ServerZmqSplit {
    public:

        ServerShellMain(zmq::context_t &context,
                        const Configuration &config,
                        nl::json::error_handler_t he);

        virtual ~ServerShellMain();

    private:

        void start_server(zmq::multipart_t &wire_msg) override;
    };

    DWARF_API
    std::unique_ptr<Server> make_xserver_shell_main(Context &context,
                                                    const Configuration &config,
                                                    nl::json::error_handler_t eh = nl::json::error_handler_t::strict);
}

