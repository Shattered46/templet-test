#include "engine.hpp"

#include <cassert>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <utility>

using string = std::string;
using ostringstream = std::ostringstream;
using istringstream = std::istringstream;

template <class T>
using option = std::optional<T>;
using string = std::string;
template <class T>
using vec = std::vector<T>;
using event = std::pair<Tag, string>;

engine::engine() { 
    current_ord = Ord { 0 };
    current_tag = Tag { 0 };
    my_task_ord = option<Ord>();
}
    
void engine::submit(task* t) {
    assert(t->active==false);
    todo_tasks[current_tag] = t;
    current_tag.val++;
    t->active = true;
}

void engine::append(vec<event> events) {
    for (const auto& ev: events) {
        auto [tag, data] = ev;
        log.add_event(tag, data);
    }
}

//catch up to latest events
void engine::catch_up() {
    option<entry> event;
    while( (event = log.get_event(current_ord)).has_value()){
        Tag tag = event->first;
        string data = event->second;

        task* t = todo_tasks[tag];
        todo_tasks.erase(tag);
        
        if(current_ord.val != my_task_ord->val){
            istringstream ins(data);
            t->on_load(ins);
        }
        
        t->active = false;
        t->on_ready();
    
        current_ord.val++;

        event = log.get_event(current_ord);
    }
}

std::pair<Tag, task*> pick_random_task(std::map<Tag, task*> todo) {
    int todo_size = todo.size();

    int num_of_task_to_exec = rand() % todo_size;
    auto task_to_exec = todo.begin();
    advance(task_to_exec, num_of_task_to_exec);

    Tag tag = task_to_exec->first;
    task* t = task_to_exec->second;

    return std::make_pair(tag, t);
}

option<event> engine::execute_random_task() {
    if(todo_tasks.empty()) return option<event>();
        
    auto [tag, task] = pick_random_task(todo_tasks);
    task->execute();
    string data = task->result();
    // save event (changes to shared state);
    event pair = std::make_pair(tag, data);
    my_task_ord = log.add_event(tag,data);
    return pair;
}

void engine::wait_all() {
    for(;;){
        catch_up();
        if(todo_tasks.empty()) return;
        execute_random_task();
    }
}
