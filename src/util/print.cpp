#include "print.hpp"
#include <cstdio>
#include <cstdarg>


namespace util {
    static FILE* __which_stream = stdout;


    void selectOutputStream(
        OutputStreamType stype, 
        void*            custom_stream
    ) {
        static FILE* convertToStreamFileDesc[4] = {
            stdout,
            stderr,
            __rcast(FILE*, custom_stream),
            nullptr
        };
        __which_stream = convertToStreamFileDesc[__scast(u8, stype)];
        return;
    }


    void print(const char* stri)
    {
        std::fputs(stri, __which_stream);
        return;
    }

    void printfmt(const char* format, ...) 
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
            print("\nprint.cpp] => printfmt(...) __VA_ARGS__ too large\n");
            invalid_state = true;
        }
        if(!invalid_state) {
            done = vsnprintf(__format_buffer.mem, size, format, argcopy);
        }
        va_end(argcopy);


        if (invalid_state || done < 0) {
            print("\nprint.cpp] => printfmt(...) Couldn't format __VA_ARGS__\n");
            __format_buffer.mem[0] = '\0'; /* if fputs encounters an eol it'll stop */
        }
        print(__format_buffer.mem);
        return;
    }

#if __UTIL_PRINT_USING_COLORED_CONSOLE__ == 1

namespace Terminal {


/* TODO Implement When the time comes, unnecessary for now */
/*
    Useful links:
    https://overflow.lunar.icu/questions/4053837/colorizing-text-in-the-console-with-c
    https://dev.to/tenry/terminal-colors-in-c-c-3dgc
    https://man7.org/linux/man-pages/man5/terminal-colors.d.5.html
*/
void setColor(
    bool           text_or_background,
    TextAttributes textType  = TextAttributes::NORMAL,
    Color const&   color_rgb = Color::white()
) {
    return;
}

void setToDefault(bool text_or_background) {
    return;
}


} // namespace Terminal

#endif


} // namespace util 