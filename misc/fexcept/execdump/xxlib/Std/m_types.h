#ifndef __MY_TYPES_DEFINITIONS
#define __MY_TYPES_DEFINITIONS

//printf-like procedure
typedef int      (MYRTLEXP_PT  *HPrintProc_t)( const char *PrintfFormat,... );
typedef int      (RTL_CALLBACK *HVPrintProc_t)( const char *PrintfFormat,va_list arglist );
typedef int      (MYRTLEXP_PT  *HFPrintProc_t)( FILE *Dest,const char *PrintfFormat,... );
typedef int      (RTL_CALLBACK *HVFPrintProc_t)( FILE *Dest,const char *PrintfFormat,va_list arglist );

//Procedure to operate buffer of data
typedef BOOL     (RTL_CALLBACK *HBufferProc_t)( char *Buff,int sz,LPVOID Param );

//Procedure used in std sort operations
typedef int      (RTL_CALLBACK *HAbstractSortProc_t)( const void *left, const void *right );

//Message procedure
typedef BOOL     (RTL_CALLBACK *HMessageProc_t)( CONSTSTR msg );

//Get string procedure
typedef CONSTSTR (RTL_CALLBACK *HGetStringProc_t)( void );


#endif
