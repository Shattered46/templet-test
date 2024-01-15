#pragma once

#include "socket.hpp"
#include <memory>
#include <vector>
#include <poll.h>
#include "src/protocol/messages.hpp"
#include "src/state/state.hpp"

namespace pool {
    template <typename T>
    using rc = std::shared_ptr<T>;
    using State = rc<SharedState>;

    class SocketPool {
        std::vector<pollfd> descriptors;
        std::vector<Socket*> sockets;

        void (*listener_ready)(Socket*, SocketPool&);
        void (*socket_ready)(Socket*, SocketPool&);

        
    public:
        State state;

        SocketPool(int size, State state);
        SocketPool* on_socket_ready(void (*on_socket_ready)(Socket*, SocketPool&));
        SocketPool* on_listener_ready(void (*on_listener_ready)(Socket*, SocketPool&));
        void add_socket(Socket*);
        void update();
        void broadcast(message::Message&);
        void remove(Socket*);
        void poll(int timeout);
        ~SocketPool();
    };
}