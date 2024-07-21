#include "pool.hpp"
#include "util.hpp"
#include "ifcrash.hpp"
#include "marker2.hpp"
#include "aligned_malloc.hpp"
#include <cstdio>


namespace detail {


template<u32 objectSizeInBytes> void* CommonPoolDef<objectSizeInBytes>::allocate() 
{
    if(!m_freeBlk) {
        markstr("Allocation Error: Not Enough Blocks (0)\n");
        return nullptr;
    }

    byte* v = &m_buffer[ objectSizeInBytes * (m_available->index - 1) ];
    m_available->index *= -1; /* now occupied */

    m_available = m_available->next;
    --m_freeBlk;
    return v;
}


template<u32 objectSizeInBytes> void CommonPoolDef<objectSizeInBytes>::free(void* ptr)
{
    u64 idx = index_from_pointer(ptr) / objectSizeInBytes;
    ifcrash_debug(!isaligned(ptr, objectSize()) || !occupied(idx) || (m_freeBlk == m_elemCount));

    m_freelist[idx].index *= -1;
    m_freelist[idx].next = m_available;
    m_available = &m_freelist[idx];
    ++m_freeBlk;

    /* completely wipe the block of old data */
    util::__memset<byte>(__scast(byte*, ptr), objectSize(), DEFAULT8);
    return;
}


template<u32 objectSizeInBytes> u64 CommonPoolDef<objectSizeInBytes>::allocate_index()
{
    if(!m_freeBlk) {
        markstr("Allocation Error: Not Enough Blocks (0)\n");
        return DEFAULT64;
    }

    u64 v = m_available->index - 1;
    m_available->index *= -1; /* now occupied */

    m_available = m_available->next;
    --m_freeBlk;
    return v;	
}


template<u32 objectSizeInBytes> void CommonPoolDef<objectSizeInBytes>::free_index(u64 idx)
{
    ifcrash_debug(!occupied(idx) || m_freeBlk == m_elemCount || idx >= m_elemCount);
    m_freelist[idx].index *= -1;
    m_freelist[idx].next = m_available;
    m_available = &m_freelist[idx];
    ++m_freeBlk;

    util::__memset<byte>(&m_buffer[idx], objectSize(), DEFAULT8); /* completely wipe the block of old data */
    return;
}


template<u32 objectSizeInBytes> void CommonPoolDef<objectSizeInBytes>::print() const
{
    static const char* strs[2] = { "Occupied", "Free    " };
    bool tmp = false;
    std::printf("Static Pool Allocator:\nObject Array[%llu]: %p\n    Free:     %u\n    Occupied: %u\n    ", m_elemCount, m_buffer, m_freeBlk, m_elemCount - m_freeBlk);
    for(u64 i = 0; i < m_elemCount; ++i)
    {
        tmp = boolean(m_freelist[i].index > 0);
        std::printf("    Object [i = %llu] [%s] => Object [%llu]\n", i, strs[tmp], __scast(u64, m_freelist[i].index));
    }
    return;
}


template<u32 objectSizeInBytes> void CommonPoolDef<objectSizeInBytes>::common_init(u64 amountOfElements)
{
    ifcrash_debug(amountOfElements == 0);
    m_elemCount = amountOfElements; 
    m_freeBlk   = amountOfElements;
    for(u64 i = 0; i < amountOfElements - 1; ++i)
    {
        m_freelist[i].index = i + 1;
        m_freelist[i].next = &m_freelist[i + 1];
    }
    m_freelist[m_elemCount - 1] = { __scast(i64, m_elemCount), nullptr }; /* last element shouldn't point anywhere */
    m_available = &m_freelist[0];
    return;
}


template struct CommonPoolDef<0x08>;
template struct CommonPoolDef<0x10>;
template struct CommonPoolDef<0x18>;
template struct CommonPoolDef<0x20>;
template struct CommonPoolDef<0x28>;
template struct CommonPoolDef<0x30>;
template struct CommonPoolDef<0x38>;
template struct CommonPoolDef<0x40>;
template struct CommonPoolDef<0x48>;
template struct CommonPoolDef<0x50>;
template struct CommonPoolDef<0x58>;
template struct CommonPoolDef<0x60>;
template struct CommonPoolDef<0x68>;
template struct CommonPoolDef<0x70>;
template struct CommonPoolDef<0x78>;
template struct CommonPoolDef<0x80>;


} // namespace detail




template<u32 objectSizeInBytes> void Pool<objectSizeInBytes, false>::create(u64 amountOfElements)
{
    this->m_buffer = __scast(byte*,     
        util::aligned_malloc<objectSizeInBytes>(objectSizeInBytes * amountOfElements)
    );
    this->m_freelist = __scast(NodeType*, 
        util::aligned_malloc<sizeof(NodeType)>(sizeof(NodeType)  * amountOfElements)
    );
    this->common_init(amountOfElements);
    return;
}

template<u32 objectSizeInBytes> void Pool<objectSizeInBytes, false>::destroy()
{
    util::aligned_free(this->m_buffer);    
    util::aligned_free(this->m_freelist);
    this->m_available = nullptr;
    this->m_elemCount = 0;
    this->m_freeBlk   = 0;
    return;
}


template<u32 objectSizeInBytes> void Pool<objectSizeInBytes, true>::create(
    void*  __aligned_allocated_memory,
    u64 amountOfElements
) {
    this->m_buffer   = __scast(byte*, __aligned_allocated_memory);
    this->m_freelist = __scast(NodeType*, util::aligned_malloc<sizeof(NodeType)>(sizeof(NodeType) * amountOfElements));
    this->common_init(amountOfElements);
    return;
}

template<u32 objectSizeInBytes> void Pool<objectSizeInBytes, true>::destroy()
{
    util::aligned_free(this->m_freelist);
    this->m_buffer    = nullptr;
    this->m_available = nullptr;
    this->m_elemCount = 0;
    this->m_freeBlk   = 0;
    return;
}


template class Pool<0x08, true>;
template class Pool<0x10, true>;
template class Pool<0x18, true>;
template class Pool<0x20, true>;
template class Pool<0x28, true>;
template class Pool<0x30, true>;
template class Pool<0x38, true>;
template class Pool<0x40, true>;
template class Pool<0x48, true>;
template class Pool<0x50, true>;
template class Pool<0x58, true>;
template class Pool<0x60, true>;
template class Pool<0x68, true>;
template class Pool<0x70, true>;
template class Pool<0x78, true>;
template class Pool<0x80, true>;
template class Pool<0x08, false>;
template class Pool<0x10, false>;
template class Pool<0x18, false>;
template class Pool<0x20, false>;
template class Pool<0x28, false>;
template class Pool<0x30, false>;
template class Pool<0x38, false>;
template class Pool<0x40, false>;
template class Pool<0x48, false>;
template class Pool<0x50, false>;
template class Pool<0x58, false>;
template class Pool<0x60, false>;
template class Pool<0x68, false>;
template class Pool<0x70, false>;
template class Pool<0x78, false>;
template class Pool<0x80, false>;