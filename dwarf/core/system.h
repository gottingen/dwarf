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

#include <dwarf/core/config.h>

namespace dwarf {
    DWARF_API
    std::string get_temp_directory_path();

    DWARF_API
    bool create_directory(const std::string &path);

    DWARF_API
    int get_current_pid();

    DWARF_API [[deprecated]]
    std::string get_cell_tmp_file(const std::string &prefix,
                                  int execution_count,
                                  const std::string &extension);

    DWARF_API
    std::size_t get_tmp_hash_seed();

    DWARF_API
    std::string get_tmp_prefix(const std::string &process_name);

    DWARF_API
    std::string get_cell_tmp_file(const std::string &prefix,
                                  const std::string &code,
                                  const std::string &suffix);
}

