#ifndef __UTIL_BASE_TYPE_DEFINITIONS__
#define __UTIL_BASE_TYPE_DEFINITIONS__
#include <cstdint>


typedef unsigned char byte;
typedef signed char   char_t;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef int8_t   i8;
typedef float    f32;
typedef double   f64;
typedef uintptr_t up64;
typedef intptr_t  ip64;


namespace type_trait {


template<bool condition_t, typename TypeA, typename TypeB> 
struct __conditional_operator{};
template<typename TypeA, typename TypeB> 
struct __conditional_operator<false, TypeA, TypeB>{ using type = TypeB; };
template<typename TypeA, typename TypeB> 
struct __conditional_operator<true,  TypeA, TypeB>{ using type = TypeA; };

template<typename T> struct __is_integral_type {};
template<> struct __is_integral_type<u64> { static constexpr bool value = true; };
template<> struct __is_integral_type<u32> { static constexpr bool value = true; };
template<> struct __is_integral_type<u16> { static constexpr bool value = true; };
template<> struct __is_integral_type<u8>  { static constexpr bool value = true; };
template<> struct __is_integral_type<i64> { static constexpr bool value = true; };
template<> struct __is_integral_type<i32> { static constexpr bool value = true; };
template<> struct __is_integral_type<i16> { static constexpr bool value = true; };
template<> struct __is_integral_type<i8>  { static constexpr bool value = true; };
template<> struct __is_integral_type<f32> { static constexpr bool value = false; };
template<> struct __is_integral_type<f64> { static constexpr bool value = false; };

template<typename T, typename U> struct __is_same_type       { static constexpr bool value = false; };
template<typename T>             struct __is_same_type<T, T> { static constexpr bool value = true;  };


template<class T> struct __remove_const_qualifier          { typedef T type; };
template<class T> struct __remove_const_qualifier<const T> { typedef T type; };

} // namespace type_trait


template<typename T> using ref 		 = typename type_trait::__conditional_operator<sizeof(T) <= 8, T, T&		>::type;
template<typename T> using const_ref = typename type_trait::__conditional_operator<sizeof(T) <= 8, T, T const&>::type;
template<typename T> using value_ptr = typename type_trait::__conditional_operator<sizeof(T) <= 8, T, T*>::type;
template<typename T> using imut_type_handle = T const*;
template<typename T> using mut_type_handle  = T*;
template<typename T> using remove_const = typename type_trait::__remove_const_qualifier<T>::type;

typedef imut_type_handle<char_t> k_char;
typedef imut_type_handle<byte>   k_byte;


#endif