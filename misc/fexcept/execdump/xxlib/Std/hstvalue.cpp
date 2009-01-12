#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

void HStringValue::DoTypeChange( HVType oldT,HVType newT,LPVOID val )
  {
    if ( newT == HVString ) {
      CONSTSTR m = *((CONSTSTR*)val);
      String.Set( m?m:"" );
      *((CONSTSTR*)val) = String.Text();
    } else
      HValue::DoTypeChange( oldT,newT,val );
}

DWORD HStringValue::DoSizeofData( void ) const
  {
 return (Type == HVString)?String.Length():HValue::DoSizeofData();
}

LPVOID HStringValue::DoPtrData( void ) const
  {
 return (Type == HVString)?String.Text():HValue::DoPtrData();
}
