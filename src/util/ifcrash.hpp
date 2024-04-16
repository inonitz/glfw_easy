#ifndef __IF_CRASH_DEBUG_MACRO__
#define __IF_CRASH_DEBUG_MACRO__
#include <stdexcept>
#include <cstdio>


#define ifcrash_generic(condition, name, ...) /* Using this as a common denominator across all ifcrash* macros. */ \
	if(!!(condition)) { \
		fprintf(stderr, "[IFCRASH_%s] %s:%u\n", name, __FILE__, __LINE__); \
		__VA_ARGS__; \
		throw std::runtime_error("ifcrash_generic() Macro Triggered."); \
	} \


#if defined(_DEBUG)
#define ifcrash_debug(condition) 		   ifcrash_generic(condition, "MESSAGE", {});
#define ifcrashdo_debug(condition, action) ifcrash_generic(condition, "INJECT", { action; });
#define ifcrashfmt_debug(condition, str, ...) \
	ifcrash_generic(condition, "MESSAGE", { \
		printf("[IFCRASH_MESSAGE] Extra: "); \
		printf(str, __VA_ARGS__); \
	});
#define ifcrashfmtdo_debug(condition, action, str, ...) \
	ifcrash_generic(condition, "MESSAGE_INJECT", { \
		printf("[IFCRASH_MESSAGE] Extra: "); printf(str, __VA_ARGS__); \
		{ action; } \
	});

#else

#define debug(...)
#define debugnobr(...)
#define debug_messagefmt(str, ...)
#define debug_message(str)
#define ifcrash_debug(condition) {}
#define ifcrashfmt_debug(condition, str, ...) {}
#define ifcrashdo_debug(condition, action) ifcrash(condition);
#define ifcrashfmt_do_debug(condition, action, str, ...) ifcrash(condition);
#endif


#define ifcrash(condition) ifcrash_generic(condition, "DEFAULT", {});
#define ifcrashstr(condition, str) ifcrash_generic(condition, "STRING", { \
		printf("[IFCRASH_STRING] Extra: %s", str); \
	});
#define ifcrashfmt(condition, str, ...) ifcrash_generic(condition, "FORMAT", { \
		printf("[IFCRASH_FORMAT] Extra: "); \
		printf(str, __VA_ARGS__); \
	});
#define ifcrashdo(condition, action) ifcrash_generic(condition, "INJECT", { action; })
#define ifcrashfmt_do(condition, action, str, ...) ifcrash_generic(condition, "MESSAGE_INJECT", { \
		printf("[IFCRASH_FORMAT_DO] Extra: "); printf(str, __VA_ARGS__); \
		{ action; } \
	});


#endif