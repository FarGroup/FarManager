#ifndef __CRT_HPP__
#define __CRT_HPP__

#ifdef _MSC_VER
 #define VC2005 1400
 #define VC2008 1500
 #define VC2010 1600
 #define VC2012 1700
#endif // _MSC_VER

#ifdef __GNUC__
 #define GCC_VER_(gcc_major,gcc_minor,gcc_patch) (100*(gcc_major) + 10*(gcc_minor) + (gcc_patch))
 #define _GCC_VER GCC_VER_(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#endif // __GNUC__

#ifdef __GNUC__
 #if _GCC_VER >= GCC_VER_(4,5,0)
  #define _CRTIMP
  #define __CRT__NO_INLINE
 #endif
#endif // __GNUC__

#ifdef _MSC_VER
 #pragma once
 #if _MSC_VER >= VC2005
  #undef _USE_DECLSPECS_FOR_SAL
  #define _USE_DECLSPECS_FOR_SAL 1
 #endif
#endif

#define _CRT_FUNCTIONS_REQUIRED 0
#define __STDC_WANT_SECURE_LIB__ 0
#define _STRALIGN_USE_SECURE_CRT 0
#define _CRT_NO_INLINE_DEPRECATED_WCSTOK
#define _CRT_NON_CONFORMING_WCSTOK

#include <stdlib.h>
#include <stddef.h>

#ifdef _MSC_VER
 #if _MSC_VER >= VC2005
  #define _WCTYPE_INLINE_DEFINED
 #endif
#endif

#ifdef __GNUC__
 #define __WCTYPE_INLINES_DEFINED
#endif

#include <wchar.h>
#include <tchar.h>

#ifndef _CONST_RETURN
# define _CONST_RETURN
# define _CONST_RETURN_W
# define __cdecl_inline  __cdecl
# ifdef _MSC_VER
#  if defined(__cplusplus) || _MSC_VER < VC2005
#   undef __cdecl_inline
#   define __cdecl_inline
#   undef _CONST_RETURN_W
#   define _CONST_RETURN_W const
#  endif
#  if defined(__cplusplus) && _MSC_VER >= VC2005
#   undef _CONST_RETURN
#   define _CONST_RETURN const
#   define _CRT_CONST_CORRECT_OVERLOADS
#  endif
# endif
#elif !defined(_CONST_RETURN_W)
# define _CONST_RETURN_W _CONST_RETURN
# define __cdecl_inline  __cdecl
#endif

#ifndef __BORLANDC__
#define WMEM  wchar_t
#define WMINT wchar_t
#else
#define WMEM  void
#define WMINT int
#endif

#ifdef __cplusplus
  void __cdecl operator delete(void *p);
  void __cdecl operator delete[] (void *ptr);
  void * __cdecl operator new(size_t size);
  void * __cdecl operator new[] (size_t size);

  template <class T>
  inline const T&Min(const T &a, const T &b) { return a<b?a:b; }

  template <class T>
  inline const T&Max(const T &a, const T &b) { return a>b?a:b; }
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#undef isdigit
  __inline int __cdecl isdigit(int c)
  {
    return (c>='0' && c<='9');
  }
#undef iswdigit
  __inline int __cdecl iswdigit(wint_t c)
  {
    return (c>='0' && c<='9');
  }
#undef isspace
  __inline int __cdecl isspace(int c)
  {
    return (c==0x20 || (c>=0x09 && c<=0x0D));
  }
#undef iswspace
  __inline int __cdecl iswspace(wint_t c)
  {
    return (c==0x20 || (c>=0x09 && c<=0x0D));
  }
#undef isxdigit
  __inline int __cdecl isxdigit(int c)
  {
    return ((c>='0' && c<='9') || (c>='A' && c<='F'));
  }
#undef iswxdigit
  __inline int __cdecl iswxdigit(wint_t c)
  {
    return ((c>='0' && c<='9') || (c>='A' && c<='F'));
  }
  void __cdecl free(void *block);
  char * __cdecl _i64toa(__int64 val, char *buf, int radix);
  wchar_t * __cdecl _i64tow(__int64 val, wchar_t *buf, int radix);
  char * __cdecl _ui64toa(unsigned __int64 val, char *buf, int radix);
  wchar_t * __cdecl _ui64tow(unsigned __int64 val, wchar_t *buf, int radix);
  void * __cdecl malloc(size_t size);
  void * __cdecl calloc(size_t nmemb, size_t size);
  _CONST_RETURN void * __cdecl memchr(const void *buf, int chr, size_t cnt);
  _CONST_RETURN_W wchar_t * __cdecl_inline wmemchr(const wchar_t *buf, wchar_t chr, size_t cnt);
  int __cdecl memcmp(const void *buf1, const void *buf2, size_t count);
  void * __cdecl memcpy(void *dst, const void *src, size_t count);
