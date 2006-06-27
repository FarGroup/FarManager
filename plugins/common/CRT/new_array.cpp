#include "new_array.hpp"
#include "new.hpp"

#ifdef __cplusplus
void *__cdecl operator new[] (size_t size)
{
  return ::operator new(size);
}
#endif
