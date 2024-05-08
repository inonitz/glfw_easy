#include "dense_grid.hpp"
#include "util/marker.hpp"


void dense_grid::create(
    ParticleBuffer* const initialData,
    u32 gridWidth,
    u32 gridHeight,
    u32 gridUnitLength
) {
    m_data = initialData;
    m_width  = gridWidth;
    m_height = gridHeight;
    m_unitInvLen = 1.0f / __scast(f32, gridUnitLength);
    m_iter_alloc.create(4);

    update();
    return;
}


void dense_grid::update()
{
    __rdirprintf("dense_grid::update()::begin() ... ");
    static std::vector<i32> tmp_postoidx;
    tmp_postoidx.resize(m_data->size());
    
    if(m_data->size() != m_sortedIndices.size()) {
        m_sortedIndices.assign(m_data->size(), 0);
    }
    m_indices.assign(m_width * m_height + 1, 0);


    countOccurences(tmp_postoidx);
    computePartialSums();
    populateDenseArray_getActiveIndices(tmp_postoidx);
    auto* new_iterpair = m_iter_alloc.allocate();
    if(m_iterator_pair != nullptr) {
        m_iter_alloc.free(m_iterator_pair);
    }
    m_iterator_pair = new_iterpair;
    m_iterator_pair->first.create(&m_sortedIndices, &m_indices, &m_activeIndices);
    m_iterator_pair->second.create(m_data, &m_sortedIndices, &m_indices, &m_activeIndices, 1);
    __rdirprintf("dense_grid::update()::end()\n");
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
    m_iter_alloc.destroy();
}


void dense_grid::print()
{
    u32 begin, end, __debug_unused countParticles = 0;
    for(size_t active = 1; active <= m_activeIndicesSize; ++active) {
        begin = m_indices[m_activeIndices[active]    ];
        end   = m_indices[m_activeIndices[active] + 1];

        __rdirprintf("[%4llu] <ai>%4u <i> %4u->%4u (%u) { ", active, m_activeIndices[active], begin, end, end-begin);
        for(size_t b = begin; b < end; ++b) {
            __rdirprintf("$%4u ", m_sortedIndices[b]);
            ++countParticles;
        }
        __rdirprintf(" }\n");
    }
    markfmt("<ai> %llu <i> %llu <si> %llu\n", m_activeIndices.size(), m_indices.size(), m_sortedIndices.size());
    markfmt("Counted total of %u ?= %llu\n", countParticles, m_data->size());
    return;
}


void dense_grid::print_sortedIndices()
{
    u32 begin, end;
    __rdirprintf("dense_grid::print_sortedIndices()::begin() ");
    for(size_t active = 1; active <= m_activeIndicesSize; ++active) {
        begin = m_indices[m_activeIndices[active]    ];
        end   = m_indices[m_activeIndices[active] + 1];

        for(size_t b = begin; b < end; ++b) {
            __rdirprintf("%u ", m_sortedIndices[b]);
        }
    }
    return;
}




