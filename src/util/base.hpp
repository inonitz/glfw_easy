#ifndef __BASE_HEADER__
#define __BASE_HEADER__
#include <cstdint>
#define USE_MARKER_IN_RELEASE_MODE true
#include "marker.hpp"
#include "ifcrash.hpp"


/* All credit goes to: https://www.fluentcpp.com/2019/08/30/how-to-disable-a-warning-in-cpp/ */
#if defined(__GNUC__) || defined(__clang__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
    #define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop) 
    #define DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)
    
    #define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    DISABLE_WARNING(-Wunused-parameter)
    #define DISABLE_WARNING_UNREFERENCED_FUNCTION            DISABLE_WARNING(-Wunused-function)
	#define DISABLE_WARNING_NESTED_ANON_TYPES                DISABLE_WARNING(-Wnested-anon-types)
	#define DISABLE_WARNING_GNU_ANON_STRUCT                  DISABLE_WARNING(-Wgnu-anonymous-struct)
	#define DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS     DISABLE_WARNING(-Wgnu-zero-variadic-macro-arguments)

#elif defined(_MSC_VER)
    #define DISABLE_WARNING_PUSH           __pragma(warning( push ))
    #define DISABLE_WARNING_POP            __pragma(warning( pop )) 
    #define DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))

    #define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    DISABLE_WARNING(4100)
    #define DISABLE_WARNING_UNREFERENCED_FUNCTION            DISABLE_WARNING(4505)

#else
    #define DISABLE_WARNING_PUSH
    #define DISABLE_WARNING_POP
    #define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
    #define DISABLE_WARNING_UNREFERENCED_FUNCTION
	#define DISABLE_WARNING_NESTED_ANON_TYPES
	#define DISABLE_WARNING_GNU_ANON_STRUCT

#endif


/* Code Expanded to Compiler-specific defines From: https://stackoverflow.com/questions/2124339/c-preprocessor-va-args-number-of-arguments?rq=1 */
#if defined( __GNUC__ ) || defined( __MINGW__ ) || defined ( __clang__ )
#define likely(cond)    __builtin_expect( boolean(cond), 1 )
#define unlikely(cond)  __builtin_expect( boolean(cond), 0 )

DISABLE_WARNING_PUSH
DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS
#define GET_ARG_COUNT(...) INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count

static_assert(GET_ARG_COUNT() == 0, "GET_ARG_COUNT() failed for 0 arguments");
static_assert(GET_ARG_COUNT(1) == 1, "GET_ARG_COUNT() failed for 1 argument");
static_assert(GET_ARG_COUNT(1,2) == 2, "GET_ARG_COUNT() failed for 2 arguments");
static_assert(GET_ARG_COUNT(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70) == 70, "GET_ARG_COUNT() failed for 70 arguments");
DISABLE_WARNING_POP

#elif defined( _MSC_VER )
#define likely(cond) (cond)
#define unlikely(cond) (cond)

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
		fprintf(stderr, "[LOG] %s:%u::%s", __FILE__, __LINE__, __FUNCTION__); \
		fprintf(stderr, extra, __VA_ARGS__); \
	} \

#ifdef _DEBUG
#define debug(...) { __VA_ARGS__; }
#define debugnobr(...) __VA_ARGS__;
#define debug_messagefmt(str, ...) { printf("[_DEBUG] "); printf(str, __VA_ARGS__); }
#define debug_message(str)		   { printf("[_DEBUG] "); printf(str); 				}
#endif


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
#ifndef __unused
#define __unused        __attribute__((unused)) /* more appropriate for functions		    */
#endif
#define notused         __attribute__((unused)) /* more appropriate for function parameters */
#define __hot           __attribute__((hot))
#define __cold          __attribute__((cold))
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
		better than this, which is ~5 instructions (atleast on x86)
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

inline std::uintptr_t __outv = 0;
#define CONDITIONAL_SET_PTR(ptr, ptr_val, cond) \
	__outv = __rcast(std::uintptr_t, ptr); \
	__outv *= !boolean(cond); \
	__outv += boolean(cond) * __rcast(std::uintptr_t, ptr_val); \
	ptr = __rcast(decltype(ptr), __outv); \

#define SET_BIT_AT(to_set, bit_index, bool_val) \
    to_set &= ~(1 << bit_index); \
    to_set |= ( __scast(  decltype( sizeof(to_set) ), bool_val  ) << bit_index); \




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

typedef imut_type_handle<char_t> k_char;
typedef imut_type_handle<byte>   k_byte;


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
template<typename T> constexpr T roundN(T powof2, T v) {
	static_assert(std::is_integral<T>::value, "Value must be an Integral Type! (Value v belongs to group N [0 -> +inf]. ");

	const auto rem = v & ( powof2 - 1);
	return (v - rem) + boolean(rem) * powof2; 
}


__force_inline size_t readTimestampCounter() { /* for whatever reason you may need this */
    u32 lo, hi;
    __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
    return ((size_t)hi << 32) | lo;
}

#endif