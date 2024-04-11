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

#include <list>
#include <array>
#include <string>

#include <collie/nlohmann/json.hpp>

#include <dwarf/core/config.h>
#include <dwarf/core/history_manager.h>

namespace nl = nlohmann;

namespace dwarf {
    class InMemoryHistoryManager : public HistoryManager {
    public:

        using entry = std::array<std::string, 4>;
        using history_type = std::list<entry>;
        using short_entry = std::array<std::string, 3>;
        using short_history_type = std::list<short_entry>;

        InMemoryHistoryManager();

        virtual ~InMemoryHistoryManager();

    private:

        void configure_impl() override;

        void store_inputs_impl(int session,
                               int line_num,
                               const std::string &input,
                               const std::string &output) override;

        nl::json get_tail_impl(int n, bool raw, bool output) const override;

        nl::json get_range_impl(int session, int start, int stop, bool raw, bool output) const override;

        nl::json search_impl(const std::string &pattern, bool raw, bool output, int n, bool unique) const override;

        history_type m_history;
    };
}
