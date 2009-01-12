#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if 1
  #define  PROC( v )
  #define  Log( v )
#else
  #define  PROC( v )  INProc __proc v ;
  #define  Log( v )   INProc::Say v
#endif

/****************************************************
   HOStream
 ****************************************************/
HOStream::HOStream( DWORD bsz )
  {
    rBuff  = new BYTE[ bsz+1 ];
    rSize  = bsz;
    rCount = 0;
}

HOStream::~HOStream()
  {
    delete[] rBuff;
}

void  HOStream::Flush( void )       { WriteCompleted(); }
void  HOStream::doReset( void )     { rCount = 0; }
DWORD HOStream::ReadySize( void )   { return rCount; }
void  HOStream::Reset( void )       { doReset(); }
BOOL  HOStream::Assigned( void )    { return doAssigned(); }
BOOL  HOStream::doAssigned( void )  { return TRUE; }

BOOL HOStream::Write( LPVOID _buff,DWORD sz )
  {  LPBYTE buff = (LPBYTE)_buff;
     DWORD  rSz;

     Log(( "iWrite: %p[%d]",buff,sz ));

     for( ; ((int)sz) > 0; sz -= rSz ) {
       rSz = Min( rSize-rCount,sz );

       memcpy( rBuff+rCount,buff,rSz );

       buff   += rSz;
       rCount += rSz;

       if ( rCount >= rSize ) {
         if ( !WriteCompleted() )
           return FALSE;
         rCount = 0;
       }
     }

 return TRUE;
}

BOOL HOStream::Write( CONSTSTR Format,... )
  {  va_list a;
     int     rc, num;

    va_start( a, Format );
      rc = VSNprintf( NULL, 0, Format, a );
    va_end( a );

    if ( !BuffData.Resize( (DWORD)(rc+1) ) )
      return FALSE;

    if ( rc ) {
      va_start( a, Format );
        num = VSNprintf( (char*)BuffData.Ptr(), rc, Format, a );
      va_end( a );
    } else
      num = 0;
    if ( num >= rc )
      ((char*)BuffData.Ptr())[ num ] = 0;

 return Write( BuffData.Ptr(), (DWORD)num+1 );
}

BOOL HOStream::WriteV( CONSTSTR Format,va_list al )
  {  int rc = VSNprintf( NULL, 0, Format, al ),
         num;

    rc++;
    if ( !BuffData.Resize( (DWORD)rc ) )
      return FALSE;

    num = VSNprintf( (char*)BuffData.Ptr(), rc, Format, al );
    if ( num >= rc )
      ((char*)BuffData.Ptr())[ rc-1 ] = 0;

 return Write( BuffData.Ptr(), (DWORD)rc );
}

/****************************************************
   HIStream
 ****************************************************/
HIStream::HIStream( DWORD bsz )
  {
    rBuff  = new BYTE[ bsz+1 ];
    rTail  = NULL;
    rSize  = bsz;
    rCount = 0;
}

HIStream::~HIStream()
  {
    delete[] rBuff;
}

void  HIStream::doReset( void )     { rCount = 0; }
DWORD HIStream::ReadySize( void )   { return rCount; }
void  HIStream::Reset( void )       { doReset(); }
BOOL  HIStream::Assigned( void )    { return doAssigned(); }
BOOL  HIStream::doAssigned( void )  { return TRUE; }

DWORD HIStream::Read( LPVOID _buff,DWORD sz )
  {  LPBYTE buff = (LPBYTE)_buff;
     DWORD  rSz,n;

     Log(( "iRead %p[%d]",buff,sz ));

     for( n = 0; ((int)sz) > 0; n += rSz,sz -= rSz ) {
       if ( !rCount ) {
         if ( !ReadCompleted() )
           break;
         rTail = rBuff;
       }

       rSz = Min( rCount,sz );

       memcpy( buff,rTail,rSz );
       buff   += rSz;

       rTail  += rSz;
       rCount -= rSz;
     }

     Log(( "rc: %d",n ));

 return n;
}
