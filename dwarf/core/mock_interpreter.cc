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


#include <dwarf/core/mock_interpreter.h>

namespace dwarf
{
    Interpreter* get_mock_interpreter()
    {
        static MockInterpreter interpreter;
        return &interpreter;
    }

    Interpreter*& get_registered_interpreter()
    {
        static Interpreter* interpreter = nullptr;
        return interpreter;
    }

    bool register_interpreter(Interpreter* interpreter)
    {
        Interpreter*& interp = get_registered_interpreter();
        if (interp != nullptr)
        {
            return false;
        }
        else
        {
            interp = interpreter;
            return true;
        }
    }

    Interpreter& get_interpreter()
    {
        Interpreter* interp = get_registered_interpreter();
        if (interp != nullptr)
            return *interp;
        else
            return *get_mock_interpreter();
    }

    MockInterpreter::MockInterpreter()
        : base_type()
        , m_comm_manager(nullptr)
    {
        base_type::register_publisher(base_type::publisher_type());
        base_type::register_stdin_sender(base_type::stdin_sender_type());
        base_type::register_comm_manager(&m_comm_manager);
    }
}
