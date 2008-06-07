#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

char DECLSPEC ToUpper( char ch )
  {
 return (char)CharUpper( (LPTSTR)(DWORD_PTR)MK_DWORD(0,ch) );
}

char DECLSPEC ToLower( char ch )
  {
 return (char)CharLower( (LPTSTR)(DWORD_PTR)MK_DWORD(0,ch) );
}
