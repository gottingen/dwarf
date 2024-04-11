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

#include <dwarf/zmq/zmq_addon.hpp>

#include <dwarf/core/message.h>

#include <dwarf/core/config.h>
#include <dwarf/dmq/authentication.h>

namespace dwarf {
    class DWARF_API xzmq_serializer {
    public:

        static zmq::multipart_t serialize(Message &&msg,
                                          const Authentication &auth,
                                          nl::json::error_handler_t error_handler = nl::json::error_handler_t::strict);

        static Message deserialize(zmq::multipart_t &wire_msg,
                                   const Authentication &auth);

        static zmq::multipart_t serialize_iopub(PubMessage &&msg,
                                                const Authentication &auth,
                                                nl::json::error_handler_t error_handler = nl::json::error_handler_t::strict);

        static PubMessage deserialize_iopub(zmq::multipart_t &wire_msg,
                                            const Authentication &auth);
    };

}

