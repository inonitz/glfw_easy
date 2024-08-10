#ifndef __UTIL_BASE_MACRO_DEFINITIONS__
#define __UTIL_BASE_MACRO_DEFINITIONS__


/* All credit goes to: https://www.fluentcpp.com/2019/08/30/how-to-disable-a-warning-in-cpp/ */
#if defined(__GNUC__) || defined(__clang__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
    #define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop) 
    #define DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)
    #define DISABLE_WARNING_UNUSED_PARAMETER              DISABLE_WARNING(-Wunused-parameter)
    #define DISABLE_WARNING_UNUSED_FUNCTION               DISABLE_WARNING(-Wunused-function)
	#define DISABLE_WARNING_NESTED_ANON_TYPES             DISABLE_WARNING(-Wnested-anon-types)
	#define DISABLE_WARNING_GNU_ANON_STRUCT               DISABLE_WARNING(-Wgnu-anonymous-struct)
	#define DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS  DISABLE_WARNING(-Wgnu-zero-variadic-macro-arguments)
	#define DISABLE_WARNING_PEDANTIC 					  DISABLE_WARNING(-Wpedantic)
    #define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER DISABLE_WARNING_UNUSED_PARAMETER
    #define DISABLE_WARNING_UNREFERENCED_FUNCTION         DISABLE_WARNING_UNUSED_FUNCTION
    #define DISABLE_WARNING_DEPRECATED_FUNCTION           DISABLE_WARNING(-Wdeprecated-declarations)


#elif defined(_MSC_VER)
    #define DISABLE_WARNING_PUSH           __pragma(warning(  push  ))
    #define DISABLE_WARNING_POP            __pragma(warning(  pop  )) 
    #define DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))
	#define DISABLE_WARNING_UNUSED_PARAMETER DISABLE_WARNING(4100)
	#define DISABLE_WARNING_UNUSED_FUNCTION  DISABLE_WARNING(4505)
	#define DISABLE_WARNING_NESTED_ANON_TYPES
	#define DISABLE_WARNING_GNU_ANON_STRUCT
	#define DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS
	#define DISABLE_WARNING_PEDANTIC
    #define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    DISABLE_WARNING_UNUSED_PARAMETER
    #define DISABLE_WARNING_UNREFERENCED_FUNCTION            DISABLE_WARNING_UNUSED_FUNCTION
	#define DISABLE_WARNING_DEPRECATED_FUNCTION              DISABLE_WARNING(4996)

#else
    #define DISABLE_WARNING_PUSH
    #define DISABLE_WARNING_POP
    #define DISABLE_WARNING(warningNumber)
	#define DISABLE_WARNING_UNUSED_PARAMETER
	#define DISABLE_WARNING_UNUSED_FUNCTION 
	#define DISABLE_WARNING_NESTED_ANON_TYPES
	#define DISABLE_WARNING_GNU_ANON_STRUCT
	#define DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS
	#define DISABLE_WARNING_PEDANTIC
    #define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
    #define DISABLE_WARNING_UNREFERENCED_FUNCTION
	#define DISABLE_WARNING_DEPRECATED_FUNCTION

#endif




/* Code Expanded to Compiler-specific defines From: https://stackoverflow.com/questions/2124339/c-preprocessor-va-args-number-of-arguments?rq=1 */
#if defined( __GNUC__ ) || defined( __MINGW__ ) || defined ( __clang__ )
#define __ATTRIBUTE_LIKELY_DEFINITION__
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
#define __ATTRIBUTE_LIKELY_DEFINITION__
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


#ifdef _DEBUG
#define debug(...) { __VA_ARGS__ }
#define debugnobr(...) __VA_ARGS__
#define debug_declaration_nobr(...) __VA_ARGS__
#else
#define debug(...)
#define debugnobr(...)
#define debug_declaration_nobr(...)
#endif


#ifndef __unused
#define __unused        __attribute__((unused)) /* more appropriate for functions		    */
#endif
#define notused         __attribute__((unused)) /* more appropriate for function parameters */
/* Usually release mode omits a lot of code => gives many unused_param errors */
#if defined(_DEBUG)
#define __release_unused
#else
#define __release_unused __attribute__((unused))
#endif

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
#define isaligned(ptr, alignment) boolean( (  reinterpret_cast<size_t>(ptr) & (static_cast<size_t>(alignment) - 1llu)  ) == 0 )
#define __scast(type, val) static_cast<type>((val))
#define __rcast(type, val) reinterpret_cast<type>((val))
#define __carraysize(__c_array) ( sizeof(__c_array) / sizeof(__c_array[0]) )


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
#define DEFAULTVOIDPTR     __rcast(void*, DEFAULT64)


#endif