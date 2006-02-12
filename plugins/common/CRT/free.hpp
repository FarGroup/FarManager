#ifndef __FREE_HPP__
#define __FREE_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void free(void *block);
#ifdef __cplusplus
};
#endif

#endif
