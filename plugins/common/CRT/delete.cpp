#include "crt.hpp"

#ifdef __cplusplus
void __cdecl operator delete(void *block)
{
  free(block);
}

#if defined(_MSC_VER) && _MSC_VER >= 1900
void __cdecl operator delete(void *block, size_t size)
{
	(void)size;
	free(block);
}
#endif

#endif
