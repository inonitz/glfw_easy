#include "pool.hpp"
#include "aligned_malloc.hpp"
#include "ifcrash.hpp"
#include "marker.hpp"


template<typename T> T* detail::CommonPoolDef<T>::allocate() 
{
    if(!m_freeBlk) {
        markstr("Allocation Error: Not Enough Blocks (0)\n");
        return nullptr;
    }

    T* v = &m_buffer[m_available->index - 1];
    m_available->index *= -1; /* now occupied */

    m_available = m_available->next;
    --m_freeBlk;
    return v;
}


template<typename T> void detail::CommonPoolDef<T>::free(T* ptr)
{
    size_t idx = index_from_pointer(ptr);
    ifcrash_debug(!isaligned(ptr, sizeof(T)) || !occupied(idx) || (m_freeBlk == m_elemCount));
    m_freelist[idx].index *= -1;
    m_freelist[idx].next = m_available;
    m_available = &m_freelist[idx];
    ++m_freeBlk;
    
    (ptr, DEFAULT8, sizeof(T)); /* completely wipe the block of old data */
    return;
}


template<typename T> size_t detail::CommonPoolDef<T>::allocate_index()
{
    if(!m_freeBlk) {
        markstr("Allocation Error: Not Enough Blocks (0)\n");
        return DEFAULT64;
    }

    size_t v = m_available->index - 1;
    m_available->index *= -1; /* now occupied */

    m_available = m_available->next;
    --m_freeBlk;
    return v;	
}


template<typename T> void detail::CommonPoolDef<T>::free_index(size_t idx)
{
    ifcrash_debug(!occupied(idx) || m_freeBlk == m_elemCount || idx >= m_elemCount);
    m_freelist[idx].index *= -1;
    m_freelist[idx].next = m_available;
    m_available = &m_freelist[idx];
    ++m_freeBlk;

    __memset(&m_buffer[idx], DEFAULT8, sizeof(T)); /* completely wipe the block of old data */
    return;
}


template<typename T> void detail::CommonPoolDef<T>::print() const
{
    static const char* strs[2] = { "Occupied", "Free    " };
    bool tmp = false;
    printf("Static Pool Allocator:\nObject Array[%llu]: %p\n    Free:     %u\n    Occupied: %u\n    ", m_elemCount, m_buffer, m_freeBlk, m_elemCount - m_freeBlk);
    for(size_t i = 0; i < m_elemCount; ++i)
    {
        tmp = boolean(m_freelist[i].index > 0);
        printf("    Object [i = %llu] [%s] => Object [%llu]\n", i, strs[tmp], __scast(u64, m_freelist[i].index));
    }
    return;
}


template<typename T> void detail::CommonPoolDef<T>::common_init(size_t amountOfElements)
{
    ifcrash_debug(amountOfElements == 0);
    m_elemCount = amountOfElements; 
    m_freeBlk   = amountOfElements;
    for(size_t i = 0; i < amountOfElements - 1; ++i)
    {
        m_freelist[i].index = i + 1;
        m_freelist[i].next = &m_freelist[i + 1];
    }
    m_freelist[m_elemCount - 1] = { __scast(i64, m_elemCount), nullptr }; /* last element shouldn't point anywhere */
    m_available = &m_freelist[0];
    return;
}




template<typename T> void Pool<T, false>::create(size_t amountOfElements)
{
    this->m_buffer   = __scast(T*,        util::aligned_malloc<T>       (sizeof(T)        * amountOfElements));
    this->m_freelist = __scast(NodeType*, util::aligned_malloc<NodeType>(sizeof(NodeType) * amountOfElements));
    this->common_init(amountOfElements);
    return;
}

template<typename T> void Pool<T, false>::destroy()
{
    util::aligned_free(this->m_buffer);    
    util::aligned_free(this->m_freelist);
    this->m_available = nullptr;
    this->m_elemCount = 0;
    this->m_freeBlk   = 0;
    return;
}


template<typename T> void Pool<T, true>::create(
    void*  __aligned_allocated_memory,
    size_t amountOfElements
) {
    this->m_buffer   = __scast(T*, __aligned_allocated_memory);
    this->m_freelist = __scast(NodeType*, util::aligned_malloc<NodeType>(sizeof(NodeType) * amountOfElements));
    this->common_init(amountOfElements);
    return;
}

template<typename T> void Pool<T, true>::destroy()
{
    util::aligned_free(this->m_freelist);
    this->m_buffer    = nullptr;
    this->m_available = nullptr;
    this->m_elemCount = 0;
    this->m_freeBlk   = 0;
    return;
}



