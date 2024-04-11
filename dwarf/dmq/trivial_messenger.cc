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

#include <dwarf/dmq/server_zmq.h>
#include <dwarf/dmq/trivial_messenger.h>

namespace dwarf {
    TrivialMessenger::TrivialMessenger(ServerZmq *server)
            : p_server(server) {
    }

    TrivialMessenger::~TrivialMessenger() {
        p_server = nullptr;
    }

    nl::json TrivialMessenger::send_to_shell_impl(const nl::json &message) {
        return p_server->notify_internal_listener(message);
    }
}

