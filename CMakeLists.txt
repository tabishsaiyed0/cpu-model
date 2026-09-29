cmake_minimum_required(VERSION 3.16)
project(dm-cli VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dm-cli src/main.cpp)

# WinINet for HTTP on Windows, no external deps
if(WIN32)
  target_link_libraries(dm-cli PRIVATE wininet)
endif()

# Enable AVX2 etc. is handled inside llama.cpp / llama-server,
# this client itself is tiny and runs on any CPU.
