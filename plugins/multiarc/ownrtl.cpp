#ifdef __USE_OWN_RTL__

/* objbase.h */
#define _OBJBASE_H_
/* oleauto.h */
#define _OLEAUTO_H_
/* oleidl.h */
#define __oleidl_h__
/* ole2.h */
#define _OLE2_H_
/* stdlib.h */
#define __STDLIB_H
/* new.h */
#define __NEW_H
/* alloc.h */
#define __ALLOC_H
/* memory.h */
#define __STD_MEMORY
/* mem.h */
#define __MEM_H
/* stddefs.h */
#define __RWSTDDEFS_H__

/*
Если определено _check_mem, то будет увеличиваться/уменьшаться
переменная _check_mem_DAT при вызове malloc/realloc/free
*/
#ifdef _DEBUG
#define _check_mem
#endif

#ifndef _SIZE_T
  #define _SIZE_T
  typedef unsigned size_t;
#endif

/* stdlib.h */
void *malloc(size_t size);
void *realloc(void *block, size_t size);
void free(void *block);

/* mem.h */
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

/* string.h */
#define __STRING_H
#define __STDC__
#define __STDIO_H
char *strdup(const char *s);
char *strchr(char *s, int c);
char *strrchr(char *s, int c);
char *strncpy(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int stricmp(const char *s1, const char *s2);
int strnicmp(const char *s1, const char *s2, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char * str);
char *strcat(char * dst, const char * src);
char *strcpy(char * dst, const char * src);

#ifdef __cplusplus
#ifndef __NEW_DEFINED
#define __NEW_DEFINED
void *operator new(size_t size)
{
  size = size ? size : 1;
  return malloc(size);
}

void *operator new[] (size_t size)
{
  return::operator new(size);
}
void *operator new(size_t /*size */ , void *p)
{
  return p;
}
void operator delete(void *p)
{
  free(p);
}
void operator delete[] (void *ptr)
{
  ::operator delete(ptr);
}
#endif // __NEW_DEFINED
#endif // __cplusplus

#include "plugin.hpp"

#ifdef __cplusplus
template <class T>inline const T&Min(const T &a, const T &b) { return a<b?a:b; }
template <class T>inline const T&Max(const T &a, const T &b) { return a>b?a:b; }
#endif

HANDLE heapNew = NULL;

#ifdef _check_mem      // проверка и подсчет выделенной памяти

DWORD _check_mem_DAT;  // счетчик, должен инициализироваться нулем при старте

void *malloc(size_t size)
{
  void *p = NULL;

  if (heapNew)
    {
      p = HeapAlloc(heapNew, HEAP_ZERO_MEMORY, size);
      if (p)
        _check_mem_DAT += HeapSize(heapNew, 0, p);
    }
  return p;
}

void *realloc(void *block, size_t size)
{
  void *p = NULL;

  if (heapNew != NULL)
    {
      if (!size)
        {
          if(block)
          {
            _check_mem_DAT -= HeapSize(heapNew, 0, block);
            HeapFree(heapNew, 0, block);
          }
        }
      else
        {
          if (block == NULL)
            p = HeapAlloc(heapNew, HEAP_ZERO_MEMORY, size);
          else
            {
              _check_mem_DAT -= HeapSize(heapNew, 0, block);
              p = HeapReAlloc(heapNew, HEAP_ZERO_MEMORY, block, size);
            }
          if (p)
            _check_mem_DAT += HeapSize(heapNew, 0, p);
        }
    }
  return p;
}

void free(void *block)
{
  if (block != NULL && heapNew)
    {
      _check_mem_DAT -= HeapSize(heapNew, 0, block);
      HeapFree(heapNew, 0, block);
    }
}
#else // обычный режим без проверки
void *malloc(size_t size)
{
  if (heapNew)
    return HeapAlloc(heapNew, HEAP_ZERO_MEMORY, size);
  return NULL;
}

void *realloc(void *block, size_t size)
{
  void *p = NULL;

  if (heapNew != NULL)
    {
      if (!size)
      {
        if(block) HeapFree(heapNew, 0, block);
      }
      else
        {
          if (block == NULL)
            p = HeapAlloc(heapNew, HEAP_ZERO_MEMORY, size);
          else
            p = HeapReAlloc(heapNew, HEAP_ZERO_MEMORY, block, size);
        }
    }
  return p;
}

void free(void *block)
{
  if (block != NULL && heapNew)
    HeapFree(heapNew, 0, block);
}
#endif // _check_mem

void *memcpy(void *dest, const void *src, size_t n)
{
  BYTE *d = (BYTE *) dest, *s = (BYTE *) src;

  while (n--)
    {
      *d = *s;
      ++d;
      ++s;
    }
  return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
  if (n && (dest != src))
    {
      BYTE *s = (BYTE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, n);

      if (s)
        {
          memcpy(s, src, n);
          memcpy(dest, s, n);
          HeapFree(GetProcessHeap(), 0, (void *) s);
        }
    }
  return dest;
}

void *memset(void *s, int c, size_t n)
{
  BYTE *dst = (BYTE *) s;

  while (n--)
    {
      *dst = (BYTE) (unsigned) c;
      ++dst;
    }
  return s;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
  if (!n)
    return (0);
  BYTE *b1 = (BYTE *) s1, *b2 = (BYTE *) s2;

  while (--n && *b1 == *b2)
    {
      ++b1;
      ++b2;
    }
  return (*b1 - *b2);
}

const char *strchr(const char *s, int c)
{
  while (*s)
    {
      if (*s == c)
        return s;
      ++s;
    }
  return NULL;
}

char *strchr(char *s, int c)
{
  while (*s)
    {
      if (*s == c)
        return s;
      ++s;
    }
  return NULL;
}

char *strrchr(char *s, int c)
{
  register const char *ss;
  register size_t i;

  for (i = strlen(s) + 1, ss = s + i; i; --i)
    {
      if (*(--ss) == (char) c)
        return ((char *) ss);
    }
  return NULL;
}

char *strncpy(char *dest, const char *source, size_t n)
{
  char *src = dest;

  while (n && 0 != (*dest++ = *source++))
    --n;
  *dest = 0;
  return (src);
}

int strcmp(const char *s1, const char *s2)
{
  BYTE *b1 = (BYTE *) s1, *b2 = (BYTE *) s2;

  while (*b1 && *b1 == *b2)
    {
      ++b1;
      ++b2;
    }
  return (*b1 - *b2);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  if (!n)
    return (0);
  while (--n && *s1 && *s1 == *s2)
    {
      ++s1;
      ++s2;
    }
  return (*(BYTE *) s1 - *(BYTE *) s2);
}

char *strdup(const char *s)
{
  char *p=NULL;
  if(s)
  {
    int len=strlen(s)+1;
    p=static_cast<char*>(malloc(len));
    if(p)
      memcpy(p, s, len);
  }
  return p;
}

size_t strlen (const char * str)
{
        const char *eos = str;

        while( *eos++ ) ;

        return( (int)(eos - str - 1) );
}

char * strcat (char * dst, const char * src)
{
        char * cp = dst;

        while( *cp )
                cp++;                   /* find end of dst */

        while( *cp++ = *src++ ) ;       /* Copy src to end of dst */

        return( dst );                  /* return dst */

}

char *strcpy(char * dst, const char * src)
{
        char * cp = dst;

        while( *cp++ = *src++ )
                ;               /* Copy src over dst */

        return( dst );
}

int stricmp(const char *s1, const char *s2)
{
  extern struct FarStandardFunctions FSF;
  return FSF.LStricmp(s1, s2);
}

int strnicmp(const char *s1, const char *s2, size_t n)
{
  extern struct FarStandardFunctions FSF;
  return FSF.LStrnicmp(s1, s2, n);
}

#endif // __USE_OWN_RTL__
