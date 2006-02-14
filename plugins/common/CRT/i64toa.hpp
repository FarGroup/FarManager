#ifndef __I64TOA_HPP__
#define __I64TOA_HPP__
#include <stdlib.h>

#ifdef __cplusplus
  char *_i64toa(__int64 val, char *buf, int radix);
  char *_ui64toa(unsigned __int64 val, char *buf, int radix);
#endif

#endif
