#include "headers.hpp"
#pragma hdrstop

extern "C" {
void  __cdecl xf_free(void *__block);
};

void operator delete(void *ptr)
{
  xf_free(ptr);
}
