#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__HWIN__)

#if defined(__BCWIN32__)
#pragma option -vGc- -vGt- -vGd-
#endif

#if 1
  #define PROC( v )
  #define Log( v )
#else
  #define PROC( v )  INProc __proc v ;
  #define Log( v )   INProc::Say v
#endif

/*
    StackItemInfo

    All information about current stack point retrived by IntelStackWalk
    on each step.
*/
STRUCT( StackItemInfo )
  SIZE_T        CodeAddr;
  SIZE_T        FrameAddr;
  CONSTSTR      ModuleName;
  HMODULE       ModuleHandle;
  LPVOID        ModuleBase;
  DWORD         Offset;
  DWORD         FileOffset;
  char          SectionName[ IMAGE_SIZEOF_SHORT_NAME ];
  StackDataItem StackData[ STK_LEN_STACK ];             //Stack contents before EIP:EBP called
};

//User callback function called for every stack entry
typedef BOOL (RTL_CALLBACK *StackWalkProc)( PStackItemInfo si,PCallStack StackInfo );

//---------------------------------------------------------------------------
// Used by IntelStackWalk to fill StackItemInfo info for each stack entry.
// Fills:
//   ModuleName, Offset, FileOffset, SectionName
BOOL GetLogicalAddressSection( LPVOID addr, char *szModule, DWORD len, PStackItemInfo si )
  {  PHModuleInformation p;

//In case of bad parameters the result data must be initialized correctly
     *szModule = 0;
     if ( si )
       MemSet( si,0,sizeof(*si) );

//Memory accessible
     p = GetModuleInformationAddr( addr,0 );
     if ( !p )
       return FALSE;

     if ( si ) {
       si->ModuleHandle = p->Module;
       si->ModuleBase   = p->LoadBase;
     }

//Query module name
     StrCpy( szModule,p->FileName,len );
     if ( !szModule[0] )
       return FALSE;

//Has no `si` - detailed info does not requested
     if ( !si )
       return TRUE;

//Have not cections
     if ( !p->Header || !p->Sections )
       return TRUE;

//Find section and file position of given code
     // Point to the DOS header in memory
     DWORD rva = (DWORD)addr - (DWORD)si->ModuleBase;

     // Iterate through the section table, looking for the one that encompasses
     // the linear address.
     for ( WORD i = 0; i < p->Header->FileHeader.NumberOfSections; i++ ) {
         DWORD sectionStart = p->Sections[i].VirtualAddress;
         DWORD sectionEnd   = sectionStart +
                              Max( (DWORD)p->Sections[i].SizeOfRawData, (DWORD)p->Sections[i].Misc.VirtualSize );

         // Is the address in this section???
         if ( (rva >= sectionStart) && (rva <= sectionEnd) ) {
             // Yes, address is in the section.
             si->Offset      = rva - sectionStart;
             si->FileOffset  = p->Sections[i].PointerToRawData + rva - sectionStart;
             strcpy( si->SectionName, (CONSTSTR)p->Sections[i].Name );
             return TRUE;
         }
     }

 return FALSE;
}

