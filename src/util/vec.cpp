#include "vec.hpp"



NAMESPACE_MATH_BEGIN




void MultiplyMat4Vec4(vec4f const& a, mat4f const& b, vec4f& out) {
	out.x = dot(a, b.column(0));
	out.y = dot(a, b.column(1));
	out.z = dot(a, b.column(2));
	out.w = dot(a, b.column(3));
}


void MultiplyMat4Mat4(mat4f const& a, mat4f const& b, mat4f& out) {
	mat4f fetch_btransposed = {
		b.column(0),
		b.column(1),
		b.column(2),
		b.column(3)
	};
	vec4f tmp0{};
	for(size_t i = 0; i < 4; ++i) {
		tmp0.x = dot(a[i], fetch_btransposed[0]);
		tmp0.y = dot(a[i], fetch_btransposed[1]);
		tmp0.z = dot(a[i], fetch_btransposed[2]);
		tmp0.w = dot(a[i], fetch_btransposed[3]);
		out[i] = tmp0;
	}
	return;
}


void MultiplyMat2Vec2(vec2f const& a, mat2f const& b, vec2f& out)
{
	/* 
		I HOPE this will get optimized into:
			promote __m64[a] -> __m128[tmp]
			shuffle __m128[tmp]
			multiply 	   __m128[tmp], __m128[b]   -> __m128[tmp]
			horizontal_add __m128[tmp], __m128[tmp] -> __m128[tmp]
			demote __m128[tmp].xy -> __m64[out]
	*/
	vec4f tmp = { a.x, a.x, a.y, a.y };
	tmp *= b.homogenised;
	out = {
		tmp.x + tmp.y,
		tmp.z + tmp.w
	};
	return;
}


void MultiplyMat2Mat2(mat2f const& a, mat2f const& b, mat2f& out)
{
	vec4f tmp0, tmp1, btrans;
	btrans = { b.x0, b.x1, b.y0, b.y1 };
	tmp0 = { a.m00, a.m01, a.m00, a.m01 };
	tmp1 = { a.m10, a.m11, a.m10, a.m11 };
	tmp0 *= btrans;
	tmp1 *= btrans;
	out.row[0] = { tmp0.x + tmp0.y, tmp0.z + tmp0.w };
	out.row[1] = { tmp1.x + tmp1.y, tmp1.z + tmp1.w };
	return;
}




void identity(mat4f& out) {
	out.mem.zero();
	out.m00 = 1.0f;
	out.m11 = 1.0f;
	out.m22 = 1.0f;
	out.m33 = 1.0f;
	return;
}


void identity(mat2f& out) {
	out = { 1.0f, 0.0f, 1.0f, 0.0f };
	return;
}


void translate(vec3f const& translate, mat4f& out) {
	identity(out);
	out[3] = translate.homogenised; 
	out[3].w = 1.0f;
	transpose(out);
	return;
}


void scale(vec3f const& scale, mat4f& out) {
	out.mem.zero();
	out.m00 = scale.x;
	out.m11 = scale.y;
	out.m22 = scale.z;
	out.m33 = 1.0f;
	return;
}


void scale(vec2f const& scale, mat2f& out)
{
	out.mem.zero();
	out.m00 = scale.x;
	out.m11 = scale.y;
	return;
}


void rotate2d(f32 angle, mat2f& out)
{
	angle = radians(angle);
	out.m00 = std::cosf(angle);
	out.m01 = std::sinf(angle);
	out.m11 = out.m00;
	out.m10 = out.m01;
	out.m01 *= -1.0f;
	return;
}


void transposed(mat4f const& in, mat4f& out) {
	for(size_t i = 0; i < 4; ++i) {
		out(i, 0) = in(0, i);
		out(i, 1) = in(1, i);
		out(i, 2) = in(2, i);
		out(i, 3) = in(3, i);
	}
}


void transpose(mat4f& inout) {
	mat4f tmp = inout;
	for(size_t i = 0; i < 4; ++i) {
		inout(i, 0) = tmp(0, i);
		inout(i, 1) = tmp(1, i);
		inout(i, 2) = tmp(2, i);
		inout(i, 3) = tmp(3, i);
	}
	return;
}


