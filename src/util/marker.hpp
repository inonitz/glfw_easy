#ifndef __UTIL_MARKER_FLAG_MACRO__
#define __UTIL_MARKER_FLAG_MACRO__
#include <cstdio>


#if defined(_DEBUG) || USE_MARKER_IN_RELEASE_MODE
#include <atomic>
extern std::atomic<size_t> markflag;


#define mark_generic(atomic_8byte_counter, ...) \
	{ \
		printf("[%llu] %s:%u", atomic_8byte_counter.load(),  __FILE__, __LINE__); \
		++atomic_8byte_counter; \
		if constexpr (GET_ARG_COUNT(__VA_ARGS__) > 1) { /*  */ \
			printf(" [ADDITIONAL_INFO] "); printf(__VA_ARGS__); \
		} \
		printf("\n"); \
	} \


#define mark() mark_generic(markflag, "");
#define markstr(str) mark_generic(markflag, "%s", str);
#define markfmt(str, ...) mark_generic(markflag, str, __VA_ARGS__);

#else
#define mark()
#define markstr(str)
#define markfmt(str, ...)

#endif


#endif