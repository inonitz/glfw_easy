#ifndef __UTIL_MARKER_FLAG_MACRO__
#define __UTIL_MARKER_FLAG_MACRO__

/* If you're too lazy to define these multiple times */
// #define MARKER_FLAG_REDIRECT_DISABLE_KEEP_MACROS
#define MARKER_FLAG_USE_IN_RELEASE_MODE
// #define MARKER_FLAG_REDIRECT_TO_FILE


#if defined(MARKER_FLAG_USE_IN_RELEASE_MODE) || defined(_DEBUG)
#include <atomic>
inline std::atomic<size_t> markflag;


#if defined(MARKER_FLAG_REDIRECT_TO_FILE)
#include "util/ifcrash.hpp"
#include <cerrno> 
inline FILE* __output_buf;


inline void marker_flag_init_file_redirect()
{
	__output_buf = fopen("__debug_output.txt", "w");
	ifcrashfmt(!__output_buf, "Error Opening File: %s\n", strerror(errno));
	fprintf(__output_buf, "================================__MARKER_FLAG_INIT_LOG__================================\n");
	return;
}
inline void marker_flag_close_file_redirect()
{
	fprintf(__output_buf, "\n================================__MARKER_FLAG_END_LOG__================================");
	fclose(__output_buf);
	return;
}


#define MARKER_FLAG_REDIRECT_TO_FILE_INIT() marker_flag_init_file_redirect();
#define MARKER_FLAG_REDIRECT_TO_FILE_END()  marker_flag_close_file_redirect();
#define __rdirprintf(...) fprintf(__output_buf, __VA_ARGS__);


#define mark_generic(atomic_8byte_counter, ...) \
	{ \
		__rdirprintf("[%llu] %s:%u", atomic_8byte_counter.load(),  __FILE__, __LINE__); \
		++atomic_8byte_counter; \
		if constexpr (GET_ARG_COUNT(__VA_ARGS__) > 1) { /*  */ \
			__rdirprintf(" [ADDITIONAL_INFO] "); __rdirprintf(__VA_ARGS__); \
		} \
		__rdirprintf("\n"); \
	} \


#else
#define mark_generic(atomic_8byte_counter, ...) \
	{ \
		printf("[%llu] %s:%u", atomic_8byte_counter.load(),  __FILE__, __LINE__); \
		++atomic_8byte_counter; \
		if constexpr (GET_ARG_COUNT(__VA_ARGS__) > 1) { /*  */ \
			printf(" [ADDITIONAL_INFO] "); printf(__VA_ARGS__); \
		} \
		printf("\n"); \
	} \

#endif


#define mark() mark_generic(markflag, "");
#define markstr(str) mark_generic(markflag, "%s", str);
#define markfmt(str, ...) mark_generic(markflag, str, __VA_ARGS__);

#else
#define MARKER_FLAG_REDIRECT_DISABLE_KEEP_MACROS
#define mark()
#define markstr(str)
#define markfmt(str, ...)

#endif


#if defined(MARKER_FLAG_REDIRECT_DISABLE_KEEP_MACROS)
#define MARKER_FLAG_REDIRECT_TO_FILE_INIT()
#define MARKER_FLAG_REDIRECT_TO_FILE_END()
#define __rdirprintf(...)
#endif


#endif