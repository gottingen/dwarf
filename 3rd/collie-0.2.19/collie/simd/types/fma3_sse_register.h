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

#ifndef COLLIE_SIMD_TYPES_FMA3_SSE_REGISTER_H_
#define COLLIE_SIMD_TYPES_FMA3_SSE_REGISTER_H_

#include <collie/simd/types/sse4_2_register.h>

namespace collie::simd {
    template<typename arch>
    struct fma3;

    /**
     * @ingroup architectures
     *
     * SSE4.2 + FMA instructions
     */
    template<>
    struct fma3<sse4_2> : sse4_2 {
        static constexpr bool supported() noexcept { return COLLIE_SIMD_WITH_FMA3_SSE; }

        static constexpr bool available() noexcept { return true; }

        static constexpr unsigned version() noexcept { return generic::version(1, 4, 3); }

        static constexpr char const *name() noexcept { return "fma3+sse4.2"; }
    };

#if COLLIE_SIMD_WITH_FMA3_SSE
    namespace types {

    COLLIE_SIMD_DECLARE_SIMD_REGISTER_ALIAS(fma3 < sse4_2 >, sse4_2);

}
#endif

}
#endif  // COLLIE_SIMD_TYPES_FMA3_SSE_REGISTER_H_