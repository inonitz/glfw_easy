#ifndef __UTIL_IF_CRASH_MACRO__
#define __UTIL_IF_CRASH_MACRO__
#include "util/base.hpp"


#if not defined(__ATTRIBUTE_LIKELY_DEFINITION__)

#if defined( __GNUC__ ) || defined( __MINGW__ ) || defined ( __clang__ )
#define likely(cond)    __builtin_expect( boolean(cond), 1 )
#define unlikely(cond)  __builtin_expect( boolean(cond), 0 )

#elif defined( _MSC_VER )

#define likely(cond) (cond)
#define unlikely(cond) (cond)

#endif

#endif


namespace detail {
	constexpr const char* str_msg_begin = "[IFCRASH_%s] %s:%u";
	constexpr const char* str_msg_mid   = "\n[IFCRASH_%s] ";
	constexpr const char* str_msg_end   = "\n[IFCRASH_%s] ifcrash(...) macro triggered\n";

	void __common_print_function_fmt(const char* format, ...);
	void __common_print_function_nofmt(const char* str);
	[[noreturn]] void __common_abort_function() noexcept;
}


DISABLE_WARNING_PUSH
DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS
/* Tested ##__VA_ARGS__ on msvc, clang, gcc and works perfectly (std=C++17) | probably not portable on older versions of msvc */
#define ifcrash_generic(condition, name, str_or_fmt_required, append_name_if_required, action, str, ...) \
if(unlikely( !!(condition)))  \
{ \
	detail::__common_print_function_fmt(detail::str_msg_begin, name, __FILE__, __LINE__, name); \
	if constexpr (str_or_fmt_required) { \
		detail::__common_print_function_fmt(detail::str_msg_mid, name); \
		detail::__common_print_function##append_name_if_required(str, ##__VA_ARGS__); \
	} \
    { action; } \
	detail::__common_print_function_fmt(detail::str_msg_end, name); \
	detail::__common_abort_function(); \
}
DISABLE_WARNING_POP


#if defined(_DEBUG)
#define ifcrash_debug(condition) 			  ifcrash_generic(condition, "DBG",      false, _nofmt,   {}, nullptr)
#define ifcrashdo_debug(condition, code)      ifcrash_generic(condition, "CODE_DBG", false, _nofmt, code, nullptr)
#define ifcrashstr_debug(condition, str) 	  ifcrash_generic(condition, "STR_DBG",  true,  _nofmt,   {}, str)
#define ifcrashfmt_debug(condition, str, ...) ifcrash_generic(condition, "FMT_DBG",  true,    _fmt,   {}, str, __VA_ARGS__)

#else

#define ifcrash_debug(condition) {}
#define ifcrashstr_debug(condition, str) {}
#define ifcrashfmt_debug(condition, str, ...) {}
#define ifcrashdo_debug(condition, action) {}

#endif


#define ifcrash(condition) 			    ifcrash_generic(condition, "REL",      false, _nofmt,   {}, nullptr)
#define ifcrashdo(condition, code)      ifcrash_generic(condition, "CODE_REL", false, _nofmt, code, nullptr)
#define ifcrashstr(condition, str) 	    ifcrash_generic(condition, "STR_REL",  true,  _nofmt,   {}, str)
#define ifcrashfmt(condition, str, ...) ifcrash_generic(condition, "FMT_REL",  true,    _fmt,   {}, str, __VA_ARGS__)
#define ifcrashfmt_do(condition, str, code, ...) ifcrash_generic(condition, "CODE_FMT_REL", true, _fmt, code, str, __VA_ARGS__)


#endif