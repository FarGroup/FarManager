#include "crt.hpp"

void __cdecl swab
#if defined(__GNUC__) & !defined(_CRT_SWAB_DEFINED)
                 (const char* b1,char* b2,size_t length)
#else
                 (char* b1,char* b2,int length)
#endif
{
#ifdef __GNUC__
  size_t
#else
  int
#endif
         ptr;
  for(ptr=1;ptr<length;ptr+=2)
  {
  	char p=b1[ptr];
    char q=b1[ptr-1];
    b2[ptr-1]=p;
    b2[ptr]=q;
  }
  if(ptr==length) b2[ptr-1]=0;
}
