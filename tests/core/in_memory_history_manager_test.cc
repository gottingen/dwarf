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


#include <collie/testing/doctest.h>

#include <vector>
#include <array>
#include <string>

#include <collie/nlohmann/json.hpp>

#include <dwarf/core/history_manager.h>

namespace nl = nlohmann;

namespace dwarf
{
    using history_manager_ptr = std::unique_ptr<HistoryManager>;
    using history_type = std::vector<std::array<std::string, 3>>;

    TEST_SUITE("InMemoryHistoryManager"){
    TEST_CASE("get_tail")
    {
        history_manager_ptr hist = dwarf::make_in_memory_history_manager();
        hist->store_inputs(0, 1, "print(3)");
        hist->store_inputs(0, 2, "a = 3");
        hist->store_inputs(0, 3, "print(a)");
        hist->store_inputs(0, 4, "a");

        nl::json tail1 = hist->get_tail(1000, true, false);
        REQUIRE_EQ(tail1["status"], "ok");
        auto history1 = tail1["history"].get<history_type>();
        REQUIRE_EQ(history1.size(), std::size_t(4));
        REQUIRE_EQ(history1[0][1], "1");
        REQUIRE_EQ(history1[0][2], "print(3)");
        REQUIRE_EQ(history1[1][1], "2");
        REQUIRE_EQ(history1[1][2], "a = 3");
        REQUIRE_EQ(history1[2][1], "3");
        REQUIRE_EQ(history1[2][2], "print(a)");
        REQUIRE_EQ(history1[3][1], "4");
        REQUIRE_EQ(history1[3][2], "a");

        nl::json tail2 = hist->get_tail(2, true, false);
        REQUIRE_EQ(tail2["status"], "ok");
        auto history2 = tail2["history"].get<history_type>();
        REQUIRE_EQ(history2.size(), std::size_t(2));
        REQUIRE_EQ(history2[0][1], "3");
        REQUIRE_EQ(history2[0][2], "print(a)");
        REQUIRE_EQ(history2[1][1], "4");
        REQUIRE_EQ(history2[1][2], "a");
    }

    TEST_CASE("get_range")
    {
        history_manager_ptr hist = dwarf::make_in_memory_history_manager();
        hist->store_inputs(0, 1, "print(3)");
        hist->store_inputs(0, 2, "a = 3");
        hist->store_inputs(0, 3, "print(a)");
        hist->store_inputs(0, 4, "a");

        nl::json range1 = hist->get_range(0, 1, 2, true, false);
        REQUIRE_EQ(range1["status"], "ok");
        auto history1 = range1["history"].get<history_type>();
        REQUIRE_EQ(history1.size(), std::size_t(1));
        REQUIRE_EQ(history1[0][1], "2");
        REQUIRE_EQ(history1[0][2], "a = 3");

        nl::json range2 = hist->get_range(0, 1, 3, true, false);
        REQUIRE_EQ(range2["status"], "ok");
        auto history2 = range2["history"].get<history_type>();
        REQUIRE_EQ(history2.size(), std::size_t(2));
        REQUIRE_EQ(history2[0][1], "2");
        REQUIRE_EQ(history2[0][2], "a = 3");
        REQUIRE_EQ(history2[1][1], "3");
        REQUIRE_EQ(history2[1][2], "print(a)");

        nl::json range3 = hist->get_range(0, 1000, 2, true, false);
        REQUIRE_EQ(range3["status"], "error");
    }

    TEST_CASE("search")
    {
        history_manager_ptr hist = dwarf::make_in_memory_history_manager();
        hist->store_inputs(0, 1, "print(36)");
        hist->store_inputs(0, 2, "a = 3");
        hist->store_inputs(0, 3, "print(a)");
        hist->store_inputs(0, 4, "a");

        nl::json search1 = hist->search("print*", true, false, 10, false);
        REQUIRE_EQ(search1["status"], "ok");
        auto history1 = search1["history"].get<history_type>();
        REQUIRE_EQ(history1.size(), std::size_t(2));
        REQUIRE_EQ(history1[0][1], "1");
        REQUIRE_EQ(history1[0][2], "print(36)");
        REQUIRE_EQ(history1[1][1], "3");
        REQUIRE_EQ(history1[1][2], "print(a)");

        nl::json search2 = hist->search("print(*)", true, false, 10, false);
        REQUIRE_EQ(search2["status"], "ok");
        auto history2 = search2["history"].get<history_type>();
        REQUIRE_EQ(history2.size(), std::size_t(2));
        REQUIRE_EQ(history2[0][1], "1");
        REQUIRE_EQ(history2[0][2], "print(36)");
        REQUIRE_EQ(history2[1][1], "3");
        REQUIRE_EQ(history2[1][2], "print(a)");

        nl::json search3 = hist->search("print(?)", true, false, 10, false);
        REQUIRE_EQ(search3["status"], "ok");
        auto history3 = search3["history"].get<history_type>();
        REQUIRE_EQ(history3.size(), std::size_t(1));
        REQUIRE_EQ(history3[0][1], "3");
        REQUIRE_EQ(history3[0][2], "print(a)");

        nl::json search4 = hist->search("print*", true, false, 1, false);
        REQUIRE_EQ(search4["status"], "ok");
        auto history4 = search4["history"].get<history_type>();
        REQUIRE_EQ(history4.size(), std::size_t(1));
        REQUIRE_EQ(history4[0][1], "3");
        REQUIRE_EQ(history4[0][2], "print(a)");

        hist->store_inputs(0, 3, "print(a)");
        nl::json search5 = hist->search("print*", true, false, 10, false);
        REQUIRE_EQ(search5["status"], "ok");
        auto history5 = search5["history"].get<history_type>();
        REQUIRE_EQ(history5.size(), std::size_t(3));
        REQUIRE_EQ(history2[0][1], "1");
        REQUIRE_EQ(history2[0][2], "print(36)");
        REQUIRE_EQ(history5[1][1], "3");
        REQUIRE_EQ(history5[1][2], "print(a)");
        REQUIRE_EQ(history5[2][1], "3");
        REQUIRE_EQ(history5[2][2], "print(a)");

        nl::json search6 = hist->search("print*", true, false, 10, true);
        REQUIRE_EQ(search6["status"], "ok");
        auto history6 = search6["history"].get<history_type>();
        REQUIRE_EQ(history2[0][1], "1");
        REQUIRE_EQ(history2[0][2], "print(36)");
        REQUIRE_EQ(history5[1][1], "3");
        REQUIRE_EQ(history5[1][2], "print(a)");
    }
    }
}
