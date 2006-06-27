#include "delete.hpp"
#include "free.hpp"

#ifdef __cplusplus
void __cdecl operator delete(void *block)
{
  free(block);
}
#endif
