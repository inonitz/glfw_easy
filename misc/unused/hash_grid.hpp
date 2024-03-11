#ifndef __HASH_GRID__
#define __HASH_GRID__
#include "util/base.hpp"
#include <vector>
#include <util/allocator.hpp>


struct HashGrid
{
public:
    typedef struct __particle_array
    {
        u8* m_data;
        u32 m_size;
    } gridBlock;
public:
    void create(
        u32 width, 
        u32 height,
        f32 blockVolume, 
        f32 particleVolume,
        u32 particleDataBytes
    );
    void destroy();


    bool insert(u32 i, u32 j, u8* particleData);
    void clear();

    std::vector<u32> const&       getActiveIndices() const { return m_activeBlocks; }
    std::vector<gridBlock> const& getGrid()          const { return m_grid;         }
private:
    using underlying_mem = PoolAllocator<>;


    u32 m_width;
    u32 m_height;
    u32 m_maxParticlesPerBlock;
    u32 m_elementSize;
    
    underlying_mem         m_alloc;
    std::vector<gridBlock> m_grid; /* array of buckets, each bucket contains a ParticleData __data[m_maxParticlesPerBlock]. */
    std::vector<u32>       m_activeBlocks; /* keeps track of which buckets have been used, to reduce access times to particles */
};




#endif