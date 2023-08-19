#pragma once
#include "util/base.hpp"
#include <immintrin.h>
#include <math.h>
#include <array>



/*
	NOTICE:
	Matrices/Vectors in this library are stored in ROW MAJOR ORDER (LINEARLY IN MEMORY); 
	BUT! The matrices themselves are column-major, and are expected to be multiplied as such. 
*/
#define NAMESPACE_MATH_BEGIN namespace math {
#define NAMESPACE_MATH_END   }




NAMESPACE_MATH_BEGIN


template<typename T> constexpr T radians(T v) {
	return v * (T)0.017453292519943295; /* value * PI * (1 / 180) */
}




template<typename T, size_t length> class Vector
{
private:
    T __data[round2<size_t>(length)];
    
public:
	void zero() { memset(__data, 0x00, bytes()); return; }


	constexpr Vector() { zero(); }
	constexpr Vector(const_ref<T> defaultVal) 
	{
		for(size_t i = 0; i < length; i += 2)  {
			__data[i    ] = defaultVal;
			__data[i + 1] = defaultVal;
		}
		return;
	}


	T& 		 operator[](uint8_t idx) 	   { ifcrashdbg((size_t)idx >= length); return __data[idx]; }
	const T& operator[](uint8_t idx) const { ifcrashdbg((size_t)idx >= length); return __data[idx]; }

	      T* begin()       { return &__data[0];      }
	      T* end()         { return &__data[length]; }
	const T* begin() const { return &__data[0];      }
	const T* end()   const { return &__data[length]; }
	constexpr size_t bytes()  const { return sizeof(T) * length;  }
	constexpr size_t len()    const { return length;		      }
};


thread_local static Vector<float, 2 > temporaryBufferVec2f{};
thread_local static Vector<float, 3 > temporaryBufferVec3f{};
thread_local static Vector<float, 4 > temporaryBufferVec4f{};
thread_local static Vector<float, 4 > temporaryBufferMat2f{};
thread_local static Vector<float, 9 > temporaryBufferMat3f{};
thread_local static Vector<float, 16> temporaryBufferMat4f{};
thread_local static Vector<u32,   2 > temporaryBufferVec2u{};
thread_local static Vector<u32,   3 > temporaryBufferVec3u{};
thread_local static Vector<u32,   4 > temporaryBufferVec4u{};
thread_local static Vector<i32,   2 > temporaryBufferVec2i{};
thread_local static Vector<i32,   3 > temporaryBufferVec3i{};
thread_local static Vector<i32,   4 > temporaryBufferVec4i{};


#define SET_MULTIPLE_VALUES 4llu
#define normalizeIndex(i) (SET_MULTIPLE_VALUES * i)
#define ni(i) normalizeIndex(i)


#define INTRMD_FUNC_GENERATOR(op_symbol, name, arg1a0, arg1a1, arg1a2, arg1a3, arg1a4, ...) \
template<typename T, size_t len> void name( \
	Vector<T, len> const& a, \
    __VA_ARGS__            , \
	Vector<T, len>& 	  dst \
) { \
	static Vector<T, SET_MULTIPLE_VALUES> tmp; \
	size_t i  = 0; \
	\
	for(; i < (len / SET_MULTIPLE_VALUES); ++i) { \
		tmp[0] = a[ni(i)    ] op_symbol arg1a0; \
		tmp[1] = a[ni(i) + 1] op_symbol arg1a1; \
		tmp[2] = a[ni(i) + 2] op_symbol arg1a2; \
		tmp[3] = a[ni(i) + 3] op_symbol arg1a3; \
		memcpy(&dst.begin()[i], tmp.begin(), tmp.bytes()); \
	} \
	for(i *= SET_MULTIPLE_VALUES; i < len; ++i) { dst[i] = a[i] op_symbol arg1a4; } \
	return; \
} \


#define GENERATE_VECTOR_OPERATOR_FUNCTIONS(op_symbol, name) \
INTRMD_FUNC_GENERATOR(op_symbol, name, b[ni(i)], b[ni(i) + 1], b[ni(i) + 2], b[ni(i) + 3], b[i], Vector<T, len> const& b) \
INTRMD_FUNC_GENERATOR(op_symbol, name, b, b, b, b, b, T b) \



GENERATE_VECTOR_OPERATOR_FUNCTIONS(+, add)
GENERATE_VECTOR_OPERATOR_FUNCTIONS(-, sub)
GENERATE_VECTOR_OPERATOR_FUNCTIONS(*, mul)
GENERATE_VECTOR_OPERATOR_FUNCTIONS(/, div)


