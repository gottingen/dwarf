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
#include <vector>

#include <collie/nlohmann/json.hpp>
#include <dwarf/core/config.h>

namespace nl = nlohmann;

namespace dwarf
{
    using binary_buffer = std::vector<char>;
    using buffer_sequence = std::vector<binary_buffer>;

    struct DWARF_API MessageBaseData
    {
        nl::json m_header;
        nl::json m_parent_header;
        nl::json m_metadata;
        nl::json m_content;
        buffer_sequence m_buffers;
    };

    class DWARF_API MessageBase
    {
    public:

        MessageBase(const MessageBase&) = delete;
        MessageBase& operator=(const MessageBase&) = delete;

        const nl::json& header() const;
        const nl::json& parent_header() const;
        const nl::json& metadata() const;
        const nl::json& content() const;

        const buffer_sequence& buffers() const&;
        buffer_sequence&& buffers() &&;

    protected:

        MessageBase() = default;
        MessageBase(nl::json header,
                      nl::json parent_header,
                      nl::json metadata,
                      nl::json content,
                      buffer_sequence buffers);
        MessageBase(MessageBaseData&& data);
        ~MessageBase() = default;

        MessageBase(MessageBase&&) = default;
        MessageBase& operator=(MessageBase&&) = default;

    private:

        nl::json m_header;
        nl::json m_parent_header;
        nl::json m_metadata;
        nl::json m_content;
        buffer_sequence m_buffers;
    };

    class DWARF_API Message : public MessageBase
    {
    public:

        using base_type = MessageBase;
        using guid_list = std::vector<std::string>;

        Message() = default;
        Message(const guid_list& zmq_id,
                 nl::json header,
                 nl::json parent_header,
                 nl::json metadata,
                 nl::json content,
                 buffer_sequence buffers);
        Message(const guid_list& zmq_id,
                 MessageBaseData&& data);

        ~Message() = default;

        Message(Message&&) = default;
        Message& operator=(Message&&) = default;

        Message(const Message&) = delete;
        Message& operator=(const Message&) = delete;

        const guid_list& identities() const;

    private:

        guid_list m_zmq_id;
    };

    class DWARF_API PubMessage : public MessageBase
    {
    public:

        using base_type = MessageBase;

        PubMessage() = default;
        PubMessage(const std::string& topic,
                     nl::json header,
                     nl::json parent_header,
                     nl::json metadata,
                     nl::json content,
                     buffer_sequence buffers);
        PubMessage(const std::string& topic,
                     MessageBaseData&& data);

        ~PubMessage() = default;

        PubMessage(PubMessage&&) = default;
        PubMessage& operator=(PubMessage&&) = default;

        PubMessage(const PubMessage&) = delete;
        PubMessage& operator=(const PubMessage&) = delete;

        const std::string& topic() const;

    private:

        std::string m_topic;
    };

    DWARF_API std::string iso8601_now();

    DWARF_API std::string get_protocol_version();

    DWARF_API nl::json make_header(const std::string& msg_type,
                                  const std::string& user_name,
                                  const std::string& session_id);
}
