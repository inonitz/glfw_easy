#pragma once
#include "util/base.hpp"
#include "util/marker.hpp"
#include "common.hpp"
#include <vector>


#define __ss(vector_ptr) vector_ptr->data()


struct grid_block 
{
    struct grid_block_iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type        = u16;
        using const_pointer     = const value_type*;
        using const_reference   = const value_type&;

        std::vector<u16> const& sortedIndices;
        u16 m_index;

        grid_block_iterator(
            std::vector<u16> const& si, 
            const u16 index
        ) : sortedIndices(si), m_index{index} {}

        value_type operator*()  {            return sortedIndices[m_index];  }
        value_type operator++() { ++m_index; return sortedIndices[m_index];  }
        friend bool operator==(const grid_block_iterator& a, const grid_block_iterator& b) { 
            return a.m_index == b.m_index;
        }
        friend bool operator!=(const grid_block_iterator& a, const grid_block_iterator& b) { 
            return a.m_index != b.m_index; 
        }
    };


    grid_block(
        std::vector<u16> const& si, 
        const u16 begin, 
        const u16 end
    ) : sortedIndices(si), m_begin{begin}, m_end{end} {}

    grid_block_iterator begin() { return grid_block_iterator{ sortedIndices, m_begin }; }
    grid_block_iterator end()   { return grid_block_iterator{ sortedIndices, m_end   }; }
    
private:
    std::vector<u16> const& sortedIndices;
    const u16 m_begin, m_end;
};


class grid_iterator 
{
public:
    using iterator_category = std::forward_iterator_tag;
    using pointer           = grid_iterator*;
    using reference         = grid_iterator&;
    

    grid_iterator(
        std::vector<u16> const& si, 
        std::vector<u16> const& i,
        std::vector<u16> const& ai,
        u16 activeIndicesBufferOffset
    ) : sortedIndices(si), indices(i), activeIndices(ai), m_activeIndicesOffset{activeIndicesBufferOffset} 
    {}


    reference operator*()  { return *this; }
    pointer   operator->() { return this;  }
    reference operator++() { ++m_activeIndicesOffset; return *this; }
    
    friend bool operator==(const grid_iterator& a, const grid_iterator& b) { 
        return a.m_activeIndicesOffset == b.m_activeIndicesOffset; 
    }
    friend bool operator!=(const grid_iterator& a, const grid_iterator& b) { 
        return a.m_activeIndicesOffset != b.m_activeIndicesOffset; 
    }

    auto particle_block() {
        return grid_block{
            sortedIndices,
            indices[ activeIndices[m_activeIndicesOffset]    ],
            indices[ activeIndices[m_activeIndicesOffset] + 1]
        };
    }
private:
    std::vector<u16> const& sortedIndices, &indices, &activeIndices;
    u16 m_activeIndicesOffset;
};


class grid_iterator_proxy_container
{
public:
    void create(
        std::vector<u16> const* si, 
        std::vector<u16> const* i,
        std::vector<u16> const* ai
    ) {
        sortedIndices = si;
        indices = i;
        activeIndices = ai;
    }


    grid_iterator begin() { return grid_iterator{ *sortedIndices, *indices, *activeIndices, 1 }; }
    grid_iterator end()   {
        markfmt("ais(%llu)", activeIndices->size());
        return grid_iterator{ *sortedIndices, *indices, *activeIndices, __scast(u16, activeIndices->size()) };
    }
private:
    std::vector<u16> const* sortedIndices, *indices, *activeIndices;
};




class particle_iterator
{
public:
    using value_type = ParticleData;
    using pointer    = value_type*;
    using reference  = value_type&;


    void create(
        ParticleBuffer* p,
        std::vector<u16> const* si, 
        std::vector<u16> const* i,
        std::vector<u16> const* ai,
        u16 active_indices_buffer_offset
    ) {
        m_actualParticles = p;
        sortedIndices     = si;
        indices           = i;
        activeIndices     = ai;
        m_activeIndicesOffset = active_indices_buffer_offset;
        indicesOffset = __ss(indices)[ __ss(activeIndices)[m_activeIndicesOffset]     ];
        indicesEnd    = __ss(indices)[ __ss(activeIndices)[m_activeIndicesOffset] + 1 ];
        return;
    }


    reference operator*()  { return __ss(m_actualParticles)[ __ss(sortedIndices)[indicesOffset] ]; }
    pointer   operator->() {
        // markfmt("si[io] = %u | io(%u)/sis(%llu)", __ss(sortedIndices)[indicesOffset], indicesOffset, sortedIndices->size());
        return &__ss(m_actualParticles)[ __ss(sortedIndices)[indicesOffset] ];
    }
    void operator++() {
        if(m_activeIndicesOffset == activeIndices->size())
            return;

        bool reached_end = (indicesOffset == indicesEnd); 
        if(reached_end)
        {
            ++m_activeIndicesOffset;
            indicesOffset = __ss(indices)[ __ss(activeIndices)[m_activeIndicesOffset]     ];
            indicesEnd    = __ss(indices)[ __ss(activeIndices)[m_activeIndicesOffset] + 1 ];
        }
        indicesOffset += __scast(u16, !(reached_end));
        return;
    }


    bool valid() const { return m_activeIndicesOffset != activeIndices->size(); }


private:
    ParticleBuffer* m_actualParticles;
    std::vector<u16> const* sortedIndices, *indices, *activeIndices;
    u16 m_activeIndicesOffset, indicesOffset;
    u16 indicesEnd;
};


#undef __ss