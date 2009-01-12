#ifndef __MY_STDREPLACE
#define __MY_STDREPLACE

//MemXXX replacement
HDECLSPEC LPVOID  MYRTLEXP    MemSet( LPVOID Buff, BYTE Val, size_t sz );
HDECLSPEC LPVOID  MYRTLEXP    MemMove( LPVOID dest, const void *src, size_t n);
HDECLSPEC LPVOID  MYRTLEXP    MemCpy( LPVOID dest, const void *src, size_t n);
//sprintf
HDECLSPEC int     MYRTLEXP_PT Sprintf( char *Buff,CONSTSTR Fmt,... );
HDECLSPEC int     MYRTLEXP_PT SNprintf( char *Buff,size_t sz,CONSTSTR Fmt,... );
HDECLSPEC int     MYRTLEXP    VSprintf( char *Buff,CONSTSTR Fmt,va_list arglist );
HDECLSPEC int     MYRTLEXP    VSNprintf( char *Buff,size_t sz,CONSTSTR Fmt,va_list arglist );

#if defined(__cplusplus)
#if defined(__WINUNICODE__)
HDECLSPEC int     MYRTLEXP_PT Sprintf( wpchar Buff,WCONSTSTR Fmt,... );
HDECLSPEC int     MYRTLEXP_PT SNprintf( wpchar Buff,size_t sz,WCONSTSTR Fmt,... );
HDECLSPEC int     MYRTLEXP    VSprintf( wpchar Buff,WCONSTSTR Fmt,va_list arglist );
HDECLSPEC int     MYRTLEXP    VSNprintf( wpchar Buff,size_t sz,WCONSTSTR Fmt,va_list arglist );
#endif
#endif

#endif
