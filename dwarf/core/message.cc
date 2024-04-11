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


#include <chrono>
#include <cstddef>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <string>
#include <utility>

#include <collie/nlohmann/json.hpp>

#include <dwarf/core/guid.h>
#include <dwarf/core/message.h>

namespace nl = nlohmann;

namespace dwarf
{
    MessageBase::MessageBase(
        nl::json header, nl::json parent_header, nl::json metadata, nl::json content, buffer_sequence buffers)
        : m_header(std::move(header))
        , m_parent_header(std::move(parent_header))
        , m_metadata(std::move(metadata))
        , m_content(std::move(content))
        , m_buffers(std::move(buffers))
    {
    }

    const nl::json& MessageBase::header() const
    {
        return m_header;
    }

    const nl::json& MessageBase::parent_header() const
    {
        return m_parent_header;
    }

    const nl::json& MessageBase::metadata() const
    {
        return m_metadata;
    }

    const nl::json& MessageBase::content() const
    {
        return m_content;
    }

    const buffer_sequence& MessageBase::buffers() const &
    {
        return m_buffers;
    }
    
    buffer_sequence&& MessageBase::buffers() &&
    {
        return std::move(m_buffers);
    }
    
    Message::Message(const guid_list& zmq_id,
                       nl::json header,
                       nl::json parent_header,
                       nl::json metadata,
                       nl::json content,
                       buffer_sequence buffers)
        : MessageBase(std::move(header),
                        std::move(parent_header),
                        std::move(metadata),
                        std::move(content),
                        std::move(buffers))
        , m_zmq_id(zmq_id)
    {
    }

    Message::Message(const guid_list& zmq_id,
                       MessageBaseData&& data)
        : MessageBase(std::move(data.m_header),
                        std::move(data.m_parent_header),
                        std::move(data.m_metadata),
                        std::move(data.m_content),
                        std::move(data.m_buffers))
        , m_zmq_id(zmq_id)
    {
    }

    auto Message::identities() const -> const guid_list&
    {
        return m_zmq_id;
    }

    PubMessage::PubMessage(const std::string& topic,
                               nl::json header,
                               nl::json parent_header,
                               nl::json metadata,
                               nl::json content,
                               buffer_sequence buffers)
        : MessageBase(std::move(header),
                        std::move(parent_header),
                        std::move(metadata),
                        std::move(content),
                        std::move(buffers))
        , m_topic(topic)
    {
    }

    PubMessage::PubMessage(const std::string& topic,
                               MessageBaseData&& data)
        : MessageBase(std::move(data.m_header),
                        std::move(data.m_parent_header),
                        std::move(data.m_metadata),
                        std::move(data.m_content),
                        std::move(data.m_buffers))
        , m_topic(topic)
    {
    }

    const std::string& PubMessage::topic() const
    {
        return m_topic;
    }

    std::string iso8601_now()
    {
        std::ostringstream ss;

        // now
        auto now = std::chrono::system_clock::now();

        // down to seconds
        auto itt = std::chrono::system_clock::to_time_t(now);
        ss << std::put_time(std::gmtime(&itt), "%FT%T");

        // down to microseconds
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
        auto fractionals = micros.count() % 1000000;
        ss << "." << fractionals << "Z";

        return ss.str();
    }

    std::string get_protocol_version()
    {
        return DWARF_KERNEL_PROTOCOL_VERSION;
    }

    nl::json make_header(const std::string& msg_type,
                      const std::string& user_name,
                      const std::string& session_id)
    {
        nl::json header;
        header["msg_id"] = new_guid();
        header["username"] = user_name;
        header["session"] = session_id;
        header["date"] = iso8601_now();
        header["msg_type"] = msg_type;
        header["version"] = get_protocol_version();
        return header;
    }
}
