#include "marker.hpp"


#if defined(_DEBUG) || USE_MARKER_IN_RELEASE_MODE
	std::atomic<size_t> markflag{0};
#endif