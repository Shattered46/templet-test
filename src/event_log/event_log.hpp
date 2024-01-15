#pragma once

#include <string>
#include <vector>
#include <utility>
#include <optional>

#include <omp.h>

struct Ord { 
    unsigned int val;
};

struct Tag { int val; };

int operator<(const Ord a, const Ord b);
int operator<(const Tag a, const Tag b);

class event_log{
    using entry = std::pair<Tag, std::string>;
    template <class T>
    using option = std::optional<T>;
public:
     event_log();
    ~event_log();
    
    void reset();

	void print();

    uint size();
	
    bool contains(Tag);

	Ord add_event(Tag tag, std::string& data);
    bool remove_event(Tag tag);
	option<entry> get_event(Ord ord);
	
private:
	std::vector<entry> log;
    omp_lock_t lock;
};
