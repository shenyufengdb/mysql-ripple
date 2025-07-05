CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9)

project(binlogserver)

# 获取git版本
execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
MESSAGE(STATUS "GIT_VERSION: ${GIT_VERSION}")

# 生成git_version.h
#CONFIGURE_FILE(
#        ${CMAKE_SOURCE_DIR}/include/git_version.h.in
#        ${CMAKE_BINARY_DIR}/include/git_version.h
#)

# 设置
SET(CMAKE_CXX_STANDARD 17)

include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)

# Always enable debug sync for debug builds.
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ")

MESSAGE(STATUS "Running cmake version ${CMAKE_VERSION}")
MESSAGE(STATUS "Runing gcc version  ${CMAKE_CXX_COMPILER_VERSION}")

INCLUDE_DIRECTORIES(
        /usr/include/mysql
        /usr/include/mysql/mysql/
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/extra/absl/lts_2019_08_08
        ${CMAKE_SOURCE_DIR}/extra/absl/lts_2019_08_08/absl
        ${CMAKE_SOURCE_DIR}/extra/absl/lts_2019_08_08/time/
        ${CMAKE_CURRENT_BINARY_DIR}/extra/absl/lts_2019_08_08/strings/
        ${CMAKE_CURRENT_BINARY_DIR}/extra/gflags/gflags-2.2.2/include/
        ${CMAKE_CURRENT_BINARY_DIR}/extra/gflags/gflags-2.2.2/
        ${CMAKE_CURRENT_BINARY_DIR}/extra/glog/glog-0.4.0
        ${CMAKE_CURRENT_BINARY_DIR}/extra/glog/glog-0.4.0/src/
        ${WITH_PROTOBUF}/src
        ${CMAKE_BINARY_DIR}/include
        ${CMAKE_SOURCE_DIR}/extra/protobuf/include/
        ${CMAKE_SOURCE_DIR}/extra/protobuf/protobuf-3.9.0/src/
)

SET(RIPPLE_SOURCES
        ${CMAKE_SOURCE_DIR}/rippled.cc
        ${CMAKE_SOURCE_DIR}/plugin.cc
        ${CMAKE_SOURCE_DIR}/monitoring.cc
        ${CMAKE_SOURCE_DIR}/session.cc
        ${CMAKE_SOURCE_DIR}/session_factory.cc
        ${CMAKE_SOURCE_DIR}/listener.cc
        ${CMAKE_SOURCE_DIR}/mysql_slave_session.cc
        ${CMAKE_SOURCE_DIR}/mysql_master_session.cc
        ${CMAKE_SOURCE_DIR}/binlog_position.cc
        ${CMAKE_SOURCE_DIR}/log_event.cc
        ${CMAKE_SOURCE_DIR}/binlog_position.cc
        ${CMAKE_SOURCE_DIR}/binlog_reader.cc
        ${CMAKE_SOURCE_DIR}/gtid.cc
        ${CMAKE_SOURCE_DIR}/file_position.cc
        ${CMAKE_SOURCE_DIR}/encryption.cc
        ${CMAKE_SOURCE_DIR}/file_util.cc
        ${CMAKE_SOURCE_DIR}/mysql_protocol.cc
        ${CMAKE_SOURCE_DIR}/mysql_server_port.cc
        ${CMAKE_SOURCE_DIR}/my_crypt.cc
        ${CMAKE_SOURCE_DIR}/init.cc
        ${CMAKE_SOURCE_DIR}/executor.cc
        ${CMAKE_SOURCE_DIR}/purge_thread.cc
        ${CMAKE_SOURCE_DIR}/mysql_init.cc
        ${CMAKE_SOURCE_DIR}/file_FILE.cc
        ${CMAKE_SOURCE_DIR}/binlog.cc
        ${CMAKE_SOURCE_DIR}/mysql_compat.cc
        ${CMAKE_SOURCE_DIR}/mysql_client_connection.cc
        ${CMAKE_SOURCE_DIR}/mysql_server_connection.cc
        ${CMAKE_SOURCE_DIR}/connection.cc
        ${CMAKE_SOURCE_DIR}/mysql_constants.cc
        ${CMAKE_SOURCE_DIR}/binlog_index.cc
        ${CMAKE_SOURCE_DIR}/my_crypt_key_management.cc
        ${CMAKE_SOURCE_DIR}/my_crypt_key_management_impl.cc
        ${CMAKE_SOURCE_DIR}/mysql_server_port_tcpip.cc
        ${CMAKE_SOURCE_DIR}/flags.cc
)

add_subdirectory(${CMAKE_SOURCE_DIR}/extra/absl/lts_2019_08_08/)
add_subdirectory(${CMAKE_SOURCE_DIR}/extra/gflags/gflags-2.2.2/)
add_subdirectory(${CMAKE_SOURCE_DIR}/extra/glog/glog-0.4.0/)

find_library(OPENSSL
        NAMES libssl.so
        PATHS /usr/lib64/openssl)

find_library(CRYPTO
        NAMES libcrypto.so
        PATHS /usr/lib64/openssl)

find_library(MYSQLCLIENT
        NAMES libmysqlclient.a
        PATHS /usr/lib64/mysql)

ADD_EXECUTABLE(binlogserver ${RIPPLE_SOURCES})
target_compile_options(binlogserver PRIVATE -DHAVE_PSI_MEMORY_INTERFACE -DHAVE_PSI_INTERFACE)

MESSAGE(STATUS "mysqlclient is: ${MYSQLCLIENT}")
MESSAGE(STATUS "openssl is: ${OPENSSL}")
MESSAGE(STATUS "crypto is: ${CRYPTO}")

TARGET_LINK_LIBRARIES(binlogserver glog absl_base absl_hashtablez_sampler gflags
        absl_raw_hash_set absl_hash dl z rt ${OPENSSL} ${CRYPTO} ${MYSQLCLIENT} pthread stdc++fs)