template<typename T, size_t len> T dot_prod(
	Vector<T, len> const& a, 
	Vector<T, len> const& b
) {
	Vector<T, SET_MULTIPLE_VALUES> tmp;
	T aggregate{0x00};
	size_t i  = 0;

	for(; i < (len / SET_MULTIPLE_VALUES); ++i) {
		tmp[0] = a[ni(i)    ] * b[ni(i)    ];
		tmp[1] = a[ni(i) + 1] * b[ni(i) + 1];
		tmp[2] = a[ni(i) + 2] * b[ni(i) + 2];
		tmp[3] = a[ni(i) + 3] * b[ni(i) + 3];
		tmp[i    ] = tmp[i	  ] + tmp[i + 1];
		tmp[i + 1] = tmp[i + 2] + tmp[i + 3];
		tmp[i] 	   = tmp[i] 	+ tmp[i + 1];
		
		aggregate += tmp[i];
	}
	for(i *= SET_MULTIPLE_VALUES; i < len; ++i) { aggregate += a[i] * b[i]; }
	return aggregate;
}


template<typename T, size_t len> void norm(Vector<T, len>& a) {
	T tmp = dot_prod(a, a); tmp = ((T)1) / tmp;
	mul<T, len>(a, tmp, a);
	return;
}


template<typename T, size_t len> T length_sqr(Vector<T, len> const& a) { return dot_prod(a, a);       }
template<typename T, size_t len> T vec_length(Vector<T, len> const& a) { return sqrt(dot_prod(a, a)); }



template<typename T> void cross_prod(
	Vector<T, 3> const& a, 
	Vector<T, 3> const& b, 
	Vector<T, 3>&       dst
) {
	dst[0] = a[1] * b[2] - b[1] * a[2];
	dst[1] = a[2] * b[0] - b[2] * a[0];
	dst[2] = a[0] * b[1] - b[0] * a[1];
	return;
}




#undef SET_MULTIPLE_VALUES


#define DEFINE_VECTOR_STRUCTURE_ARGS(len, dtype, aptn, declare_union_structs, declare_n_args_ctor, ...) \
struct vec##aptn \
{ \
    union { \
        Vector<dtype, len> mem; \
        \
        \
        declare_union_structs; \
    }; \
\
\
    vec##aptn() : mem() {} \
    explicit vec##aptn(dtype value) : mem(value) {} \
    declare_n_args_ctor \
    explicit vec##aptn(const dtype* validAddr) { \
        memcpy(mem.begin(), validAddr, mem.bytes()); \
        return; \
    } \
    vec##aptn(std::array<dtype, len> const& arr) : vec##aptn(arr.begin()) {} \
    vec##aptn(Vector<dtype, len>     const& vec) : mem(vec)               {} \
	vec##aptn(vec##aptn 			   const& cpy) : mem(cpy.mem) 		  {} \
    vec##aptn& operator=(const vec##aptn& cpy) { \
        memcpy(begin(), cpy.begin(), bytes()); \
        return *this; \
    } \
