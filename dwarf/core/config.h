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

#ifdef _WIN32
    #ifdef DWARF_STATIC_LIB
        #define DWARF_API
    #else
        #ifdef DWARF_EXPORTS
            #define DWARF_API __declspec(dllexport)
        #else
            #define DWARF_API __declspec(dllimport)
        #endif
    #endif
#else
    #define DWARF_API
#endif

// Project version
#define DWARF_VERSION_MAJOR 3
#define DWARF_VERSION_MINOR 0
#define DWARF_VERSION_PATCH 5

// Binary version
#define DWARF_BINARY_CURRENT 8
#define DWARF_BINARY_REVISION 1
#define DWARF_BINARY_AGE 0

// Kernel protocol version
#define DWARF_KERNEL_PROTOCOL_VERSION_MAJOR 5
#define DWARF_KERNEL_PROTOCOL_VERSION_MINOR 3

// Composing the protocol version string from major, and minor
#define DWARF_CONCATENATE(A, B) DWARF_CONCATENATE_IMPL(A, B)
#define DWARF_CONCATENATE_IMPL(A, B) A##B
#define DWARF_STRINGIFY(a) DWARF_STRINGIFY_IMPL(a)
#define DWARF_STRINGIFY_IMPL(a) #a

#define DWARF_KERNEL_PROTOCOL_VERSION DWARF_STRINGIFY(DWARF_CONCATENATE(    DWARF_KERNEL_PROTOCOL_VERSION_MAJOR,\
                                                    DWARF_CONCATENATE(.,  DWARF_KERNEL_PROTOCOL_VERSION_MINOR)))

#ifndef __has_feature
#define __has_feature(x) 0
#endif

// Attempt to discover whether we're being compiled with exception support
#if (defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)) && !defined(DWARF_NO_EXCEPTIONS)
// Exceptions are enabled.
#else
// Exceptions are disabled.
#define DWARF_NO_EXCEPTIONS
#endif

#if defined(DWARF_NO_EXCEPTIONS)
#define DWARF_THROW(_, msg)              \
    {                                  \
        std::cerr << msg << std::endl; \
        std::abort();                  \
    }
#else
#define DWARF_THROW(exception, msg) throw exception(msg)
#endif