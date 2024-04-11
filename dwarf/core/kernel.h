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

#include <dwarf/core/debugger.h>
#include <dwarf/core/config.h>
#include <dwarf/core/context.h>
#include <dwarf/core/history_manager.h>
#include <dwarf/core/interpreter.h>
#include <dwarf/core/kernel_configuration.h>
#include <dwarf/core/server.h>
#include <dwarf/core/logger.h>
#include <memory>
#include <string>

namespace dwarf {
    class KernelCore;

    DWARF_API
    std::string get_user_name();

    class DWARF_API Kernel {
    public:

        using context_ptr = std::unique_ptr<Context>;
        using history_manager_ptr = std::unique_ptr<HistoryManager>;
        using interpreter_ptr = std::unique_ptr<Interpreter>;
        using kernel_core_ptr = std::unique_ptr<KernelCore>;
        using logger_ptr = std::unique_ptr<Logger>;
        using server_ptr = std::unique_ptr<Server>;
        using debugger_ptr = std::unique_ptr<Debugger>;
        using server_builder = server_ptr (*)(Context &context,
                                              const Configuration &config,
                                              nl::json::error_handler_t eh);
        using debugger_builder = debugger_ptr (*)(Context &context,
                                                  const Configuration &config,
                                                  const std::string &,
                                                  const std::string &,
                                                  const nl::json &);

        Kernel(const Configuration &config,
                const std::string &user_name,
                context_ptr context,
                interpreter_ptr interpreter,
                server_builder sbuilder,
                history_manager_ptr history_manager = make_in_memory_history_manager(),
                logger_ptr logger = nullptr,
                debugger_builder dbuilder = make_null_debugger,
                nl::json debugger_config = nl::json::object(),
                nl::json::error_handler_t eh = nl::json::error_handler_t::strict);

        Kernel(const std::string &user_name,
                context_ptr context,
                interpreter_ptr interpreter,
                server_builder sbuilder,
                history_manager_ptr history_manager = make_in_memory_history_manager(),
                logger_ptr logger = nullptr,
                debugger_builder dbuilder = make_null_debugger,
                nl::json debugger_config = nl::json::object(),
                nl::json::error_handler_t eh = nl::json::error_handler_t::strict);

        ~Kernel();

        void start();

        void stop();

        const Configuration &get_config();

        Server &get_server();

    private:

        void init(server_builder sbuilder,
                  debugger_builder dbuilder);

        Configuration m_config;
        std::string m_kernel_id;
        std::string m_session_id;
        std::string m_user_name;
        // The context must be declared before any other
        // middleware component since it must be destroyed
        // last
        context_ptr p_context;
        interpreter_ptr p_interpreter;
        history_manager_ptr p_history_manager;
        logger_ptr p_logger;
        server_ptr p_server;
        debugger_ptr p_debugger;
        kernel_core_ptr p_core;
        nl::json m_debugger_config;
        nl::json::error_handler_t m_error_handler;
    };
}