void IntelStackWalk( SIZE_T ip,SIZE_T bp,StackWalkProc cb,PCallStack StackInfo )
  {  PROC(("IntelStackWalk",REG_FMT ":" REG_FMT,ip,bp))
     PSIZE_T       pFrame,
                   pPrevFrame;
     SIZE_T        pc,pcPrev;
     StackItemInfo si;
     char          ModuleName[MAX_PATH_SIZE];
     BOOL          cycleDetected = FALSE;
     int           n;

     pcPrev = MAX_RVA;
     pc     = ip;
     pFrame = (PSIZE_T)bp;
     do {
       Log(( "Frame: %p",pFrame ));
       //Frame pointer must be aligned on a LPVOID boundary.  Bail if not so.
       if ( !pFrame || ((SIZE_T)pFrame & (sizeof(LPVOID)-1)) != 0 )
           break;

       // Can two DWORDs be read from the supposed frame address?
       if ( !CheckReadable(pFrame, sizeof(PVOID)*2) )
         break;

       //Query information of current EIP address
       MemSet( &si,0,sizeof(si) );
       if ( GetLogicalAddressSection( (LPVOID)pc,ModuleName,sizeof(ModuleName),&si) ) {
         //Call user callback only for different EIP`s
         if ( pc != pcPrev ) {
           si.CodeAddr   = pc;
           si.FrameAddr  = (SIZE_T)pFrame;
           si.ModuleName = ModuleName;

           if ( IS_FLAG(StackInfo->Flags,STK_STACKDATA) )
             for ( n = 0; n < STK_LEN_STACK; n++ ) {
               if ( !CheckReadable( pFrame+2+n,sizeof(LPVOID) ) )
                 break;
               si.StackData[n].Ptr = (LPBYTE)pFrame[2+n];
             }

           if (cb && !cb(&si,StackInfo) )
             break;
         }
       }

       // precede to next higher frame on stack
       pcPrev = pc;
       pc     = pFrame[1] - ((pc)?0:0x23);  //Then the first EIP is zero the EBP is shifted
                                            //to some bytes. I do not know why.
                                            //0x23 is experimental value, working with borland-made
                                            //executables.
       //Go to next frame
       pPrevFrame = pFrame;
       pFrame     = (PSIZE_T)pFrame[0];

       //Check retrived EIP.
       //  EIP may be zero only in first time because impossible to execute after
       //  NULL address once reached
       if ( !pc ) {
         Log(("!pc"));
         break;
       }
       //Check next frame varies
       if ( pFrame == pPrevFrame ) {
         Log(("pFrame == pPrevFrame"));
         break;
       }
       //Check EIP varies
       if ( pc == pcPrev ) {
         if (!cycleDetected)
           if ( IS_FLAG( StackInfo->Flags, STK_FILELOG ) )
             FILELog( "Stack cycled at addres " REG_FMT " (possible as a result of stack overflow error)",pc );
         cycleDetected = TRUE;
         //Skip all same addresses in stack overflow stack frames
         continue;
       }

     }while( 1 );
}

PHModuleInformation sm1 = NULL,
                    sm2 = NULL;

void IntelStackWalkSP( SIZE_T sp,StackWalkProc cb,PCallStack StackInfo )
  {  PROC(("IntelStackWalkSP",REG_FMT,sp))
     SIZE_T        pc;
     DWORD         i, cn, n;
     StackItemInfo si;
     char          ModuleName[MAX_PATH_SIZE];

     if ( !sp )
       return;

     for( cn = i = 0; cn < STK_STACK_DEPTH*2 && i < STK_STACK_DEPTH; cn++ ) {
       if ( !CheckReadable( (LPVOID)sp,sizeof(SIZE_T) ) )
         continue;

       pc   = *((PSIZE_T)sp);
       sp += sizeof(SIZE_T);

       //Query information of current EIP address
       if ( GetLogicalAddressSection( (LPVOID)pc, ModuleName, sizeof(ModuleName), &si ) ) {
         //sm1 = StackModules->Item(0);
         //sm2 = StackModules->Item(1);

         si.CodeAddr   = pc;
         si.FrameAddr  = pc+sizeof(SIZE_T);
         si.ModuleName = ModuleName;

         if ( IS_FLAG(StackInfo->Flags,STK_STACKDATA) ) {
           PSIZE_T pFrame = (PSIZE_T)(pc+sizeof(SIZE_T));
           for ( n = 0; n < STK_LEN_STACK; n++ ) {
             if ( !CheckReadable( pFrame+n,sizeof(LPVOID) ) )
               break;
             si.StackData[n].Ptr = (LPBYTE)pFrame[n];
           }
         }

         if (cb && !cb(&si,StackInfo) )
           break;

         i++;
       }
     }
}

