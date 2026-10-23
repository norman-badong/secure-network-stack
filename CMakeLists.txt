cmake_minimum_required(VERSION 3.12)
project(Module2TLSServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenSSL package on the system
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${OPENSSL_INCLUDE_DIR})

# Executable target
add_executable(tls_server
    src/main.cpp
    src/server.cpp
)

# Link OpenSSL libraries and pthread for multithreading
target_link_libraries(tls_server PRIVATE OpenSSL::SSL OpenSSL::Crypto pthread)