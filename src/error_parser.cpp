#include "error_parser.hpp"

#include <errno.h>
#include <iostream>
#include <sys/socket.h>

void parse_errno(int error) {
    switch(error) {
        case EADDRNOTAVAIL:
            std::cout << "EADDRNOTAVAIL" << std::endl;
            break;
        case ECONNREFUSED:
            std::cout << "ECONNREFUSED" << std::endl;
            break;
        case EBADF:
            std::cout << "EBADF" << std::endl;
            break;
        case EAFNOSUPPORT:
            std::cout << "EAFNOSUPPORT" << std::endl;
            break;
        case EFAULT:
            std::cout << "EFAULT" << std::endl;
            break;
        case EINPROGRESS:
            std::cout << "EINPROGRESS" << std::endl;
            break;
        case EINTR:
            std::cout << "EINTR" << std::endl;
            break;
        case EISCONN:
            std::cout << "EISCONN" << std::endl;
            break;
        case ENETUNREACH:
            std::cout << "ENETUNREACH" << std::endl;
            break;
        case ENOTSOCK:
            std::cout << "ENOTSOCK" << std::endl;
            break;
        case EPROTOTYPE:
            std::cout << "EPROTOTYPE" << std::endl;
            break;
        case ETIMEDOUT:
            std::cout << "ETIMEDOUT" << std::endl;
            break;
        case EACCES:
            std::cout << "EACCES" << std::endl;
            break;
        case EADDRINUSE:
            std::cout << "EADDRINUSE" << std::endl;
            break;
        case ECONNRESET:
            std::cout << "ECONNRESET" << std::endl;
            break;
        case EAGAIN:
            std::cout << "EAGAIN | EWOULDBLOCK" << std::endl;
            break;
        case EINVAL:
            std::cout << "EAGAIN | EWOULDBLOCK" << std::endl;
            break;
        case ENOTCONN:
            std::cout << "ENOTCONN" << std::endl;
            break;
        default:
            std::cout << "unknown " << error << std::endl;
    }
}

void print_error() {
    int error = errno;
    parse_errno(error);
}