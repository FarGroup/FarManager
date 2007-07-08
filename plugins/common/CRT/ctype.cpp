#include "crt.hpp"

extern __inline int __cdecl isdigit(int c)
{
  return (c>='0' && c<='9');
}

extern __inline int __cdecl iswdigit(wint_t c)
{
  return (c>='0' && c<='9');
}

extern __inline int __cdecl isspace(int c)
{
  return (c==0x20 || (c>=0x09 && c<=0x0D));
}

extern __inline int __cdecl iswspace(wint_t c)
{
  return (c==0x20 || (c>=0x09 && c<=0x0D));
}

extern __inline int __cdecl isxdigit(int c)
{
  return ((c>='0' && c<='9') || (c>='A' && c<='F'));
}

extern __inline int __cdecl iswxdigit(wint_t c)
{
  return ((c>='0' && c<='9') || (c>='A' && c<='F'));
}

