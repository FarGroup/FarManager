#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#include <limits.h>

//[sprintf_.cpp]
HDECLSPEC int MYRTLEXP _VSNprintf( char *buf, size_t cnt, const char *fmt, va_list args );
#if defined(__UNICODE__)
HDECLSPEC int MYRTLEXP _VSNwprintf( wchar_t *buf, size_t cnt, const wchar_t *fmt, va_list args );
#endif

/* NEED_USE_MYSPRINTF
     defined if platform does not support vsnprintf or does not support vsnprimtf( NULL,0,... )
*/
#if defined(__QNX__)
  #define NEED_USE_MYSPRINTF 1
#endif

#if defined(__BORLAND)
  #if __BORLANDC__ <= __BCB10__
    #define NEED_USE_MYSPRINTF 1
  #endif
#endif

/* RTL_SNPRINTF  - RTL vsnprintf
   RTL_SNNPRINTF - RTL wide-char vsnprintf
*/
#if defined(__MSOFT) || defined(__INTEL) || defined(__DMC)
  #define RTL_SNPRINTF  _vsnprintf
  #define RTL_SNWPRINTF _vsnwprintf
#else
#if defined(__QNX__)
  #define RTL_SNPRINTF  _vbprintf
#else
  #define RTL_SNPRINTF  vsnprintf
  #define RTL_SNWPRINTF vsnwprintf
#endif //QNX
#endif //MS, Intel, DMC

/* SAFE_SNPRINTF, SAFE_SNWPRINTF - used with NULL buffer
   NORM_SNPRINTF, NORM_SNWPRINTF - normal platform vsnprintf
*/
#if defined(NEED_USE_MYSPRINTF)
  #define SAFE_SNPRINTF   _VSNprintf
  #define NORM_SNPRINTF   RTL_SNPRINTF
#else
  #define SAFE_SNPRINTF   RTL_SNPRINTF
  #define NORM_SNPRINTF   RTL_SNPRINTF
#endif

#if defined(__UNICODE__)
  #if defined(NEED_USE_MYSPRINTF)
    #define SAFE_SNWPRINTF  __VSNwprintf
    #define NORM_SNWPRINTF  RTL_SNWPRINTF
  #else
    #define SAFE_SNWPRINTF  RTL_SNWPRINTF
    #define NORM_SNWPRINTF  RTL_SNWPRINTF
  #endif
#endif

//---------------------------------------------------------------------------
int MYRTLEXP VSNprintf( char *Buff,size_t sz,CONSTSTR Fmt,va_list arglist )
  {  size_t rc;

     if ( !Buff || !sz )
       rc = SAFE_SNPRINTF( NULL,0,Fmt,arglist );
      else {
       if ( sz == 1 ) {
         *Buff = 0;
         return 0;
       }
       if ( sz < INT_MAX-10 ) sz--;
       rc = NORM_SNPRINTF( Buff,sz,Fmt,arglist );
       if ( sz < INT_MAX-10 && rc >= sz ) Buff[sz] = 0;
      }

 return (int)rc;
}

int MYRTLEXP VSprintf( char *Buff,CONSTSTR Fmt,va_list arglist )
  {

    if ( !Buff )
      return VSNprintf( NULL,0,Fmt,arglist );

//      return VSprintf( Buff,Fmt,arglist );
      return VSNprintf( Buff,INT_MAX,Fmt,arglist );
}

int MYRTLEXP_PT SNprintf( char *Buff,size_t sz,CONSTSTR Fmt,... )
  {  va_list a;
     int     res;
     va_start( a, Fmt );
     res = VSNprintf( Buff,sz,Fmt,a );
     va_end( a );
 return res;
}

int MYRTLEXP_PT Sprintf( char *Buff,CONSTSTR Fmt,... )
  {  va_list a;
     int     res;
     va_start( a, Fmt );
     res = VSprintf( Buff,Fmt,a );
     va_end( a );
 return res;
}

//---------------------------------------------------------------------------
#if defined(__UNICODE__)
int MYRTLEXP VSNprintf( wchar_t *Buff,size_t sz,const wchar_t *Fmt,va_list arglist )
  {  int rc;

     if ( !Buff || !sz )
       rc = SAFE_SNWPRINTF( NULL,0,Fmt,arglist );
      else {
       if ( sz == 1 ) {
         *Buff = 0;
         return 0;
       }
       sz--;
       rc = NORM_SNWPRINTF( Buff,sz,Fmt,arglist );
       if ( rc >= sz ) Buff[sz] = 0;
      }

 return rc;
}

int MYRTLEXP VSprintf( wchar_t *Buff,const wchar_t *Fmt,va_list arglist )
  {
 return VSNprintf( Buff,INT_MAX,Fmt,arglist );
}

int MYRTLEXP_PT SNprintf( wchar_t *Buff,size_t sz,const wchar_t *Fmt,... )
  {  va_list a;
     int     res;
     va_start( a, Fmt );
     res = VSNprintf( Buff,sz,Fmt,a );
     va_end( a );
 return res;
}

int MYRTLEXP_PT Sprintf( wchar_t *Buff,const wchar_t *Fmt,... )
  {  va_list a;
     int     res;
     va_start( a, Fmt );
     res = VSprintf( Buff,Fmt,a );
     va_end( a );
 return res;
}
#endif //UNICODE
