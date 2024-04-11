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

#include <functional>
#include <list>
#include <map>
#include <string>
#include <utility>

#include <collie/nlohmann/json.hpp>

#include <dwarf/core/guid.h>
#include <dwarf/core/message.h>

namespace nl = nlohmann;

namespace dwarf {
    /***********************
     * Target declaration *
     ***********************/

    class Comm;

    class CommManager;

    /**
     * @class Target
     * @brief Comm target.
     */
    class DWARF_API Target {
    public:

        using function_type = std::function<void(Comm &&, Message)>;

        Target();

        Target(const std::string &name, const function_type &callback, CommManager *manager);

        std::string &name() & noexcept;

        const std::string &name() const & noexcept;

        std::string name() const && noexcept;

        void operator()(Comm &&comm, Message request) const;

        void publish_message(const std::string &, nl::json, nl::json, buffer_sequence) const;

        void register_comm(Guid, Comm *) const;

        void unregister_comm(Guid) const;

    private:

        std::string m_name;
        function_type m_callback;
        CommManager *p_manager;
    };

    /*********************
     * Comm declaration *
     *********************/

    /**
     * @class Comm
     * @brief Comm object.
     *
     */
    class DWARF_API Comm {
    public:

        using handler_type = std::function<void(Message)>;

        Comm() = delete;

        ~Comm();

        Comm(Comm &&);

        Comm(const Comm &);

        Comm(Target *target);

        Comm(Target *target, Guid id);

        Comm &operator=(Comm &&);

        Comm &operator=(const Comm &);

        void open(nl::json metadata, nl::json data, buffer_sequence buffers);

        void close(nl::json metadata, nl::json data, buffer_sequence buffers);

        void send(nl::json metadata, nl::json data, buffer_sequence buffers) const;

        Target &target() noexcept;

        const Target &target() const noexcept;

        void handle_message(Message request);

        void handle_close(Message request);

        Guid id() const noexcept;

        template<class T>
        void on_message(T &&handler);

        template<class T>
        void on_close(T &&handler);

    private:

        friend class CommManager;

        /**
         * Send comm message on iopub. Compose iopub message with specified data
         * and rebinds to target's publish_message
         * {
         *    "comm_id": m_id,
         *    "target_name": specified target name,
         *    "data": specified data
         * }
         */
        void send_comm_message(const std::string &msg_type,
                               nl::json metadata,
                               nl::json data,
                               buffer_sequence) const;

        void send_comm_message(const std::string &msg_type,
                               nl::json metadata,
                               nl::json data,
                               buffer_sequence,
                               const std::string &target_name) const;

        handler_type m_close_handler;
        handler_type m_message_handler;
        Target *p_target;
        Guid m_id;
        bool m_moved_from;
    };

    /*****************************
     * CommManager declaration *
     *****************************/

    class KernelCore;

    /**
     * @class CommManager
     * @brief Manager and registry for comms and comm targets in the kernel.
     */
    class DWARF_API CommManager {
    public:

        CommManager(KernelCore *kernel = nullptr);

        using target_function_type = Target::function_type;

        void register_comm_target(const std::string &target_name,
                                  const target_function_type &callback);

        void unregister_comm_target(const std::string &target_name);

        void comm_open(Message request);

        void comm_close(Message request);

        void comm_msg(Message request);

        std::map<Guid, Comm *> &comms() & noexcept;

        const std::map<Guid, Comm *> &comms() const & noexcept;

        std::map<Guid, Comm *> comms() const && noexcept;

        Target *target(const std::string &target_name);

    private:

        friend class Target;

        void register_comm(Guid, Comm *);

        void unregister_comm(Guid);

        nl::json get_metadata() const;

        std::map<Guid, Comm *> m_comms;
        std::map<std::string, Target> m_targets;
        KernelCore *p_kernel;
    };

    /**************************
     * Target implementation *
     **************************/

    inline Target::Target()
            : m_name(), m_callback(), p_manager(nullptr) {
    }

    inline Target::Target(const std::string &name,
                            const function_type &callback,
                            CommManager *manager)
            : m_name(name), m_callback(callback), p_manager(manager) {
    }

    inline std::string &Target::name() & noexcept {
        return m_name;
    }

    inline const std::string &Target::name() const & noexcept {
        return m_name;
    }

    inline std::string Target::name() const && noexcept {
        return m_name;
    }

    inline void Target::operator()(Comm &&comm, Message message) const {
        return m_callback(std::move(comm), std::move(message));
    }

    inline void Target::register_comm(Guid id, Comm *comm) const {
        p_manager->register_comm(id, comm);
    }

