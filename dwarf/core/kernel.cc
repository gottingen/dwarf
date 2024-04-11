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
#include <random>

#include <dwarf/core/kernel.h>
#include <dwarf/core/guid.h>
#include <dwarf/core/history_manager.h>
#include <dwarf/core/control_messenger.h>
#include <dwarf/core/kernel_core.h>
#include <dwarf/core/logger_impl.h>

#if (defined(__linux__) || defined(__unix__))
#define LINUX_PLATFORM
#elif (defined(_WIN32) || defined(_WIN64))
#define WINDOWS_PLATFORM
#elif defined(__APPLE__)
#define APPLE_PLATFORM
#endif

#if (defined(LINUX_PLATFORM) || defined(APPLE_PLATFORM))

#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#elif (defined(_WIN32) || defined(_WIN64))
#include <windows.h>
#include <Lmcons.h>
#endif

namespace dwarf {
    std::string get_user_name() {
#if defined(DWARF_EMSCRIPTEN_WASM_BUILD)
        return "unspecified user";
#elif (defined(LINUX_PLATFORM) || defined(APPLE_PLATFORM))
        struct passwd *pws;
        pws = getpwuid(geteuid());
        if (pws != nullptr) {
            std::string res = pws->pw_name;
            return res;
        } else {
            const char *user = std::getenv("USER");
            if (user != nullptr) {
                std::string res = user;
                return res;
            } else {
                return "unspecified user";
            }
        }
#elif defined(WINDOWS_PLATFORM)
        char username[UNLEN + 1];
        DWORD username_len = UNLEN + 1;
        GetUserName(username, &username_len);
        return username;
#else
        return "unspecified user";
#endif
    }

    Kernel::Kernel(const Configuration &config,
                     const std::string &user_name,
                     context_ptr context,
                     interpreter_ptr interpreter,
                     server_builder sbuilder,
                     history_manager_ptr history_manager,
                     logger_ptr logger,
                     debugger_builder dbuilder,
                     nl::json debugger_config,
                     nl::json::error_handler_t eh)
            : m_config(config), m_user_name(user_name), p_context(std::move(context)),
              p_interpreter(std::move(interpreter)), p_history_manager(std::move(history_manager)),
              p_logger(std::move(logger)), m_debugger_config(debugger_config), m_error_handler(eh) {
        init(sbuilder, dbuilder);
    }

    Kernel::Kernel(const std::string &user_name,
                     context_ptr context,
                     interpreter_ptr interpreter,
                     server_builder sbuilder,
                     history_manager_ptr history_manager,
                     logger_ptr logger,
                     debugger_builder dbuilder,
                     nl::json debugger_config,
                     nl::json::error_handler_t eh)
            : m_user_name(user_name), p_context(std::move(context)), p_interpreter(std::move(interpreter)),
              p_history_manager(std::move(history_manager)), p_logger(std::move(logger)),
              m_debugger_config(debugger_config), m_error_handler(eh) {
        init(sbuilder, dbuilder);
    }

    Kernel::~Kernel() {
    }

    void Kernel::init(server_builder sbuilder, debugger_builder dbuilder) {
        m_kernel_id = new_guid();
        m_session_id = new_guid();

        if (m_config.m_key.size() == 0) {
            m_config.m_key = new_guid();
        }

        if (p_logger == nullptr || std::getenv("DWARF_LOG") == nullptr) {
            p_logger = std::make_unique<LoggerNolog>();
        }

        p_server = sbuilder(*p_context, m_config, m_error_handler);
        p_server->update_config(m_config);

        p_debugger = dbuilder(*p_context, m_config, m_user_name, m_session_id, m_debugger_config);

        p_core = std::make_unique<KernelCore>(m_kernel_id,
                                              m_user_name,
                                              m_session_id,
                                              p_logger.get(),
                                              p_server.get(),
                                              p_interpreter.get(),
                                              p_history_manager.get(),
                                              p_debugger.get(),
                                              m_error_handler);

        ControlMessenger &messenger = p_server->get_control_messenger();

        if (p_debugger != nullptr) {
            p_debugger->register_control_messenger(messenger);
        }

        p_interpreter->register_control_messenger(messenger);
        p_interpreter->register_history_manager(*p_history_manager);
        p_interpreter->configure();
    }

    void Kernel::start() {
        PubMessage start_msg = p_core->build_start_msg();
        p_server->start(std::move(start_msg));
    }

    void Kernel::stop() {
        p_interpreter->shutdown_request();
        p_server->stop();
    }

    const Configuration &Kernel::get_config() {
        return m_config;
    }

    Server &Kernel::get_server() {
        return *p_server;
    }
}
