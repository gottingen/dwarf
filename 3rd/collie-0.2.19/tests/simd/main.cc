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

#ifndef EMSCRIPTEN
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <collie/testing/doctest.h>
#else

#define DOCTEST_CONFIG_IMPLEMENT
#include <collie/testing/doctest.h>
#include <emscripten/bind.h>

int run_tests()
{
    doctest::Context context;
    return context.run();
}

EMSCRIPTEN_BINDINGS(my_module)
{
    emscripten::function("run_tests", &run_tests);
}

#endif