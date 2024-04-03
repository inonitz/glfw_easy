#include "dense_grid.hpp"


void dense_grid::create(
    ParticleBuffer& initialData,
    u32 gridWidth,
    u32 gridHeight
) {
    mark(); m_data = &initialData;
    mark(); m_width  = gridWidth;
    mark(); m_height = gridHeight;
    mark(); update();
    mark(); return;
}


void dense_grid::update()
{
    /* Problem is here, out of bounds array access for m_indices. */
    mark(); if(m_data->size() != m_sortedIndices.size()) {
        m_sortedIndices.assign(m_data->size(), 0);
        m_activeIndices.assign(m_data->size(), 0);
    }
    mark(); m_indices.assign(m_width * m_height + 1, 0);

    /* Count all entries for each index */
    /* Compute indices (partial sums) for each entry */
    /* 
        Place all particles in the dense array
        according to the indices computed at the previous comment
    */
    mark(); countOccurences();
    mark(); computePartialSums();
    mark(); populateDenseArray();
    mark(); findActiveIndices();
    mark(); return;
}


void dense_grid::destroy() {
    m_sortedIndices.resize(0);
    m_indices.resize(0);
    m_activeIndices.resize(0);
    m_activeIndicesSize = 0;
    m_height = 0;
    m_width  = 0;
}




void dense_grid::countOccurences()
{
    math::vec2i index{0, 0};
    for(size_t i = 0; i < m_data->size(); ++i) {
        index = math::vec2i{m_data->data()[i].pos};
        markfmt("data size is %llu | pos is (%d, %d) | (width, height) = (%u, %u)", m_data->size(), index.i, index.j, m_width, m_height );
        ++m_indices[index.j + index.i * m_width];        
    }
    mark(); 
    for(auto& p : *m_data) {
        index = math::vec2i{p.pos};
        ++m_indices[index.j + index.i * m_width];
    }
    mark(); 
    return;
}


void dense_grid::computePartialSums()
{
    u32 partialSum = 0;
    for(auto& index : m_indices) {
        partialSum += index;
        index = partialSum;
    }
    m_indices[m_height * m_width] = __scast(u16, partialSum);
    return;     
}


void dense_grid::populateDenseArray()
{
    math::vec2i index;
    u16& finalIndex = m_indices[0];
    for(size_t i = 0; i < m_data->size(); ++i) {
        index = math::vec2i{ m_data->data()[i].pos};
        finalIndex = --m_indices[index.j + index.i * m_width];
        m_sortedIndices[finalIndex] = i;
    }
    return;
}


void dense_grid::findActiveIndices()
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