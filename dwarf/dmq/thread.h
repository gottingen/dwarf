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

#include <thread>
#include <type_traits>

namespace dwarf {

    /**
     * Joining std::thread
     */
    class ZmqThread {
    public:

        using id = std::thread::id;
        using native_handle_type = std::thread::native_handle_type;

        ZmqThread() noexcept = default;

        // Last arguments SFINAE out copy constructor
        template<class Function, class... Args,
                typename = std::enable_if_t<!std::is_same<std::decay_t<Function>, ZmqThread>::value>>
        explicit ZmqThread(Function &&f, Args &&... args);

        ~ZmqThread();

        ZmqThread(const ZmqThread &) = delete;

        ZmqThread &operator=(const ZmqThread &) = delete;

        ZmqThread(ZmqThread &&) = default;

        ZmqThread &operator=(ZmqThread &&);

        bool joinable() const noexcept;

        id get_id() const noexcept;

        native_handle_type native_handle();

        static unsigned int hardware_concurrency() noexcept;

        void join();

        void detach();

        void swap(ZmqThread &other) noexcept;

    private:

        std::thread m_thread;

    };

    /**************************
     * ZmqThread implementation *
     **************************/
    template<class Function, class... Args, typename>
    inline ZmqThread::ZmqThread(Function &&func, Args &&... args)
            : m_thread{
            std::forward<Function>(func),
            std::forward<Args>(args)...
    } {
    }

    inline ZmqThread::~ZmqThread() {
        if (joinable()) {
            join();
        }
    }

    inline ZmqThread &ZmqThread::operator=(ZmqThread &&rhs) {
        if (joinable()) {
            join();
        }
        m_thread = std::move(rhs.m_thread);
        return *this;
    }

    inline bool ZmqThread::joinable() const noexcept {
        return m_thread.joinable();
    }

    inline ZmqThread::id ZmqThread::get_id() const noexcept {
        return m_thread.get_id();
    }

    inline ZmqThread::native_handle_type ZmqThread::native_handle() {
        return m_thread.native_handle();
    }

    inline unsigned int ZmqThread::hardware_concurrency() noexcept {
        return std::thread::hardware_concurrency();
    }

    inline void ZmqThread::join() {
        m_thread.join();
    }

    inline void ZmqThread::detach() {
        m_thread.detach();
    }

    inline void ZmqThread::swap(ZmqThread &other) noexcept {
        m_thread.swap(other.m_thread);
    }
}
