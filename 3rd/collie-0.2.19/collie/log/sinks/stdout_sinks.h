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

#include <cstdio>
#include <collie/log/details/console_globals.h>
#include <collie/log/details/synchronous_factory.h>
#include <collie/log/sinks/sink.h>

#ifdef _WIN32
#include <collie/log/details/windows_include.h>
#endif

namespace clog {

    namespace sinks {

        template<typename ConsoleMutex>
        class stdout_sink_base : public sink {
        public:
            using mutex_t = typename ConsoleMutex::mutex_t;

            explicit stdout_sink_base(FILE *file);

            ~stdout_sink_base() override = default;

            stdout_sink_base(const stdout_sink_base &other) = delete;

            stdout_sink_base(stdout_sink_base &&other) = delete;

            stdout_sink_base &operator=(const stdout_sink_base &other) = delete;

            stdout_sink_base &operator=(stdout_sink_base &&other) = delete;

            void log(const details::log_msg &msg) override;

            void flush() override;

            void set_pattern(const std::string &pattern) override;

            void set_formatter(std::unique_ptr<clog::formatter> sink_formatter) override;

        protected:
            mutex_t &mutex_;
            FILE *file_;
            std::unique_ptr<clog::formatter> formatter_;
#ifdef _WIN32
            HANDLE handle_;
#endif  // WIN32
        };

        template<typename ConsoleMutex>
        class stdout_sink : public stdout_sink_base<ConsoleMutex> {
        public:
            stdout_sink();
        };

        template<typename ConsoleMutex>
        class stderr_sink : public stdout_sink_base<ConsoleMutex> {
        public:
            stderr_sink();
        };

        using stdout_sink_mt = stdout_sink<details::console_mutex>;
        using stdout_sink_st = stdout_sink<details::console_nullmutex>;

        using stderr_sink_mt = stderr_sink<details::console_mutex>;
        using stderr_sink_st = stderr_sink<details::console_nullmutex>;

    }  // namespace sinks

// factory methods
    template<typename Factory = clog::synchronous_factory>
    std::shared_ptr<logger> stdout_logger_mt(const std::string &logger_name);

    template<typename Factory = clog::synchronous_factory>
    std::shared_ptr<logger> stdout_logger_st(const std::string &logger_name);

    template<typename Factory = clog::synchronous_factory>
    std::shared_ptr<logger> stderr_logger_mt(const std::string &logger_name);

    template<typename Factory = clog::synchronous_factory>
    std::shared_ptr<logger> stderr_logger_st(const std::string &logger_name);

}  // namespace clog

#include <collie/log/sinks/stdout_sinks-inl.h>