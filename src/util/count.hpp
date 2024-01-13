#ifndef __EXECUTION_COUNTERS__
#define __EXECUTION_COUNTERS__
#include <stdint.h>
#include <vector>
#include "allocator.hpp"


/* 
    [NOTE]: This is NOT THREAD SAFE!
    Make sure you're using appropriate thread sync objects
*/
template<typename T> struct Counters 
{
    using counter_type = T;
    static constexpr size_t counter_type_bytes = sizeof(counter_type);

    void create(u8 amountOfCounters) 
    {
        m_underlying_mem = amalloc_t(
            counter_type, 
            counter_type_bytes * amountOfCounters, 
            counter_type_bytes
        );
        m_availableCounters.create(m_underlying_mem, amountOfCounters);
        m_countStack.reserve(amountOfCounters);
        return;
    }

    void destroy()
    {
        m_availableCounters.destroy();
        m_countStack.resize(0);
        afree_t(m_underlying_mem);
        return;
    }


    counter_type& allocate() {
        return *m_availableCounters.allocate();
    }
    void free(counter_type& counter) {
        m_availableCounters.free(&counter);
    }


    void push_counter() {
        if(m_availableCounters.availableBlocks() == 0) {
            debug_message("Counters::push_counter() => Tried to push_counter(), Counter Buffer is Full\n");
            return;
        }
        m_countStack.push_back(m_availableCounters.allocate_index());
        return;
    }

    void pop_counter()
    {
        if(m_countStack.empty()) {
            debug_message("Counters::pop_counter() => Tried to pop_counter() before push_counter()\n");
            return;
        }
        m_availableCounters.free_index(m_countStack.back());
        m_countStack.pop_back();
        return;
    }


    counter_type& active() {
        return m_underlying_mem[m_countStack.back()];
    }


    size_t remaining() const {
        return m_availableCounters.availableBlocks();
    }
    size_t size() const { 
        return m_availableCounters.size(); 
    }


private:
    StaticPoolAllocator<counter_type, true> m_availableCounters;
    mut_type_handle<counter_type>           m_underlying_mem;
    std::vector<counter_type>               m_countStack;
};


template<typename T> inline void reset_counter(T& count) {
    memset(&count, 0x00, sizeof(T));
}


static Counters<u32> local_u32;


#define CREATE_LOCAL_COUNTER_BUFFER(size) local_u32.create(size);
#define DESTROY_LOCAL_COUNTER_BUFFER() local_u32.destroy();
#define ALLOCATE_COUNTER() local_u32.allocate();
#define FREE_COUNTER(counter_name) local_u32.free(counter_name);
#define RESET_COUNTER(counter_name) reset_counter(counter_name);


#define PUSH_COUNTER() local_u32.push_counter();
#define POP_COUNTER() local_u32.pop_counter();
#define RESET_ACTIVE_COUNTER() RESET_COUNTER(local_u32.active())


#define __rconce(__finished, code_block) \
	if(!boolean(__finished)) { \
		{ \
			code_block; \
			++__finished; \
		} \
	} \

#define __rcblock(times, __finished, code_block) \
	if(boolean((uint16_t)times - __finished)) { \
		{ \
			code_block; \
			++__finished; \
		} \
	} \


/* 
    Must use PUSH, POP, before and after calling this respectively
*/
#define __ronce(code_block) \
	if(!boolean(local_u32.active())) { \
		{ \
			code_block; \
			++local_u32.active(); \
		} \
	} \

#define __rblock(times, code_block) \
	if(boolean((uint32_t)times - local_u32.active())) { \
		{ \
			code_block; \
			++local_u32.active(); \
		} \
	} \



#endif