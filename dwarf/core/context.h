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

#include <memory>

#include <dwarf/core/config.h>

namespace dwarf {

    struct EmptyContextTag {
    };

    template<class T>
    class ContextImpl;

    class DWARF_API Context {
    public:

        virtual ~Context() = default;

        Context(const Context &) = delete;

        Context &operator=(const Context &) = delete;

        Context(Context &&) = delete;

        Context &operator=(Context &&) = delete;

        template<class T>
        T &get_wrapped_context();

    protected:

        Context() = default;
    };

    template<class T>
    class ContextImpl : public Context {

    public:

        template<class... U>
        ContextImpl(U &&... u)
                : m_context(std::forward<U>(u)...) {
        }

        virtual ~ContextImpl() = default;

        T m_context;
    };

    template<class T>
    T &Context::get_wrapped_context() {
        auto *impl = dynamic_cast<ContextImpl<T> *>(this);
        return impl->m_context;
    }

    template<class T, class... U>
    std::unique_ptr<ContextImpl<T>> make_context(U &&... u) {
        return std::unique_ptr<ContextImpl<T>>(new ContextImpl<T>(std::forward<U>(u)...));
    }

    inline std::unique_ptr<ContextImpl<EmptyContextTag>> make_empty_context() {
        return std::unique_ptr<ContextImpl<EmptyContextTag>>(
                new ContextImpl<EmptyContextTag>()
        );
    }
}
