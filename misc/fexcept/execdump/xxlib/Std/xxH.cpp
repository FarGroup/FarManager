#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__HOOKS_USED__)

#pragma option -vGc- -vGt- -vGd-
#pragma inline

#include <Global/pack1.h>
 //jmp|call [dword ptr <memory>] op code
 struct JMPOpCode {
   unsigned char  Call[2];
   void          *Addr;
 };
#include <Global/pop.h>

//---------------------------------------------------------------------------
PProcHookInfo _CurrentProcHookInfo;
//---------------------------------------------------------------------------
static PProcHookInfo Array[ 20 ];
static int        SubstCount = 0;
static DWORD      SavedEAX;  //Used to temprorary save EAX
static DWORD      CallProc;  //Used to make call[CallProc] if all registers used
//---------------------------------------------------------------------------
/*  PushProcHookInfo
    PopProcHookInfo

    Save and restore used `ProcHookInfo` without stack using.

    WARN:
      - function MUST be `__fastcall`
      - functions may change ecx,edx

    TODO:
      - rewrite it on ASM
*/
static PProcHookInfo __fastcall PushProcHookInfo( PProcHookInfo p )
  {
    Assert( SubstCount < sizeof(Array)/sizeof(Array[0]) );
    Array[SubstCount++] = p;
 return p;
}
static PProcHookInfo __fastcall PopProcHookInfo( PProcHookInfo p )
  {
    Assert( SubstCount );
 return Array[--SubstCount];
}
//---------------------------------------------------------------------------
static __declspec(naked) void __cdecl doBPUnwindJMP( PProcHookInfo si )
  {
     asm {
       mov   [SavedEAX],eax

       pop   eax                         // Return
       pop   eax                         // Param `si`

       push  edx                         //Restore patched bytes
       push  ecx

       mov   edx,[eax.OldProc]
       mov   [CallProc],edx
       mov   ecx,[eax.SubstCode+0]
       mov   [edx+0],ecx
       mov   ecx,[eax.SubstCode+4]
       mov   [edx+4],ecx

       pop   ecx
       pop   edx

       pop   eax                         //Caller EBP frame

       mov   eax,[SavedEAX]
       jmp   dword ptr [CallProc]
     }
}
static __declspec(naked) void __cdecl doBPUnwind( void )
  {
     asm {
       mov   [SavedEAX],eax              //Save IN eax

       push  ecx                         //Save ecx,edx
       push  edx
//Correct info
       mov   eax,_CurrentProcHookInfo    //Get current INFO
       call  PushProcHookInfo
//Restore patched
       mov   ecx,[esp + 2*4 + 0]         //Get caller EIP (shifted by two PUSH)
       mov   [eax.SavedCALLER],ecx

       mov   edx,[eax.OldProc]           //Restore patched bytes
       mov   ecx,[eax.SubstCode+0]
       mov   [edx+0],ecx
       mov   ecx,[eax.SubstCode+4]
       mov   [edx+4],ecx

       pop   edx                         //Restore saved ecx,edx
       pop   ecx

       add   esp,4                       //Remove caller EIP
//Call old
       mov   eax,[eax.OldProc]           //Call old caller
       mov   [CallProc],eax
       mov   eax,[SavedEAX]
       call  [CallProc]
       mov   [SavedEAX],eax              //Save OUT eax
//Patch code back
       push  ecx                         //Save ecx,edx
       push  edx

       call  PopProcHookInfo             //Pop last INFO

       mov   edx,[eax.OldProc]           //Restore "call []"
       mov   word ptr [edx], 025FFh
       lea   ecx,[eax.CallSpace]
       mov   dword ptr [edx+2], ecx

       pop   edx                         //Restore ecx,edx
       pop   ecx

       push  [eax.SavedCALLER]           //Restore OUT eax
       mov   eax,[SavedEAX]
       ret
     }
}
//---------------------------------------------------------------------------
static BOOL __fastcall Subst_Function( LPBYTE RTLProc,void *UserProc,PProcHookInfo si )
  {
     si->OldProc      = RTLProc;
     si->SubstCode[0] = ((LPDWORD)RTLProc)[0];
     si->SubstCode[1] = ((LPDWORD)RTLProc)[1];
     si->Unwind       = (HandlerUnwindProc)doBPUnwind;
     si->UnwindJMP    = (HandleUnwindJMPProc)doBPUnwindJMP;
     si->CallSpace    = (DWORD)UserProc;

//Patch first 6 byte to call new handler
    ((JMPOpCode*)RTLProc)->Call[0] = 0xFF;
    ((JMPOpCode*)RTLProc)->Call[1] = 0x25;
    ((JMPOpCode*)RTLProc)->Addr    = &si->CallSpace;

 return TRUE;
}
//---------------------------------------------------------------------------
BOOL MYRTLEXP MakeAddrWritable( void *p )
  {  MEMORY_BASIC_INFORMATION mi;
     SYSTEM_INFO              si;
     LPVOID                   base;
     DWORD                    dw;

     GetSystemInfo( &si );

     base = (LPVOID)(((DWORD)p) - ((DWORD)p) % si.dwPageSize);
     if ( VirtualQuery( base,&mi,sizeof(mi)) != sizeof(mi) ) return FALSE;

     if ( !VirtualProtect( base,si.dwPageSize,PAGE_EXECUTE_READWRITE,&dw ) ) return FALSE;

 return TRUE;
}

BOOL MYRTLEXP InitHook_Proc( PProcHookInfo si,void *RTLProc,void *UserProc )
  {  static BYTE jmpCode[ 2 ]  = { 0xFF, 0x25 }; // jmp  [dword ptr]
     static BYTE callCode[ 2 ] = { 0xFF, 0x15 }; // call [dword ptr]

     BOOL   isRestore = RTLProc == NULL;
     void  *Prev;

     if ( !si ||
          (RTLProc!=NULL) != (UserProc!=NULL) )
       return FALSE;

     if (!RTLProc)  RTLProc = si->OldProc;
     if (!UserProc) UserProc = (void*)si->CallSpace;

//Find addr
     do{
       //Check valid`n`writable
       if ( !RTLProc ||
            !CheckReadable(RTLProc,sizeof(DWORD)*2 ) ||
            !MakeAddrWritable(RTLProc) )
         return FALSE;

       Prev = RTLProc;
       //JMP [] | CALL []
       if ( memcmp( RTLProc, jmpCode,  sizeof(jmpCode)/sizeof(jmpCode[0]) ) == 0 ||
            memcmp( RTLProc, callCode, sizeof(callCode)/sizeof(callCode[0]) ) == 0 ) {
         RTLProc = (void*)**((LPDWORD*)(((LPBYTE)RTLProc)+2));

         //Allready ptched
         if ( RTLProc == UserProc )
           break;

       } else
         break;
     }while( 1 );

//Restore old
     if ( RTLProc == UserProc ) {
       if ( isRestore ) {
         ((LPDWORD)Prev)[0] = si->SubstCode[0];
         ((LPDWORD)Prev)[1] = si->SubstCode[1];
       }
       return TRUE;
     }

//Code patch
  return Subst_Function( (LPBYTE)RTLProc,UserProc,si );
}
#endif
