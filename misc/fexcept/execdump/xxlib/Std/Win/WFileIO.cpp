#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__HWIN32__)

/* Write WHOLE sz bytes
*/
BOOL MYRTLEXP FIOWrite( HANDLE f, LPCVOID msg, DWORD sz, OVERLAPPED *os, DWORD tmout )
  {  DWORD dw;

     while( sz ){
       if ( os ) {
         ResetEvent( os->hEvent );

         if ( !WriteFile( f, msg, sz, &dw, os ) ) {
           if ( FIO_ERRORN == ERROR_IO_PENDING ) {
             dw = WaitForSingleObject( os->hEvent, tmout );
             if ( dw != WAIT_OBJECT_0 )
               return FALSE;

             if ( !GetOverlappedResult(f,os,&dw,TRUE) )
               return FALSE;
           } else
             return FALSE;
         }
       } else
       if ( !WriteFile( f, msg, sz, &dw, NULL ) )
         return FALSE;

       sz -= dw;
       msg = ((LPBYTE)msg) + dw;
     }
 return TRUE;
}

/* Read WHOLE sz bytes
*/
BOOL MYRTLEXP FIORead( HANDLE f, LPVOID msg, DWORD sz, OVERLAPPED *os, DWORD tmout )
  {  DWORD        dw;

     while( sz ) {
       if ( os ) {
         ResetEvent( os->hEvent );

         if ( !ReadFile( f, msg, sz, &dw, os ) ) {
           if ( FIO_ERRORN == ERROR_IO_PENDING ) {
             dw = WaitForSingleObject( os->hEvent, tmout );
             if ( dw != WAIT_OBJECT_0 )
               return FALSE;

             if ( !GetOverlappedResult(f,os,&dw,TRUE) )
               return FALSE;
           } else
             return FALSE;
         }
       } else
       if ( !ReadFile( f, msg, sz, &dw, NULL ) )
         return FALSE;

       sz -= dw;
       msg = ((LPBYTE)msg) + dw;
     }
 return TRUE;
}

#endif
