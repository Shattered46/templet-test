#pragma once

#include "src/event_log/event_log.hpp"

#include <mutex>
#include <thread>

class SharedState {
    using vec = std::vector<int>;
public:

    int size;

    std::vector<int> A;
    std::vector<int> B;
    std::vector<int> C;

    event_log log;
    std::mutex log_mutex;

    SharedState(int _size) : 
        size(_size), 
        A(vec(size*size)), 
        B(vec(size*size)), 
        C(vec(size*size)) { }
};