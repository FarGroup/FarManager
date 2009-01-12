#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

MyString MYRTLEXP FindFilePath( CONSTSTR path,CONSTSTR sep,CONSTSTR fname )
  {  CONSTSTR  m;
     int       cn = StrColCount( path,sep );
     MyString  s;

     for ( int n = 1; n <= cn; n++ ) {
       m = (char*)StrGetCol( path,n,sep );
       s = MakePathName( m,fname );
       if ( FIO_ACCESS(s.Text(),accEXIST) ) return s;
     }
     if ( FIO_ACCESS(fname,accEXIST) )
       return fname;
      else
       return "";
}
