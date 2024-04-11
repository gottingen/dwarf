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

#include <dwarf/core/comm.h>

#include <dwarf/core/kernel_core.h>

namespace dwarf {
    void Target::publish_message(const std::string &msg_type,
                                 nl::json metadata,
                                 nl::json content,
                                 buffer_sequence buffers) const {
        if (p_manager->p_kernel != nullptr) {
            p_manager->p_kernel->publish_message(
                    msg_type, std::move(metadata), std::move(content), std::move(buffers), channel::SHELL);
        }
    }

    CommManager::CommManager(KernelCore *kernel) {
        p_kernel = kernel;
    }

    nl::json CommManager::get_metadata() const {
        // TODO: handle duplication
        nl::json metadata;
        metadata["started"] = iso8601_now();
        return metadata;
    }

    void CommManager::register_comm_target(const std::string &target_name,
                                           const target_function_type &callback) {
        m_targets[target_name] = Target(target_name, callback, this);
    }

    void CommManager::unregister_comm_target(const std::string &target_name) {
        m_targets.erase(target_name);
    }

    void CommManager::register_comm(Guid id, Comm *comm) {
        m_comms[id] = comm;
    }

    void CommManager::unregister_comm(Guid id) {
        m_comms.erase(id);
    }

    void CommManager::comm_open(Message request) {
        const nl::json &content = request.content();
        std::string target_name = content["target_name"];
        auto position = m_targets.find(target_name);

        if (position == m_targets.end()) {
            // Directly close the comm, as specified in the protocol
            if (p_kernel != nullptr) {
                p_kernel->publish_message(
                        "comm_close", nl::json::object(), std::move(content), buffer_sequence(), channel::SHELL
                );
            }
        } else {
            Target &target = position->second;
            Guid id = content["comm_id"];
            Comm comm = Comm(&target, id);
            target(std::move(comm), std::move(request));
        }
    }

    void CommManager::comm_close(Message request) {
        const nl::json &content = request.content();
        Guid id = content["comm_id"];
        auto position = m_comms.find(id);
        if (position == m_comms.end()) {
            throw std::runtime_error("No such comm registered: " + std::string(id));
        } else {
            position->second->handle_close(std::move(request));
        }
        m_comms.erase(id);
    }

    void CommManager::comm_msg(Message request) {
        const nl::json &content = request.content();
        Guid id = content["comm_id"];
        auto position = m_comms.find(id);
        if (position == m_comms.end()) {
            throw std::runtime_error("No such comm registered: " + std::string(id));
        } else {
            position->second->handle_message(std::move(request));
        }
    }
}
