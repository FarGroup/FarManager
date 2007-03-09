#ifndef __CRT_HPP__
#define __CRT_HPP__

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif

#include <stdlib.h>
#include <stddef.h>

#ifndef _CONST_RETURN
#define _CONST_RETURN
#if (defined(__cplusplus) && defined(_MSC_VER) && _MSC_VER > 0x700)
#undef _CONST_RETURN
#define _CONST_RETURN  const
#define _CRT_CONST_CORRECT_OVERLOADS
#endif
#endif


#ifdef __cplusplus
  void __cdecl operator delete(void *p);
  void __cdecl operator delete[] (void *ptr);
  void * __cdecl operator new(size_t size);
  void * __cdecl operator new[] (size_t size);
#endif

#ifdef __cplusplus
extern "C"
{
#endif
  int __cdecl isdigit(int c);
  int __cdecl isspace(int c);
  int __cdecl isxdigit(int c);
  void __cdecl free(void *block);
  char * __cdecl _i64toa(__int64 val, char *buf, int radix);
  char * __cdecl _ui64toa(unsigned __int64 val, char *buf, int radix);
  void * __cdecl malloc(size_t size);
  _CONST_RETURN void * __cdecl memchr(const void *buf, int chr, size_t cnt);
  int __cdecl memcmp(const void *buf1, const void *buf2, size_t count);
  void * __cdecl memcpy(void *dst, const void *src, size_t count);
  int __cdecl memicmp(const void *first, const void *last, size_t count);
  void * __cdecl memmove(void *dst, const void *src, size_t count);
  void * __cdecl memset(void *dst, int val, size_t count);
  void * __cdecl realloc(void *block, size_t size);
  _CONST_RETURN char * __cdecl strchr(register const char *s,int c);
  size_t __cdecl strcspn(const char *string, const char *control);
  char * __cdecl strdup(const char *block);
  char * __cdecl strncat(char *first, const char *last, size_t count);
  int __cdecl strncmp(const char *first, const char *last, size_t count);
  _CONST_RETURN char * __cdecl strpbrk(const char *string, const char *control);
  _CONST_RETURN char * __cdecl strrchr(const char *string, int ch);
  _CONST_RETURN char * __cdecl strstr(const char * str1, const char * str2);
  char * __cdecl strtok(char *string, const char *control);
  long __cdecl strtol(const char *nptr, char **endptr, int ibase);
  unsigned long __cdecl strtoul(const char *nptr, char **endptr, int ibase);
#ifdef __cplusplus
};
#endif

#endif
