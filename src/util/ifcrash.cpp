#include "ifcrash.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>


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