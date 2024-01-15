#include "socket.hpp"

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "src/error_parser.hpp"
#include "src/socket/stream_socket.hpp"
#include "src/logger/logger.hpp"
#include <netdb.h>
#include <sys/types.h>
#include <cstring>
#include <poll.h>

sockaddr* fun_cast(sockaddr_in* internet_socket_address) {
    return (sockaddr*) internet_socket_address;
}

Socket::~Socket() {
    delete input_stream_buf;
    delete output_stream_buf;
    delete out_stream;
    delete in_stream;
    close(socket_handle);
}

void Socket::flush() {
    out_stream->flush();
}

Socket::Socket() {
    this->socket_handle = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket_handle == -1) {
        Logger::error() << "Error creating socket; {} in ::socket() ";
        parse_errno(errno);
    }
    out_stream = new stream_socket(socket_handle);
    in_stream = new stream_socket(socket_handle);
    output_stream_buf = new std::ostream(out_stream);
    input_stream_buf = new std::istream(out_stream);
} 

Socket::Socket(int socket_handle) {
    this->socket_handle = socket_handle;
    out_stream = new stream_socket(socket_handle);
    in_stream = new stream_socket(socket_handle);
    output_stream_buf = new std::ostream(out_stream);
    input_stream_buf = new std::istream(out_stream);
} 

bool Socket::connect(const char* ip_address_text, const char* port) {
    address_helper(ip_address_text, port);
    int connect_return = ::connect(socket_handle, socket_address, sizeof(*socket_address));
    return connect_return == 0;
}

int Socket::socket_descriptor() {
    return this->socket_handle;
}

bool Socket::bind(const char* ip_address_text, const char* port) {
    int success = 0;
    address_helper(ip_address_text, port);
    success = ::bind(socket_handle, socket_address, sizeof(*socket_address));
    if (success != 0) {
        Logger::error() << "bind failed; ERROR: " << std::to_string(success); Logger::errorln();

        print_error();
    }
    success = ::listen(socket_handle, 5);
    if (success != 0) {
        Logger::error() << "listen failed"; Logger::errorln();
    }
    return success == 0;
}

bool Socket::address_helper(const char* ip_address_text, const char* port) {
    addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* server_info = nullptr;
    int success = getaddrinfo(ip_address_text, port, &hints, &server_info);
    if (success != 0) {
        Logger::error() << "failed to parse ip address";
    } else {
        this->socket_address = server_info->ai_addr;
    }

    return success == 0;
}

int Socket::send(const char* buffer, int len) {
    return ::send(socket_handle, buffer, len, 0);
}

int Socket::recv(char* buffer, int len) {
    int byte = ::recv(socket_handle, buffer, len, 0);

    if (byte == -1) {
        Logger::log() << "recv -1";
    }

    return byte;
}

bool Socket::can_read() {
    pollfd fds[1];
    
    pollfd poll_fd {
        .fd = socket_handle,
        .events = POLLIN,
        .revents = 0
    };

    fds[0] = poll_fd;

    int poll_result = ::poll(fds, 1, 0);
    if (poll_result < 0) {
        Logger::error() << "cant read from socket due to error";
        print_error();
    } else if (poll_result == 1) {
        return true;
    }

    return false;
}

Socket* Socket::accept() {
    
    sockaddr_in client_socket_address;
    socklen_t client_socket_length;

    int maybe_client = ::accept(socket_handle, fun_cast(&client_socket_address), &client_socket_length);
    if (maybe_client == -1) {
        int err = errno;
        Logger::error() << "errno";
        parse_errno(err);
        return NULL;
    } else {
        return new Socket(maybe_client);
    }
}

std::istream* Socket::input_stream() {
    if (input_stream_buf == nullptr) {
        input_stream_buf = new std::istream(in_stream);
    }
    return input_stream_buf;
}

std::ostream* Socket::output_stream() {
    if (output_stream_buf == nullptr) {

        if (out_stream == nullptr) {
            Logger::error() << "out_stream == nullptr";
        }
        output_stream_buf = new std::ostream(out_stream);
    }
    return output_stream_buf;
}