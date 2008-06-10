#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

char DECLSPEC ToUpper( char ch )
  {
 return (char)(DWORD_PTR)CharUpper( (LPTSTR)(DWORD_PTR)MK_DWORD(0,ch) );
}

char DECLSPEC ToLower( char ch )
  {
 return (char)(DWORD_PTR)CharLower( (LPTSTR)(DWORD_PTR)MK_DWORD(0,ch) );
}
