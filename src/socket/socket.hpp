#pragma once

#include "stream_socket.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <sys/types.h>
#include <cstring>

sockaddr* fun_cast(sockaddr_in* internet_socket_address);

class Socket {
private:
    int socket_handle;
    sockaddr* socket_address = nullptr;

    stream_socket* out_stream = nullptr;
    stream_socket* in_stream = nullptr;

    std::istream* input_stream_buf = nullptr;
    std::ostream* output_stream_buf = nullptr;

    
public:

    void flush();

    ~Socket();

    Socket();

    Socket(Socket&) = delete;
    Socket(Socket&&);
    Socket(int socket_handle);

    bool connect(const char* ip_address_text, const char* port);

    int socket_descriptor();

    bool bind(const char* ip_address_text, const char* port);

    bool address_helper(const char* ip_address_text, const char* port);

    int send(const char* buffer, int len);
    
    int recv(char* buffer, int len);
    
    bool can_read();

    Socket* accept();

    std::istream* input_stream();

    std::ostream* output_stream();
};