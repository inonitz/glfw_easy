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
        if(m_data.size() != m_sortedData.size()) {
            m_sortedData.resize(m_data.size());
        }
        m_indices.assign(0, m_width * m_height + 1);
        

        /* Count all entries for each index */
        /* Compute indices (partial sums) for each entry */
        /* 
            Place all particles in the dense array
            according to the indices computed at the previous comment
        */
        countOccurences();    
        computePartialSums();
        populateDenseArray();
    }


    void destroy() {
        m_sortedData.resize(0);
        m_indices.resize(0);
        m_height = 0;
        m_width  = 0;
    }


    auto& sorted_data() { return m_sortedData; }


private:
    /* m_data has to be kept sorted to improve locality for countOccurances() */
    /* Need to find out how to improve locality on populateDenseArray() */
    std::vector<Particle>& m_data;
    std::vector<Particle> m_sortedData;
    std::vector<u16>      m_indices;
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
    }


    void populateDenseArray()
    {
        math::vec2i index;
        u16& finalIndex = m_indices[0];
        for(auto& p : m_data) {
            index = math::vec2i{p.pos};
            finalIndex = m_indices[index.j + index.i * m_width]; 
            m_sortedData[finalIndex] = m_data[finalIndex];
            --finalIndex;
        }
        return;
    }
};