#ifdef __BORLANDC__
#undef wmemcpy
#define wmemcpy _wmemcpy
#endif
  WMEM * __cdecl wmemcpy(WMEM *dst, const WMEM *src, size_t count);
  int __cdecl _memicmp(const void *first, const void *last, size_t count);

  void * __cdecl memmove(void *dst, const void *src, size_t count);
#ifdef __BORLANDC__
#undef wmemmove
#define wmemmove _wmemmove
#endif
  WMEM * __cdecl wmemmove(WMEM *dst, const WMEM *src, size_t count);

  void * __cdecl memset(void *dst, int val, size_t count);
#ifdef __BORLANDC__
#undef wmemset
#define wmemset _wmemset
#endif
  WMEM * __cdecl wmemset(WMEM *dst, WMINT val, size_t count);
  void * __cdecl realloc(void *block, size_t size);
  void * __cdecl _recalloc(void *block, size_t num, size_t size);
  _CONST_RETURN char * __cdecl strchr(register const char *s,int c);
  _CONST_RETURN_W wchar_t * __cdecl wcschr(register const wchar_t *s,wchar_t c);
  char * __cdecl strcpy(char *dst, const char *src);
  wchar_t * __cdecl wcscpy(wchar_t *dst, const wchar_t *src);
  size_t __cdecl strcspn(const char *string, const char *control);
  size_t __cdecl wcscspn(const wchar_t *string, const wchar_t *control);
  char * __cdecl strdup(const char *block);
#undef _strdup
#define _strdup strdup
  wchar_t * __cdecl wcsdup(const wchar_t *block);
#undef _wcsdup
#define _wcsdup wcsdup
  char * __cdecl strncat(char *first, const char *last, size_t count);
  wchar_t * __cdecl wcsncat(wchar_t *first, const wchar_t *last, size_t count);
  int __cdecl strncmp(const char *first, const char *last, size_t count);
  int __cdecl wcsncmp(const wchar_t *first, const wchar_t *last, size_t count);
  _CONST_RETURN char * __cdecl strpbrk(const char *string, const char *control);
  _CONST_RETURN_W wchar_t * __cdecl wcspbrk(const wchar_t *string, const wchar_t *control);
  char * __cdecl strncpy(char *dest, const char *src, size_t count);
  wchar_t * __cdecl wcsncpy(wchar_t *dest, const wchar_t *src, size_t count);
  _CONST_RETURN char * __cdecl strrchr(const char *string, int ch);
  _CONST_RETURN_W wchar_t * __cdecl wcsrchr(const wchar_t *string, wchar_t ch);
  _CONST_RETURN char * __cdecl strstr(const char * str1, const char * str2);
  _CONST_RETURN_W wchar_t * __cdecl wcsstr(const wchar_t * str1, const wchar_t * str2);
  char * __cdecl strtok(char *string, const char *control);
//  wchar_t * __cdecl wcstok(wchar_t *string, const wchar_t *control);
  long __cdecl strtol(const char *nptr, char **endptr, int ibase);
  long __cdecl wcstol(const wchar_t *nptr, wchar_t **endptr, int ibase);
  unsigned long __cdecl strtoul(const char *nptr, char **endptr, int ibase);
  unsigned long __cdecl wcstoul(const wchar_t *nptr, wchar_t **endptr, int ibase);
  void __cdecl swab
#if defined(__GNUC__) & !defined(_CRT_SWAB_DEFINED)
                   (const char* b1,char* b2,size_t length);
#else
                   (char* b1,char* b2,int length);
#endif
#ifdef __cplusplus
};
#endif

#if defined(__GNUC__) && !defined(ARRAYSIZE)
#define ARRAYSIZE(a)  (sizeof(a)/sizeof(a[0]))
#endif

// macros for plugin's

#ifdef __GNUC__
#define _i64(n)   n ## ll
#define _ui64(n)  n ## ull
#else
#define _i64(n)   n ## i64
#define _ui64(n)  n ## ui64
#endif

#ifdef _MSC_VER
 #if _MSC_VER < VC2010
  #define nullptr NULL
 #endif
#endif // __GNUC__

#ifdef __GNUC__
 #if _GCC_VER < GCC_VER_(4,6,0)
  #define nullptr NULL
 #endif
#endif // __GNUC__

#endif