void transposed(mat2f const& in, mat2f& out) {
	out = { 
		in.x0, in.x1, 
		in.y0, in.y1 
	};
	return;
}


void transpose(mat2f& inout) {
	/* I'm sure this won't get compiled to a shuffle_ps but whatever... */
	vec2f tmp = { inout.x1, inout.y0 };
	inout.m01 = tmp.x;
	inout.m10 = tmp.y;
	return;
}


void perspective(float aspectRatio, float fovy, float near, float far, mat4f& out) {
	float t, r, nmf;
	aspectRatio = 1.0f / aspectRatio;

	t    = 1.0f / std::tan(fovy * 0.5f);
	r    = t * aspectRatio;
	nmf  = 1.0f / (near - far);
	fovy = 2.0f * far * near;

	out.mem.zero();
	out.m00 = r;
	out.m11 = t;
	out.m22 = (far + near) * nmf;
	out.m23 = -1.0f;
	out.m32 = fovy * nmf;
	// out.mem[ 0] = r;
	// out.mem[ 5] = t;
	// out.mem[10] = (far + near) * nmf;
	// out.mem[11] = -1.0f;
	// out.mem[14] = fovy * nmf;
	return;
}


void inv_perspective(const mat4f &in, mat4f &out)
{
	out.mem.zero();
	math::vec4f inXY = { in(0, 0), in(1, 1), in(3, 2), in(2, 3) };
	math::vec4f one{1.0f};

	inXY = one / inXY;
	out.m00 = inXY.m0;
	out.m11 = inXY.m1;
	out.m23 = inXY.m2;
	out.m32 = inXY.m3;
	out.m33 = -in(2, 2) * inXY.m2 * inXY.m3; 
	return;
}


void orthographic(
	vec2f  leftRight, 
	vec2f  topBottom, 
	vec2f  nearFar,
	mat4f& out
) {
	/* 
		[NOTE]:
		Would prob be more efficient to create a tmp mat4, 
		set it and copy the whole cache line directly to memory,
		instead of writing each float individually to memory 
		(
			The latter is the way its done now in code, 
			although who knows how the compiler will handle it :/
		)
	*/
	f32 rml = 1.0f / (leftRight[1] - leftRight[0]);
	f32 tmb = 1.0f / (topBottom[0] - topBottom[1]);
	f32 nmf = 1.0f / (nearFar[0]   - nearFar[1]  );
	out.mem.zero();

	out.m00 = 2.0f * rml;
	out.m11 = 2.0f * tmb;
	out.m22 = 2.0f * nmf;
	out.m33 = 1.0f;
	out.m03 = -1.0f * (leftRight[1] + leftRight[0]) * rml;
	out.m13 = -1.0f * (topBottom[0] + topBottom[1]) * tmb;
	out.m23 =         (nearFar[0]   + nearFar[1]  ) * nmf;
	return;
}


void orthographic(
	f32 left,   f32 right, 
	f32 bottom, f32 top,
	f32 near,   f32 far,
	mat4f& out
) {
	orthographic({ left, right }, { top, bottom }, { near, far }, out);
	return;
}


void lookAt(
	vec3f const& eyePos, 
	vec3f const& at, 
	vec3f const& up,
	mat4f& 		 out
) {
	vec3f forward, right, newup, trans;

	/* (eye - at).div(|(eye-at)|) because 'forward' is towards the screen (us), 'eye' is AT the screen and 'at' is what we want to look at */
	forward = eyePos - at; 
	forward.normalize();

	/* camera_view_dir x up_basis_vector = right_basis_vector */
	right = cross(up, forward);
	right.normalize();
	
	/* right_basis_vector(x_axis) x forward_basis_vector(z_axis) = up_basis_vector(y_axis) */
	newup = cross(forward, right);

	/* Set View Matrix Basis Vectors (R^-1)*/
	out.mem.zero();
	out[0] = right.homogenised;
	out[1] = newup.homogenised;
	out[2] = forward.homogenised;
	transpose(out);

	/* Set The Translation Vector to (-eyePos, 1.0f) (T^-1) */
	trans  = -1.0f * eyePos;
	out[3] = {
		dot(right,   trans),
		dot(newup,   trans),
		dot(forward, trans),
		1.0f
	};
	return;
}


