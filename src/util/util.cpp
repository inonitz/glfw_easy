#include "util.hpp"



namespace util {


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
template void __memcpy(byte*   destinationptr, byte   const* sourceptr, u64 amount_of_values_to_copy);
template void __memcpy(char_t* destinationptr, char_t const* sourceptr, u64 amount_of_values_to_copy);
template void __memcpy(u64*    destinationptr, u64    const* sourceptr, u64 amount_of_values_to_copy);
template void __memcpy(u32*    destinationptr, u32    const* sourceptr, u64 amount_of_values_to_copy);
template void __memcpy(u16*    destinationptr, u16    const* sourceptr, u64 amount_of_values_to_copy);
template void __memcpy(i64*    destinationptr, i64    const* sourceptr, u64 amount_of_values_to_copy);
template void __memcpy(i32*    destinationptr, i32    const* sourceptr, u64 amount_of_values_to_copy);
template void __memcpy(i16*    destinationptr, i16    const* sourceptr, u64 amount_of_values_to_copy);
template void __memcpy(f32*    destinationptr, f32    const* sourceptr, u64 amount_of_values_to_copy);
template void __memcpy(f64*    destinationptr, f64    const* sourceptr, u64 amount_of_values_to_copy);


} // namespace util