    inline void Target::unregister_comm(Guid id) const {
        p_manager->unregister_comm(id);
    }

    /************************
     * Comm implementation *
     ************************/

    inline Target &Comm::target() noexcept {
        return *p_target;
    }

    inline const Target &Comm::target() const noexcept {
        return *p_target;
    }

    inline void Comm::handle_close(Message message) {
        if (m_close_handler) {
            m_close_handler(std::move(message));
        }
    }

    inline void Comm::handle_message(Message message) {
        if (m_message_handler) {
            m_message_handler(std::move(message));
        }
    }

    inline void Comm::send_comm_message(const std::string &msg_type,
                                         nl::json metadata,
                                         nl::json data,
                                         buffer_sequence buffers) const {
        nl::json content;
        content["comm_id"] = m_id;
        content["data"] = std::move(data);
        target().publish_message(msg_type, std::move(metadata), std::move(content), std::move(buffers));
    }

    inline void Comm::send_comm_message(const std::string &msg_type,
                                         nl::json metadata,
                                         nl::json data,
                                         buffer_sequence buffers,
                                         const std::string &target_name) const {
        nl::json content;
        content["comm_id"] = m_id;
        content["target_name"] = target_name;
        content["data"] = std::move(data);
        target().publish_message(
                msg_type, std::move(metadata), std::move(content), std::move(buffers));
    }

    inline Comm::Comm(Comm &&comm)
            : m_close_handler(std::move(comm.m_close_handler)), m_message_handler(std::move(comm.m_message_handler)),
              p_target(std::move(comm.p_target)), m_id(std::move(comm.m_id)), m_moved_from(false) {
        comm.m_moved_from = true;
        p_target->register_comm(m_id,
                                this);  // Replacing the address of the moved comm with `this`.
    }

    inline Comm::Comm(const Comm &comm)
            : p_target(comm.p_target), m_id(dwarf::new_guid()), m_moved_from(false) {
        p_target->register_comm(m_id, this);
    }

    inline Comm &Comm::operator=(Comm &&comm) {
        m_close_handler = std::move(comm.m_close_handler);
        m_message_handler = std::move(comm.m_message_handler);
        p_target = std::move(comm.p_target);
        p_target->unregister_comm(m_id);
        m_id = std::move(comm.m_id);
        m_moved_from = false;
        comm.m_moved_from = true;
        p_target->register_comm(m_id,
                                this);  // Replacing the address of the moved comm with `this`.
        return *this;
    }

    inline Comm &Comm::operator=(const Comm &comm) {
        p_target = comm.p_target;
        p_target->unregister_comm(m_id);
        m_id = new_guid();
        m_moved_from = false;
        p_target->register_comm(m_id, this);
        return *this;
    }

    inline Comm::Comm(Target *target)
            : p_target(target), m_id(dwarf::new_guid()) {
        p_target->register_comm(m_id, this);
    }

    inline Comm::Comm(Target *target, Guid id)
            : p_target(target), m_id(id) {
        p_target->register_comm(m_id, this);
    }

    inline Comm::~Comm() {
        if (!m_moved_from) {
            p_target->unregister_comm(m_id);
        }
    }

    inline void Comm::open(nl::json metadata, nl::json data, buffer_sequence buffers) {
        send_comm_message("comm_open",
                          std::move(metadata),
                          std::move(data),
                          std::move(buffers),
                          p_target->name());
    }

    inline void Comm::close(nl::json metadata, nl::json data, buffer_sequence buffers) {
        send_comm_message("comm_close", std::move(metadata), std::move(data), std::move(buffers));
    }

    inline void Comm::send(nl::json metadata, nl::json data, buffer_sequence buffers) const {
        send_comm_message("comm_msg", std::move(metadata), std::move(data), std::move(buffers));
    }

    inline Guid Comm::id() const noexcept {
        return m_id;
    }

    template<class T>
    inline void Comm::on_message(T &&handler) {
        m_message_handler = std::forward<T>(handler);
    }

    template<class T>
    inline void Comm::on_close(T &&handler) {
        m_close_handler = std::forward<T>(handler);
    }

    /********************************
     * CommManager implementation *
     ********************************/

    inline Target *CommManager::target(const std::string &target_name) {
        return &m_targets[target_name];
    }

    inline std::map<Guid, Comm *> &CommManager::comms() & noexcept {
        return m_comms;
    }

    inline const std::map<Guid, Comm *> &CommManager::comms() const & noexcept {
        return m_comms;
    }

    inline std::map<Guid, Comm *> CommManager::comms() const && noexcept {
        return m_comms;
    }
}
