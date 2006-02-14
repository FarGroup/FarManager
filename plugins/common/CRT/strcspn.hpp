#ifndef __STRCSPN_HPP__
#define __STRCSPN_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  size_t strcspn(const char *string, const char *control);
#ifdef __cplusplus
};
#endif

#endif