//---------------------------------------------------------------------------
static void ReadCode( PCallStack StackInfo,PStackEntry p )
  {
     p->CodeSize = 0;
     p->CodePos  = 0;

     if ( !IS_FLAG(StackInfo->Flags,STK_READCODE) ||
          !p->CodeAddr )
       return;

     Log(( "ReadCode: " REG_FMT, p->CodeAddr ));

     LPBYTE addr,low,hi,ptr;

     addr = ((LPBYTE)p->CodeAddr);
     low  = addr - sizeof(p->Code) / 2;
     hi   = addr + sizeof(p->Code) / 2;

     if ( low > hi ) low = addr;
     if ( hi < low ) hi  = addr;

     //Correct low bound
     if ( !CheckReadable(low,sizeof(p->Code)/2) )
       for( ptr = low; ptr < addr; ptr++ ) if ( !CheckReadable(ptr,1) ) low = ptr+1;

     //Correct high bound
     if ( !CheckReadable(addr,sizeof(p->Code)/2) )
       for( ptr = hi;  ptr > addr; ptr-- ) if ( !CheckReadable(ptr,1) ) hi  = ptr-1;

     //Copy code
     if ( low < hi ) {
       p->CodeSize = (DWORD)(hi-low);
       p->CodePos  = (DWORD)(addr-low);
       MemMove( p->Code,low,p->CodeSize );
     }
}
static void ReadSymbol( PCallStack StackInfo,PStackEntry p )
  { PSymbol sym;

    Assert( p );

    Log(( "ReadSymbol: " REG_FMT, p->CodeAddr ));

    sym = LocateModuleSymbol( p->ModuleHandle,p->CodeAddr,StackInfo->Flags );
    if ( sym ) {
      StrCpy( p->Symbol,sym->Name,sizeof(p->Symbol) );
      p->SymbolOffset = DWORD(p->CodeAddr - sym->Addr);
    } else {
      p->Symbol[0]    = 0;
      p->SymbolOffset = 0;
    }
}
static void ReadStackData( PCallStack StackInfo,PStackEntry p )
  {  int n,i;
     LPBYTE src,dst;

     if ( !IS_FLAG(StackInfo->Flags,STK_STACKDATA) )
       return;

     for ( n = 0; n < STK_LEN_STACK; n++ ) {
       if ( p->StackData[n].Ptr ) {
         Log(( "ReadStackData[%d]: " REG_FMT, n, p->StackData[n].Ptr ));
         src = p->StackData[n].Ptr;
         dst = p->StackData[n].Data;

         for ( i = 0; i < STK_LEN_DATA; *dst++ = *src++,i++ )
           if ( !CheckReadable(src,1) )
             break;
         for ( ; i < STK_LEN_DATA; i++ )
           *dst++ = 0;
       }
     }
}

static BOOL RTL_CALLBACK idStackWalk( PStackItemInfo si,PCallStack StackInfo )
  {  StackEntry se;

     while( StackInfo->Items.Count() > STK_STACK_DEPTH )
        StackInfo->Items.DeleteNum( 0 );

     Log(( "SW[%d]: " REG_FMT ":" REG_FMT " " REG_FMT "[%s]",StackInfo->Items.Count(), si->CodeAddr,si->FrameAddr,si->ModuleHandle,si->ModuleName ));

     TStrCpy( se.Module,  si->ModuleName );
     TStrCpy( se.Section, si->SectionName );
     MemMove( &se.StackData, &si->StackData,sizeof(se.StackData) );
     se.Offset       = si->Offset;
     se.FileOffset   = si->FileOffset;
     se.CodeAddr     = si->CodeAddr;
     se.FrameAddr    = si->FrameAddr;
     se.ModuleHandle = si->ModuleHandle;
     se.ModuleBase   = si->ModuleBase;
     se.SysModule    = (BOOL)IsSystemModule(se.Module);

     ReadCode( StackInfo,&se );
     ReadSymbol( StackInfo,&se );
     ReadStackData( StackInfo,&se );

 return StackInfo->Items.Add(se) != NULL;
}
//---------------------------------------------------------------------------
StackDisasm::StackDisasm( PStackEntry p )
    : SymDisasm( 0 )
  {
    Assign( p );
}
void StackDisasm::Assign( PStackEntry p )
  {
    MainAddress = p ? p->CodeAddr : NULL;
    Stack       = p;
    Pos         = 0;
}

