#include "base.hpp"


std::uintptr_t __out = 0;

#if defined(_DEBUG) || USE_MARKER_IN_RELEASE_MODE
	std::atomic<size_t> markflag{0};
#endif