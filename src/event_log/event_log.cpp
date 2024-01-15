#include "event_log.hpp"

#include <iostream>
#include <omp.h>
#include <optional>
#include <vector>

using entry = std::pair<Tag, std::string>;
template <class T>
using option = std::optional<T>;
using string = std::string;
template <class T>
using vec = std::vector<T>;


int operator<(const Ord a, const Ord b) {
    return a.val < b.val;
}

int operator<(const Tag a, const Tag b) {
    return a.val < b.val;
}


event_log::event_log(){ omp_init_lock(&lock); }
event_log::~event_log(){ omp_destroy_lock(&lock); }

void event_log::reset() { log.clear(); }

void event_log::print() {
    for (unsigned int i = 0; i < log.size(); i++) {
        std::cout << "tag  = " << log[i].first.val << std::endl;
        std::cout << "data = \"" << log[i].second << "\"" << std::endl;
        std::cout << "======" << std::endl;
    }
}

option<Ord> indexof(vec<entry>& vec, Tag& tag) {
    for(uint i = 0; i < vec.size(); i++) {
        if(vec[i].first.val == tag.val) {
            return option<Ord>(Ord { i });
        }
    }
    return option<Ord>();
}

bool event_log::contains(Tag tag) {
    return indexof(log, tag).has_value();
}

Ord event_log::add_event(Tag tag, string& data) {
    omp_set_lock(&lock);

    option<Ord> maybe_index = indexof(log, tag);
    if (maybe_index.has_value()) {
        omp_unset_lock(&lock);
        return maybe_index.value();
    }

    log.push_back(entry(tag, data));
    uint num = log.size() - 1;
    omp_unset_lock(&lock);
    return Ord { num }; 
}

uint event_log::size() {
    return this->log.size();
}

bool event_log::remove_event(Tag tag) {
    auto index = indexof(log, tag);
    if (index.has_value()) {
        uint idx = index.value().val;
        auto delete_pos = log.begin() + idx;
        log.erase(delete_pos, delete_pos + 1);
        return true;
    }
    return false;
}

option<entry> event_log::get_event(Ord order) {
    unsigned int index = order.val;
    omp_set_lock(&lock);
    if(index < log.size()){
        entry value = this->log[index];
        
        omp_unset_lock(&lock);
        return value;
    }
    
    omp_unset_lock(&lock);
    return option<entry>();
}