#ifndef __MY_EXCEPTION_XX_REPLACEMENT
#define __MY_EXCEPTION_XX_REPLACEMENT

#include <Global/pack1.h>

#if defined(__BCWIN32__) && __BORLANDC__ >= __BC402__
  #define __HOOKS_USED__ 1

/*******************************************************************
    STRUCTS | TYPES
 *******************************************************************/
PRESTRUCT( ProcHookInfo );

typedef void __cdecl (*HandlerUnwindProc)( void );
typedef void __cdecl (*HandleUnwindJMPProc)( PProcHookInfo si );

STRUCT( ProcHookInfo )
//Internal used fields (!!THE OldProc MUST BE FIRST FIELD!!)
  void               *OldProc;          //ptr
  unsigned            CallSpace;        //4 byte
  unsigned            SubstCode[ 2 ];   //8 bytes
  unsigned            SavedCALLER;      //saved register
//user usable fields
  HandlerUnwindProc   Unwind;      //Procedure to call original code
  HandleUnwindJMPProc UnwindJMP;   //Procedure to JMP to original code in context of caller
                                   //!!Unpatch patched code back to original
};

/* Definition and declaration of subst procedure
   USAGE:
     1. DECL_HOOKPROC( <return type and declspec options>,
                        <subs`ed RTL procedure>,
                        <original procedure parameters> )
          Declarec procedure and ProcHookInfo structure for Subst
          `name` procedure.

     2. CALL_HOOKED( <subs`ed RTL procedure> ) ( <parameters> ) ;
          Place call opearations for correct call original
          procedure.
          Return value as in 0riginal procedure.

     3. DECL_HOOKPROC( <user procedure name> )
          Substitute procedure.
          Use prev declared ProcHookInfo and user procedure.

   SAMPLE:
     Subst `LocalAlloc` API procedure.

     DECL_HOOKPROC( HLOCAL, LocalAlloc, (UINT uFlags,UINT uBytes) )
       {  HLOCAL  res;
          printf( "MYLocalAlloc called with: %d,%d ",uFlags,uBytes );
            res = CALL_HOOKED( LocalAlloc ) ( uFlags,uBytes );
          printf( "= %p\n",res );
       return res;
     }

     ...
     //Do a subst
       DEF_HOOKPROC( LocalAlloc );
     //Call procedure (should enter into created subst)
       LocalAlloc( 0,10 );
     ...
*/
#define DEF_HOOKPROC( nm )                 InitHook_Proc( &nm##Info,nm,nm##Handler )

#define PRE_HOOKPROC( ret,decl,nm,paras )   typedef ret decl (* Original##nm##Proc ) paras ;  \
                                            extern ProcHookInfo nm##Info;                      \
                                            extern ret decl nm##Handler paras

#define DECL_HOOKPROC( ret,decl,nm,paras ) typedef ret decl (* Original##nm##Proc ) paras ;    \
                                           ProcHookInfo nm##Info;                              \
                                           ret decl nm##Handler paras

#define CALL_HOOKED( nm )                  ( (Original##nm##Proc)                              \
                                            ( ( (_CurrentProcHookInfo=(&(nm##Info))) != NULL ) \
                                              ? ( nm##Info.Unwind )                            \
                                              : NULL ) )

#define JMP_HOOKED( nm )                   nm##Info.UnwindJMP( &nm##Info );

#define HOOKINFOTYPE( nm )                 Original##nm##Proc
#define HOOKPROC( nm )                     nm##Handler

/*******************************************************************
    LOW LEVEL [xxHw.cpp]
 *******************************************************************/
/*
    InitHook_Proc

    Attach user procedure to specified another procedure.

    si               - stricture to save used data and state of subst
    RTLProc          - procedure inject code into
    UserCallbackProc - user callback, called tyhen ejected procedure call
*/
HDECLSPEC BOOL MYRTLEXP InitHook_Proc( PProcHookInfo si,void *RTLProc,void *UserCallbackProc );
/*
   MakeAddrWritable

   Try to make an address writable.
   Change access type of whole address page.
*/
HDECLSPEC BOOL MYRTLEXP MakeAddrWritable( void *p );

#define SaveALLRegs       __emit__( 0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57 )
#define RestoreALLRegs    __emit__( 0x5F,0x5E,0x5D,0x5C,0x5B,0x5A,0x59,0x58 )

//DATA
HDECLSPEC PProcHookInfo _CurrentProcHookInfo;

#endif

#include <Global/pop.h>

#endif
