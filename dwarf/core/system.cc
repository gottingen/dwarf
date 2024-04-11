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


#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include "Windows.h"
#include <algorithm>
#else

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#endif

#include <dwarf/core/hash.h>

#include <dwarf/core/system.h>

namespace dwarf {
    std::string remove_ending_separator(const char *path) {
        std::size_t s = strlen(path);
        if (path[s - 1] == '/') {
            --s;
        }
        return std::string(path, s);
    }

    std::string get_temp_directory_path_impl() {
#ifdef _WIN32
        std::string tmp_prefix;
        char char_path[MAX_PATH];
        if(auto s = GetTempPathA(MAX_PATH, char_path))
        {
            tmp_prefix = std::string(char_path, std::size_t(s - 1));
        }
        std::replace(tmp_prefix.begin(), tmp_prefix.end(), '\\', '/');
        return tmp_prefix;
#else
        const char *tmpdir = std::getenv("TMPDIR");
        const char *tmp = std::getenv("TMP");
        const char *tempdir = std::getenv("TEMPDIR");
        const char *temp = std::getenv("TEMP");
        if (tmpdir != nullptr) return remove_ending_separator(tmpdir);
        else if (tmp != nullptr) return remove_ending_separator(tmp);
        else if (tempdir != nullptr) return remove_ending_separator(tempdir);
        else if (temp != nullptr) return remove_ending_separator(temp);
        else return "/tmp";
#endif
    }

    std::string get_temp_directory_path() {
        static const std::string path = get_temp_directory_path_impl();
        return path;
    }

    bool create_directory(const std::string &path) {
        std::size_t pos = path.rfind('/');
        if (pos != 0 && pos != std::string::npos) {
            create_directory(path.substr(0, pos));
        }
#ifdef _WIN32
        return CreateDirectoryA(path.c_str(), NULL);
#else
        struct stat st;
        st.st_dev = 0;
        int ret = 0;
        if (stat(path.c_str(), &st) == -1) {
            ret = mkdir(path.c_str(), 0700);
        }
        return ret == 0;
#endif
    }

    int get_current_pid() {
#ifdef _WIN32
        return GetCurrentProcessId();
#else
        return ::getpid();
#endif
    }

    std::string get_cell_tmp_file(const std::string &prefix,
                                  int execution_count,
                                  const std::string &extension) {
        return prefix + "/[" + std::to_string(execution_count) + "]" + extension;
    }

    std::size_t get_tmp_hash_seed() {
        return static_cast<std::size_t>(0xc70f6907UL);
    }

    std::string get_tmp_prefix(const std::string &process_name) {
        std::string tmp_prefix = dwarf::get_temp_directory_path()
                                 + '/' + process_name + '_'
                                 + std::to_string(dwarf::get_current_pid())
                                 + '/';
        return tmp_prefix;
    }

    std::string get_cell_tmp_file(const std::string &prefix,
                                  const std::string &content,
                                  const std::string &suffix) {
        std::uint32_t seed = static_cast<uint32_t>(get_tmp_hash_seed());
        std::string id = std::to_string(murmur2_x86(content.data(), content.size(), seed));
        return prefix + id + suffix;
    }
}


