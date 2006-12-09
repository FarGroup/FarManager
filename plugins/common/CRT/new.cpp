#include "crt.hpp"

#ifdef __cplusplus
void * __cdecl operator new(size_t size)
{
  return malloc(size);
}
#endif
