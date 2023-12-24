#include "base.hpp"


std::uintptr_t __out = 0;

#if defined(_DEBUG)
	std::atomic<size_t> markflag{0};
#endif