void  StackDisasm::returnbyte( BYTE Byte ) { if ( Pos ) Pos--; }
SIZE_T StackDisasm::Position( void )        { return Stack->CodeAddr + Pos - Stack->CodePos - instruction_length; }

BYTE StackDisasm::getbyte( BOOL *OutOfBounds )
  {
    if ( Pos >= Stack->CodeSize ) {
      *OutOfBounds = TRUE;
      return 0;
    } else
      return Stack->Code[ Pos++ ];
}


//---------------------------------------------------------------------------
static void PStackSymbol( HPrintProc_t print,CONSTSTR m,PCallStack StackInfo,int num,BOOL stackData )
  {  PStackEntry p = StackInfo->Items[num];
     char        str[500];

//Name
     { char *t = str;
       t += Sprintf( t,"Addr[%2d]: " REG_FMT " Mod:" REG_FMT " lMod:" REG_FMT " \"%s\"",
                     num+1,
                     p->CodeAddr,
                     p->ModuleHandle, p->ModuleBase, m );

       if ( p->FileOffset )
         t += Sprintf( t,"+%08X", p->FileOffset );

       if ( p->Section[0] )
         t += Sprintf( t," at \"%s\"", p->Section );

       t += Sprintf( t,"+%08X", p->Offset );

       if ( p->Symbol[0] )
         Sprintf( t,": \"%s\" + %08X", p->Symbol, p->SymbolOffset );
     }

     print( "%s\n",str );

//Stack data
     int  cn,n;
     BYTE ch;

     do{
       if ( !stackData || !IS_FLAG(StackInfo->Flags,STK_STACKDATA) )
         break;

       for ( cn = n = 0; n < STK_LEN_STACK; n++ )
         if ( p->StackData[n].Ptr )
           cn++;

       if ( !cn ) break;

       if ( !IS_FLAG(StackInfo->Flags,STK_STACKSYSTEM) && p->SysModule ) {
          char *t;

          t = str + Sprintf( str,"  Stack: " );
          for ( n = 0; n < STK_LEN_STACK; n++ )
            t += Sprintf( t, REG_FMT " ", p->StackData[n].Ptr );
          print( "%s\n",str );
       } else {
         char *t;

         print( "  Stack:\n" );
         for ( n = 0; n < STK_LEN_STACK; n++ ) {
           t = str + Sprintf( str, "  %2d) " REG_FMT " = ",n+1,p->StackData[n].Ptr );

           for ( cn = 0; cn < STK_LEN_DATA; cn++ )
             t += Sprintf( t,"%02X ",p->StackData[n].Data[cn] );

           for ( cn = 0; cn < STK_LEN_DATA; cn++ )
             if ( (ch=p->StackData[n].Data[cn]) >= 32 && ch <= 250 )
               t += Sprintf( t,"%c",ch);
              else
               t += Sprintf( t,"." );

           print( "%s\n",str );
         }
       }
     }while(0);
}

