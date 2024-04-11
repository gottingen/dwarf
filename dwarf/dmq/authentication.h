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
#include <string>

#include <dwarf/core/config.h>

namespace dwarf {
    class DWARF_API RawBuffer {
    public:

        RawBuffer(const unsigned char *data,
                    size_t size);

        const unsigned char *data() const;

        size_t size() const;

    private:

        const unsigned char *m_data;
        size_t m_size;
    };

    class DWARF_API Authentication {
    public:

        virtual ~Authentication() = default;

        Authentication(const Authentication &) = delete;

        Authentication &operator=(const Authentication &) = delete;

        Authentication(Authentication &&) = delete;

        Authentication &operator=(Authentication &&) = delete;

        std::string sign(const RawBuffer &header,
                         const RawBuffer &parent_header,
                         const RawBuffer &meta_data,
                         const RawBuffer &content) const;

        bool verify(const RawBuffer &signature,
                    const RawBuffer &header,
                    const RawBuffer &parent_header,
                    const RawBuffer &meta_data,
                    const RawBuffer &content) const;

    protected:

        Authentication() = default;

    private:

        virtual std::string sign_impl(const RawBuffer &header,
                                      const RawBuffer &parent_header,
                                      const RawBuffer &meta_data,
                                      const RawBuffer &content) const = 0;

        virtual bool verify_impl(const RawBuffer &signature,
                                 const RawBuffer &header,
                                 const RawBuffer &parent_header,
                                 const RawBuffer &meta_data,
                                 const RawBuffer &content) const = 0;
    };

    DWARF_API
    std::unique_ptr<Authentication> make_authentication(const std::string &scheme,
                                                          const std::string &key);
}