#ifndef __OPENGL_OBJECTS_TIMER__
#define __OPENGL_OBJECTS_TIMER__
#include "util/types.hpp"


namespace globj {


/* Implemented with help from https://www.lighthouse3d.com/tutorials/opengl-timer-query/ */
template<u32 QueryCountForSwapping = 2> struct Timer 
{
public:
    void create();
	void destroy();
    void startQuery();
    void endQuery();
	void nextQuery()
    {
		++swapIndex;
		swapIndex *= (swapIndex != QueryCountForSwapping); /* if not equal, keep swapIdx; else set 0 */
		return;
	}
	u64 currentFrameCount()  const { return nextFrame; }
	u64 previousFrameCount() const { return prevFrame; }


private:
	/* 2 queries per swap, so that 1 query won't stall the rest of the program. */
    struct timer_query_pair_array
    {
        u32 query[2];
    } buf[QueryCountForSwapping];
	u32 swapIndex{0};
	u64 prevFrame{0}, nextFrame{0};
};


} // namespace globj


#endif