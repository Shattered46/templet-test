cmake_minimum_required(VERSION 3.10)
project(Templet)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(BASEPATH "${CMAKE_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${BASEPATH}")

add_compile_options("-Wall" "-Wextra" "-Werror")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
    stream_server
    src/stream_server.cpp 
    src/socket/socket_pool.cpp
    src/socket/stream_socket.cpp
    src/socket/socket.cpp
    src/error_parser.cpp
    src/protocol/messages.cpp
    src/logger/logger.cpp
    src/event_log/event_log.cpp
)

add_executable(
    stream_client
    src/stream_client.cpp 
    src/socket/socket_pool.cpp
    src/socket/stream_socket.cpp
    src/socket/socket.cpp
    src/error_parser.cpp
    src/protocol/messages.cpp
    src/logger/logger.cpp
    src/event_log/event_log.cpp
    src/engine/engine.cpp
)

find_package(OpenMP REQUIRED)
target_link_libraries(stream_server PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(stream_client PUBLIC OpenMP::OpenMP_CXX)

target_compile_features(stream_server PRIVATE cxx_std_17)
target_compile_features(stream_client PRIVATE cxx_std_17)