#! /usr/bin/env bash
# Copyright 2023 The titan-search Authors.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

THIS_PATH=$(
    cd $(dirname "$0")
    pwd
)

ROOT_PATH=${THIS_PATH}/..
THIRD_PARTY_PATH=${ROOT_PATH}/3rd
OUTPUT_PATH=${ROOT_PATH}/output
INSTALL_PATH=${ROOT_PATH}/output/install

if [ -d ${OUTPUT_PATH} ]; then
    rm ${OUTPUT_PATH} -rf
fi

mkdir ${OUTPUT_PATH}


#include collie

collie_SRC_PATH=${THIRD_PARTY_PATH}/collie-0.2.18
collie_BUILD_PATH=${OUTPUT_PATH}/collie

mkdir ${collie_BUILD_PATH}
cmake -S ${collie_SRC_PATH} -B ${collie_BUILD_PATH} \
  -DCMAKE_INSTALL_PREFIX=${INSTALL_PATH} \
  -DCARBIN_ENABLE_INSTALL=ON \
  -DCARBIN_BUILD_TEST=OFF \
  -DCARBIN_BUILD_BENCHMARK=OFF \
  -DCARBIN_BUILD_EXAMPLES=OFF

cmake --build ${collie_BUILD_PATH} -j 4
cmake --install ${collie_BUILD_PATH} --prefix ${INSTALL_PATH}

#include openssl

openssl_SRC_PATH=${THIRD_PARTY_PATH}/openssl-OpenSSL_1_1_1n
openssl_BUILD_PATH=${OUTPUT_PATH}/openssl
cp -r ${openssl_SRC_PATH} ${openssl_BUILD_PATH}
cd ${openssl_BUILD_PATH}
./config -fPIC no-shared  --prefix=${INSTALL_PATH}
make -j 4
make install INSTALLTOP=${INSTALL_PATH}

cd ${ROOT_PATH}

# install zeromq

zeromq_SRC_PATH=${THIRD_PARTY_PATH}/libzmq-4.3.5
zeromq_BUILD_PATH=${OUTPUT_PATH}/libzmq

mkdir ${zeromq_BUILD_PATH}
cmake -S ${zeromq_SRC_PATH} -B ${zeromq_BUILD_PATH} \
  -DCMAKE_INSTALL_PREFIX=${INSTALL_PATH} \
  -DWITH_PERF_TOOL=OFF \
  -DBUILD_SHARED=OFF \
  -DWITH_DOCS=OFF \
  -DWITH_LIBSODIUM=OFF \
  -DZMQ_BUILD_TESTS=OFF \
  -DWITH_TLS=OFF

cmake --build ${zeromq_BUILD_PATH} -j 4
cmake --install ${zeromq_BUILD_PATH} --prefix ${INSTALL_PATH}

