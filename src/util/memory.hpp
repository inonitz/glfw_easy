#ifndef __UTIL_NEW_DELETE_OVERRIDE__
#define __UTIL_NEW_DELETE_OVERRIDE__


void* operator new(decltype(sizeof(0)) n) noexcept(false);
void  operator delete(void * p)           noexcept(true);

void* operator new[](decltype(sizeof(0)) n) noexcept(false);
void  operator delete[](void * p)           noexcept(true);


#endif