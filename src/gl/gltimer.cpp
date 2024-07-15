#include "gltimer.hpp"
#include <glbinding/gl/functions.h>
#include <glbinding/gl/enum.h>


namespace globj {


template<u32 querySwapSize> void Timer<querySwapSize>::create()
{
    gl::glCreateQueries(gl::GL_TIMESTAMP, querySwapSize << 1, &buf[0].query[0]);
    return;
}

template<u32 querySwapSize> void Timer<querySwapSize>::destroy()
{
    gl::glDeleteQueries(querySwapSize << 1, &buf[0].query[0]);
    return;
}


template<u32 querySwapSize> void Timer<querySwapSize>::startQuery() 
{
    /* query current elapsed time on gpu into q0 [the 'backbuffer'] (q1 is the 'frontbuffer') */
    gl::glBeginQuery(gl::GL_TIME_ELAPSED, buf[swapIndex].query[0]);
    return;
}

template<u32 querySwapSize> void Timer<querySwapSize>::endQuery()
{
    gl::glEndQuery(gl::GL_TIME_ELAPSED);
    prevFrame = nextFrame;
    gl::glGetQueryObjectui64v(
        buf[swapIndex].query[1], 
        gl::GL_QUERY_RESULT, 
        &nextFrame
    );
    return;
}


template void Timer<2>::create();
template void Timer<2>::destroy();
template void Timer<2>::startQuery();
template void Timer<2>::endQuery();
template void Timer<4>::create();
template void Timer<4>::destroy();
template void Timer<4>::startQuery();
template void Timer<4>::endQuery();
template void Timer<8>::create();
template void Timer<8>::destroy();
template void Timer<8>::startQuery();
template void Timer<8>::endQuery();


} // namespace globj
/* 
    int done = 0;
    while(!done) {
        glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done);
    }
    [NOTE]:
    we're not waiting for a query to be available for performance sake =>
    calling GL_QUERY_RESULT_AVAILABLE flushes the pipeline & waits for everything to sync,
    which is probably not good (lol)
    Therefore, we'll query multiple objects every frame
    such that a result will MOST LIKELY be available
*/