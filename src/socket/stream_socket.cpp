#include "stream_socket.hpp"

#include <cerrno>
#include <streambuf>
#include <istream>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../error_parser.hpp"
#include "src/logger/logger.hpp"

stream_socket::int_type stream_socket::underflow() {
    char* buffer_p = buffer;
    Logger::debug() << "underflow; start;"; Logger::debugln();
    int bytes_read = ::recv(socket_descriptor, &buffer, BUFFER_SIZE, 0);
    if (bytes_read == 0) {
        Logger::log() << "zero bytes read";
        setg(buffer_p, nullptr, buffer_p);
        return traits_type::eof();
    } else if (bytes_read == -1) {
        Logger::log() << "failed to read bytes";
        print_error();
        setg(buffer_p, nullptr, buffer_p);
        return traits_type::eof();     
    } else {
        setg(buffer_p, buffer_p, buffer_p + bytes_read + 1);
    }
    
    std::ostream& debug = Logger::debug();
    for(int i = 0; i < bytes_read; ++i) {
        int val = buffer_p[i];
        debug << std::to_string(val) << " ";
    }

    auto eb = eback();
    auto gp = gptr();
    auto ep = egptr();

    Logger::debug() << "underflow; pointer values: start 0, current " << gp - eb << " end " << ep - eb << ";"; 
    return buffer[0];
}

stream_socket::int_type stream_socket::overflow(int_type ) {
    Logger::debug() << "overflow; start;";
    flush();
    auto eb = pbase();
    auto gp = pptr();
    auto ep = epptr();
    Logger::debug() << "overflow; pointer values: start 0, current " << gp - eb << " end " << ep - eb; 
    return -13;
}
stream_socket::stream_socket(int socket_descriptor) {
    set_socket(socket_descriptor);
}

void stream_socket::set_socket(int socket_descriptor) {
    this->socket_descriptor = socket_descriptor;

    char* out_buffer_p = out_buffer;
    setp(out_buffer_p, out_buffer_p + BUFFER_SIZE + 1);
}
    
void stream_socket::flush() {
    Logger::debug() << "flush"; Logger::debugln();
    char* out_buffer_start = pbase();
    char* out_buffer_current = pptr();

    int data_length = out_buffer_current - out_buffer_start;

    if (data_length > 0) {
        int bytes_sent = ::send(socket_descriptor, &out_buffer, data_length, 0);

        if (bytes_sent == -1) {
            int error = errno;
            auto& err = Logger::error() << "error sending bytes ";
            if (error == EBADF || error == ENOTSOCK) {
                err << "Bad socket descriptor: " << std::to_string(socket_descriptor);
            } else {
                print_error();
            }
        } else
        if (bytes_sent == data_length) {
            char* out_buffer_p = out_buffer;
            setp(out_buffer_p, out_buffer_p + BUFFER_SIZE + 1);
        } else {
            Logger::error() << "bytes_sent != data_length; bytes_sent=" << std::to_string(bytes_sent); Logger::errorln();
        }
    }
}