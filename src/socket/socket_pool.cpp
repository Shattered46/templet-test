#include "socket_pool.hpp"

#include <poll.h>
#include <vector>

#include "socket.hpp"
#include "src/error_parser.hpp"
#include "src/logger/logger.hpp"
#include "src/protocol/messages.hpp"

namespace pool {
    SocketPool::SocketPool(int, State state_) :
        descriptors(std::vector<pollfd>()),
        sockets(std::vector<Socket*>()),
        state(state_) {
    }

    void SocketPool::add_socket(Socket* socket) {

        Logger::log() << "added socket with descriptor #" << socket->socket_descriptor();

        pollfd poll_descriptor {
            .fd =  socket->socket_descriptor(),
            .events = POLLIN,
            .revents = 0
        };

        this->descriptors.push_back(poll_descriptor);
        this->sockets.push_back(socket);
    }

    void SocketPool::poll(int timeout) {
        int poll_result = ::poll(this->descriptors.data(), this->descriptors.size(), timeout);
        Logger::debug() << "poll result " << poll_result;
        if (poll_result == -1) {
            Logger::error() << "polling sockets error";
            print_error();
        } else if (poll_result > 0) {
            update();
        }
    }

    void SocketPool::broadcast(message::Message& msg) {
        auto socs = sockets.data();
        for (uint i = 1; i < sockets.size(); i++) {
            
            auto socket = socs[i];
            if (socket != nullptr) {
                msg.write(*socket);
            }
        }
    }

    void SocketPool::update() {
        Logger::debug() << "update " << descriptors.size();
        if (descriptors.size() == 0) {
            return;
        }
        auto listener_socket = sockets.data()[0];
        auto listener_descriptor = &descriptors.data()[0];

        Logger::log() << "checking socket with descriptor #" << listener_socket->socket_descriptor();

        if (listener_descriptor->revents & POLLIN) {
            Logger::log() << "pollin";
            listener_descriptor->revents = 0;
            this->listener_ready(listener_socket, *this);
        }

        for(auto i = 1u; i < descriptors.size(); i++) {
            auto socket = sockets.data()[i];
            auto descriptor = &descriptors.data()[i];
            if (descriptor->fd >= 0)
                Logger::log() << "checking socket with descriptor #" << socket->socket_descriptor();
            if (descriptor->revents & POLLIN) {
                Logger::log() << "pollin";
                this->socket_ready(socket, *this);
            }
            if (descriptor->revents & POLLHUP) {
                Logger::log() << "socket hung up";
            }
            if (descriptor->revents & POLLNVAL) {
                Logger::log() << "descriptor haven't been opened";
            }
            if (descriptor->revents & POLLERR) {
                Logger::log() << "POLLERR";
            }
            descriptor->revents = 0;
        }
    }

    SocketPool* SocketPool::on_socket_ready(void (*on_socket_ready)(Socket*, SocketPool&)) {
        this->socket_ready = on_socket_ready;
        return this;
    }

    SocketPool* SocketPool::on_listener_ready(void (*on_listener_ready)(Socket*, SocketPool&)) {
        this->listener_ready = on_listener_ready;
        return this;
    }

    void SocketPool::remove(Socket* socket) {
        int idx = 1;
        for(auto i = 0u; i < sockets.size(); i++) {
            auto d_socket = sockets.data()[i];
            if (d_socket == socket) {
                idx = i;
                break;
            }
        }

        sockets.data()[idx] = nullptr;
        descriptors.data()[idx].fd = -1;

        delete socket;
    }

    SocketPool::~SocketPool() {
        for(auto i = 1u; i < sockets.size(); i++) { // because server socket is on stack;
            auto socket = sockets.data()[i];
            delete socket;
        }
    }
}