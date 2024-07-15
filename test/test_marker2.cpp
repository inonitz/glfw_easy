#ifndef __UTIL_MARKER_FLAG_MACRO__
#define __UTIL_MARKER_FLAG_MACRO__
#define MARKER_FLAG_LOG_TO_FILE 1
#define MARKER_FLAG_KEEP_RELEASE 0
#define MARKER_FLAG_DEFINE_IMPLEMENTATION 0
#define MARKER_FLAG_EXTERNAL_DEFINITION 0
#ifndef MARKER_FLAG_LOG_TO_FILE
    #define MARKER_FLAG_LOG_TO_FILE 0
#endif


#if defined(_DEBUG) || (MARKER_FLAG_KEEP_RELEASE == 1)
#undef MARKER_FLAG_DEFINE_IMPLEMENTATION
#undef MARKER_FLAG_EXTERNAL_DEFINITION
    #define MARKER_FLAG_DEFINE_IMPLEMENTATION 1 
    #define MARKER_FLAG_EXTERNAL_DEFINITION 1
#endif




#if MARKER_FLAG_DEFINE_IMPLEMENTATION == 1
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


namespace detail::marker {
    void __begin_exclusion();
    void __end_exclusion();
    auto __load_atomic_counter();
    void __increment_atomic_counter();
    void __common_print_function_nofmt(const char* str);
    void __common_print_function_fmt(const char* format, ...);
} // namespace detail::marker



DISABLE_WARNING_PUSH
DISABLE_WARNING_GNU_ZERO_VARIADIC_MACRO_ARGS

#define mark_generic(str_or_fmt, append_name_for_str_or_fmt, str, ...) \
	{ \
        detail::marker::__begin_exclusion(); \
		detail::marker::__common_print_function_fmt("[%llu] %s:%u", detail::marker::__load_atomic_counter(),  __FILE__, __LINE__); \
		detail::marker::__increment_atomic_counter(); \
		if constexpr (str_or_fmt) { \
			detail::marker::__common_print_function_nofmt(" [ADDITIONAL_INFO] "); \
            detail::marker::__common_print_function##append_name_for_str_or_fmt(str, ##__VA_ARGS__); \
		} \
		detail::marker::__common_print_function_nofmt("\n"); \
        detail::marker::__end_exclusion(); \
	} \

DISABLE_WARNING_POP

#endif


#if MARKER_FLAG_EXTERNAL_DEFINITION == 0
#define mark()            
#define markstr(str)      
#define markfmt(str, ...) 

#elif MARKER_FLAG_EXTERNAL_DEFINITION == 1
#define mark()            mark_generic(false, _nofmt,   nullptr     )
#define markstr(str)      mark_generic(true,  _nofmt,   str         )
#define markfmt(str, ...) mark_generic(true,  _fmt, str, __VA_ARGS__)

#endif


#endif




#if MARKER_FLAG_DEFINE_IMPLEMENTATION == 1
#include <atomic>
#include <mutex>
#include <cstdarg>
#include <cstdio>


namespace detail::marker {
    using write_to_file_one_at_a_time = std::mutex;
    using write_lock_type = write_to_file_one_at_a_time;
    
    static std::atomic<size_t> __markflag{0};
    static write_lock_type __write_lock;
    static FILE* __output_buf = (MARKER_FLAG_LOG_TO_FILE) ? fopen("__debug_output.txt", "w") : stdout;


    void __begin_exclusion() { __write_lock.lock();   }
    void __end_exclusion()   { __write_lock.unlock(); }
    auto __load_atomic_counter()      { return __markflag.load(); }
    void __increment_atomic_counter() { ++__markflag;             }

    void __common_print_function_nofmt(const char* stri)
    {
        std::fputs(stri, __output_buf);
        return;
    }

    void __common_print_function_fmt(const char* format, ...) 
    {
        static struct __generic_format_buffer {
            char mem[2048];
        } __format_buffer;
        va_list arg, argcopy;
        int size, done = 1;
        bool invalid_state = false;    


        __format_buffer.mem[2047] = '\0';
        va_start(arg, format);
        va_copy(argcopy, arg);
        size = 1 + vsnprintf(NULL, 0, format, arg);
        va_end(arg);
        if(size > 2048) {
            std::fputs("\n[marker2.cpp] => __common_print_function_fmt() __VA_ARGS__ too large\n", __output_buf);
            invalid_state = true;
        }
        if(!invalid_state) {
            done = vsnprintf(__format_buffer.mem, size, format, argcopy);
        }
        va_end(argcopy);

        if (invalid_state || done < 0)
            std::fputs("\n[marker2.cpp] => __common_print_function_fmt() Couldn't format __VA_ARGS__\n", __output_buf);
        
        std::fputs(__format_buffer.mem, __output_buf);
        return;
    }


} // namespace detail::marker

#endif




int main()
{
    std::printf("blalalalalalla\n");
    for(size_t i = 0; i < 100; ++i) {
        mark();
        markstr("test");
        markfmt("test %i", i);
    }
    fclose(detail::marker::__output_buf);
    return 0;
}