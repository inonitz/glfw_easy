#pragma once
#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <stdexcept>
#include <atomic>
#ifndef USE_MARKER_IN_RELEASE_MODE
#define USE_MARKER_IN_RELEASE_MODE false
#endif




#if defined( __GNUC__ ) || defined( __MINGW__ ) || defined ( __clang__ )
#define likely(cond)    __builtin_expect( boolean(cond), 1 )
#define unlikely(cond)  __builtin_expect( boolean(cond), 0 )


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define GET_ARG_COUNT(...) INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count

static_assert(GET_ARG_COUNT() == 0, "GET_ARG_COUNT() failed for 0 arguments");
static_assert(GET_ARG_COUNT(1) == 1, "GET_ARG_COUNT() failed for 1 argument");
static_assert(GET_ARG_COUNT(1,2) == 2, "GET_ARG_COUNT() failed for 2 arguments");
static_assert(GET_ARG_COUNT(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70) == 70, "GET_ARG_COUNT() failed for 70 arguments");

#pragma GCC diagnostic pop
#pragma GCC diagnostic pop




#elif defined( _MSC_VER )
#define likely(cond) (cond)
#define unlikely(cond) (cond)

/* Code Expanded to Compiler-specific defines From: https://stackoverflow.com/questions/2124339/c-preprocessor-va-args-number-of-arguments?rq=1 */
#define GET_ARG_COUNT(...)  INTERNAL_EXPAND_ARGS_PRIVATE(INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))
#define INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#define INTERNAL_EXPAND(x) x
#define INTERNAL_EXPAND_ARGS_PRIVATE(...) INTERNAL_EXPAND(INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count


static_assert(GET_ARG_COUNT() == 0, "GET_ARG_COUNT() failed for 0 arguments");
static_assert(GET_ARG_COUNT(1) == 1, "GET_ARG_COUNT() failed for 1 argument");
static_assert(GET_ARG_COUNT(1,2) == 2, "GET_ARG_COUNT() failed for 2 arguments");
static_assert(GET_ARG_COUNT(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70) == 70, "GET_ARG_COUNT() failed for 70 arguments");


/* For easier allocations in msvc, aligned_malloc already defined in most major compilers. */
#define aligned_alloc(size, align) _aligned_malloc(size, align)
#define aligned_free(ptr) _aligned_free(ptr)

#else
#   warning "Unknown compiler, there might be some troubles during compilation, such as undefined macros."
#endif


#define LOG_ERR_FMT(extra, ...) { \
		fprintf(stderr, "[LOG] ==> [file]: %s [func]: %s [line]: %u | ", __FILE__, __FUNCTION__, __LINE__); \
		fprintf(stderr, extra, __VA_ARGS__); \
	} \




#define ifcrash_generic(condition, name, ...) /* Using this as a common denominator across all ifcrash* macros. */ \
	if(!!(condition)) { \
		fprintf(stderr, "[IFCRASH_%s] [FROM] %s [LINE] %u\n", name, __FILE__, __LINE__); \
		__VA_ARGS__; \
		throw std::runtime_error(""); \
	} \

#define ifcrash(condition) ifcrash_generic(condition, "DEFAULT", {});
#define ifcrashfmt(condition, str, ...) ifcrash_generic(condition, "FORMAT", { \
		printf("[IFCRASH_DEFAULT] Extra: "); \
		printf(str, __VA_ARGS__); \
	});
#define ifcrashdo(condition, action) ifcrash_generic(condition, "INJECT", { action; })
#define ifcrashfmtdo_debug(condition, action, str, ...) ifcrash_generic(condition, "MESSAGE_INJECT", { \
		printf("[IFCRASH_MESSAGE] Extra: "); printf(str, __VA_ARGS__); \
		{ action; } \
	});



#define mark_generic(atomic_8byte_counter, ...) \
	{ \
		printf("[MARKER %llu] [FROM] %s [Line] %u", atomic_8byte_counter.load(),  __FILE__, __LINE__); \
		++atomic_8byte_counter; \
		if constexpr (GET_ARG_COUNT(__VA_ARGS__) > 1) { /*  */ \
			printf(" [ADDITIONAL_INFO] "); printf(__VA_ARGS__); \
		} \
		printf("\n"); \
	} \
\


extern uint8_t __run_once;
#define RESET_RUN_ONCE_COUNT() __run_once = 0;
#define __once(code_block) \
	if(!boolean(__run_once)) { \
		{ \
			code_block; \
			++__run_once; \
		} \
	} \


extern uint16_t __finished;
#define RESET_RUN_TIMES_COUNT() __finished = 0;
#define run_block(times, code_block) \
	if(boolean((uint16_t)times - __finished)) { \
		{ \
			code_block; \
			++__finished; \
		} \
	} \





#if defined(_DEBUG)

#define debug(...) { __VA_ARGS__; }
#define debugnobr(...) __VA_ARGS__;


#define debug_messagefmt(str, ...) { \
        printf("[_DEBUG] "); \
        printf(str, __VA_ARGS__); \
    } \

#define debug_message(str) { \
        printf("[_DEBUG] "); \
        printf(str); \
    } \


