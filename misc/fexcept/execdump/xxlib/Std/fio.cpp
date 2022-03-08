#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#include "fio.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
#if defined(__HWIN32__)
#define NOFILE INVALID_HANDLE_VALUE
BOOL MYRTLEXP OpenMAPFileRO( PMAPFileRO mf, CONSTSTR fnm,int mode )
  {
     do{
       mf->Handle = CreateFile( fnm, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0 );
       if ( mf->Handle == NOFILE ) return FALSE;

       mf->MaxSize = GetFileSize(mf->Handle,NULL);

       mf->Map = CreateFileMapping( mf->Handle, NULL, PAGE_READONLY, 0, mf->MaxSize, NULL );
       if ( !mf->Map || mf->Map == NOFILE )
         break;

       mf->View = (LPBYTE)MapViewOfFile( mf->Map, FILE_MAP_READ, 0, 0, mf->MaxSize );
       if ( !mf->View ) break;

       mf->Position = 0;
       return TRUE;
     }while(0);

     mf->DoClose();

 return FALSE;
}

#undef NOFILE
#endif

//------------------------------------------------------------------------
int MYRTLEXP HWriteText( HFile& dest, CONSTSTR fmt, va_list a )
  {  char str[1000];
     int  rc = VSNprintf( str, sizeof(str), fmt, a );
     if ( !rc ) return 0;
 return (int)dest.Write( str, rc );
}

//------------------------------------------------------------------------
BOOL MYRTLEXP HCopyFile( HFile& dest, HFile& src, DWORD sz )
  {  BYTE  buff[ 1000 ];
     DWORD nw, cp;

     do{
       nw = Min( sz, (DWORD)sizeof(buff) );
       cp = src.Read(buff,nw);
       if ( cp > nw )
         return FALSE;

       if ( cp == nw ) {
         cp = dest.Write(buff,nw);
         if ( cp != nw )
           return FALSE;
       } else {
         dest.Write(buff,cp);
         return FALSE;
       }

       sz -= nw;
     }while( sz );

 return TRUE;
}
