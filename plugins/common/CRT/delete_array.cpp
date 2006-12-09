#include "crt.hpp"

#ifdef __cplusplus
void __cdecl operator delete[] (void *ptr)
{
  ::operator delete(ptr);
}
#endif
