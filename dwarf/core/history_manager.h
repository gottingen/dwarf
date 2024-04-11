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

#include <string>
#include <vector>

#include <collie/nlohmann/json.hpp>

#include <dwarf/core/config.h>

namespace nl = nlohmann;

namespace dwarf {
    class DWARF_API HistoryManager {
    public:

        HistoryManager();

        virtual ~HistoryManager() = default;

        HistoryManager(const HistoryManager &) = delete;

        HistoryManager &operator=(const HistoryManager &) = delete;

        HistoryManager(HistoryManager &&) = delete;

        HistoryManager &operator=(HistoryManager &&) = delete;

        void configure();

        void store_inputs(int session,
                          int line_num,
                          const std::string &input,
                          const std::string &output = "");

        nl::json process_request(const nl::json &content) const;

        nl::json get_tail(int n, bool raw, bool output) const;

        nl::json get_range(int session, int start, int stop, bool raw, bool output) const;

        nl::json search(const std::string &pattern, bool raw, bool output, int n, bool unique) const;

    private:

        virtual void configure_impl() = 0;

        virtual void store_inputs_impl(int session,
                                       int line_num,
                                       const std::string &input,
                                       const std::string &output) = 0;

        virtual nl::json get_tail_impl(int n, bool raw, bool output) const = 0;

        virtual nl::json get_range_impl(int session, int start, int stop, bool raw, bool output) const = 0;

        virtual nl::json search_impl(const std::string &pattern, bool raw, bool output, int n, bool unique) const = 0;
    };

    DWARF_API
    std::unique_ptr<HistoryManager> make_in_memory_history_manager();
}
