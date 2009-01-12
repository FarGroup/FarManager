#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

BOOL MYRTLEXP FindFileText( const MyString& fname,BYTE bytes[],DWORD count,BOOL Case )
  {  BYTE  fbuff[ 1000 ];
     int   file;
     DWORD rcount,roff;
     BOOL  res = FALSE;

     if ( count == 0 ) return TRUE;
     file = FIO_OPEN( fname.Text(),O_RDONLY );
     if ( file == -1 ) return FALSE;
     roff = 0;
     rcount = FIO_READ( file,fbuff,sizeof(fbuff) );
     while( rcount > count ) {
       if ( roff+count > rcount ) {
         MemMove( fbuff,fbuff+roff,rcount-roff );
         rcount = FIO_READ( file,fbuff+rcount-roff,sizeof(fbuff)-(rcount-roff) );
         if ( rcount <= 0 ) break;
         roff = 0;
       }
       if ( BuffCmp( fbuff+roff,bytes,count,Case ) ) { res = TRUE; break; }
       roff++;
     }
     FIO_CLOSE( file );

 return res;
}
