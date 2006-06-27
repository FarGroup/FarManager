#ifndef __STRTOL_HPP__
#define __STRTOL_HPP__

#ifdef __cplusplus
extern "C"
{
#endif
  long strtol(const char *nptr, char **endptr, int ibase);
  unsigned long strtoul(const char *nptr, char **endptr, int ibase);
#ifdef __cplusplus
};
#endif

#endif
