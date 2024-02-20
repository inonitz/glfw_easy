#pragma once
#include "base.hpp"


struct LoadedImage 
{
private:    
    u8 m_channelCount; /* Last Bit Enabled means data is floats */

public:
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnested-anon-types"
    union {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
        struct { u32 x, y; };
        struct { u32 m_dims[2] = { DEFAULT32, DEFAULT32 }; };
        #pragma GCC diagnostic pop
    };
    #pragma GCC diagnostic pop
    void* m_data = nullptr;


    LoadedImage() {}


    u8 getChannelCount() const { return m_channelCount; }


    static void load_image(
        const char*  fileName, 
        bool         convert_to_float,
        LoadedImage& result
    );


    static void destroy(LoadedImage&);
};


void convertRGBA_U32_TO_F32(
    u8*  inData_AlignmentNotRequired, 
    f32* outData_AlloateAligned64Bytes, 
    i32  dimx, 
    i32  dimy
);