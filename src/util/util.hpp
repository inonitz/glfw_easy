#ifndef __UTIL_HEADER__
#define __UTIL_HEADER__
#include "macro.hpp"
#include "types.hpp"


/*
	[NOTE]: 
		Just use the ternary operator [?], 
		it'll be optimized to a conditional move which is way
		better than this, which is ~5 instructions (atleast on x86)
	if cond:
		var *= false       => var = 0;
		var += true * val  => var = val;
	else:
		var *= true 	   => var = var;
		var += false * val => var += 0;
*/
#define CONDITIONAL_SET(var, val, cond) \
	var *= !boolean(cond); \
	var += boolean(cond) * (val); \

inline std::uintptr_t __outv = 0;
#define CONDITIONAL_SET_PTR(ptr, ptr_val, cond) \
	__outv = __rcast(std::uintptr_t, ptr); \
	__outv *= !boolean(cond); \
	__outv += boolean(cond) * __rcast(std::uintptr_t, ptr_val); \
	ptr = __rcast(decltype(ptr), __outv); \

#define SET_BIT_AT(to_set, bit_index, bool_val) \
    to_set &= ~(1 << bit_index); \
    to_set |= ( __scast(  decltype( sizeof(to_set) ), bool_val  ) << bit_index); \




namespace util {


/* 
	for whatever reason you may need this 
*/
__force_inline size_t __readtsc() {
    u32 lo, hi;
    __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
    return ((size_t)hi << 32) | lo;
}


template<typename T> constexpr T round2(T v) 
{
	static_assert(detail::__is_integral_type<T>::value, 
	"Value must be an Integral Type! (Value v belongs to group N [0 -> +inf]. ");
	
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	++v;
	return v;
}

template<typename T> constexpr T roundN(T powof2, T v) 
{
	static_assert(detail::__is_integral_type<T>::value, 
		"Value must be an Integral Type! (Value v belongs to group N [0 -> +inf]. "
	);

	const auto rem = v & ( powof2 - 1);
	return (v - rem) + boolean(rem) * powof2; 
}

template<typename T> __force_inline void __memset(T* p, u64 count, T val)
{
	for(u64 i = 0; i < count; ++i) {
		*p = val;
	}
	return;
}


template byte   round2(byte v);
template char_t round2(char_t v);
template u64    round2(u64 v);
template u32    round2(u32 v);
template u16    round2(u16 v);
template i64    round2(i64 v);
template i32    round2(i32 v);
template i16    round2(i16 v);
template byte   roundN(byte   powof2, byte   v);
template char_t roundN(char_t powof2, char_t v);
template u64    roundN(u64    powof2, u64 	 v);
template u32    roundN(u32    powof2, u32 	 v);
template u16    roundN(u16    powof2, u16 	 v);
template i64    roundN(i64    powof2, i64 	 v);
template i32    roundN(i32    powof2, i32 	 v);
template i16    roundN(i16    powof2, i16 	 v);
template void __memset(byte*   addr, u64 amount_values, byte   value = DEFAULT8);
template void __memset(char_t* addr, u64 amount_values, char_t value = DEFAULT8);
template void __memset(u64*    addr, u64 amount_values, u64    value = DEFAULT64);
template void __memset(u32*    addr, u64 amount_values, u32    value = DEFAULT32);
template void __memset(u16*    addr, u64 amount_values, u16    value = DEFAULT16);
template void __memset(i64*    addr, u64 amount_values, i64    value = DEFAULT64);
template void __memset(i32*    addr, u64 amount_values, i32    value = DEFAULT32);
template void __memset(i16*    addr, u64 amount_values, i16    value = DEFAULT16);
template void __memset(f32*    addr, u64 amount_values, f32    value = __scast(f32, DEFAULT32));
template void __memset(f64*    addr, u64 amount_values, f64    value = __scast(f32, DEFAULT64));


} // namespace util


#endif