#pragma once
#include "common_def.hpp"


struct block_iterator
{
    ParticleBuffer* const   m_pts;
    std::vector<u16> const& m_indexBuffer;
    const u16 m_indexBegin, m_indexEnd;

    block_iterator(
        ParticleBuffer*  const  the_particles, 
        std::vector<u16> const& indices_of_particles,
        u16 const begin, 
        u16 const end
    ) : m_pts(the_particles), 
        m_indexBuffer(indices_of_particles),
        m_indexBegin{begin}, 
        m_indexEnd{end}
    {}


    Particle& operator[](u16 which_particle_index) {
        return m_pts->data()[ m_indexBuffer[ m_indexBegin + which_particle_index ]];
    }


    size_t size() const { return m_indexEnd - m_indexBegin + 1; }
};


class grid_block_iterator
{
private:
    ParticleBuffer* const m_particleData;
    std::vector<u16> const& m_particleIndex, &m_indexBuffer, &m_active;
    u32 m_activeIndices_off{1};

    /* m_particleData[ m_particleIndex[ m_indexBuffer[ m_active[m_activeIndices_off] ]]] */
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = grid_block_iterator;
    using pointer           = value_type*;
    using reference         = value_type&;


    grid_block_iterator(
        ParticleBuffer*  const  m_particlesRef,
        std::vector<u16> const& m_particleIndicesRef,
        std::vector<u16> const& m_indicesRef,
        std::vector<u16> const& active_indices, 
        u32 offset_in_active_indices
    ) : m_particleData(m_particlesRef),
        m_particleIndex(m_particleIndicesRef),
        m_indexBuffer(m_indicesRef),
        m_active(active_indices), 
        m_activeIndices_off{offset_in_active_indices}
    {}


    reference operator*()  { return *this; }
    pointer   operator->() { return this;  }
    grid_block_iterator& operator++() { ++m_activeIndices_off; return *this; }


    block_iterator get_particles()
    {
        return block_iterator(
            m_particleData,
            m_particleIndex,
            m_indexBuffer[ m_active[m_activeIndices_off]     ],
            m_indexBuffer[ m_active[m_activeIndices_off] + 1 ]
        );
    }
    friend bool operator==(const grid_block_iterator& a, const grid_block_iterator& b) { return a.m_activeIndices_off == b.m_activeIndices_off; };
    friend bool operator!=(const grid_block_iterator& a, const grid_block_iterator& b) { return a.m_activeIndices_off != b.m_activeIndices_off; };  
};


class grid_iterator_proxy_container
{
private:
    ParticleBuffer* const m_actualParticles;
    const std::vector<u16> &m_particleIndexBuffer, &m_indexBuffer, &m_active;
    u32 m_activeBufferSize;
public:
    grid_iterator_proxy_container(
        ParticleBuffer* const  m_particlesRef,
        std::vector<u16> const& m_particleIndicesRef,
        std::vector<u16> const& m_indicesRef,
        std::vector<u16> const& active_indices,
        u32 activeIndicesBufferSize
    ) : 
        m_actualParticles(m_particlesRef),
        m_particleIndexBuffer(m_particleIndicesRef),
        m_indexBuffer(m_indicesRef),
        m_active(active_indices),
        m_activeBufferSize{activeIndicesBufferSize}
    {}

    grid_block_iterator begin() const { return grid_block_iterator(m_actualParticles, m_particleIndexBuffer, m_indexBuffer, m_active, 1); }
    grid_block_iterator end()   const { return grid_block_iterator(m_actualParticles, m_particleIndexBuffer, m_indexBuffer, m_active, m_activeBufferSize); }
};




class particle_iterator
{
private:
    ParticleBuffer* const m_actualParticles;
    const std::vector<u16>& m_particleIndexBuffer, &m_indexBuffer, &m_active;
    u32 m_activeIndicesLoop{1}, m_indexBufferEnd, m_currIndexBufferIndex;

public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = Particle;
    using pointer           = value_type*;
    using reference         = value_type&;


    particle_iterator(
        ParticleBuffer* const m_particlesRef,
        std::vector<u16> const& m_particleIndicesRef,
        std::vector<u16> const& m_indicesRef,
        std::vector<u16> const& active_indices,
        u32 m_whichActiveIndex
    ) : 
        m_actualParticles(m_particlesRef),
        m_particleIndexBuffer(m_particleIndicesRef),
        m_indexBuffer(m_indicesRef),
        m_active(active_indices),
        m_activeIndicesLoop{m_whichActiveIndex} 
    {
        m_currIndexBufferIndex = m_indexBuffer[ m_active[m_activeIndicesLoop]     ];
        m_indexBufferEnd       = m_indexBuffer[ m_active[m_activeIndicesLoop] + 1 ];
    }

    reference operator*()  { 
        return m_actualParticles->data()[m_particleIndexBuffer[ 
                m_currIndexBufferIndex 
            ]];
    }
    pointer   operator->() { 
        return &m_actualParticles->data()[m_particleIndexBuffer[ 
                m_currIndexBufferIndex 
            ]];
    }
    particle_iterator& operator++() { 
        if(m_currIndexBufferIndex < m_indexBufferEnd)
            ++m_currIndexBufferIndex;
        else {
            ++m_activeIndicesLoop;
            m_currIndexBufferIndex = m_indexBuffer[ m_active[m_activeIndicesLoop]     ];
            m_indexBufferEnd       = m_indexBuffer[ m_active[m_activeIndicesLoop] + 1 ];
        }
        return *this;
    }
    friend bool operator==(const particle_iterator& a, const particle_iterator& b) { return a.m_activeIndicesLoop == b.m_activeIndicesLoop; };
    friend bool operator!=(const particle_iterator& a, const particle_iterator& b) { return a.m_activeIndicesLoop != b.m_activeIndicesLoop; };
};


class particle_iterator_proxy_container
{
private:
    ParticleBuffer* const m_actualParticles;
    const std::vector<u16>& m_particleIndexBuffer, &m_indexBuffer, &m_active;
    u32 m_activeBufferSize;
public:
    particle_iterator_proxy_container(
        ParticleBuffer* const m_particlesRef,
        std::vector<u16> const& m_particleIndicesRef,
        std::vector<u16> const& m_indicesRef,
        std::vector<u16> const& active_indices,
        u32 activeIndicesBufferSize
    ) : 
        m_actualParticles(m_particlesRef),
        m_particleIndexBuffer(m_particleIndicesRef),
        m_indexBuffer(m_indicesRef),
        m_active(active_indices),
        m_activeBufferSize{activeIndicesBufferSize}
    {}

    particle_iterator begin() const { return particle_iterator(m_actualParticles, m_particleIndexBuffer, m_indexBuffer, m_active, 1); }
    particle_iterator end()   const { return particle_iterator(m_actualParticles, m_particleIndexBuffer, m_indexBuffer, m_active, m_activeBufferSize); }
};