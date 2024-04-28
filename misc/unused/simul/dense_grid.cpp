#include "dense_grid.hpp"


void dense_grid::create(
    ParticleBuffer& initialData,
    u32 gridWidth,
    u32 gridHeight,
    u32 gridUnitLength
) {
    m_data = &initialData;
    m_width  = gridWidth;
    m_height = gridHeight;
    m_unitInvLen = 1.0f / __scast(f32, gridUnitLength);
    iter_alloc.create(4);

    update();
    return;
}


void dense_grid::update()
{
    if(m_data->size() != m_sortedIndices.size()) {
        m_sortedIndices.assign(m_data->size(), 0);
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
    print();
    ifcrash(true);
    markfmt("%p | %p", (void*)grid_iter, (void*)particle_iter);
    if(grid_iter == nullptr || particle_iter == nullptr) {
        auto itpair = iter_alloc.allocate();
        grid_iter     = &itpair->first;
        particle_iter = &itpair->second;

        mark(); grid_iter->create(&m_sortedIndices, &m_indices, &m_activeIndices);
        mark(); particle_iter->create(m_data, &m_sortedIndices, &m_indices, &m_activeIndices, 1);
    }
    mark();
    return;
}


void dense_grid::destroy() {
    m_sortedIndices.resize(0);
    m_indices.resize(0);
    m_activeIndices.resize(0);
    m_activeIndicesSize = 0;
    m_height = 0;
    m_width  = 0;
    m_unitInvLen = 0;
    iter_alloc.destroy();
}


void dense_grid::print()
{
    u32 begin, end, countParticles = 0;
    for(size_t active = 1; active < m_activeIndicesSize; ++active) {
        begin = m_activeIndices[active];
        end   = m_activeIndices[active] + 1;

        printf("[%4llu] <ai>%4u <i> %4u->%4u { ", active, begin, m_indices[begin], m_indices[end]);
        for(size_t b = m_indices[begin]; b <= m_indices[end]; ++b) {
            u16 sorted_index = m_sortedIndices[b]; /* use this to access m_data */

            printf("$%4u ", sorted_index);
            ++countParticles;
        }
        // countParticles += m_indices[end] - m_indices[begin] + 1;
        printf(" }\n");
    }
    markfmt("<ai> %llu <i> %llu <si> %llu", m_activeIndices.size(), m_indices.size(), m_sortedIndices.size());
    markfmt("Counted total of %u ?= %llu", countParticles, m_data->size());
}




void dense_grid::countOccurences()
{
    math::vec2i index{0, 0};
    for(size_t i = 0; i < m_data->size(); ++i) {
        index = math::vec2i{ m_unitInvLen * m_data->data()[i].pos };
        ++m_indices[index.j + index.i * m_width];
        // markfmt("data size is %llu | pos is (%d, %d) | (width, height) = (%u, %u)", m_data->size(), index.i, index.j, m_width, m_height );
    }
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
        index = math::vec2i{ m_unitInvLen * m_data->data()[i].pos };
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
    m_activeIndices.resize(m_activeIndicesSize);


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