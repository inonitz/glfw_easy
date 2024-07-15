#pragma once
#include "vec.hpp"
#include "matrix.hpp"
#include <cmath>




constexpr f32 DivByZeroEpsilon = 1e-20;


#define DIVISOR_NOT_ZERO(val) (val + DivByZeroEpsilon)
#define INVERSE(val) (1.0f / DIVISOR_NOT_ZERO(val))


using Vec3 = math::vec3f;
using Vec4 = math::vec4f;
using Mat4 = math::mat4f;
using GridPosition = math::vec3u;
using Pressure = f32;
using Velocity = f32;


inline math::vec3u cvtu32(math::vec3f const& A) { return _mm_cvttps_epi32(A.xmm); }
inline math::vec3i cvti32(math::vec3f const& A) { return _mm_cvttps_epi32(A.xmm); }
inline math::vec4u cvtu32(math::vec4f const& A) { return _mm_cvttps_epi32(A.xmm); }
inline math::vec4i cvti32(math::vec4f const& A) { return _mm_cvttps_epi32(A.xmm); }

inline math::vec3f cvtf32(math::vec3u const& A) { return _mm_cvtepi32_ps(A.xmm); }
inline math::vec3f cvtf32(math::vec3i const& A) { return _mm_cvtepi32_ps(A.xmm); }
inline math::vec4f cvtf32(math::vec4u const& A) { return _mm_cvtepi32_ps(A.xmm); }
inline math::vec4f cvtf32(math::vec4i const& A) { return _mm_cvtepi32_ps(A.xmm); }


#define LERP(val_x, val_y, __t) (val_y * __t + (1.0f - __t) * val_x) \


#define BILERP(val_x, val_y, val_z, val_w, __t0, __t1) \
	LERP( \
		LERP(val_x, val_y, __t0), \
		LERP(val_z, val_w, __t0), \
		__t1 \
	) \


/*
	Same As Naive Version Except Change of Arg Order in C:
	C = {
		C_000, C_001, C_010, C_011,
		C_100, C_101, C_110, C_111 
	} (I transposed it)
	More specifically, the order is changed such that the columns can be multiplied straight away by their respective constant,
	instead of needlessly shuffling:
	Old  => New
	C[0] => C[0],
	C[2] => C[1],
	C[4] => C[2],
	C[6] => C[3],

	C[1] => C[4],
	C[3] => C[5]
	C[5] => C[6]
	C[7] => C[7]	
*/
inline f32 TriLerpSimd(alignsz(16) std::array<f32, 8> const& C, math::vec3f const& ti)
{
    __m128 Ct, Comt, _t, _omt, A0, A1, B, t, omti;
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
	union M128
	{
		__m128 v;
		struct alignpk(16) {
			float x, y, z, w;
		};
	};
	#pragma GCC diagnostic pop


    t    = _mm_load_ps(ti.begin());              /* t = ti           */
    omti = _mm_sub_ps(_mm_set_ps1(1.0f), t); /* omti = 1.0f - ti */
    _omt = _mm_shuffle_ps(omti, omti, 0); /* omt = { 1.0f - ti[0] }*/
    _t   = _mm_shuffle_ps(t, t, 0);       /* _t  = { ti[0] }       */


    Comt = _mm_load_ps(&C[0]); /* Comt = C[0 -> 3] */
    Ct   = _mm_load_ps(&C[4]); /* Ct   = C[4 -> 7] */
    A0 = _mm_mul_ps(Ct, _t);     /* A0 = C[0 -> 3] * (1.0f - ti[0])        */
    A1 = _mm_mul_ps(Comt, _omt); /* A1 = C[4 -> 7] * ti[0]                 */
    B  = _mm_add_ps(A0, A1);     /* B  = lerp(C[0 -> 3], C[4 -> 7], ti[0]) */


    _omt = _mm_shuffle_ps(omti, omti, 0b11); /* _omt = { 1.0f - ti[2] } */
    _t   = _mm_shuffle_ps(t, t, 0b11);       /* _t   = { ti[2] }        */
    
    _t = _mm_shuffle_ps(_t, _omt, 0); /* _t = { ti[2], ti[2], 1.0f - ti[2], 1.0f - ti[2] }*/
    B = _mm_shuffle_ps(B, B, 0b00 | (0b10 << 2) | (0b01 << 4) | (0b11 << 6)); /* Convert to B[0], B[2], B[1], B[3] */
    
	B = _mm_mul_ps(B, _t);
    /* bsB = before_shuffle_B 
        B = {
            bsB.x * ti[2],
            bsB.z * ti[2],
            bsB.y * (1.0f - ti[2]),
            bsB.w * (1.0f - ti[2]),
        }
    */
    B = _mm_shuffle_ps(B, B, 0b00 | (0b10 << 2) | (0b01 << 4) | (0b11 << 6)); /* Convert back to B[0], B[1], B[2], B[3] */
    B = _mm_hadd_ps(B, B);
    /*
        B = {
            lerp(bsB.x, bsB.y, ti[2]),
            lerp(bsB.z, bsB.w, ti[2]),
            ...,
        }
    */

    M128 cvt{B};
    return cvt.x * (1.0f - ti[1]) + cvt.y * ti[1];
}




