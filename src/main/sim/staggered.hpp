#ifndef __DEFINE_STAGGERED_GRID_DATA_STRUCTURE__
#define __DEFINE_STAGGERED_GRID_DATA_STRUCTURE__
#include "util/base.hpp"
#include "util/marker.hpp"
#include "util/vec.hpp"
#include <vector>


struct StaggeredGrid
{
    /*
        X component Shifted down half unit
        Y component shifted right half unit
    */
    std::array<std::vector<f32>, 2> b;
    StaggeredGrid() : b({}) {}


    void create(size_t width, size_t height)
    {
        // markfmt("(%llu, %llu) | %llu, %llu", 
        //     width, 
        //     height, 
        //     height * __scast(size_t, (width  + 1)),
        //     width  * __scast(size_t, (height + 1))
        // );
        auto size0 = height * __scast(size_t, (width  + 1)),
                size1 = width  * __scast(size_t, (height + 1));
        markfmt("%llu | %llu ", size0, size1);
        b[0].resize(size0);
        b[1].resize(size1);
        return;
    }


    void create_copy(StaggeredGrid const& grid) /* structure must not be created YET */
    {
        if(grid.b[0].size() == b[0].size() && grid.b[1].size() == b[1].size()) {
            std::memcpy(b[0].data(), grid.b[0].data(), sizeof(b[0].size()));
            std::memcpy(b[1].data(), grid.b[1].data(), sizeof(b[1].size()));
        }
        return;
    }


    void set(f32 value)
    {
        for(size_t i = 0; i < 2; ++i) {
            b[i].assign(b[i].size(), value);
        }
        return;
    }


    void destroy()
    {
        b[0].resize(0);
        b[1].resize(0);
        return;           
    }


#define STAGGERED_GRID_OPERATOR(name, __src0_src1__operation_expression, __f32_expression, __f32_scalar_op) \
    void name(StaggeredGrid const& grid) { /* Assuming sizes are same */ \
        static math::vec4f __src1, __src0; \
        for(u32 index = 0; index < 2; ++index) \
        { \
            u32 i = 0; \
            for(; i < b[index].size() / 4; ++i) { \
                memcpy(__src0.begin(), &grid.b[index][4 * i], sizeof(math::vec4f)); \
                memcpy(__src1.begin(),      &b[index][4 * i], sizeof(math::vec4f)); \
                __src0_src1__operation_expression; \
                memcpy(&b[index][4 * i], __src1.begin(), sizeof(math::vec4f)); \
            } \
            i *=4 ; \
            for(; i < b[index].size(); ++i) { \
                __f32_expression; \
            } \
        } \
        return; \
    } \
    void name(f32 val) { \
        for(u32 index = 0; index < 2; ++index) \
        { \
            for(u32 i = 0; i < b[index].size(); ++i) { \
                __f32_scalar_op; \
            } \
        } \
        return; \
    } \


STAGGERED_GRID_OPERATOR(sub, __src1 -= __src0, b[index][i] -= grid.b[index][i], b[index][i] -= val)
STAGGERED_GRID_OPERATOR(add, __src1 += __src0, b[index][i] += grid.b[index][i], b[index][i] += val)
STAGGERED_GRID_OPERATOR(mul, __src1 *= __src0, b[index][i] *= grid.b[index][i], b[index][i] *= val)
STAGGERED_GRID_OPERATOR(div, __src1 *= ( math::vec4f{1.0f} / __src0 ), b[index][i] *= (1.0f / grid.b[index][i]), b[index][i] *= (1.0f / val))


};

#endif