void inv_lookAt(
	vec3f const& eyePos, 
	vec3f const& at, 
	vec3f const& up,
	mat4f& 		 out
) {
	vec3f forward, right, newup, trans;

	/* Same as lookAt */
	forward = eyePos - at; 		  forward.normalize();
	right   = cross(up, forward); right.normalize();
	newup   = cross(forward, right);
	
	/* Reset the Matrix (out.m33 = 1.0f is the important part )*/
	identity(out);

	/* Set View Matrix Basis Vectors */
	out[0] = right.homogenised;
	out[1] = newup.homogenised;
	out[2] = forward.homogenised;

	/* Set Translation Vector */
	out.m03 = dot(right,   eyePos);
	out.m13 = dot(newup,   eyePos);
	out.m23 = dot(forward, eyePos);
	return;
}


/* 
	DOESNT WORK AS INTENDED, DO NOT USE!!!
*/
void inv_lookAt(const mat4f& in, mat4f& out)
{
	out = in;
	out[3] *= -1.0f;
	out.m33 = 1.0f;
	transpose(out);
	return;
}


void inverse(mat4f const& m, mat4f& out) 
{
	float A2323, A1323, A1223, A0323,
		  A0223, A0123, A2313, A1313, 
		  A1213, A2312, A1312, A1212, 
		  A0313, A0213, A0312, A0212, 
		  A0113, A0112, det;
	

	A2323 = m.z2 * m.w3 - m.w2 * m.z3;
	A1323 = m.y2 * m.w3 - m.w2 * m.y3;
	A1223 = m.y2 * m.z3 - m.z2 * m.y3;
	A0323 = m.x2 * m.w3 - m.w2 * m.x3;
	A0223 = m.x2 * m.z3 - m.z2 * m.x3;
	A0123 = m.x2 * m.y3 - m.y2 * m.x3;
	A2313 = m.z1 * m.w3 - m.w1 * m.z3;
	A1313 = m.y1 * m.w3 - m.w1 * m.y3;
	A1213 = m.y1 * m.z3 - m.z1 * m.y3;
	A2312 = m.z1 * m.w2 - m.w1 * m.z2;
	A1312 = m.y1 * m.w2 - m.w1 * m.y2;
	A1212 = m.y1 * m.z2 - m.z1 * m.y2;
	A0313 = m.x1 * m.w3 - m.w1 * m.x3;
	A0213 = m.x1 * m.z3 - m.z1 * m.x3;
	A0312 = m.x1 * m.w2 - m.w1 * m.x2;
	A0212 = m.x1 * m.z2 - m.z1 * m.x2;
	A0113 = m.x1 * m.y3 - m.y1 * m.x3;
	A0112 = m.x1 * m.y2 - m.y1 * m.x2;

	det =
	  m.x0 * ( m.y1 * A2323 - m.z1 * A1323 + m.w1 * A1223 )
	- m.y0 * ( m.x1 * A2323 - m.z1 * A0323 + m.w1 * A0223 )
	+ m.z0 * ( m.x1 * A1323 - m.y1 * A0323 + m.w1 * A0123 )
	- m.w0 * ( m.x1 * A1223 - m.y1 * A0223 + m.z1 * A0123 );
	det = 1 / det;

	out.x0 = det *   ( m.y1 * A2323 - m.z1 * A1323 + m.w1 * A1223 );
	out.y0 = det * - ( m.y0 * A2323 - m.z0 * A1323 + m.w0 * A1223 );
	out.z0 = det *   ( m.y0 * A2313 - m.z0 * A1313 + m.w0 * A1213 );
	out.w0 = det * - ( m.y0 * A2312 - m.z0 * A1312 + m.w0 * A1212 );
	out.x1 = det * - ( m.x1 * A2323 - m.z1 * A0323 + m.w1 * A0223 );
	out.y1 = det *   ( m.x0 * A2323 - m.z0 * A0323 + m.w0 * A0223 );
	out.z1 = det * - ( m.x0 * A2313 - m.z0 * A0313 + m.w0 * A0213 );
	out.w1 = det *   ( m.x0 * A2312 - m.z0 * A0312 + m.w0 * A0212 );
	out.x2 = det *   ( m.x1 * A1323 - m.y1 * A0323 + m.w1 * A0123 );
	out.y2 = det * - ( m.x0 * A1323 - m.y0 * A0323 + m.w0 * A0123 );
	out.z2 = det *   ( m.x0 * A1313 - m.y0 * A0313 + m.w0 * A0113 );
	out.w2 = det * - ( m.x0 * A1312 - m.y0 * A0312 + m.w0 * A0112 );
	out.x3 = det * - ( m.x1 * A1223 - m.y1 * A0223 + m.z1 * A0123 );
	out.y3 = det *   ( m.x0 * A1223 - m.y0 * A0223 + m.z0 * A0123 );
	out.z3 = det * - ( m.x0 * A1213 - m.y0 * A0213 + m.z0 * A0113 );
	out.w3 = det *   ( m.x0 * A1212 - m.y0 * A0212 + m.z0 * A0112 );
	return;
}