void dense_grid::countOccurences(std::vector<i32>& flattenIndex)
{
    const math::vec2i trunc1d = { __scast(i32, m_width), 1 };
    math::vec2i currpos;
    for(size_t i = 0; i < m_data->size(); ++i) {
        currpos = math::vec2i{ m_unitInvLen *  m_data->data()[i].pos };
        flattenIndex[i] = math::dot(currpos, trunc1d);
        ++m_indices[flattenIndex[i]];
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


void dense_grid::populateDenseArray_getActiveIndices(std::vector<i32>& flattenIndex)
{
    u32 diff = 0, countDiff = 0;
    u16 finalIndex;
    for(size_t i = 0; i < m_data->size(); ++i) {
        finalIndex = --m_indices[ flattenIndex[i] ];
        m_sortedIndices[finalIndex] = i;
    }


    for(size_t i = 0; i < m_indices.size() - 1; ++i) { /* find m_activeIndices size */
        diff = m_indices[i + 1] - m_indices[i];
        countDiff += boolean(diff);
    }
    m_activeIndicesSize = countDiff;
    m_activeIndices.resize(m_activeIndicesSize + 1);
    countDiff = 0;
    for(size_t i = 0; i < m_indices.size() - 1; ++i) { /*  m_activeIndices ranges from [1, n], NOT [0, n-1] */
        if(m_indices[i + 1] - m_indices[i]) {
            m_activeIndices[++countDiff] = i;
        }
    }
    return;
}




void dense_grid::test_countOccurences(std::vector<math::vec2i>& indices)
{
    const math::vec2i trunc1d = { __scast(i32, m_width), 1 };
    math::vec2f temp;
    for(size_t i = 0; i < m_data->size(); ++i) {
        indices[i] = math::vec2i{ m_unitInvLen *  m_data->data()[i].pos };
        ++m_indices[ math::dot(indices[i], trunc1d) ];

        // if(index.i >= 100 || index.j >= 100) {
        //     auto* pstr = m_data->data()[i].pos.to_string(), *iptr = index.to_string();
        //     printf("[%3llu] %s | %s\n", i, pstr, iptr);
        //     free(pstr);
        //     free(iptr);
        // }
        
        // markfmt("data size is %llu | pos is (%d, %d) | (width, height) = (%u, %u)", m_data->size(), index.i, index.j, m_width, m_height );
    }

    auto __debug_unused countIndices = 0;
    for(size_t i = 0; i < m_indices.size(); ++i) {
        countIndices += m_indices[i];
    }
    markfmt("countOccurances() ==> Counted total of %u ?= %llu", countIndices, m_data->size());
    return;
}


void dense_grid::test_computePartialSums()
{
    u32 partialSum = 0;
    for(auto& index : m_indices) {
        partialSum += index;
        index = partialSum;
    }
    m_indices[m_height * m_width] = __scast(u16, partialSum);
    markstr("computePartialSums() ==> Final m_indices {\n");
    for(size_t i = 0; i < m_indices.size(); ++i) {
        if((i & 15) == 0) __rdirprintf("\n    ");   
        __rdirprintf("%4u, ", m_indices[i]);
    }
    __rdirprintf("}\n");
    return;
}


void dense_grid::test_populateDenseArray_getActiveIndices(std::vector<math::vec2i>& indices)
{
    const math::vec2i trunc1d = { __scast(i32, m_width), 1 };
    u16 finalIndex;
    for(size_t i = 0; i < m_data->size(); ++i) {
        finalIndex = --m_indices[ math::dot(indices[i], trunc1d) ];
        m_sortedIndices[finalIndex] = i;
    }

    u32 __debug_unused sum = 0;
    u32 countDiff = 0;
    u32 diff = 0;
    for(size_t i = 0; i < m_indices.size() - 1; ++i) { /* find array size */
        diff = m_indices[i + 1] - m_indices[i];
        sum += diff;
        countDiff += boolean(diff);
        // if(diff) {
        //     markfmt("found diff at %3llu[=%u] ==> { %3hu %3hu }", i, diff, m_indices[i + 1], m_indices[i]);
        //     ++countDiff;
        // }
    }
    markfmt("populateDenseArray() ==> countDiff arrived at %u", countDiff);
    m_activeIndicesSize = countDiff;
    m_activeIndices.resize(m_activeIndicesSize + 1);
    countDiff = 0;
    for(size_t i = 0; i < m_indices.size() - 1; ++i) { /* find array size */
        // diff = m_indices[i + 1] - m_indices[i];
        if(m_indices[i + 1] - m_indices[i]) {
            m_activeIndices[++countDiff] = i;
        }
    }
    
    markfmt("populateDenseArray() ==> Counted total of %u ?= %llu", sum, m_data->size());
    markfmt("populateDenseArray() ==> countDiff arrived at %u", countDiff);
    markstr("populateDenseArray() ==> Final m_sortedIndices {");
    for(size_t i = 0; i < m_sortedIndices.size(); ++i) {
        if((i & 15) == 0) __rdirprintf("\n    ");   
        __rdirprintf("%4u, ", m_sortedIndices[i]);
    }
    __rdirprintf("}\n");
    return;
}


void dense_grid::test_print()
{
    u32 begin, end, __debug_unused countParticles = 0;
    for(size_t active = 1; active <= m_activeIndicesSize; ++active) {
        begin = m_indices[m_activeIndices[active]    ];
        end   = m_indices[m_activeIndices[active] + 1];

        __rdirprintf("[%4llu] <ai>%4u <i> %4u->%4u { ", active, begin, m_indices[begin], m_indices[end]);
        for(size_t b = begin; b < end; ++b) {
            __rdirprintf("$%4u ", m_sortedIndices[b]);
            ++countParticles;
        }
        // countParticles += m_indices[end] - m_indices[begin] + 1;
        __rdirprintf(" }\n");
    }
    markfmt("<ai> %llu <i> %llu <si> %llu", m_activeIndices.size(), m_indices.size(), m_sortedIndices.size());
    markfmt("Counted total of %u ?= %llu", countParticles, m_data->size());
}


void dense_grid::test_update()
{
    mark(); if(m_data->size() != m_sortedIndices.size()) {
        m_sortedIndices.assign(m_data->size(), 0);
    }
    mark(); m_indices.assign(m_width * m_height + 1, 0);

    /* Count all entries for each index */
    /* Compute indices (partial sums) for each entry */
    /* 
        Place all particles in the dense array
        according to the indices computed at the previous comment
    */
    static std::vector<math::vec2i> tmp_postoidx;
    tmp_postoidx.resize(m_data->size());
    mark(); test_countOccurences(tmp_postoidx);
    mark(); test_computePartialSums();
    mark(); test_populateDenseArray_getActiveIndices(tmp_postoidx);
    mark(); test_print();
    markfmt("%p\n", (void*)m_iterator_pair);
    mark(); auto* new_iterpair = m_iter_alloc.allocate();
    if(m_iterator_pair != nullptr) {
        mark(); m_iter_alloc.free(m_iterator_pair);
    }
    mark(); m_iterator_pair = new_iterpair;
    mark(); m_iterator_pair->first.create(&m_sortedIndices, &m_indices, &m_activeIndices);
    mark(); m_iterator_pair->second.create(m_data, &m_sortedIndices, &m_indices, &m_activeIndices, 1);
    mark();
    return;
}