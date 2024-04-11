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


#include <collie/simd/simd.h>
#ifndef COLLIE_SIMD_NO_SUPPORTED_ARCHITECTURE

#include "test_utils.hpp"

template <class B>
struct poly_evaluation_test
{
    using batch_type = B;
    using value_type = typename B::value_type;
    static constexpr size_t size = B::size;
    using vector_type = std::vector<value_type>;

    size_t nb_input;
    vector_type input;
    vector_type horner_res;
    vector_type estrin_res;

    poly_evaluation_test()
    {
        nb_input = size * 10000;
        input.resize(nb_input);
        for (size_t i = 0; i < nb_input; ++i)
        {
            input[i] = value_type(i) / 4 + value_type(1.2) * std::sqrt(value_type(i + 0.25));
        }
        horner_res.resize(nb_input);
        estrin_res.resize(nb_input);
    }

    void test_poly_evaluation()
    {
        batch_type in, out;
        for (size_t i = 0; i < nb_input; i += size)
        {
            detail::load_batch(in, input, i);
            out = collie::simd::kernel::horner<typename batch_type::value_type, typename batch_type::arch_type, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16>(in);
            detail::store_batch(out, horner_res, i);
            out = collie::simd::kernel::estrin<typename batch_type::value_type, typename batch_type::arch_type, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16>(in);
            detail::store_batch(out, estrin_res, i);
        }
        size_t diff = detail::get_nb_diff(horner_res, estrin_res);
        CHECK_EQ(diff, 0);
    }
};

TEST_CASE_TEMPLATE("[poly evaluation]", B, BATCH_FLOAT_TYPES)
{

    poly_evaluation_test<B> Test;
    Test.test_poly_evaluation();
}
#endif