\
\
    __force_inline 		 Vector<dtype, len>& operator+(vec##aptn const& b) 		 { add(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
    __force_inline 		 Vector<dtype, len>& operator-(vec##aptn const& b) 		 { sub(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
    __force_inline 		 Vector<dtype, len>& operator*(vec##aptn const& b) 		 { mul(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
    __force_inline 		 Vector<dtype, len>& operator/(vec##aptn const& b) 		 { div(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline 		 Vector<dtype, len>& operator*(dtype            a) 		 { mul(mem, a,     temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
    __force_inline 		 Vector<dtype, len>& operator/(dtype            a) 		 { div(mem, a,     temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
    __force_inline const Vector<dtype, len>& operator+(vec##aptn const& b) const { add(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline const Vector<dtype, len>& operator-(vec##aptn const& b) const { sub(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline const Vector<dtype, len>& operator*(vec##aptn const& b) const { mul(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline 		 Vector<dtype, len>& operator/(vec##aptn const& b) const { div(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline const Vector<dtype, len>& operator*(dtype     const  a) const { mul(mem, a,     temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline const Vector<dtype, len>& operator/(dtype     const  a) const { div(mem, a,     temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline vec##aptn& operator+=(vec##aptn const& b) { add(mem, b.mem, mem); return *this;  } \
    __force_inline vec##aptn& operator-=(vec##aptn const& b) { sub(mem, b.mem, mem); return *this;  } \
    __force_inline vec##aptn& operator*=(vec##aptn const& b) { mul(mem, b.mem, mem); return *this;  } \
    __force_inline vec##aptn& operator*=(dtype            a) { mul(mem, a,     mem); return *this;  } \
    __force_inline vec##aptn& operator/=(dtype            b) { div(mem, b,     mem); return *this;  } \
\
	__force_inline dtype               length()     const     		 { temporaryBufferVec##aptn = mem; 								   return vec_length(temporaryBufferVec##aptn); } \
    __force_inline dtype               lensqr()     const     		 { temporaryBufferVec##aptn = mem; 								   return length_sqr(temporaryBufferVec##aptn); } \
    __force_inline Vector<dtype, len>& normalized() const     		 { temporaryBufferVec##aptn = mem; norm(temporaryBufferVec##aptn); return 			 temporaryBufferVec##aptn;  } \
	__force_inline void                normalize()                   {  norm(mem); return; } \
\
\
    __force_inline dtype&       operator[](uint8_t idx)       { return mem[idx]; } \
    __force_inline const dtype& operator[](uint8_t idx) const { return mem[idx]; } \
		  dtype*        begin()       { return mem.begin();  } \
	      dtype*        end()         { return mem.end();    } \
	const dtype*        begin() const { return mem.begin();  } \
	const dtype*        end()   const { return mem.end();    } \
    constexpr size_t    bytes() const { return mem.bytes();  } \
    __force_inline void print() const { printf(__VA_ARGS__); } \
    \
}; \
\
\
__force_inline Vector<dtype, len>& operator*(dtype a, 			 vec##aptn const& b) { mul       (b.mem, a,         temporaryBufferVec##aptn); return temporaryBufferVec##aptn; } \
__force_inline dtype               dot      (vec##aptn const& a, vec##aptn const& b) { return dot_prod(a.mem, b.mem);       											    } \


#define GENERATE_CROSSPROD_FUNC(len, dtype, aptn) \
__force_inline Vector<dtype, len>& cross    (vec##aptn const& a, vec##aptn const& b) { cross_prod(a.mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn; } \

#define GENERATE_NEGATE_FUNC(len, dtype, aptn) \
__force_inline 		 Vector<dtype, len>& operator-(vec##aptn const& b) { mul(b.mem, ((dtype)-1), temporaryBufferVec##aptn); return temporaryBufferVec##aptn; } \



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

DEFINE_VECTOR_STRUCTURE_ARGS( \
	2,
	float, 
	2f, 
	struct { float x;   float y;     };
	struct { float u;   float v;     };
	struct { float i;   float j;     };
	struct { float yaw; float pitch; };
	struct { float m0;  float m1;    },
	vec2f(float a, float b) 
	{ 
		x = a; 
		y = b; 
		return; 
	}, 
	"vec2f %p: ( %0.5f, %0.5f )\n", (void*)begin(), x, y
)
GENERATE_NEGATE_FUNC(2, float, 2f)


DEFINE_VECTOR_STRUCTURE_ARGS( \
	4,
	float,
	4f,
	__m128 xmm;
	struct { float x;   float y;     float z;    float w;     }; 
	struct { float r;   float g;     float b;    float a;     };
	struct { float i;   float j;     float k;    float l;     };
	struct { float yaw; float pitch; float roll; float theta; };
	struct { float m0;  float m1;    float m2;   float m3;    },
	vec4f(float a, float b, float c, float d) 
	{ 
		x = a; 
		y = b; 
		z = c; 
		w = d;
		return; 
	}
	vec4f(__m128 mm) : xmm(mm) {},
	"vec4f %p: ( %.05f, %.05f, %.05f, %.05f )\n", (void*)begin(), x, y, z, w
)
GENERATE_NEGATE_FUNC(4, float, 4f)


DEFINE_VECTOR_STRUCTURE_ARGS( \
	3,
	float,
	3f,
	__m128 xmm;
	struct { float x;   float y;     float z;    };
	struct { float u;   float v;     float w;    };
	struct { float r;   float g;     float b;    };
	struct { float i;   float j;     float k;    };
	struct { float yaw; float pitch; float roll; };
	struct { float m0;  float m1;    float m2;   };
	vec4f homogenised,
	vec3f(float a, float b, float c) 
	{ 
		x = a; 
		y = b; 
		z = c;
		homogenised.w = 1.0f;
		return; 
	}
	vec3f(__m128 mm) : xmm(mm) {},
	"vec3f %p: ( %.05f, %.05f, %.05f )\n", (void*)begin(), x, y, z
)
GENERATE_CROSSPROD_FUNC(3, float, 3f)
GENERATE_NEGATE_FUNC(3, float, 3f)



DEFINE_VECTOR_STRUCTURE_ARGS( \
	2,
	u32, 
	2u,
	u64 qword;
	struct { u32 x;   u32 y;     };
	struct { u32 u;   u32 v;     };
	struct { u32 i;   u32 j;     };
	struct { u32 yaw; u32 pitch; };
	struct { u32 m0;  u32 m1;    },
	vec2u(u32 a, u32 b) 
	{ 
		x = a; 
		y = b; 
		return; 
	}, 
	"vec2u %p: ( %u, %u )\n", (void*)begin(), x, y
)
GENERATE_NEGATE_FUNC(2, u32, 2u)


DEFINE_VECTOR_STRUCTURE_ARGS( \
	4,
	u32,
	4u,
	__m128i_u xmm;
	struct { u32 x;   u32 y;     u32 z;    u32 w;     }; 
	struct { u32 r;   u32 g;     u32 b;    u32 a;     };
	struct { u32 i;   u32 j;     u32 k;	   u32 l;     };
	struct { u32 yaw; u32 pitch; u32 roll; u32 theta; };
	struct { u32 m0;  u32 m1;    u32 m2;   u32 m3;    },
	vec4u(u32 a, u32 b, u32 c, u32 d) 
	{ 
		x = a; 
		y = b; 
		z = c; 
		w = d;
		return; 
	}
	vec4u(__m128i mm) : xmm(mm) {}, 
	"vec4u %p: ( %u, %u, %u, %u )\n", (void*)begin(), x, y, z, w
)
GENERATE_NEGATE_FUNC(4, u32, 4u)


DEFINE_VECTOR_STRUCTURE_ARGS( \
	4,
	i32,
	4i,
	__m128i xmm;
	struct { i32 x;   i32 y;     i32 z;    i32 w;     }; 
	struct { i32 r;   i32 g;     i32 b;    i32 a;     };
	struct { i32 i;   i32 j;     i32 k;	   i32 l;     };
	struct { i32 yaw; i32 pitch; i32 roll; i32 theta; };
	struct { i32 m0;  i32 m1;    i32 m2;   i32 m3;    },
	vec4i(i32 a, i32 b, i32 c, i32 d) 
	{ 
		x = a; 
		y = b; 
		z = c; 
		w = d;
		return; 
	}
	vec4i(__m128i mm) : xmm(mm) {},
	"vec4i %p: ( %d, %d, %d, %d )\n", (void*)begin(), x, y, z, w
)
GENERATE_NEGATE_FUNC(4, i32, 4i)


DEFINE_VECTOR_STRUCTURE_ARGS( \
	3,
	u32,
	3u,
	__m128i_u xmm;
	struct { u32 x;   u32 y;     u32 z;    };
	struct { u32 u;   u32 v;     u32 w;    };
	struct { u32 r;   u32 g;     u32 b;    };
	struct { u32 i;   u32 j;     u32 k;    };
	struct { u32 yaw; u32 pitch; u32 roll; };
	struct { u32 m0;  u32 m1;    u32 m2;   };
	vec4u homogenised,
	vec3u(u32 a, u32 b, u32 c) 
	{
		x = a; 
		y = b; 
		z = c;
		homogenised.w = 1.0f;
		return; 
	}
	vec3u(i32 a, i32 b, i32 c)
	{
		x = __scast(u32, a);
		y = __scast(u32, b);
		z = __scast(u32, c);
		return;
	}
	vec3u(__m128i mm)   : xmm(mm) {},

	"vec3u %p: ( %u, %u, %u )\n", (void*)begin(), x, y, z
)
GENERATE_CROSSPROD_FUNC(3, u32, 3u)
GENERATE_NEGATE_FUNC(3, u32, 3u)


DEFINE_VECTOR_STRUCTURE_ARGS( \
	3,
	i32,
	3i,
	__m128i xmm;
	struct { i32 x;   i32 y;     i32 z;    };
	struct { i32 u;   i32 v;     i32 w;    };
	struct { i32 r;   i32 g;     i32 b;    };
	struct { i32 i;   i32 j;     i32 k;    };
	struct { i32 yaw; i32 pitch; i32 roll; };
	struct { i32 m0;  i32 m1;    i32 m2;   };
	vec4i homogenised,
	vec3i(i32 a, i32 b, i32 c) 
	{ 
		x = a; 
		y = b; 
		z = c;
		homogenised.w = 1.0f;
		return; 
	}
	vec3i(vec3u const& uv)
	{
		x = __scast(i32, uv.x);
		y = __scast(i32, uv.y);
		z = __scast(i32, uv.z);
		return;
	}
	vec3i(__m128i mm) : xmm(mm) {},
	"vec3i %p: ( %d, %d, %d )\n", (void*)begin(), x, y, z
)
GENERATE_CROSSPROD_FUNC(3, i32, 3i)
GENERATE_NEGATE_FUNC(3, i32, 3i)




#undef GENERATE_NEGATE_FUNC
#undef GENERATE_CROSSPROD_FUNC
#undef DEFINE_VECTOR_STRUCTURE_ARGS
#undef GENERATE_VECTOR_OPERATOR_FUNCTIONS
#undef INTRMD_FUNC_GENERATOR
#undef normalizeIndex
#undef ni
#undef SET_MULTIPLE_VALUES




struct mat2f 
{
	using underlying_buffer = Vector<float, 4>;
	union {
		Vector<float, 4> mem;
		struct { vec2f row[2]; };
		struct { float x0,   y0,  x1,  y1; };
		struct { float m00, m01, m10, m11; };
		vec4f homogenised;
	};


	mat2f() : mem() 
	{ 
		/* Identity Matrix initialization is pretty useful. */
		mem[0] = 1.0f; 
		mem[2] = 1.0f;
		return;
	}
	mat2f(float value) : mem(value) {}
	mat2f(float x0, float y0, float x1, float y1) : homogenised{ x0, y0, x1, y1 } {}
	mat2f(vec2f const& a, vec2f const& b) 		  : row{ a, b } {}
	mat2f(const float* validAddr) {
		memcpy(mem.begin(), validAddr, mem.bytes());
		return;
	}
	mat2f(std::array<float, 4> const& arr) : mat2f(arr.begin()) {}
	mat2f(Vector<float, 4>     const& vec) : mem(vec)           {}
	mat2f(mat2f 			   const& cpy) : mem(cpy.mem) 	    {}
	mat2f& operator=(const mat2f& cpy) {
		homogenised = cpy.homogenised;
		return *this;
	}


	__force_inline underlying_buffer& operator+(mat2f const& b) { add(mem, b.mem, temporaryBufferMat2f); return temporaryBufferMat2f; }
	__force_inline underlying_buffer& operator-(mat2f const& b) { sub(mem, b.mem, temporaryBufferMat2f); return temporaryBufferMat2f; }
	__force_inline underlying_buffer& operator*(mat2f const& b) { mul(mem, b.mem, temporaryBufferMat2f); return temporaryBufferMat2f; }
	__force_inline underlying_buffer& operator*(float        b) { mul(mem, b,     temporaryBufferMat2f); return temporaryBufferMat2f; }
	__force_inline underlying_buffer& operator/(float        b) { div(mem, b,     temporaryBufferMat2f); return temporaryBufferMat2f; }

	__force_inline 		 vec2f  column(	   uint8_t idx		   ) 	   { ifcrashdbg(idx >= 2); return { mem[idx], mem[idx + 2] }; }
	__force_inline const vec2f  column(	   uint8_t idx		   ) const { ifcrashdbg(idx >= 2); return { mem[idx], mem[idx + 2] }; }
	__force_inline       vec2f& operator[](uint8_t idx	       )       { ifcrashdbg(idx >= 2); return row[idx];       }
	__force_inline const vec2f& operator[](uint8_t idx	       ) const { ifcrashdbg(idx >= 2); return row[idx];       }
	__force_inline float&       operator()(uint8_t i, uint8_t j)       { 					   return mem[i * 2 + j]; }
	__force_inline const float& operator()(uint8_t i, uint8_t j) const { 					   return mem[i * 2 + j]; }

		  float*        begin()        { return mem.begin();  }
		  float*        end()          { return mem.end();    }
	const float*        begin()  const { return mem.begin();  }
	const float*        end()    const { return mem.end();    }
	constexpr size_t    bytes()  const { return mem.bytes();  }
	constexpr size_t    length() const { return mem.len();    }
	__force_inline void print()  const 
	{ 
		printf("mat2f %p:\n", (void*)begin());
		printf("( %-5.05f, %-5.05f )\n", row[0].x, row[0].y);
		printf("( %-5.05f, %-5.05f )\n", row[1].x, row[1].y);
		return;
	}
};


__force_inline mat2f::underlying_buffer& operator*(float a, mat2f const& b) { mul(b.mem, a, temporaryBufferMat2f); return temporaryBufferMat2f; }




struct mat4f 
{
	using underlying_buffer = Vector<float, 16>;
	union {
		Vector<float, 16> mem;
		struct { vec4f row[4];  };
		struct { vec3f row3[4]; };
		struct {
			float   x0, y0, z0, w0,
					x1, y1, z1, w1, 
					x2, y2, z2, w2, 
					x3, y3, z3, w3;
		};
		struct {
			float   m00, m01, m02, m03,
					m10, m11, m12, m13, 
					m20, m21, m22, m23, 
					m30, m31, m32, m33;
		};
	};


	mat4f() : mem() 
	{ 
		/* Identity Matrix initialization is pretty useful. */
		mem[ 0] = 1.0f; 
		mem[ 5] = 1.0f; 
		mem[10] = 1.0f; 
		mem[15] = 1.0f; 
		return;
	}
	mat4f(float value) : mem(value) {}
	mat4f(
		float x0, float y0, float z0, float w0, 
		float x1, float y1, float z1, float w1, 
		float x2, float y2, float z2, float w2, 
		float x3, float y3, float z3, float w3
	) {
		row[0] = { x0, y0, z0, w0 }; 
		row[1] = { x1, y1, z1, w1 }; 
		row[2] = { x2, y2, z2, w2 };
		row[3] = { x3, y3, z3, w3 };
		return;
	}
	mat4f(
		vec4f const& a, 
		vec4f const& b,
		vec4f const& c, 
		vec4f const& d
	) {
		row[0] = a;
		row[1] = b;
		row[2] = c;
		row[3] = d;
		return;
	}
	mat4f(const float* validAddr) {
		memcpy(mem.begin(), validAddr, mem.bytes());
		return;
	}
	mat4f(mat2f const& promote) : mem() {
		mem[ 0] = promote.m00;
		mem[ 1] = promote.m01;
		mem[ 4] = promote.m10;
		mem[ 5] = promote.m11; 
		mem[10] = 1.0f; 
		mem[15] = 1.0f; 
	}
	mat4f(std::array<float, 16> const& arr) : mat4f(arr.begin()) {}
	mat4f(Vector<float, 16>     const& vec) : mem(vec)           {}
	mat4f(mat4f 			    const& cpy) : mem(cpy.mem) 	     {}
	mat4f& operator=(const mat4f& cpy) {
		memcpy(begin(), cpy.begin(), bytes());
		return *this;
	}


	__force_inline underlying_buffer& operator+(mat4f const& b) { add(mem, b.mem, temporaryBufferMat4f); return temporaryBufferMat4f; }
	__force_inline underlying_buffer& operator-(mat4f const& b) { sub(mem, b.mem, temporaryBufferMat4f); return temporaryBufferMat4f; }
	__force_inline underlying_buffer& operator*(mat4f const& b) { mul(mem, b.mem, temporaryBufferMat4f); return temporaryBufferMat4f; }
	__force_inline underlying_buffer& operator*(float        b) { mul(mem, b,     temporaryBufferMat4f); return temporaryBufferMat4f; }
	__force_inline underlying_buffer& operator/(float        b) { div(mem, b,     temporaryBufferMat4f); return temporaryBufferMat4f; }

	__force_inline 		 vec4f  column(	   uint8_t idx		   ) 	   { ifcrashdbg(idx >= 4); return { row[0][idx], row[1][idx], row[2][idx], row[3][idx] }; }
	__force_inline const vec4f  column(	   uint8_t idx		   ) const { ifcrashdbg(idx >= 4); return { row[0][idx], row[1][idx], row[2][idx], row[3][idx] }; }
	__force_inline       vec4f& operator[](uint8_t idx	       )       { ifcrashdbg(idx >= 4); return row[idx];       }
	__force_inline const vec4f& operator[](uint8_t idx	       ) const { ifcrashdbg(idx >= 4); return row[idx];       }
	__force_inline float&       operator()(uint8_t i, uint8_t j)       { 					   return mem[i * 4 + j]; }
	__force_inline const float& operator()(uint8_t i, uint8_t j) const { 					   return mem[i * 4 + j]; }

		  float*        begin()        { return mem.begin();  }
		  float*        end()          { return mem.end();    }
	const float*        begin()  const { return mem.begin();  }
	const float*        end()    const { return mem.end();    }
	constexpr size_t    bytes()  const { return mem.bytes();  }
	constexpr size_t    length() const { return mem.len();    }
	__force_inline void print()  const 
	{ 
		printf("mat4f %p:\n", (void*)begin());
		printf("( %-5.05f, %-5.05f, %-5.05f, %-5.05f )\n", row[0].x, row[0].y, row[0].z, row[0].w);
		printf("( %-5.05f, %-5.05f, %-5.05f, %-5.05f )\n", row[1].x, row[1].y, row[1].z, row[1].w);
		printf("( %-5.05f, %-5.05f, %-5.05f, %-5.05f )\n", row[2].x, row[2].y, row[2].z, row[2].w);
		printf("( %-5.05f, %-5.05f, %-5.05f, %-5.05f )\n", row[3].x, row[3].y, row[3].z, row[3].w);
		return;
	}
};


__force_inline mat4f::underlying_buffer& operator*(float a, mat4f const& b) { mul(b.mem, a, temporaryBufferMat4f); return temporaryBufferMat4f; }




void MultiplyMat4Vec4(vec4f const& a, mat4f const& b, vec4f& out);
void MultiplyMat4Mat4(mat4f const& a, mat4f const& b, mat4f& out);
void MultiplyMat2Vec2(vec2f const& a, mat2f const& b, vec2f& out);
void MultiplyMat2Mat2(mat2f const& a, mat2f const& b, mat2f& out);




/*
	Returns the following matrix in mat4f& out:
	[ 1, 0, 0, 0 ],
	[ 0, 1, 0, 0 ],
	[ 0, 0, 1, 0 ],
	[ 0, 0, 0, 1 ]
*/
void identity  (					    mat4f& out);


/*
	Returns the following matrix in mat4f& out (where t = translate):
	[ 1, 0, 0, t.x ],
	[ 0, 1, 0, t.y ],
	[ 0, 0, 1, t.z ],
	[ 0, 0, 0, 1   ]
*/
void translate (vec3f const& translate, mat4f& out);


/*
	Returns the following matrix in mat4f& out (where s = scale):
	[ s.x, 0,   0,   0 ],
	[  0, s.y,  0,   0 ],
	[  0,  0,  s.z,  0 ],
	[  0,  0,   0,   1 ]
*/
void scale     (vec3f const& scale,     mat4f& out);


/*
	Returns the following matrix in mat2f& out (where s = scale):
	[ s.x,  0  ],
	[  0,  s.y ]
*/
void scale     (vec2f const& scale, 	mat2f& out);


/*
	Returns the following matrix in mat2f out (where t = radians(angle) ):
	[ cos(t), -sin(t) ],
	[ sin(t),  cos(t) ]
*/
void rotate2d(f32 angle, mat2f& out);


/*
	Returns the following matrix in mat4f& out (where 
	in = [
		m00, m01, m02, m03,
		m04, m05, m06, m07,
		m08, m09, m10, m11,
		m12, m13, m14, m15
	]):
	[ m00, m04, m08, m12 ],
	[ m01, m05, m09, m13 ],
	[ m02, m06, m10, m14 ],
	[ m03, m07, m11, m15 ]
*/
void transposed(mat4f const& in, 		mat4f& out);


/*
	Same as tranposed(); Uses temporary mat4f to directly modify inout.
*/
void transpose (					  mat4f& inout);


/* 
	Full Explanation & Derivation:
	https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/opengl-perspective-projection-matrix
	Quick Derivation (If Link doesn't work, or you're lazy :| ):
	A. glFrustum(left, right, top, bottom, near, far):
		[ 2n/(r - l)     0,      (r + l)/(r - l),      0      ],
		[     0,     2n/(t - b), (t + b)/(t - b),      0      ],
		[     0,         0,      (f + n)/(n - f), 2fn/(n - f) ],
		[     0,         0,            -1,             0,     ]
	B. Take glFrustum(...) and plug
		t = tan(fovy/2) * near
		b = -1 * t
		r = t * windowAspectRatio
		l = -1 * r


	C. Returns the following matrix in mat4f& out (where 
		A = aspectRatio,
		t = radians(fovy),
		n = near, 
		f = far
	):
	[ cot(t / 2) / A,      0, 	       0,         0     ],
	[      0,         cot(t / 2),      0,         0     ],
	[      0,   		   0,     (n+f)/(n-f) 2fn/(n-f) ],
	[	   0,              0,         -1,         0     ]

*/
void perspective(
	float  aspectRatio, 
	float  fovy, 
	float  near, 
	float  far, 
	mat4f& out
);


/* 
	[NOTE]: Formula Calculated using 4x4 inverse formula + regex + python str replace.
	Returns the following matrix in mat4f& out (where m%d%d = in[i][j]):
	[ 1/m00,   0,     0,          0,        ]
	[   0,   1/m11,   0,          0,        ]
	[   0,     0,     0,        1/m32,      ]
	[   0,     0,   1/m23, -m22/(m23 * m32) ]
*/
void inv_perspective(mat4f const& in, mat4f& out);



/*
	Explanation & Derivation:
	https://learnwebgl.brown37.net/08_projections/projections_ortho.html

	Returns the following matrix in mat4f& out (where
		l, r = leftRight[0], leftRight[1]
		t, b = topBottom[0], topBottom[1],
		n, f = nearFar[0]  ,   nearFar[1]
	):
	[ 2/(r - l),      0, 	    0, 	   (r + l)/(l - r) ],
	[     0,      2/(t - b),    0, 	   (t + b)/(b - t) ],
	[     0,          0,	2/(n - f), (f + n)/(n - f) ],
	[     0,          0,        0, 	   	      1 	   ]
*/
void orthographic(
	vec2f  leftRight, 
	vec2f  topBottom, 
	vec2f  nearFar,
	mat4f& out
);


/*
	Same as function above, just easier interface.
*/
void orthographic(
	f32 left,   f32 right, 
	f32 bottom, f32 top,
	f32 near,   f32 far,
	mat4f& out
);



/* 
	Using a Right Handed Coordinate System (+z towards viewer, +x is right, +y is up)
	 +y
	 |
	 |
	 |
	   _____ +x
	 /
	/
	+z (camera looks to origin)
	Compared to Left Handed:
		  +y
		    |
		    |
		    |
	+x _____|
		   /
    	  /
    	-z
	[NOTE]: This is how the coordinate systems look when looking from -Z to the Origin (Use your fingers for demonstration).

	This Function is the World->Camera Transform Matrix.
	given:
	V_world = V_cam * X, where X = Camera's Transform Matrix, Or Camera_Space->World_Space
	Because we want V_cam =>

	V_world * X^-1 = V_cam.

	In Row Major Convention, X = R * T, where T = translation, R = rotation across each basis in 3d.
		==> X^-1 = (R * T)^-1 = T^-1 * R^-1
		<==> View_Matrix = T^-1 * R^-1 = translate(-eyePos) * BasisVectorMatrix (RotateXYZ, ...)
	This link will help - https://stackoverflow.com/questions/349050/calculating-a-lookat-matrix?noredirect=1&lq=1
	NOTE ABOUT LINK CONTENTS: The Matrix is left Handed, but the math is the same.
	
	Resulting View Matrix (World->Camera):
		where xAxis = right, yAxis = newup, zAxis = forward
		[         xaxis.x          yaxis.x          zaxis.x  0 ]
		[         xaxis.y          yaxis.y          zaxis.y  0 ]
		[         xaxis.z          yaxis.z          zaxis.z  0 ]
		[ dot(xaxis,-eye)  dot(yaxis,-eye)  dot(zaxis,-eye)  1 ]
*/
void lookAt(
	vec3f const& eyePos, 
	vec3f const& at, 
	vec3f const& up,
	mat4f& 		 out
);


/* 
	Inverse of a homogenous transform matrix (V) =
		[ xaxis.x  xaxis.y  xaxis.z  dot(xaxis, eye) ]
		[ yaxis.x  yaxis.y  yaxis.z  dot(yaxis, eye) ]
		[ zaxis.x  zaxis.y  zaxis.z  dot(zaxis, eye) ]
		[    0 	      0 	   0       	    1    	 ]
*/
void inv_lookAt(
	vec3f const& eyePos, 
	vec3f const& at, 
	vec3f const& up,
	mat4f& 		 out
);


/*
	Same as the previous inv_lookAt(),
	except it takes the produced matrix 
	instead of the args to the original matrix. (Still doesn't work, when needed [eventually then fix this])
*/
void inv_lookAt(const mat4f& in, mat4f& out);



/* 
	Source Code: https://github.com/willnode/N-Matrix-Programmer/blob/master/Info/Matrix_4x4.txt
*/
void inverse(
	mat4f const& in,
	mat4f&       out
);


/*
	Source (God Bless you!): https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html
*/
void inverseSimd(
	mat4f const& in,
	mat4f& 		 out
);


/* 
	Returns the 2D Model Matrix 'TRS' in mat2f out (where t = radians(rotationAngle), s = scale, t = translate):
	[ s.x * cos(t), -s.y * sin(t), t.x, 0 ],
	[ s.x * sin(t),  s.y * cos(t), t.y, 0 ],
	[     0,             0, 	    1,  0 ],
	[     0,             0, 	    0,  1 ]
*/
void modelMatrix2d(
	math::vec2f const& translate,
	math::vec2f const& scale,
	f32 			   rotationAngle,
	math::mat4f& 	   out
);


#pragma GCC diagnostic pop


using point2 = vec2f;
using point3 = vec3f;
using point4 = vec4f;




NAMESPACE_MATH_END