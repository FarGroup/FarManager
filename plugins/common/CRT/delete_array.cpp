#include "delete_array.hpp"
#include "delete.hpp"

#ifdef __cplusplus
void __cdecl operator delete[] (void *ptr)
{
  ::operator delete(ptr);
}
#endif
