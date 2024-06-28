#pragma once
#include "dense_grid_iterator2.hpp"
#include "common.hpp"
#include "util/allocator.hpp"


struct sim_grid
{
public:
    sim_grid() : m_data{nullptr} {}

    
    void create(
        ParticleBuffer* const initialData,
        u32 gridWidth,
        u32 gridHeight,
        u32 gridUnitLength
    );
    void update();
    void destroy();
    void print(bool less_verbose);
    void print_sortedIndices();
    

    __force_inline void     uploadSortedParticleData(std::vector<ParticleData>& to_update)
    {
        /* We Assume to_update is BIG ENOUGH to hold every entry in sorted_indices */
        for(size_t i = 0; i < m_sortedIndices.size(); ++i) {
            std::memcpy(&to_update[i].pos, &m_data->data()[m_sortedIndices[i]], sizeof(ParticleData));
        }
        return;
    }


    auto& as_particles() const { return m_iterator_pair->second; }
    auto& as_blocks()    const { return m_iterator_pair->first;  }
private:
    /* m_data has to be kept sorted to improve locality for countOccurances() */
    /* Need to find out how to improve locality on populateDenseArray() */
    ParticleBuffer* m_data;
    std::vector<u16> m_sortedIndices;
    std::vector<u16> m_indices;
    std::vector<u16> m_activeIndices; /* m_activeIndices[0] is a dud; vector starts at index 1->n */
    u32 m_activeIndicesSize;
    u32 m_width, m_height;
    f32 m_unitInvLen;
// private:
    using iter_pair = std::pair<grid_iterator_proxy_container, particle_iterator>;
    using iterptr_pair = std::pair<grid_iterator_proxy_container, particle_iterator>;
    StaticPoolAllocator<iter_pair> m_iter_alloc;
    iter_pair* m_iterator_pair = nullptr;


    void countOccurences(std::vector<i32>& flattenIndex);
    void computePartialSums();
    void populateDenseArray_getActiveIndices(std::vector<i32>& flattenIndex);
#ifdef __rdirprintf
    void test_countOccurences(std::vector<math::vec2i>& indices);
    void test_computePartialSums();
    void test_populateDenseArray_getActiveIndices(std::vector<math::vec2i>& indices);
    void test_print();
    void test_update();

#endif
};