/*
	In a Coordinate system where 
	X+ = Right,
	Y+ = Up,
	Z+ = Towards Viewer,
	The Order of Args in C = 
	{ 
		C_000, C_100,  
		C_001, C_101,  
		C_010, C_110,  
		C_011, C_111  
	}
	First we interpolate over the X axis, then Z then Y.

*/
inline f32 TriLerpNaive(alignsz(16) std::array<f32, 8> const& C, math::vec3f const& t)
{
	math::vec4f C2 = {
		LERP(C[0], C[1], t.x),
		LERP(C[2], C[3], t.x),
		LERP(C[4], C[5], t.x),
		LERP(C[6], C[7], t.x)
	};
	math::vec2f C1 = {
		LERP(C2[0], C2[1], t.z),
		LERP(C2[2], C2[3], t.z),
	};
	return LERP(C1[0], C1[1], t.y);
}



/* Solve for Linear System Ax = b, where A is a Unit-Lower-Triangular Matrix. */
template<u16 __N> void BackwardSolver(Matrixf<__N> A, Vectorf<__N>& x, Vectorf<__N> b)
{
	f32 sum = 0.0f;


	x[0] = b[0];
	for(size_t i = 1; i < __N; ++i) 
	{
		sum = 0.0f;
		for(size_t j = 0; j < i; ++j) { /* sum[ A(row i from 0:i) * x(from 0:i) ]*/
			sum += A(i, j) * x[j];
		}
		x[i] = b[i] - sum;
	}
}


template<u16 __N> void ForwardSolver(Matrixf<__N> A, Vectorf<__N>& x, Vectorf<__N> b)
{
	f32 sum = 0.0f;
	
	x[__N - 1] = b[__N - 1] * ( 1.0f/A(__N - 1, __N - 1) ); /* x[n] = b[n] - A[n, n] */
	for(size_t i = __N - 2; i >= 0; --i) 
	{
		sum = 0.0f;
		for(size_t j = i; j < __N; ++j) { /* sum[ A(row i from i:n) * x(from i:n) ]*/
			sum += A(i, j) * x[j];
		}

		x[i] = b[i] - sum;
		x[i] *= (1.0f / A(i, i));
	}
	return;
}



/* 
	Matrix 'L' MUST be init to 0!
	A will be modified, make sure it is a copy.
	(
		Unfortunately I didn't spend enough time to optimize the algorithm,
		besides extracting the diagonal first. 
	)
*/
// template<u16 __N> void IncompleteCholeskyPreconditioner(Matrixf<__N> A, Matrixf<__N>& L)
// {
// 	Vectorf<__N> diagRoots{0.0f};
// 	f32 tmp{0.0f};
	
