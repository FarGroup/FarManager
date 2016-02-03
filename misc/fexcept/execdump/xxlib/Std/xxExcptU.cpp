#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if !defined(STATUS_ILLEGAL_VLM_REFERENCE)
  #define STATUS_ILLEGAL_VLM_REFERENCE     ((DWORD   )0xC00002C0L)
#endif

#if !defined(STATUS_REG_NAT_CONSUMPTION)
  #define STATUS_REG_NAT_CONSUMPTION       ((DWORD   )0xC00002C9L)
#endif

CONSTSTR MYRTLEXP ExceptType2Str( DWORD fl )
  {  CONSTSTR m = "<unknown>";
     static char message[ 100 ];

#undef DFL
#define DFL( nm )  if ( fl == STATUS_##nm ) m = #nm; else

     DFL( WAIT_0 )
     DFL( ABANDONED_WAIT_0 )
     DFL( USER_APC )
     DFL( TIMEOUT )
     DFL( PENDING )
     DFL( SEGMENT_NOTIFICATION )
     DFL( GUARD_PAGE_VIOLATION )
     DFL( DATATYPE_MISALIGNMENT )
     DFL( BREAKPOINT )
     DFL( SINGLE_STEP )
     DFL( ACCESS_VIOLATION )
     DFL( IN_PAGE_ERROR )
     DFL( INVALID_HANDLE )
     DFL( NO_MEMORY )
     DFL( ILLEGAL_INSTRUCTION )
     DFL( NONCONTINUABLE_EXCEPTION )
     DFL( INVALID_DISPOSITION )
     DFL( ARRAY_BOUNDS_EXCEEDED )
     DFL( FLOAT_DENORMAL_OPERAND )
     DFL( FLOAT_DIVIDE_BY_ZERO )
     DFL( FLOAT_INEXACT_RESULT )
     DFL( FLOAT_INVALID_OPERATION )
     DFL( FLOAT_OVERFLOW )
     DFL( FLOAT_STACK_CHECK )
     DFL( FLOAT_UNDERFLOW )
     DFL( INTEGER_DIVIDE_BY_ZERO )
     DFL( INTEGER_OVERFLOW )
     DFL( PRIVILEGED_INSTRUCTION )
     DFL( STACK_OVERFLOW )
     DFL( CONTROL_C_EXIT )
     DFL( FLOAT_MULTIPLE_FAULTS )
     DFL( FLOAT_MULTIPLE_TRAPS )
     DFL( ILLEGAL_VLM_REFERENCE )

#undef DFL
#define DFL( nm )  if ( fl == nm ) m = #nm; else

     DFL( DBG_CONTINUE )
     DFL( DBG_TERMINATE_THREAD )
     DFL( DBG_TERMINATE_PROCESS )
     DFL( DBG_CONTROL_C )
     DFL( DBG_CONTROL_BREAK )
     DFL( DBG_EXCEPTION_NOT_HANDLED )

#undef DFL
#define DFL( nm )  if ( fl == EXCEPTION_##nm ) m = #nm; else

     DFL( DELPHI_EXCEPTION )
     DFL( DELPHI_RERISE )
     DFL( DELPHI_EXCEPT )
     DFL( DELPHI_FINALLY )
     DFL( DELPHI_TERMINATE )
     DFL( DELPHI_UNHANDLED )
     DFL( NONDELPHI_EXCEPTION )
     DFL( DELPHI_EXITFINALLY )
     DFL( CPP_EXCEPTION )
     ;
     SNprintf( message,sizeof(message),"%08X \"%s\"",fl,m );
  return message;
}

void MYRTLEXP MakeExceptionInfo( HPrintProc_t print,PEXCEPTION_RECORD rc /*=NULL*/,PCONTEXT ctx /*=NULL*/ )
  {
      for( ; rc; rc = rc->ExceptionRecord ) {
        print( "Exception %s at " REG_FMT " has " REG_FMT " flags.\n"
               "",
               ExceptType2Str(rc->ExceptionCode), rc->ExceptionAddress,
               ctx ? ctx->ContextFlags : 0 );

        if ( rc->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && rc->ExceptionInformation[1] != (DWORD)-1 ) {
          CONSTSTR m;
          switch( rc->ExceptionInformation[0] ) {
            case  8: m = "EXECUTE"; break;
            case  1: m = "WRITE";   break;
            case  0:
            default: m = "READ";
          }
          print( "Attempt to %s at address " REG_FMT "\n", m, rc->ExceptionInformation[1] );
        }
      }

      if ( ctx )
        MakeContextInfo(print,ctx);
}

