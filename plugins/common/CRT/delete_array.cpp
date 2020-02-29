#include "crt.hpp"

#ifdef __cplusplus
void __cdecl operator delete[] (void *ptr)
{
  ::operator delete(ptr);
}

#if defined(_MSC_VER) && _MSC_VER >= 1900
void __cdecl operator delete[] (void *ptr, size_t size)
{
  (void)size;
  ::operator delete(ptr);
}
#endif

#endif
