#include "image.hpp"
#include <immintrin.h>
#include <array>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.hpp>


void LoadedImage::destroy(LoadedImage& image)
{
    if(image.m_channelCount & 0x80) {
        afree_t(image.m_data);
    } else {
        stbi_image_free(image.m_data);
    }
}


void LoadedImage::load_image(
    const char*  fileName, 
    bool         convert_to_float,
    LoadedImage& result
) {
    i32 x, y, channels;
    u8*  data = stbi_load(fileName, &x, &y, &channels, 4);
    f32* optionalFloatData = nullptr;

    if(convert_to_float) {
        optionalFloatData = amalloc_t(f32, x * y * 4 * sizeof(f32), CACHE_LINE_BYTES);
        convertRGBA_U32_TO_F32(data, optionalFloatData, x, y);

        stbi_image_free(data);
    }
    result.x = __scast(u32, x);
    result.y = __scast(u32, y);
    result.m_channelCount = 4 | ( __scast(u8, convert_to_float) << 7 );
    result.m_data = convert_to_float ? (void*)optionalFloatData : (void*)data;
    return;
}


void convertRGBA_U32_TO_F32(u8* inData, float* outData, i32 dimx, i32 dimy)
{
    size_t currPixel = 0;
    size_t bufLength = 4 * static_cast<size_t>(dimx) * dimy;
    debug_message("convertRGBA_U32_TO_F32() debug output BEGIN\n");
#ifdef __AVX2__
    debug_message("AVX2\n");
    ifcrashdo( ( (size_t)outData & (8 * sizeof(float) - 1) ) != 0, {
        printf("outData isn't aligned on (atleast) a 32 Byte memory boundary\n");
    });
	__m256  constf32;
	__m128i A; 
	__m256i B;
	__m256  C;
	__m256  D;
	constf32 = _mm256_broadcastss_ps(_mm_set_ss(0.00392156862745098f)); /* load constant 1/255 to each element */
    // for(i32 c = 0; c < 4; ++c) 
	// {
	// 	for(i32 y = 0; y < dimy; ++y) 
	// 	{
	// 		for(i32 x = 0; x < dimx; x += 8) 
	// 		{
	// 			A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* Load 16 bytes 				       */
	// 			B = _mm256_cvtepu8_epi32 (A); 	     	                   /* Convert first 8 bytes to u32's       */
	// 			C = _mm256_cvtepi32_ps(B);   	     	                   /* Convert u32's to f32's 		       */
	// 			D = _mm256_mul_ps(C, constf32);                            /* Multiply f32's by (1/255). 		   */
	// 		    _mm256_store_ps(&outData[currPixel], D);                   /* Store result in appropriate location */
	// 			currPixel += 8;
	// 		}
	// 	}  
	// }
    for(currPixel = 0; currPixel < bufLength; currPixel += 8) {
        A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* Load 16 bytes 				       */
        B = _mm256_cvtepu8_epi32 (A); 	     	                   /* Convert first 8 bytes to u32's       */
        C = _mm256_cvtepi32_ps(B);   	     	                   /* Convert u32's to f32's 		       */
        D = _mm256_mul_ps(C, constf32);                            /* Multiply f32's by (1/255). 		   */
        _mm256_store_ps(&outData[currPixel], D);                   /* Store result in appropriate location */
    }
#elif defined __SSE4_1__
    debug_message("SSE4.1\n");
    ifcrashdo( ( (size_t)outData & (4 * sizeof(float) - 1) ) != 0, {
        printf("outData isn't aligned on (atleast) a 16 Byte memory boundary\n");
    });
    alignsz(16) std::array<f32, 4> const128 = { 0.00392156862745098f, 0.00392156862745098f, 0.00392156862745098f, 0.00392156862745098f };
    __m128 cf32;
    __m128i A, B;
	__m128  C, D;


    cf32 = _mm_load_ps(const128.begin()); /* SSE */
    // for(i32 c = 0; c < 4; ++c) 
	// {
	// 	for(i32 y = 0; y < dimy; ++y) {
    //         for(i32 x = 0; x < dimx; x += 4) 
	// 		{
    //             A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* SSE3 */
    //             B = _mm_cvtepu8_epi32(A); /* SSE4.1 */
    //             C = _mm_cvtepi32_ps(B);   /* SSE2   */
    //             D = _mm_mul_ps(C, cf32);  /* SSE    */
    //             currPixel += 4;
    //         }
    //     }
    // }
    for(currPixel = 0; currPixel < bufLength; currPixel += 4) {
        A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* SSE3 */
        B = _mm_cvtepu8_epi32(A); /* SSE4.1 */
        C = _mm_cvtepi32_ps(B);   /* SSE2   */
        D = _mm_mul_ps(C, cf32);  /* SSE    */
        _mm_store_ps(&outData[currPixel], D);
    }
#else
    debug_message("FALLBACK (COMPILER_OPTIMIZE march=native, mtune=native\n");
    alignsz(16) std::array<f32, 4> tmpf{};
    alignsz(16) std::array<u32, 4> tmpu{};
    u32   tmp = 0;


    // for(i32 c = 0; c < 4; ++c) 
	// {   
    //     for(i32 y = 0; y < dimy; ++y) {
    //         for(i32 x = 0; x < dimx; x += 4) {
    //             memcpy(&tmp, &inData[currPixel], 4);
    //             tmpu = {
    //                 tmp & 0x000000FF, /* First  Byte */
    //                 tmp & 0x0000FF00, /* Second Byte */
    //                 tmp & 0x00FF0000, /* Third  Byte */
    //                 tmp & 0xFF000000  /* Fourth Byte */
    //             };
    //             tmpf = {
    //                 static_cast<f32>(tmpu[0]),
    //                 static_cast<f32>(tmpu[1]),
    //                 static_cast<f32>(tmpu[2]),
    //                 static_cast<f32>(tmpu[3]),
    //             };
    //             memcpy(&outData[currPixel], tmpf.begin(), sizeof(f32) * 4);

    //             currPixel += 4;
    //         }
    //     }
    // }
    for(currPixel = 0; currPixel < bufLength; currPixel += 4) 
    {
        memcpy(&tmp, &inData[currPixel], 4);
        tmpu = {
            tmp & 0x000000FF, /* First  Byte */
            tmp & 0x0000FF00, /* Second Byte */
            tmp & 0x00FF0000, /* Third  Byte */
            tmp & 0xFF000000  /* Fourth Byte */
        };
        tmpf = {
            static_cast<f32>(tmpu[0]),
            static_cast<f32>(tmpu[1]),
            static_cast<f32>(tmpu[2]),
            static_cast<f32>(tmpu[3]),
        };
        memcpy(&outData[currPixel], tmpf.begin(), sizeof(f32) * 4);
    }
#endif

    debug_message("convertRGBA_U32_TO_F32() debug output END\n");
    return;
}