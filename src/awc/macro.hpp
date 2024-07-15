#ifndef __AWC_MACRO_DEFINITION_HEADER__
#define __AWC_MACRO_DEFINITION_HEADER__
#include "util/types.hpp"


#define AWC_LIB_CONTEXT_MAX              (0b111)
#define AWC_LIB_INIT_MASK                (0x1)
#define AWC_LIB_ATLEAST_ONE_CONTEXT_MASK (0b00000010)
#define AWC_LIB_ACTIVE_CONTEXT_MASK      (0b00011100)
#define AWC_LIB_CONTEXT_COUNT_MASK       (0b11100000)

#define AWC_LIB_INIT_SHIFT                (0)
#define AWC_LIB_ATLEAST_ONE_CONTEXT_SHIFT (1)
#define AWC_LIB_ACTIVE_CONTEXT_SHIFT      (2)
#define AWC_LIB_CONTEXT_COUNT_SHIFT       (5)

#define AWC_LIB_GET_BITS(num, bitmask, shift) ( ( (num) & (bitmask) ) >> (shift) )
#define AWC_LIB_RESET_BITS(num, bitmask) (num) &= ~bitmask
#define AWC_LIB_SET_BITS(num, bitmask) (num) |= bitmask
#define AWC_LIB_RESET_SET_BITS(num, bitmask, newBits) ( (num & ~(bitmask) ) | (newBits) );
#define AWC_LIB_MODIFY_VAR_BITS(num, bitmask, newBits) { num = ( num & ~(bitmask) ) | (newBits); }


namespace AWC {
    bool __awc_lib_initialized();
    bool __awc_lib_atleast_one_existing_context();
    u8   __awc_lib_active_context();
    u8   __awc_lib_context_count();
} // namespace AWC


#endif