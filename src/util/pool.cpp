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
template struct CommonPoolDef<0xc0>;
template struct CommonPoolDef<0x100>;
template struct CommonPoolDef<0x140>;
template struct CommonPoolDef<0x180>;
template struct CommonPoolDef<0x1c0>;
template struct CommonPoolDef<0x200>;


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


template class Pool<0x008, true>;
template class Pool<0x010, true>;
template class Pool<0x018, true>;
template class Pool<0x020, true>;
template class Pool<0x028, true>;
template class Pool<0x030, true>;
template class Pool<0x038, true>;
template class Pool<0x040, true>;
template class Pool<0x048, true>;
template class Pool<0x050, true>;
template class Pool<0x058, true>;
template class Pool<0x060, true>;
template class Pool<0x068, true>;
template class Pool<0x070, true>;
template class Pool<0x078, true>;
template class Pool<0x080, true>;
template class Pool<0x0c0, true>;
template class Pool<0x100, true>;
template class Pool<0x140, true>;
template class Pool<0x180, true>;
template class Pool<0x1c0, true>;
template class Pool<0x200, true>;
template class Pool<0x008, false>;
template class Pool<0x010, false>;
template class Pool<0x018, false>;
template class Pool<0x020, false>;
template class Pool<0x028, false>;
template class Pool<0x030, false>;
template class Pool<0x038, false>;
template class Pool<0x040, false>;
template class Pool<0x048, false>;
template class Pool<0x050, false>;
template class Pool<0x058, false>;
template class Pool<0x060, false>;
template class Pool<0x068, false>;
template class Pool<0x070, false>;
template class Pool<0x078, false>;
template class Pool<0x080, false>;
template class Pool<0x0c0, false>;
template class Pool<0x100, false>;
template class Pool<0x140, false>;
template class Pool<0x180, false>;
template class Pool<0x1c0, false>;
template class Pool<0x200, false>;