//---------------------------------------------------------------------------
BOOL MYRTLEXP UnwindCallStack( PCallStack StackInfo,SIZE_T ip,SIZE_T bp,SIZE_T sp )
  {  static int Inside = 0;

    if ( IS_FLAG( StackInfo->Flags, STK_FILELOG ) )
      FILELog( "UnwindCallStack(%d) for "
#ifndef _WIN64
               "EIP:%08X EBP:%08X ESP:%08X"
#else
               "RIP:%16I64X RBP:%16I64X RSP:%16R64X"
#endif
               , Inside,ip,bp,sp );

    if ( !StackInfo )
      return FALSE;

    //In case of call[0] the EIP and EBP are zero
    //In case of stack overrun EIP and EBP after RET are contains trash and equal
    if ( sp &&
         ( (!ip && !bp) ||
           (!CheckReadable((LPVOID)ip,sizeof(SIZE_T)) && !CheckReadable((LPVOID)bp,sizeof(SIZE_T))) ||
           (ip == bp) ) ) {
      Inside++;
        StackInfo->Items.DeleteAll();
        if ( IS_FLAG( StackInfo->Flags, STK_FILELOG ) )
          FILELog( "Unwind: Stack frames unreachable. Analyze stack contents..." );
        IntelStackWalkSP( sp, idStackWalk, StackInfo );
      Inside--;
    } else
    if ( ip || bp ) {
      Inside++;
        StackInfo->Items.DeleteAll();
        if ( IS_FLAG( StackInfo->Flags, STK_FILELOG ) )
          FILELog( "Unwind: Decode stack frames..." );
        IntelStackWalk( ip, bp, idStackWalk, StackInfo );
      Inside--;
    } else {
      if ( IS_FLAG( StackInfo->Flags, STK_FILELOG ) )
        FILELog( "END.UnwindCallStack impossible to unwind!" );
      return FALSE;
    }

    if ( IS_FLAG( StackInfo->Flags, STK_FILELOG ) )
      FILELog( "END.UnwindCallStack: %d items",StackInfo->Items.Count() );

 return TRUE;
}

void MYRTLEXP MakeStackWalkItem( HPrintProc_t print,PStackEntry p )
  {
     if ( !print || !p || !p->CodeSize )
       return;

     StackDisasm ui(p);

     MakeSymDisasm( print,ui );
}

BOOL MYRTLEXP MakeStackWalkListSTK( HPrintProc_t print,PCallStack si )
  {  int n;

     if ( !print || !si )
       return FALSE;

//Header
     for ( n = si->SkipFrames; n < si->Items.Count(); n++ ) {
       PStackEntry p = si->Items[n];
       PStackSymbol(print, FName(p->Module), si, n, FALSE);
     }

//Details
     if ( IS_FLAG(si->Flags,STK_EACHDETAILS) ) {
       if ( si->SkipFrames < ((DWORD)si->Items.Count()) )
         print( "\nStack details:\n" );

       for ( n = si->SkipFrames; n < si->Items.Count(); n++ ) {
         PStackEntry p = si->Items[n];

         PStackSymbol(print,FName(p->Module),si,n,TRUE );

         //stack item disassembler only for non-system module
         if ( IS_FLAG(si->Flags,STK_STACKSYSTEM) || !p->SysModule )
           MakeStackWalkItem( print,p );

         print( "\n" );
       }
     }

 return TRUE;
}

BOOL MYRTLEXP MakeStackWalkListIP( HPrintProc_t print,DWORD SkipFrames,DWORD ip,DWORD bp,DWORD Flags )
  {  CallStack si;

     si.Flags      = Flags;
     si.SkipFrames = SkipFrames;

     if ( UnwindCallStack(&si,ip,bp,0) )
       return MakeStackWalkListSTK( print,&si );
      else
       return FALSE;
}

BOOL MYRTLEXP MakeStackWalkListCTX( HPrintProc_t print,CONTEXT *ctx,DWORD Flags,DWORD SkipFrames  )
  {  CallStack si;

     si.Flags      = Flags;
     si.SkipFrames = SkipFrames;

     if ( UnwindCallStack(&si,
#ifndef _WIN64
                          ctx->Eip,ctx->Ebp,ctx->Esp
#else
                          ctx->Rip,ctx->Rbp,ctx->Rsp
#endif
        ) )
          return MakeStackWalkListSTK( print,&si );
     else
      return FALSE;
}

static DWORD WINAPI idExcept( HPrintProc_t print,DWORD Flags,DWORD SkipFrames,LPEXCEPTION_POINTERS ex )
  {
     MakeStackWalkListCTX( print,ex->ContextRecord,Flags,SkipFrames+1 );
 return EXCEPTION_EXECUTE_HANDLER;
}

BOOL MYRTLEXP MakeStackWalkListCUR( HPrintProc_t print,DWORD SkipFrames,DWORD Flags )
  {
     __try{
       *(int*)0 = 0;
     }__except( idExcept(print,Flags,SkipFrames,GetExceptionInformation()) ) {
     }

 return TRUE;
}

#endif
