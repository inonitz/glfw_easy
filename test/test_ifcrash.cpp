#ifndef __UTIL_IF_CRASH_MACRO__
#define __UTIL_IF_CRASH_MACRO__
#include <cstdarg>
#include <cstdio>
#include <cstdlib>


namespace detail {
	/* Start Thinking about hiding more impl details, and replacing macros... */
	constexpr const char* str_msg_begin = "[IFCRASH_%s] %s:%u";
	constexpr const char* str_msg_mid   = "\n[IFCRASH_%s] ";
	constexpr const char* str_msg_end   = "\n[IFCRASH_%s] ifcrash(...) macro triggered\n";

	void __common_print_function_fmt(const char* format, ...);
	void __common_print_function_nofmt(const char* str);
	[[noreturn]] void __common_abort_function() noexcept;
}


#define ifcrash_generic(condition, name, str_or_fmt_required, append_name_if_required, action, str, ...) \
if(!!(condition))  \
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


#if defined(_DEBUG)
#define ifcrash_debug(condition) 			  ifcrash_generic(condition, "DBG",      false, _nofmt, nullptr, {})
#define ifcrashdo_debug(condition, code)      ifcrash_generic(condition, "CODE_DBG", false, _nofmt, nullptr, code)
#define ifcrashstr_debug(condition, str) 	  ifcrash_generic(condition, "STR_DBG",  true,  _nofmt, str, {})
#define ifcrashfmt_debug(condition, str, ...) ifcrash_generic(condition, "FMT_DBG",  true,    _fmt, str, {}, __VA_ARGS__)

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
#define ifcrashfmt_do(condition, str, code, ...) \
                                        ifcrash_generic(condition, "FMT_REL",  true,    _fmt, code, str, __VA_ARGS__)


void testfunc(int a, int b) {
    fprintf(stderr, "testfunc() %u %u\n", a, b);
    return;
}

int main()
{
    ifcrash_debug(1);
    ifcrashdo_debug(1, {
        testfunc(1, 2);
        testfunc(1, 2);
        testfunc(1, 2);
    });
    ifcrashstr_debug(1, "fuckyou");
    ifcrashfmt_debug(1, "fuckyou %u", 5);


    ifcrash(1);
    ifcrashdo(1, {
        testfunc(3, 4);
        testfunc(3, 4);
        testfunc(3, 4);
    });
    ifcrashstr(1, "fuckyou");
    ifcrashfmt(1, "fuckyou %u", 5);
    ifcrashfmt_do(1, "fuckyou %u %3", {
        testfunc(5, 6);
        testfunc(5, 6);
    }, 5, 4
    );
}


namespace detail {


struct __generic_buffer
{
    char* mem;
    unsigned int size;
};


void __common_print_function_nofmt(const char* stri)
{
    std::fprintf(stderr, stri);
    return;
}

void __common_print_function_fmt(const char* format, ...) 
{
    __generic_buffer out;
    va_list arg, argcopy;
    unsigned int done = 1;


    va_start(arg, format);
    va_copy(argcopy, arg);
    out.size = 1 + vsnprintf(NULL, 0, format, arg);
    out.mem  = reinterpret_cast<char*>(std::malloc(out.size));
    va_end(arg);
    done = vsnprintf(out.mem, out.size, format, argcopy);
    va_end(argcopy);


    if(done < 0) {
        std::fprintf(stderr, "[ifcrash.cpp] => __get_formatted_string() Encoding Error\n");
        __common_abort_function();
    } else {
        std::fprintf(stderr, out.mem);
        std::free(out.mem);
    }
    return;
}


[[noreturn]] void __common_abort_function() noexcept 
{
    exit(-1);
}


} // namespace detail


#endif