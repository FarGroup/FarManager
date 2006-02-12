#include "ctype.hpp"

int isdigit(int c)
{
  return (c>='0' && c<='9');
}

int isspace(int c)
{
  return (c==0x20 || (c>=0x09 && c<=0x0D));
}

int isxdigit(int c)
{
  return ((c>='0' && c<='9') || (c>='A' && c<='F'));
}