// 	for(u16 i = 0; i < __N; ++i) {
// 		diagRoots[i] = INVERSE( std::sqrt(A(i, i)) ); 
// 	}


// 	for(u16 k = 0; k < __N - 1; ++k)
// 	{
// 		for(u16 i = k + 1; i < __N; ++i) {
// 			L(k, i) = A(k, i) * diagRoots[k];
// 		}
// 		for(u16 i = k + 1; i < __N; ++i) 
// 		{	
// 			/* Blocking Can Improve the performance of this significantly for better cache usage. */
// 			for(u16 j = i; j < __N; ++j) {
// 				tmp = L(k, i) * L(k, j); /* element-wise-mul between the upper row in L to the upper column in L */
// 				tmp *= boolean( A(i, j) != 0.0f );
// 				A(i, j) -= tmp;
// 			}
// 		}
// 	}
// 	return;
// }


/* 
	Use SparseMatrix Struct for this, 
	this is inefficient as hell without a bitmap or something (or Blocking for that matter lol)
	Source (Only Pseudocode Correct): https://en.wikipedia.org/wiki/Incomplete_Cholesky_factorization

	NOTE: This is The Triangular-Lower Part L. The True preconditioner is (L * L^T)
*/
template<u16 __N> void IncompleteCholeskyPreconditioner(Matrixf<__N>& A)
{
	Vectorf<__N> diagRoots{0.0f};
	// f32 tmp{0.0f};
	
	for(u16 i = 0; i < __N; ++i) {
		diagRoots[i] = INVERSE( std::sqrt(A(i, i)) ); 
	}


	for(size_t k = 0; k < __N; ++k)
	{
		for(size_t i = k + 1; i < __N; ++i) {
			if(A(i, k) != 0.0f) { A(i, k) *= diagRoots[k]; }
		}
		
		for(size_t j = k + 1; j < __N; ++j) {
			for(size_t i = j; i < __N; ++i) {
				A(i, j) -= A(i, k)*A(j, k);
			}
		}
	}
	for(size_t i = 0; i < __N; ++i) { 
		std::fill(&A.data()[i+1 + i*__N], &A.data()[__N - 1 + i*__N], 0.0f);
	}


	return;
}




template<u16 N> bool PreconditionedConjugateGradient(Matrixf<N> A, Matrixf<N> Minv, Vectorf<N>& initialX, Vectorf<N> b, f32 tolerance = 1e-6f, u16 max_iter = 100)
{
	Vectorf<N> residue = b, z, p, Ap, tmp;
	f32 alpha, beta;


	if(initialX.mag() > 1.0f) { /* This is expensive, so just incase initialX is NOT the 0 vector. */
		residue -= A * initialX;
	}


	z = Minv * residue; /* z_0 = M^-1 * r0 */
	p = z; /* p1 */
	Ap = A * p; /* Save computation cost */


	alpha = INVERSE(dot(p, Ap));       /* (p1_T * Ap)^-1  */
	tmp   = dot(residue, z);      /* r0_T * z0 	  */
	alpha *= tmp;
	initialX += alpha * p; 				/* x1 = x0 + a1*p1 */
	residue  -= alpha * Ap; 			/* r1 = r0 - a1*Ap0*/
	--max_iter;


	do {
		z = Minv * residue; 			   /* z_k = M^-1 * r_k */
		beta = dot(residue, z) * tmp; /* beta_k =  */
		p = z + beta * p;				   /* p_k+1  */
		Ap = A * p; 					   /* Ap_k+1 */
		

		tmp   = dot(residue, z);
		alpha = INVERSE(dot(p, Ap));
		alpha *= tmp; 			           /* a_k+1 */
		initialX += alpha * p;             /* x_k+1 */
		residue  -= alpha * Ap;            /* r_k+1 */

		--max_iter;
	} while(max_iter && residue.mag() > tolerance);


	return max_iter == 0; /* Success if not reached 0. */
}