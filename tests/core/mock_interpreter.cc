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

#include <collie/nlohmann/json.hpp>

#include "mock_interpreter.h"

#include <dwarf/core/helper.h>
#include <dwarf/core/guid.h>

namespace nl = nlohmann;

namespace dwarf
{

    void MockInterpreter::configure_impl()
    {
        auto handle_comm_opened = [](dwarf::Comm&& comm, const dwarf::Message&) {
            std::cout << "Comm opened for target: " << comm.target().name() << std::endl;
        };
        comm_manager().register_comm_target("test_target", handle_comm_opened);
        using function_type = std::function<void(dwarf::Comm&&, const dwarf::Message&)>;
    }

    nl::json MockInterpreter::execute_request_impl(int execution_counter,
                                                     const std::string& code,
                                                     bool /* silent */,
                                                     bool /* store_history */,
                                                     nl::json /* user_expressions */,
                                                     bool /* allow_stdin */)
    {
        if (code.compare("hello, world") == 0)
        {
            publish_stream("stdout", code);
        }

        if (code.compare("error") == 0)
        {
            publish_stream("stderr", code);
        }

        if (code.compare("?") == 0)
        {
            std::string html_content = R"(<iframe class="xpyt-iframe-pager" src="
                                            https://hercules-docs.readthedocs.io"></iframe>)";

            auto payload = nl::json::array();
            payload[0] = nl::json::object({
                            {"data", {
                                {"text/plain", "https://hercules-docs.readthedocs.io"},
                                {"text/html", html_content}}
                            },
                            {"source", "page"},
                            {"start", 0}
                        });

            return dwarf::create_successful_reply(payload);
        }

        nl::json pub_data;
        pub_data["text/plain"] = code;
        publish_execution_result(execution_counter, std::move(pub_data), nl::json::object());

        return dwarf::create_successful_reply();
    }

    nl::json MockInterpreter::complete_request_impl(const std::string& /* code */,
                                                     int /* cursor_pos */)
    {
        return dwarf::create_complete_reply({"a.test1", "a.test2"}, 2, 6);
    }

    nl::json MockInterpreter::inspect_request_impl(const std::string& /* code */,
                                                    int /* cursor_pos */,
                                                    int /* detail_level */)
    {
        return dwarf::create_inspect_reply(true, {{"text/plain", ""}}, {{"text/plain", ""}});
    }

    nl::json MockInterpreter::is_complete_request_impl(const std::string& code)
    {
        nl::json result = dwarf::create_is_complete_reply(code);
        if (code.compare("incomplete") == 0)
        {
            result["indent"] = "   ";
        }
        return result;
    }

    nl::json MockInterpreter::kernel_info_request_impl()
    {
        return dwarf::create_info_reply("",
                                       "cpp_test",
                                       "1.0.0",
                                       "cpp",
                                       "14.0.0",
                                       "text/x-c++src",
                                       ".cpp",
                                       "",
                                       "",
                                       "",
                                       "test_kernel");
    }

    void MockInterpreter::shutdown_request_impl()
    {
    }
}