void MYRTLEXP MakeContextInfo( HPrintProc_t print,PCONTEXT ctx /*=NULL*/ )
  {
     if ( IS_FLAG(ctx->ContextFlags,CONTEXT_CONTROL) ) {
#ifndef _WIN64
       print( "  EBP: %08X EIP: %08X ESP: %08X",
               ctx->Ebp,   ctx->Eip,   ctx->Esp);
#else
       print( "  RBP: %16I64X RIP: %16I64X RSP: %16I64X\n",
               ctx->Rbp,   ctx->Rip,   ctx->Rsp);
#endif
       print( "  CS: %08X  SS: %08X Flags: %08X\n",
               ctx->SegCs, ctx->SegSs, ctx->EFlags );
     }

     if ( IS_FLAG(ctx->ContextFlags,CONTEXT_INTEGER) ) {
#ifndef _WIN64
       print( "  EDI: %08X ESI: %08X EBX: %08X EDX: %08X ECX: %08X EAX: %08X\n",
               ctx->Edi, ctx->Esi, ctx->Ebx,
               ctx->Edx, ctx->Ecx, ctx->Eax );
#else
       print( "  RDI: %16I64X RSI: %16I64X RBX: %16I64X\n",
               ctx->Rdi, ctx->Rsi, ctx->Rbx);
       print( "  RDX: %16I64X RCX: %16I64X RAX: %16I64X\n",
               ctx->Rdx, ctx->Rcx, ctx->Rax );
       print( "   R8: %16I64X  R9: %16I64X R10: %16I64X\n",
               ctx->R8, ctx->R9, ctx->R10 );
       print( "  R11: %16I64X R12: %16I64X R13: %16I64X\n",
               ctx->R11, ctx->R12, ctx->R13 );
       print( "  R14: %16I64X R15: %16I64X\n",
               ctx->R14, ctx->R15 );
#endif
     }

     if ( IS_FLAG(ctx->ContextFlags,CONTEXT_SEGMENTS) )
       print( "   GS: %08X  FS: %08X  ES: %08X  DS: %08X\n",
               ctx->SegGs,ctx->SegFs,ctx->SegEs,ctx->SegDs );

     if ( IS_FLAG(ctx->ContextFlags,CONTEXT_FLOATING_POINT) ) {
#ifdef _WIN64
#define FloatSave FltSave
#endif
       print( "   Cw: %08X  Sw: %08X  Tw: %08X  EOf: %08X Es: %08X DOf: %08X Ds: %08X ",
              ctx->FloatSave.ControlWord, ctx->FloatSave.StatusWord, ctx->FloatSave.TagWord,
              ctx->FloatSave.ErrorOffset, ctx->FloatSave.ErrorSelector, ctx->FloatSave.DataOffset,
              ctx->FloatSave.DataSelector);
#ifndef _WIN64
#if _MSC_VER < 1900
       print( "Cr0: %08X\n", ctx->FloatSave.Cr0NpxState );
#else
       print( "Cr0: %08X\n", ctx->FloatSave.Spare0 );
#endif
#else
#undef FloatSave
       print( "MxCsr: %08X MxCsr_Mask: %08X\n", ctx->FltSave.MxCsr, ctx->FltSave.MxCsr_Mask );
#endif
     }

     if ( IS_FLAG(ctx->ContextFlags,CONTEXT_DEBUG_REGISTERS) ) {
#ifndef _WIN64
       print( "  dr0: %08X dr1: %08X dr2: %08X dr3: %08X dr6: %08X dr7: %08X\n",
               ctx->Dr0,ctx->Dr1,ctx->Dr2,
               ctx->Dr3,ctx->Dr6,ctx->Dr7 );
#else
       print( "  dr0: %16I64X dr1: %16I64X dr2: %16I64X\n",
               ctx->Dr0,ctx->Dr1,ctx->Dr2 );
       print( "  dr3: %16I64X dr6: %16I64X dr7: %16I64X\n",
               ctx->Dr3,ctx->Dr6,ctx->Dr7 );
#endif
     }
}

BOOL MYRTLEXP IsNonContinuable( LPEXCEPTION_RECORD ex )
  {
 return ex->ExceptionFlags == EXCEPTION_NONCONTINUABLE ||
        ex->ExceptionCode == EXCEPTION_NONCONTINUABLE_EXCEPTION;
}
