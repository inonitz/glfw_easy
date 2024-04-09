#pragma once
#include "common_def.hpp"




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

    auto particles() {
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
    grid_iterator_proxy_container(
        std::vector<u16> const& si, 
        std::vector<u16> const& i,
        std::vector<u16> const& ai
    ) : sortedIndices(si), indices(i), activeIndices(ai) {}


    grid_iterator begin() { return grid_iterator{ sortedIndices, indices, activeIndices, 1 }; }
    grid_iterator end()   {
        markfmt("si(%u)",  sortedIndices[0]);
        markfmt("i(%u)",   indices[0]);
        markfmt("ai(%u)",  activeIndices[0]);
        markfmt("ais(%u)", activeIndices.size());

        auto val = grid_iterator{ 
            sortedIndices, 
            indices, 
            activeIndices, 
            __scast(u16, activeIndices.size()) 
        };
        return val; 
    }
private:
    std::vector<u16> const& sortedIndices, &indices, &activeIndices;
};




class particle_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = Particle;
    using pointer           = value_type*;
    using reference         = value_type&;

    particle_iterator(
        ParticleBuffer& p,
        std::vector<u16> const& si, 
        std::vector<u16> const& i,
        std::vector<u16> const& ai,
        u16 active_indices_buffer_offset
    ) : m_actualParticles{p}, 
        sortedIndices(si), 
        indices(i), 
        activeIndices(ai), 
        m_activeIndicesOffset{active_indices_buffer_offset}
    {
        indicesOffset = indices[ activeIndices[m_activeIndicesOffset]    ];
        indicesEnd    = indices[ activeIndices[m_activeIndicesOffset] + 1];
    }


    reference operator*()  { return  m_actualParticles[ sortedIndices[indicesOffset] ]; }
    pointer   operator->() { return &m_actualParticles[ sortedIndices[indicesOffset] ]; }
    reference operator++() {
        markfmt("aio(%u) => io(%u) ie(%u)", m_activeIndicesOffset, indicesOffset, indicesEnd);
        auto& val = m_actualParticles[ sortedIndices[indicesOffset] ];
        ++indicesOffset;
        if(indicesOffset == indicesEnd) {
            ++m_activeIndicesOffset;
            indicesOffset = indices[ activeIndices[m_activeIndicesOffset]    ];
            indicesEnd    = indices[ activeIndices[m_activeIndicesOffset] + 1];
        }
        return val;
    }
    
    friend bool operator==(const particle_iterator& a, const particle_iterator& b) { 
        return a.m_activeIndicesOffset == b.m_activeIndicesOffset; 
    }
    friend bool operator!=(const particle_iterator& a, const particle_iterator& b) { 
        return a.m_activeIndicesOffset != b.m_activeIndicesOffset; 
    }
private:
    ParticleBuffer& m_actualParticles;
    std::vector<u16> const& sortedIndices, &indices, &activeIndices;
    u16 m_activeIndicesOffset, indicesOffset;
    u16 indicesEnd;
};


class grid_particle_iterator_proxy_container
{
public:
    grid_particle_iterator_proxy_container(
        ParticleBuffer& p,
        std::vector<u16> const& si,
        std::vector<u16> const& i,
        std::vector<u16> const& ai
    ) : m_actualParticles{p}, sortedIndices(si), indices(i), activeIndices(ai) {}

    particle_iterator begin() const { 
        auto ret = particle_iterator(m_actualParticles, sortedIndices, indices, activeIndices, 1); 
        return ret;
    }
    particle_iterator end()   const { 
        auto ret = particle_iterator(m_actualParticles, sortedIndices, indices, activeIndices, __scast(u16, activeIndices.size()) ); 
        return ret;
    }

    auto const& data() { return m_actualParticles; }
private:
    ParticleBuffer& m_actualParticles;
    const std::vector<u16>& sortedIndices, &indices, &activeIndices;
};
