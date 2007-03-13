#include "crt.hpp"

int __cdecl isdigit(int c)
{
  return (c>='0' && c<='9');
}

int __cdecl iswdigit(wint_t c)
{
  return (c>='0' && c<='9');
}

int __cdecl isspace(int c)
{
  return (c==0x20 || (c>=0x09 && c<=0x0D));
}

int __cdecl iswspace(wint_t c)
{
  return (c==0x20 || (c>=0x09 && c<=0x0D));
}

int __cdecl isxdigit(int c)
{
  return ((c>='0' && c<='9') || (c>='A' && c<='F'));
}

int __cdecl iswxdigit(wint_t c)
{
  return ((c>='0' && c<='9') || (c>='A' && c<='F'));
}
