#include "marker2.hpp"
#include <cstdarg>


#if MARKER_FLAG_DEFINE_IMPLEMENTATION == 1
#include <atomic>
#include <mutex>
#include <cstdio>


namespace detail::marker {
    using write_to_file_one_at_a_time = std::mutex;
    using write_lock_type = write_to_file_one_at_a_time;
    
    static std::atomic<uint64_t> __markflag{0};
    static write_lock_type __write_lock;
    static FILE* __output_buf = (MARKER_FLAG_LOG_TO_FILE) ? std::fopen("__debug_output.txt", "w") : stdout;


    void __begin_exclusion() { __write_lock.lock();   }
    void __end_exclusion()   { __write_lock.unlock(); }
    unsigned long long __load_atomic_counter() { 
        return __markflag.load(); 
    }
    void __increment_atomic_counter() { 
        ++__markflag; 
        return;
    }

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


        __format_buffer.mem[2047] = '\0'; /* incase of overflow */
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


        if (invalid_state || done < 0) {
            std::fputs("\n[marker2.cpp] => __common_print_function_fmt() Couldn't format __VA_ARGS__\n", __output_buf);
            __format_buffer.mem[0] = '\0'; /* if fputs encounters an eol it'll stop */
        }
        std::fputs(__format_buffer.mem, __output_buf);
        return;
    }


} // namespace detail::marker


#if MARKER_FLAG_LOG_TO_FILE == 1
namespace marker 
{
    void marker_flag_close_logfile_handle() {
        std::fclose(detail::marker::__output_buf);
        return;
    }
}
#endif


#endif