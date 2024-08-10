#ifndef __AWC2_INTERNAL_STRUCTURE_INSTANCE_HEADER__
#define __AWC2_INTERNAL_STRUCTURE_INSTANCE_HEADER__


namespace AWC2::internal {


struct AWC2Data;
struct AWC2ContextData;


AWC2Data*        __awc2_lib_get_instance();
AWC2ContextData& __awc2_lib_get_context(unsigned char id);
AWC2ContextData& __awc2_lib_get_active_context();
unsigned char    __awc2_lib_get_active_context_id();


} // namespace AWC2::internal


#endif