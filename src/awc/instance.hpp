#ifndef __AWC_INTERNAL_INSTANCE_HEADER__
#define __AWC_INTERNAL_INSTANCE_HEADER__


namespace AWC {


class  AWCData;
struct AWCContext;

AWCData*      __get_instance();
AWCContext*   __get_context(unsigned char id);
AWCContext*   __active_context();
unsigned char __active_context_id();


} // namespace AWC


#endif