#define ifcrash_debug(condition) ifcrash_generic(condition, "MESSAGE", {});
#define ifcrashfmt_debug(condition, str, ...) ifcrash_generic(condition, "MESSAGE", { \
		printf("[IFCRASH_MESSAGE] Extra: "); \
		printf(str, __VA_ARGS__); \
	});

#define ifcrashdo_debug(condition, action) ifcrash_generic(condition, "INJECT", { action; });
#define ifcrashfmtdo_debug(condition, action, str, ...) ifcrash_generic(condition, "MESSAGE_INJECT", { \
		printf("[IFCRASH_MESSAGE] Extra: "); printf(str, __VA_ARGS__); \
		{ action; } \
	});

#else

#define debug(...)
#define debugnobr(...)
#define debug_messagefmt(str, ...)
#define debug_message(str)
#define ifcrashdbg(condition) 			{}
#define ifcrashfmt(condition, str, ...) {}
#define ifcrashdo(condition, action) 			   ifcrash(condition);
#define ifcrashfmt_do(condition, action, str, ...) ifcrash(condition);

#endif




#if defined(_DEBUG) || USE_MARKER_IN_RELEASE_MODE
extern std::atomic<size_t> markflag;

#define mark() mark_generic(markflag, "");
#define markstr(str) mark_generic(markflag, "%s", str);
#define markfmt(str, ...) mark_generic(markflag, str, __VA_ARGS__);

#else
#define mark()
#define markstr(str)
#define markfmt(str, ...)

#endif



#define SET_BIT_AT(to_set, bit_index, bool_val) \
    to_set &= ~(1 << bit_index); \
    to_set |= ( __scast(  decltype( sizeof(to_set) ), bool_val  ) << bit_index); \




#define boolean(arg) !!(arg)
#define KB           	   (1024llu)
#define MB           	   (KB*KB)
#define GB           	   (MB*MB)
#define PAGE         	   (4 * KB)
#define __M64_ALIGN_BYTES  (0x08llu)
#define __M128_ALIGN_BYTES (0x0fllu)
#define __M256_ALIGN_BYTES (0x1fllu)
#define __M512_ALIGN_BYTES (0x3fllu)
#define __M64_SIZE_BYTES   (0x08llu)
#define __M128_SIZE_BYTES  (0x10llu)
#define __M256_SIZE_BYTES  (0x20llu)
#define __M512_SIZE_BYTES  (0x40llu)
#define CACHE_LINE_BYTES   (64ul)
#define DEFAULT8           (0xAA)
#define DEFAULT16          (0xF00D)
#define DEFAULT32          (0xBABEBABE)
#define DEFAULT64          (0xFACADE00FACADE00)
#define DEFAULT128         (0xAAAC0FFEEAC1DAAA)


#define __hot           __attribute__((hot))
#define __cold          __attribute__((cold))
#ifndef __unused
#define __unused        __attribute__((unused)) /* more appropriate for functions		    */
#endif
#define notused         __attribute__((unused)) /* more appropriate for function parameters */
#define pack            __attribute__((packed))
#define alignpk(size)   __attribute__((packed, aligned(size)))
#define alignsz(size)   __attribute__((aligned(size)))
#ifndef __force_inline 
#define __force_inline inline __attribute__((always_inline))
#else
#define __force_inline __always_inline
#endif


#define amalloc_t(type, size, align) (type*)_mm_malloc(size, align)
#define afree_t(ptr) _mm_free(ptr)
#define isaligned(ptr, alignment) boolean( (  reinterpret_cast<size_t>(ptr) & (static_cast<size_t>(alignment) - 1llu)  ) == 0 )
#define __scast(type, val) static_cast<type>(val)
#define __rcast(type, val) reinterpret_cast<type>(val)


/*
	[NOTE]: 
		Just use the ternary operator [?], 
		it'll be optimized to a conditional move which is way
		better than this, which is ~5 instructions
	if cond:
		var *= false       => var = 0;
		var += true * val  => var = val;
	else:
		var *= true 	   => var = var;
		var += false * val => var += 0;
*/
#define CONDITIONAL_SET(var, val, cond) \
	var *= !boolean(cond); \
	var += boolean(cond) * (val); \
	\


extern std::uintptr_t __out;
#define CONDITIONAL_SET_PTR(ptr, ptr_val, cond) \
	__out = __rcast(std::uintptr_t, ptr); \
	__out *= !boolean(cond); \
	__out += boolean(cond) * __rcast(std::uintptr_t, ptr_val); \
	ptr = __rcast(decltype(ptr), __out);
	\




typedef unsigned char byte;
typedef char          char_t;


typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef int8_t   i8;
typedef float    f32;
typedef double   f64;


template<typename T> using ref 		 = typename std::conditional<sizeof(T) <= 8, T, T&		>::type;
template<typename T> using const_ref = typename std::conditional<sizeof(T) <= 8, T, T const&>::type;
template<typename T> using value_ptr = typename std::conditional<sizeof(T) <= 8, T, T*>::type;
template<typename T> using imut_type_handle = T const*;
template<typename T> using mut_type_handle  = T*;


template<typename T> constexpr T round2(T v) {
	static_assert(std::is_integral<T>::value, "Value must be an Integral Type! (Value v belongs to group N [0 -> +inf]. ");
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	++v;
	return v;
}


__force_inline size_t readTimestampCounter() {
    u32 lo, hi;
    __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
    return ((size_t)hi << 32) | lo;
}