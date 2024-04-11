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


#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <stdexcept>
#include <vector>

#include <openssl/hmac.h>
#include <openssl/sha.h>

#include <dwarf/core/string_utils.h>
#include <dwarf/dmq/authentication.h>


namespace dwarf
{
    // RawBuffer implementation
    RawBuffer::RawBuffer(const unsigned char* data, size_t size)
        : m_data(data), m_size(size)
    {
    }

    const unsigned char* RawBuffer::data() const
    {
        return m_data;
    }

    size_t RawBuffer::size() const
    {
        return m_size;
    }

    // Specialization of Authentication using OpenSSL.
    class openssl_xauthentication : public Authentication
    {
    public:

        openssl_xauthentication(const std::string& scheme,
                                const std::string& key);
        virtual ~openssl_xauthentication();

    private:

        std::string sign_impl(const RawBuffer& header,
                              const RawBuffer& parent_header,
                              const RawBuffer& meta_data,
                              const RawBuffer& content) const override;

        bool verify_impl(const RawBuffer& signature,
                         const RawBuffer& header,
                         const RawBuffer& parent_header,
                         const RawBuffer& meta_data,
                         const RawBuffer& content) const override;

        const EVP_MD* m_evp;
        std::string m_key;
        HMAC_CTX* m_hmac;
        mutable std::mutex m_hmac_mutex;
    };

    // Specialization of Authentication without any signature checking.
    class no_xauthentication : public Authentication
    {
    public:

        no_xauthentication() = default;
        virtual ~no_xauthentication() = default;

    private:

        std::string sign_impl(const RawBuffer& header,
                              const RawBuffer& parent_header,
                              const RawBuffer& meta_data,
                              const RawBuffer& content) const override;

        bool verify_impl(const RawBuffer& signature,
                         const RawBuffer& header,
                         const RawBuffer& parent_header,
                         const RawBuffer& meta_data,
                         const RawBuffer& content) const override;
    };

    std::string Authentication::sign(const RawBuffer& header,
                                      const RawBuffer& parent_header,
                                      const RawBuffer& meta_data,
                                      const RawBuffer& content) const
    {
        return sign_impl(header, parent_header, meta_data, content);
    }

    bool Authentication::verify(const RawBuffer& signature,
                                 const RawBuffer& header,
                                 const RawBuffer& parent_header,
                                 const RawBuffer& meta_data,
                                 const RawBuffer& content) const
    {
        return verify_impl(signature, header, parent_header, meta_data, content);
    }

    std::unique_ptr<Authentication> make_authentication(const std::string& scheme,
                                                          const std::string& key)
    {
        if (scheme == "none")
        {
            return std::make_unique<no_xauthentication>();
        }
        else
        {
            return std::make_unique<openssl_xauthentication>(scheme, key);
        }
    }

    inline const EVP_MD* asevp(const std::string& scheme)
    {
        static const std::map<std::string, const EVP_MD*(*)()> schemes = {
            {"hmac-md5", EVP_md5},
            {"hmac-sha1", EVP_sha1},
            // MDC2 is disabled by default unless enable-mdc2 is specified
            // {"hmac-mdc2", EVP_mdc2},
            {"hmac-ripemd160", EVP_ripemd160},
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
            {"hmac-blake2b512", EVP_blake2b512},
            {"hmac-blake2s256", EVP_blake2s256},
#endif
            {"hmac-sha224", EVP_sha224},
            {"hmac-sha256", EVP_sha256},
            {"hmac-sha384", EVP_sha384},
            {"hmac-sha512", EVP_sha512}
        };
        return schemes.at(scheme)();
    }

    openssl_xauthentication::openssl_xauthentication(const std::string& scheme, const std::string& key)
        : m_evp(asevp(scheme)), m_key(key)
    {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        // OpenSSL 1.0.x
        m_hmac = new HMAC_CTX();
        HMAC_CTX_init(m_hmac);
#else
        m_hmac = HMAC_CTX_new();
#endif
    }

    openssl_xauthentication::~openssl_xauthentication()
    {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        // OpenSSL 1.0.x
        HMAC_CTX_cleanup(m_hmac);
#else
        HMAC_CTX_free(m_hmac);
#endif
    }

    std::string openssl_xauthentication::sign_impl(const RawBuffer& header,
                                                   const RawBuffer& parent_header,
                                                   const RawBuffer& meta_data,
                                                   const RawBuffer& content) const
    {
        std::lock_guard<std::mutex> lock(m_hmac_mutex);
        HMAC_Init_ex(m_hmac, m_key.c_str(), m_key.size(), m_evp, nullptr);

        HMAC_Update(m_hmac, header.data(), header.size());
        HMAC_Update(m_hmac, parent_header.data(), parent_header.size());
        HMAC_Update(m_hmac, meta_data.data(), meta_data.size());
        HMAC_Update(m_hmac, content.data(), content.size());

        auto sig = std::vector<unsigned char>(EVP_MD_size(m_evp));
        HMAC_Final(m_hmac, sig.data(), nullptr);

        std::string hex_sig = hex_string(sig);
        return hex_sig;
    }

    bool openssl_xauthentication::verify_impl(const RawBuffer& signature,
                                              const RawBuffer& header,
                                              const RawBuffer& parent_header,
                                              const RawBuffer& meta_data,
                                              const RawBuffer& content) const
    {
        std::lock_guard<std::mutex> lock(m_hmac_mutex);
        HMAC_Init_ex(m_hmac, m_key.c_str(), m_key.size(), m_evp, nullptr);

        HMAC_Update(m_hmac, header.data(), header.size());
        HMAC_Update(m_hmac, parent_header.data(), parent_header.size());
        HMAC_Update(m_hmac, meta_data.data(), meta_data.size());
        HMAC_Update(m_hmac, content.data(), content.size());

        auto sig = std::vector<unsigned char>(EVP_MD_size(m_evp));
        HMAC_Final(m_hmac, sig.data(), nullptr);

        std::string hex_sig = hex_string(sig);
        auto cmp = CRYPTO_memcmp(reinterpret_cast<const void*>(hex_sig.c_str()), signature.data(), hex_sig.size());
        return cmp == 0;
    }

    std::string no_xauthentication::sign_impl(const RawBuffer& /*header*/,
                                              const RawBuffer& /*parent_header*/,
                                              const RawBuffer& /*meta_data*/,
                                              const RawBuffer& /*content*/) const
    {
        return std::string();
    }

    bool no_xauthentication::verify_impl(const RawBuffer& /*signature*/,
                                         const RawBuffer& /*header*/,
                                         const RawBuffer& /*parent_header*/,
                                         const RawBuffer& /*meta_data*/,
                                         const RawBuffer& /*content*/) const
    {
        return true;
    }
}
