#pragma once

#include <iostream>
#include <ostream>
#include <functional>
#include <map>
#include <sstream>

#include "src/event_log/event_log.hpp"

class task{
friend class engine;

    using ostream = std::ostream;
    using istream = std::istream;
    template <class T>
    using function = std::function<T>;
    
public:
	bool is_active() { return active; }
    void set_on_ready_func(function<void(task*,void*)>f,void*cnxt){
         func_on_ready=f; on_ready_cnxt=cnxt; 
    }

private:
    function<void(task*,void*)> func_on_ready=[](task*,void*){};
    void* on_ready_cnxt;

    std::string result() {
        std::ostringstream outs;
        this->on_save(outs);
        std::string data = outs.str();
        return data;
    }

    void execute() {
        this->on_exec();
    }
protected:
    virtual void on_exec() {}
    virtual void on_ready(){ func_on_ready(this,on_ready_cnxt); }
    virtual void on_save(ostream&) {}
    virtual void on_load(istream&) {}
    
private:
	bool active = false;
};

const int N = 5;

class mult_task: public task{
    using ostream = std::ostream;
    using istream = std::istream;
    template <class T>
    using vec = std::vector<T>;
public:
    mult_task(vec<int>& Ap, vec<int>& Bp): A(Ap), B(Bp) { };

protected:
    void on_exec() override{
       int i = cur_i;
		for(int j=0; j<N; j++){
		    c[j] = 0;
		    for(int k=0; k<N; k++) c[j] += A[N * i + k] * B[N * k + j];
		} 
    }

    void on_save(ostream& out) override{ 
        for(int j=0; j<N; j++) out << c[j] << " ";
    }
    
    

    void on_ready() override {

    }
public:
    void on_load(istream& in) override{
        for(int j=0; j<N; j++) in >> c[j];
    }

    vec<int>& A;
    vec<int>& B;
    int cur_i;
	int c[N];
};


class engine{
    template <class K, class V>
    using map = std::map<K, V>;
    using entry = std::pair<Tag, std::string>;
    template <class T>
    using option = std::optional<T>;
    using string = std::string;
    template <class T>
    using vec = std::vector<T>;
    using event = std::pair<Tag, string>;
public:
    engine();
    
	void submit(task* t);
	void append(vec<event> events);
    option<event> execute_random_task();
	void wait_all();
    void catch_up();

private:
    event_log log;
	Ord current_ord;
	Tag current_tag;
	option<Ord> my_task_ord;
	map<Tag,task*> todo_tasks;
};