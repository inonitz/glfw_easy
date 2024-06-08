#ifndef __UTIL_IF_CRASH_MACRO__
#define __UTIL_IF_CRASH_MACRO__
#include <cstdio>
#include <stdexcept>


#define ifcrash_generic(condition, name, ...) /* Using this as a common denominator across all ifcrash* macros. */ \
	if(!!(condition)) { \
		fprintf(stderr, "[IFCRASH_%s] %s:%u\n", name, __FILE__, __LINE__); \
		__VA_ARGS__; \
		throw std::runtime_error("ifcrash_generic() Macro Triggered."); \
	} \


#if defined(_DEBUG)
#define ifcrash_debug(condition) 		 ifcrash_generic(condition, "MESSAGE", {});
#define ifcrashstr_debug(condition, str) ifcrash_generic(condition, "STRING", { \
		fprintf(stderr, "[IFCRASH_STRING] %s", str); \
	});
#define ifcrashdo_debug(condition, action) ifcrash_generic(condition, "INJECT", { action; });
#define ifcrashfmt_debug(condition, str, ...) \
	ifcrash_generic(condition, "MESSAGE", { \
		fprintf(stderr, "[IFCRASH_MESSAGE] Extra: "); \
		fprintf(stderr, str, __VA_ARGS__); \
	});
#define ifcrashfmtdo_debug(condition, action, str, ...) \
	ifcrash_generic(condition, "MESSAGE_INJECT", { \
		fprintf(stderr, "[IFCRASH_MESSAGE] Extra: "); fprintf(stderr, str, __VA_ARGS__); \
		{ action; } \
	});

#else

#define debug(...)
#define debugnobr(...)
#define debug_messagefmt(str, ...)
#define debug_message(str)
#define ifcrash_debug(condition) {}
#define ifcrashstr_debug(condition, str) {}
#define ifcrashfmt_debug(condition, str, ...) {}
#define ifcrashdo_debug(condition, action) {}
#define ifcrashfmt_do_debug(condition, action, str, ...) {}

#endif


#define ifcrash(condition) ifcrash_generic(condition, "DEFAULT", {});
#define ifcrashstr(condition, str) ifcrash_generic(condition, "STRING", { \
		fprintf(stderr, "[IFCRASH_STRING] %s", str); \
	});
#define ifcrashfmt(condition, str, ...) ifcrash_generic(condition, "FORMAT", { \
		fprintf(stderr, "[IFCRASH_FORMAT] "); \
		fprintf(stderr, str, __VA_ARGS__); \
	});
#define ifcrashdo(condition, action) ifcrash_generic(condition, "INJECT", { action; })
#define ifcrashfmt_do(condition, action, str, ...) ifcrash_generic(condition, "MESSAGE_INJECT", { \
		fprintf(stderr, "[IFCRASH_FORMAT_DO] "); \
		fprintf(stderr, str, __VA_ARGS__); \
		{ action; } \
	});


#endif