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
public:
    void create(
        std::vector<Particle>& initialData,
        u32 gridWidth,
        u32 gridHeight
    ) {
        m_data = initialData;
        m_width  = gridWidth;
        m_height = gridHeight;
        update();
        return;
    }
    
    
    void update()
    {
        if(m_data.size() != m_sortedIndices.size()) {
            m_sortedIndices.assign(m_data.size(), 0);
            m_activeIndices.assign(m_data.size(), 0);
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
        u32 push = 0;
        for(size_t i = 0; i < m_sortedIndices.size(); ++i) {
            to_update[i] = m_data[m_sortedIndices[i]];
        }
        return;
    }


    auto&       sorted_data()         { return m_sortedIndices; }
    auto const& hash_table()          { return m_indices;       }
    auto const& filtered_hash_table() { return m_activeIndices; }

private:
    /* m_data has to be kept sorted to improve locality for countOccurances() */
    /* Need to find out how to improve locality on populateDenseArray() */
    std::vector<Particle>& m_data;
    std::vector<u16> m_sortedIndices;
    std::vector<u16> m_indices;
    std::vector<u16> m_activeIndices; /* m_activeIndices[0] is a dud; vector starts at index 1->n */
    u32 m_activeIndicesSize;
    u32 m_width, m_height;


    void countOccurences()
    {
        math::vec2i index;
        for(auto& p : m_data) {
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
        for(size_t i = 0; i < m_data.size(); ++i) {
            index = math::vec2i{m_data[i].pos};
            finalIndex = --m_indices[index.j + index.i * m_width];
            m_sortedIndices[finalIndex] = i;
        }
        return;
    }


    void findActiveIndices()
    {
        u32 push = 0, cond = 0;
        for(size_t i = 0; i < m_indices.size() - 1; ++i) { /* find array size */
            push += (m_indices[i + 1] - m_indices[i] > 1);
        }
        m_activeIndicesSize = push;


        for(size_t i = 0; i < m_indices.size() - 1; ++i) /* Populate indices in m_activeIndices[1->n] */
        {
            cond = (m_indices[i + 1] - m_indices[i] > 1) * (1 + push);
            /* if true: 
                    m_activeIndices[1 & onwards] will be set. 
                else: 
                    set m_activeIndices[0] 
            */
            m_activeIndices[cond] = i;
            push += boolean(cond);
        }
        return;
    }
};