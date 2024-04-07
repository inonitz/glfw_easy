#pragma once
#include "dense_grid_iterator.hpp"


struct dense_grid
{
public:
    dense_grid() : m_data(nullptr) {}

    
    void create(
        std::vector<Particle>& initialData,
        u32 gridWidth,
        u32 gridHeight,
        u32 gridUnitLength
    );
    void update();
    void destroy();

    
    __force_inline void updateInitialDataBuffer(std::vector<Particle>& to_update)
    {
        /* We Assume to_update is BIG ENOUGH to hold every entry in sorted_indices */
        for(size_t i = 0; i < m_sortedIndices.size(); ++i) {
            to_update[i] = m_data->data()[m_sortedIndices[i]];
        }
        return;
    }


    auto as_blocks() const { 
        return grid_iterator_proxy_container(
            m_data,
            m_sortedIndices,
            m_indices,
            m_activeIndices,
            1+m_activeIndicesSize
        );
    }
    auto as_particles() const { 
        return particle_iterator_proxy_container(
            m_data,
            m_sortedIndices,
            m_indices,
            m_activeIndices,
            1+m_activeIndicesSize
        );
    }

private:
    /* m_data has to be kept sorted to improve locality for countOccurances() */
    /* Need to find out how to improve locality on populateDenseArray() */
    ParticleBuffer*  m_data;
    std::vector<u16> m_sortedIndices;
    std::vector<u16> m_indices;
    std::vector<u16> m_activeIndices; /* m_activeIndices[0] is a dud; vector starts at index 1->n */
    u32 m_activeIndicesSize;
    u32 m_width, m_height;
    f32 m_unitInvLen;

    // grid_iterator_proxy_container     local_grid_iterator{
    //         m_data,
    //         m_sortedIndices,
    //         m_indices,
    //         m_activeIndices,
    //         1+m_activeIndicesSize
    // };
    // particle_iterator_proxy_container local_particle_iterator;


    void countOccurences();
    void computePartialSums();
    void populateDenseArray();
    void findActiveIndices();
};