#define MakeShuffleMask(x,y,z,w)           (x | (y<<2) | (z<<4) | (w<<6))
// vec(0, 1, 2, 3) -> (vec[x], vec[y], vec[z], vec[w])
#define VecSwizzleMask(vec, mask)          _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), mask))
#define VecSwizzle(vec, x, y, z, w)        VecSwizzleMask(vec, MakeShuffleMask(x,y,z,w))
#define VecSwizzle1(vec, x)                VecSwizzleMask(vec, MakeShuffleMask(x,x,x,x))
// special swizzle
#define VecSwizzle_0022(vec)               _mm_moveldup_ps(vec)
#define VecSwizzle_1133(vec)               _mm_movehdup_ps(vec)
// return (vec1[x], vec1[y], vec2[z], vec2[w])
#define VecShuffle(vec1, vec2, x,y,z,w)    _mm_shuffle_ps(vec1, vec2, MakeShuffleMask(x,y,z,w))
// special shuffle
#define VecShuffle_0101(vec1, vec2)        _mm_movelh_ps(vec1, vec2)
#define VecShuffle_2323(vec1, vec2)        _mm_movehl_ps(vec2, vec1)
/* 

	for row major matrix
	we use __m128 to represent 2x2 matrix as A = | A0  A1 |
	                                             | A2  A3 |
*/
__force_inline __m128 Mat2Mul(__m128 vec1, __m128 vec2) /* 2x2 row major Matrix multiply A*B */
{
	return _mm_add_ps(
			_mm_mul_ps(                     vec1, VecSwizzle(vec2, 0,3,0,3)),
		    _mm_mul_ps(VecSwizzle(vec1, 1,0,3,2), VecSwizzle(vec2, 2,1,2,1))
		);
}
__force_inline __m128 Mat2AdjMul(__m128 vec1, __m128 vec2) /* 2x2 row major Matrix adjugate multiply (A#)*B */
{
	return _mm_sub_ps(
			_mm_mul_ps(VecSwizzle(vec1, 3,3,0,0), vec2),
		    _mm_mul_ps(VecSwizzle(vec1, 1,1,2,2), VecSwizzle(vec2, 2,3,0,1))
		);

}
__force_inline __m128 Mat2MulAdj(__m128 vec1, __m128 vec2) /* 2x2 row major Matrix multiply adjugate A*(B#) */
{
	return _mm_sub_ps(
			_mm_mul_ps(                     vec1, VecSwizzle(vec2, 3,0,3,0)),
		    _mm_mul_ps(VecSwizzle(vec1, 1,0,3,2), VecSwizzle(vec2, 2,1,2,1))
		);
}



