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

#include <dwarf/core/fixed_string.h>
#include <dwarf/core/config.h>

namespace dwarf {
    // FixedString contains 2 members:
    // - a stack-allocated buffer of N (template parameter) + 1 (null termination) char
    // - a size_t for the number of characters actually used
    // So its memory footprint is N + 9 bytes. In order to not waste memory
    // due to padding, it is good to choose N so the size of the structure is a
    // multiple of the stackframe alignment (8 or 16 depending on the architecture
    // and compilers).
    // Here we want a size of 64 bytes, which gives room for 55 characters (64 - 8 - 1).
    using Guid = FixedString<55>;

    DWARF_API Guid new_guid();
}  // namespace dwarf
