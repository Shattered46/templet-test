#pragma once

#include <streambuf>

#define BUFFER_SIZE 128

class stream_socket : public std::basic_streambuf<char> {
    char buffer[BUFFER_SIZE];
    char out_buffer[BUFFER_SIZE];
    int socket_descriptor = -1;    

    int_type underflow() override;
    int_type overflow(int_type ch) override;
public:
    stream_socket(stream_socket&) = delete;
    stream_socket(stream_socket&&) = delete;
    stream_socket(int socket_descriptor);
    void set_socket(int socket_descriptor);
    void flush();
};
