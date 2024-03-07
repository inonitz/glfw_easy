#include "hash_grid.hpp"
#include <cmath>


void HashGrid::create(
    u32 width, 
    u32 height,
    f32 blockVolume, 
    f32 particleVolume,
    u32 particleDataBytes
) {
    u32 maxParticlesPerBlock = __scast(u32, std::floorf(blockVolume / particleVolume) );
    m_width = width;
    m_height = height;
    m_maxParticlesPerBlock = maxParticlesPerBlock;
    m_elementSize = particleDataBytes;
    /*
        Each element in the allocator is an array T[nearestMultiple(width * height, maxParticlesPerBlock) / maxParticlesPerBlock],
        where T is the datatype for the particle
    */
    m_alloc.create(m_width * m_height, m_elementSize * m_maxParticlesPerBlock);
    m_grid.assign(m_width * m_height, { nullptr, 0 });
}


void HashGrid::destroy()
{
    m_activeBlocks.clear();
    m_grid.clear();
    m_alloc.destroy();
    m_width = 0;
    m_height = 0;
    m_maxParticlesPerBlock = 0;
    m_elementSize = 0;
    return;
}


bool HashGrid::insert(u32 i, u32 j, u8* particleData)
{
    gridBlock grid_block = m_grid[j + i * m_width];
    u8* chosenBlock = nullptr;
    
    if(unlikely(grid_block.m_size == m_maxParticlesPerBlock))
        return false;
    
    if(unlikely(grid_block.m_data == nullptr)) {
        grid_block.m_data = __rcast(u8*, m_alloc.allocate());
        m_activeBlocks.push_back(j + i * m_width);
        std::sort( /* Keep vector sorted such that memory accesses will ALWAYS be consecutive in memory */
            m_activeBlocks.begin(), 
            m_activeBlocks.end(), 
            [](u32 a, u32 b) -> bool { return a < b; }
        );
    }
    chosenBlock = &grid_block.m_data[grid_block.m_size * m_elementSize];
    ++grid_block.m_size;
    std::memcpy(chosenBlock, particleData, m_elementSize);
    return true;
}


void HashGrid::clear()
{
    for(auto& active : m_activeBlocks) {
        m_alloc.free(m_grid[active].m_data);
        m_grid[active] = { nullptr, 0 };
    }
    m_activeBlocks.clear();
}