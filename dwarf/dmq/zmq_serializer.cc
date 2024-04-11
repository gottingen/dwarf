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


#include <dwarf/dmq/zmq_serializer.h>

namespace dwarf {
    namespace {
        const std::string DELIMITER = "<IDS|MSG>";

        bool is_delimiter(zmq::message_t &frame) {
            std::size_t frame_size = frame.size();
            if (frame_size != DELIMITER.size()) {
                return false;
            }

            std::string check(frame.data<const char>(), frame_size);
            return check == DELIMITER;
        }

        RawBuffer make_raw_buffer(zmq::message_t &msg) {
            return RawBuffer(msg.data<const unsigned char>(), msg.size());
        }

        void parse_zmq_message(const zmq::message_t &msg, nl::json &json) {
            const char *buf = msg.data<const char>();
            json = nl::json::parse(buf, buf + msg.size());
        }

        zmq::message_t write_zmq_message(const nl::json &json, nl::json::error_handler_t error_handler) {
            std::string buffer = json.dump(-1, ' ', false, error_handler);
            return zmq::message_t(buffer.c_str(), buffer.size());
        }

        void serialize_message_base(MessageBase &&msg,
                                    const Authentication &auth,
                                    nl::json::error_handler_t error_handler,
                                    zmq::multipart_t &wire_msg) {
            zmq::message_t header = write_zmq_message(msg.header(), error_handler);
            zmq::message_t parent_header = write_zmq_message(msg.parent_header(), error_handler);
            zmq::message_t metadata = write_zmq_message(msg.metadata(), error_handler);
            zmq::message_t content = write_zmq_message(msg.content(), error_handler);
            std::string sig = auth.sign(make_raw_buffer(header),
                                        make_raw_buffer(parent_header),
                                        make_raw_buffer(metadata),
                                        make_raw_buffer(content));
            zmq::message_t signature(sig.begin(), sig.end());

            wire_msg.add(std::move(signature));
            wire_msg.add(std::move(header));
            wire_msg.add(std::move(parent_header));
            wire_msg.add(std::move(metadata));
            wire_msg.add(std::move(content));

            // was not const and  could only be called on rvalues.
            for (const binary_buffer &buffer: std::move(msg).buffers()) {
                wire_msg.add(zmq::message_t(buffer.data(), buffer.size()));
            }
        }

        MessageBaseData deserialize_message_base(zmq::multipart_t &wire_msg,
                                                 const Authentication &auth) {
            zmq::message_t signature = wire_msg.pop();
            zmq::message_t header = wire_msg.pop();
            zmq::message_t parent_header = wire_msg.pop();
            zmq::message_t metadata = wire_msg.pop();
            zmq::message_t content = wire_msg.pop();

            MessageBaseData data;
            parse_zmq_message(header, data.m_header);
            parse_zmq_message(parent_header, data.m_parent_header);
            parse_zmq_message(metadata, data.m_metadata);
            parse_zmq_message(content, data.m_content);

            while (!wire_msg.empty()) {
                zmq::message_t msg = wire_msg.pop();
                const char *buf = msg.data<const char>();
                data.m_buffers.emplace_back(buf, buf + msg.size());
            }

            // TODO: should we verify with buffers
            if (!auth.verify(make_raw_buffer(signature),
                             make_raw_buffer(header),
                             make_raw_buffer(parent_header),
                             make_raw_buffer(metadata),
                             make_raw_buffer(content))) {
                throw std::runtime_error("ERROR: Signatures don't match");
            }

            return data;
        }

        void serialize_zmq_id(const Message &msg, zmq::multipart_t &wire_msg) {
            auto app = [&wire_msg](const std::string &uid) {
                wire_msg.add(zmq::message_t(uid.begin(), uid.end()));
            };
            std::for_each(msg.identities().begin(), msg.identities().end(), app);
            wire_msg.add(zmq::message_t(DELIMITER.begin(), DELIMITER.end()));
        }

        Message::guid_list deserialize_zmq_id(zmq::multipart_t &wire_msg) {
            Message::guid_list zmq_id;
            zmq::message_t frame = wire_msg.pop();

            // ZMQ identites
            while (!is_delimiter(frame) && wire_msg.size() != 0) {
                zmq_id.emplace_back(frame.data<const char>(), frame.size());
                frame = wire_msg.pop();
            }

            // if wire_msg is empty, that means frame doesn't contain <IDS|MSG>
            if (wire_msg.size() == 0) {
                throw std::runtime_error("ERROR: Delimiter not present in message");
            }
            return zmq_id;
        }


        void serialize_topic(const PubMessage &msg, zmq::multipart_t &wire_msg) {
            wire_msg.add(zmq::message_t(msg.topic().begin(), msg.topic().end()));
            wire_msg.add(zmq::message_t(DELIMITER.begin(), DELIMITER.end()));
        }

        std::string deserialize_topic(zmq::multipart_t &wire_msg) {
            zmq::message_t topic_msg = wire_msg.pop();
            std::string topic = std::string(topic_msg.data<const char>(), topic_msg.size());
            wire_msg.pop();
            return topic;
        }
    }

    zmq::multipart_t xzmq_serializer::serialize(Message &&msg,
                                                const Authentication &auth,
                                                nl::json::error_handler_t error_handler) {
        zmq::multipart_t wire_msg;
        serialize_zmq_id(msg, wire_msg);
        serialize_message_base(std::move(msg), auth, error_handler, wire_msg);
        return wire_msg;
    }

    Message xzmq_serializer::deserialize(zmq::multipart_t &wire_msg,
                                         const Authentication &auth) {
        Message::guid_list zmq_id = deserialize_zmq_id(wire_msg);
        MessageBaseData data = deserialize_message_base(wire_msg, auth);
        return Message(zmq_id, std::move(data));
    }

    zmq::multipart_t xzmq_serializer::serialize_iopub(PubMessage &&msg,
                                                      const Authentication &auth,
                                                      nl::json::error_handler_t error_handler) {
        zmq::multipart_t wire_msg;
        serialize_topic(msg, wire_msg);
        serialize_message_base(std::move(msg), auth, error_handler, wire_msg);
        return wire_msg;
    }

    PubMessage xzmq_serializer::deserialize_iopub(zmq::multipart_t &wire_msg,
                                                  const Authentication &auth) {
        std::string topic = deserialize_topic(wire_msg);
        MessageBaseData data = deserialize_message_base(wire_msg, auth);
        return PubMessage(topic, std::move(data));
    }
}

