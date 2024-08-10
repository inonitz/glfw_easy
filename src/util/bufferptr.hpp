#ifndef __UTIL_BUFFER_POINTER_MANAGER__
#define __UTIL_BUFFER_POINTER_MANAGER__
#include "types.hpp"
#include "ifcrash.hpp"


namespace util {


template<typename T, bool deallocate_on_destroy, class Deallocator> class __buffer_pointer_manager
{
public:
    void create(void* buffer, u32 maximal_size) {
        m_buffer = __rcast(T*, buffer);
        m_size = maximal_size;
        return;
    }
    void destroy() {
        if constexpr (deallocate_on_destroy) {
            Deallocator()(m_buffer);
        }
        m_buffer = __rcast(T*, DEFAULT64);
        m_size   = DEFAULT32;
        return;
    }


    T& operator[](u32 index) {
        ifcrashfmt_debug(index >= m_size, "Out-of-bounds memory access, %u >= %u (idx, buf_size)", index, m_size);
        return m_buffer[index];
    }

    const T& operator[](u32 index) const {
        ifcrashfmt_debug(index >= m_size, "Out-of-bounds memory access, %u >= %u (idx, buf_size)", index, m_size);
        return m_buffer[index];
    }


    T* const data() const { return m_buffer; }
    u32      size() const { return m_size;   }

    T* begin() { return &m_buffer[0];      }
    T* end()   { return &m_buffer[m_size]; }
    const T* begin() const { return &m_buffer[0];      }
    const T* end()   const { return &m_buffer[m_size]; }
private:
    T*  m_buffer = __rcast(T*, DEFAULT64);
    u32 m_size   = DEFAULT32;
};


template<typename T> using BufferPointer = __buffer_pointer_manager<T, false, void>;

template<typename T, class DeallocatorClass> 
using ManagedBufferPointer = __buffer_pointer_manager<T, true, DeallocatorClass>; 


} // namespace util


#endif