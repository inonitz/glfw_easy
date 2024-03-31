#pragma once
#include "util/base.hpp"
#include "util/vec.hpp"
#include <vector>


struct Particle
{
    math::vec2f pos;
    math::vec2f vel;
};


struct dense_grid
{
private:
    /* m_data has to be kept sorted to improve locality for countOccurances() */
    /* Need to find out how to improve locality on populateDenseArray() */
    std::vector<Particle>* m_data;
    std::vector<u16> m_sortedIndices;
    std::vector<u16> m_indices;
    std::vector<u16> m_activeIndices; /* m_activeIndices[0] is a dud; vector starts at index 1->n */
    u32 m_activeIndicesSize;
    u32 m_width, m_height;


    struct block_iterator
    {
        std::vector<Particle>&  m_pts;
        std::vector<u16> const& m_indexBuffer;
        const u16 m_indexBegin, m_indexEnd;

        block_iterator(
            std::vector<Particle>& the_particles, 
            std::vector<u16> const& indices_of_particles,
            u16 const begin, 
            u16 const end
        ) : m_pts(the_particles), 
            m_indexBuffer(indices_of_particles),
            m_indexBegin{begin}, 
            m_indexEnd{end}
        {}


        Particle& operator[](u16 which_particle_index) {
            return m_pts[ m_indexBuffer[ m_indexBegin + which_particle_index ]];
        }


        size_t size() const { return m_indexEnd - m_indexBegin + 1; }
    };


    class grid_block_iterator
    {
    private:
        std::vector<Particle>& m_particleData;
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
            std::vector<Particle>& m_particlesRef,
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
        std::vector<Particle>& m_actualParticles;
        const std::vector<u16> &m_particleIndexBuffer, &m_indexBuffer, &m_active;
        u32 m_activeBufferSize;
    public:
        grid_iterator_proxy_container(
            std::vector<Particle>&  m_particlesRef,
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
        std::vector<Particle>& m_actualParticles;
        const std::vector<u16>& m_particleIndexBuffer, &m_indexBuffer, &m_active;
        u32 m_activeIndicesLoop{1}, m_indexBufferEnd, m_currIndexBufferIndex;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = Particle;
        using pointer           = value_type*;
        using reference         = value_type&;


        particle_iterator(
            std::vector<Particle>&  m_particlesRef,
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
            return m_actualParticles[m_particleIndexBuffer[ 
                    m_currIndexBufferIndex 
                ]];
        }
        pointer   operator->() { 
            return &m_actualParticles[m_particleIndexBuffer[ 
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
        std::vector<Particle>& m_actualParticles;
        const std::vector<u16>& m_particleIndexBuffer, &m_indexBuffer, &m_active;
        u32 m_activeBufferSize;
    public:
        particle_iterator_proxy_container(
            std::vector<Particle>&  m_particlesRef,
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


public:
    void create(
        std::vector<Particle>& initialData,
        u32 gridWidth,
        u32 gridHeight
    ) {
        mark(); m_data = &initialData;
        mark(); m_width  = gridWidth;
        mark(); m_height = gridHeight;
        mark(); update();
        mark(); return;
    }
    
    
    void update()
    {
        if(m_data->size() != m_sortedIndices.size()) {
            m_sortedIndices.assign(m_data->size(), 0);
            m_activeIndices.assign(m_data->size(), 0);
        }
        m_indices.assign(m_width * m_height + 1, 0);

        /* Count all entries for each index */
        /* Compute indices (partial sums) for each entry */
        /* 
            Place all particles in the dense array
            according to the indices computed at the previous comment
        */
        countOccurences();
        computePartialSums();
        populateDenseArray();
        findActiveIndices();
        return;
    }



    void destroy() {
        m_sortedIndices.resize(0);
        m_indices.resize(0);
        m_activeIndices.resize(0);
        m_activeIndicesSize = 0;
        m_height = 0;
        m_width  = 0;
    }

    
    void updateInitialDataBuffer(std::vector<Particle>& to_update)
    {
        /* We Assume to_update is BIG ENOUGH to hold every entry in sorted_indices */
        for(size_t i = 0; i < m_sortedIndices.size(); ++i) {
            to_update[i] = (*m_data)[m_sortedIndices[i]];
        }
        return;
    }


    auto as_blocks() const { 
        return grid_iterator_proxy_container(
            *m_data,
            m_sortedIndices,
            m_indices,
            m_activeIndices,
            1+m_activeIndicesSize
        );
    }
    auto as_particles() const { 
        return particle_iterator_proxy_container(
            *m_data,
            m_sortedIndices,
            m_indices,
            m_activeIndices,
            1+m_activeIndicesSize
        );
    }


private:
    void countOccurences()
    {
        math::vec2i index;
        for(auto& p : *m_data) {
            index = math::vec2i{p.pos};
            ++m_indices[index.j + index.i * m_width];
        }
        return;
    }


    void computePartialSums()
    {
        u32 partialSum = 0;
        for(auto& index : m_indices) {
            partialSum += index;
            index = partialSum;
        }
        m_indices[m_height * m_width] = __scast(u16, partialSum);
        return;     
    }


    void populateDenseArray()
    {
        math::vec2i index;
        u16& finalIndex = m_indices[0];
        for(size_t i = 0; i < m_data->size(); ++i) {
            index = math::vec2i{ (*m_data)[i].pos};
            finalIndex = --m_indices[index.j + index.i * m_width];
            m_sortedIndices[finalIndex] = i;
        }
        return;
    }


    void findActiveIndices()
    {
        u32 push = 0, cond_idx = 0;
        for(size_t i = 0; i < m_indices.size() - 1; ++i) { /* find array size */
            push += (m_indices[i + 1] - m_indices[i] > 1);
        }
        m_activeIndicesSize = push;


        push = 0;
        for(size_t i = 0; i < m_indices.size() - 1; ++i) /* Populate indices in m_activeIndices[1->n] */
        {
            cond_idx = ( (m_indices[i + 1] - m_indices[i]) > 1) * (1 + push);
            /* if true: 
                    m_activeIndices[1 & onwards] will be set. 
                else: 
                    set m_activeIndices[0] 
            */
            m_activeIndices[cond_idx] = i;
            push += boolean(cond_idx);
        }
        return;
    }
};