void inverseSimd(
	mat4f const& in,
	mat4f& 		 out
) {
	__m128 A, B, C, D;
	__m128 detA, detB, detC, detD;
	__m128 detSub, detM, rDetM, tr;
	__m128 D_C, A_B, X_, Y_, Z_, W_;
	A = VecShuffle_0101(in.row[0].xmm, in.row[1].xmm);
	B = VecShuffle_2323(in.row[0].xmm, in.row[1].xmm);
	C = VecShuffle_0101(in.row[2].xmm, in.row[3].xmm);
	D = VecShuffle_2323(in.row[2].xmm, in.row[3].xmm);

	detSub = _mm_sub_ps(
		_mm_mul_ps(VecShuffle(in.row[0].xmm, in.row[2].xmm, 0,2,0,2), VecShuffle(in.row[1].xmm, in.row[3].xmm, 1,3,1,3)),
		_mm_mul_ps(VecShuffle(in.row[0].xmm, in.row[2].xmm, 1,3,1,3), VecShuffle(in.row[1].xmm, in.row[3].xmm, 0,2,0,2))
	);
	detA = VecSwizzle1(detSub, 0);
	detB = VecSwizzle1(detSub, 1);
	detC = VecSwizzle1(detSub, 2);
	detD = VecSwizzle1(detSub, 3);

	D_C = Mat2AdjMul(D, C);
	A_B = Mat2AdjMul(A, B);
	X_ = _mm_sub_ps(_mm_mul_ps(detD, A), Mat2Mul(B, D_C));
	W_ = _mm_sub_ps(_mm_mul_ps(detA, D), Mat2Mul(C, A_B));

	detM = _mm_mul_ps(detA, detD);
	Y_ = _mm_sub_ps(_mm_mul_ps(detB, C), Mat2MulAdj(D, A_B));
	Z_ = _mm_sub_ps(_mm_mul_ps(detC, B), Mat2MulAdj(A, D_C));
	detM = _mm_add_ps(detM, _mm_mul_ps(detB, detC));

	tr = _mm_mul_ps(A_B, VecSwizzle(D_C, 0,2,1,3));
	tr = _mm_hadd_ps(tr, tr);
	tr = _mm_hadd_ps(tr, tr);
	detM = _mm_sub_ps(detM, tr);

	const __m128 adjSignMask = _mm_setr_ps(1.f, -1.f, -1.f, 1.f);
	rDetM = _mm_div_ps(adjSignMask, detM);

	X_ = _mm_mul_ps(X_, rDetM);
	Y_ = _mm_mul_ps(Y_, rDetM);
	Z_ = _mm_mul_ps(Z_, rDetM);
	W_ = _mm_mul_ps(W_, rDetM);
	out.row[0].xmm = VecShuffle(X_, Y_, 3,1,3,1);
	out.row[1].xmm = VecShuffle(X_, Y_, 2,0,2,0);
	out.row[2].xmm = VecShuffle(Z_, W_, 3,1,3,1);
	out.row[3].xmm = VecShuffle(Z_, W_, 2,0,2,0);
	return;
}


#undef VecShuffle_2323
#undef VecShuffle_0101
#undef VecShuffle
#undef VecSwizzle_1133
#undef VecSwizzle_0022
#undef VecSwizzle1
#undef VecSwizzle
#undef VecSwizzleMask
#undef MakeShuffleMask




void modelMatrix2d(
	math::vec2f const& translate,
	math::vec2f const& scaling,
	f32 			   rotationAngle,
	math::mat4f& 	   out
) {
	/* To multiply a vec2f by this matrix promote it to { v.x, v.y, 0.0f, 1.0f } to keep the translation component */
	math::mat2f s, r, rs;
	rotate2d(rotationAngle, r);
	scale(scaling, s);
	MultiplyMat2Mat2(r, s, rs);

	out = mat4f{rs};
	out.m03 = translate.x;
	out.m13 = translate.y;
	return;
}



NAMESPACE_MATH_END