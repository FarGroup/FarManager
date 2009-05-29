#include <all_lib.h>
#pragma hdrstop

//----------------------------------------------------------------------------
/*
#define _D_MSK  (MNG_NORETTYPE | MNG_NOBASEDT | MNG_NOSCTYP | \
                 MNG_NOTHROW   | MNG_NOECSU   | MNG_NOUNALG | \
                 MNG_NOMANAGE  | MNG_SHORT_U  | MNG_SHORT_S | \
                 MNG_DROP_IMP)

#define DEM_MSK32 (_D_MSK | MNG_NOPTRTYP)
#define DEM_MSK64 (_D_MSK | MNG_DEFPTR64)
*/
#ifndef _WIN64
#define DEM_MASK  0x00D32CC7
#else
#define DEM_MASK  0x00D32CC5
#endif

//-----------------------------------
typedef          __int32  int32;
typedef unsigned __int32  uint32;

//-----------------------------------
typedef int32 (__stdcall *demangler_t)(char *ans, unsigned ans_len,
                                       const char *str, uint32 disable_mask );


//----------------------------------------------------------------------------

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
  #define PSYMBOLS( v )
#else
  #define PROC( v )  INProc __proc v ;
  #define Log( v )   INProc::Say v
  #define PSYMBOLS( v ) PSymbols( this,v )

void MYRTLEXP PSymbols( PSymModule mod,HPrintProc_t print )
  {
     if (print)
       for ( int n = 0; n < mod->Count(); n++ ) {
         PSymbol p = mod->Item(n);
         print( REG_FMT " %s\n",p->Addr,p->Name );
       }
}
#endif

//---------------------------------------------------------------------------
SymArray::SymArray( void )
   : MyRefArray<Symbol>(500,5000)
  {
     memset( Sections,0,sizeof(Sections) );
     LoadBase  = NULL;
     ImageBase = NULL;
}

DECL_QSORT( SymArray, Symbol, SortAddrINT )
  {
 return ((signed int)Left->Addr) - ((signed int)Right->Addr);
}

DECL_BSEARCH( SymArray, Symbol, FindAddr )
  {
 return ((signed int)Left->Addr) - ((signed int)Right->Addr);
}

void SymArray::SortAddr( void )
  {
     SortAddrINT();
     for ( DWORD n = 0; n < (DWORD)Count(); n++ )
       Item(n)->Index = n;
}

PSymbol SymArray::Find( SIZE_T addr )
  {  Symbol sym;
     int    pos, num;

     sym.Addr = addr;

     num = FindAddr( sym,&pos );

 return Item( (num == -1) ? (pos-1) : num );
}

//---------------------------------------------------------------------------
static SIZE_T RTL_CALLBACK idRVA( SIZE_T LoadBase, DWORD Offset, PIMAGE_SECTION_HEADER Section )
  {
 return LoadBase + Offset;
}

//---------------------------------------------------------------------------
struct FuncAddr {
  WORD Index;
  WORD Function;
};
static int RTL_CALLBACK idFuncSort( const FuncAddr* left, const FuncAddr* right )
  {
 return ((signed)left->Function) - ((signed)right->Function);
}

//---------------------------------------------------------------------------
static char SysPath[ MAX_PATH_SIZE ];
static int  SysPathSize = 0;

static void InitSysPath( void )
  {
    if ( SysPathSize ) return;
    SysPathSize = (int)GetSystemDirectory( SysPath,sizeof(SysPath)-1 );
}

static BOOL MYRTLEXP _isstring( BYTE ch )
  {
 return ch == '\n' || ch == '\r' || ch == '\t' ||
        ch >= 0x20 && ch <= 0xFE;
}

static BOOL MYRTLEXP _isprint( BYTE ch )
  {
 return ch >= 0x20 && ch <= 0xFE;
}

static BOOL MYRTLEXP _IsBadStringPtr( LPCTSTR lpsz, UINT ucchMax )
  {
     while( ucchMax-- ) {
       if ( !CheckReadable( (LPVOID)lpsz,1 ) ) return TRUE;
       if ( !*lpsz )                           return FALSE;
       if ( !_isstring(*lpsz) )                 return TRUE;

       lpsz++;
     }

 return FALSE;
}
//---------------------------------------------------------------------------
CLASSBASE( SymModuleArray, public MyArray<PSymModule> )
  public:
    MYRTLEXP SymModuleArray( void ) : MyArray<PSymModule>(10,TRUE,50) {}

    PSymModule MYRTLEXP Find( HMODULE Module );
    PSymModule MYRTLEXP FindBase( LPVOID Module );
};

//+ DATA
static PSymModuleArray Modules = NULL;
static BOOL            ExitAdded = FALSE;
static AbortProc       aProc;

static void RTL_CALLBACK idExit( void )
  {
    delete Modules;
    Modules = NULL;
    if (aProc) aProc();
}

static void HSYMInitialize( void )
  {
    if ( !Modules ) {
      Modules = new SymModuleArray;
      aProc = AtExit( idExit );
    }
}

PSymModule MYRTLEXP SymModuleArray::Find( HMODULE h )
  {
    for ( int n = 0; n < Count(); n++ ) {
      PSymModule p = Item(n);
      if (p->Handle == h)
        return p;
    }
 return NULL;
}
PSymModule MYRTLEXP SymModuleArray::FindBase( LPVOID b )
  {
    for ( int n = 0; n < Count(); n++ ) {
      PSymModule p = Item(n);
      if ( p->LoadBase == b )
        return p;
    }
 return NULL;
}

//---------------------------------------------------------------------------
SymModule::SymModule( PHModuleInformation p,Off2RVA_t rva /*=NULL*/,DWORD Flags /*=STK_LOADEXPORTS|STK_LOADLOCALS*/ )
  {
     Handle     = p->Module;
     ImageBase  = p->Header->OptionalHeader.ImageBase;
     LoadBase   = p->LoadBase;
     if (!rva) rva = idRVA;

     Log(( "SYM: h:" REG_FMT " ib:" REG_FMT " lb:" REG_FMT,Handle,ImageBase,LoadBase ));

     //pathname & Name of executable
     PathName = p->FileName;
     char *tmp = strrchr( (char*)PathName.Text(),'\\' );
     if ( tmp )
       Name = tmp+1;
      else
       Name = PathName;

     //Start os code section
     for ( int n = 0; n < p->Header->FileHeader.NumberOfSections && n < MAX_SECTIONS; n++ ) {
       Sections[n] = p->Sections[n].VirtualAddress;
       Log(( "seg[%d]=" REG_FMT,n,Sections[n] ));
     }

     //Symbols
     if ( IS_FLAG(Flags,STK_LOADIMPORTS) ) LoadImports(rva,p);
     if ( IS_FLAG(Flags,STK_LOADEXPORTS) ) LoadExports(rva,p);
     if ( IS_FLAG(Flags,STK_LOADLOCALS) )  LoadLocals();

     SortAddr();
}

void SymModule::LoadExports( Off2RVA_t rva,PHModuleInformation mi )
  {  PIMAGE_DATA_DIRECTORY dd;
     PIMAGE_SECTION_HEADER ps;
     Symbol                sym;
     FuncAddr             *Functions = NULL,
                          *p;
     DWORD                 n,i;
     int                   OldNumber;

     for ( n = 0; n < (DWORD)mi->Header->FileHeader.NumberOfSections; n++ ) {
       ps = &mi->Sections[n];
       for( i = 0; (dd=FindDataDirectory( mi->Header,ps,i )) != NULL; i++ )
         if ( i == IMAGE_DIRECTORY_ENTRY_EXPORT )
           goto Work;
     }
     return;

#define RVA( off ) rva( (DWORD)mi->LoadBase, (DWORD)off, ps )

Work:
     OldNumber = Count();

     X_TRY
       PIMAGE_EXPORT_DIRECTORY  expTable      = (PIMAGE_EXPORT_DIRECTORY)RVA(dd->VirtualAddress);
       DWORD*                   funcNameTable = (DWORD*) RVA(expTable->AddressOfNames);
       DWORD*                   funcAddrTable = (DWORD*) RVA(expTable->AddressOfFunctions);
       WORD*                    ordTable      = (WORD*)  RVA(expTable->AddressOfNameOrdinals);
       char*                    nm;

       //Store and sort names
       if ( expTable->NumberOfNames ) {
         Functions = (FuncAddr*)_Alloc( sizeof(FuncAddr)*expTable->NumberOfNames );
         if ( Functions ) {
           memset( Functions,0,sizeof(FuncAddr)*expTable->NumberOfNames );
           for( n=0; n < expTable->NumberOfNames; n++ ) {
              Functions[n].Index = (WORD)n;
              Functions[n].Function = ordTable[n];
           }
           qsort( Functions, expTable->NumberOfNames, sizeof(FuncAddr),
                  (int (RTL_CALLBACK*)(const void *, const void *))idFuncSort );
         }
       }

       //Enumerate exports
       for( n = 0; n < expTable->NumberOfFunctions; n++ ) {
          if( !funcAddrTable[n] )
            continue;

          p = (!Functions || n >= expTable->NumberOfNames) ? NULL : (Functions+n);

          if ( !p ||
               p->Index > (WORD)expTable->NumberOfNames ||
               !funcNameTable[p->Index] ||
               (nm=(char*)RVA(funcNameTable[p->Index])) == NULL ||
               !nm[0] ) {
            SNprintf( sym.Name,sizeof(sym.Name),"ORD.%d",n+expTable->Base );
          } else {
            DemangleName( nm, sym.Name, sizeof(sym.Name) );
          }
          sym.Addr    = ((DWORD)LoadBase) + funcAddrTable[n];
          sym.Owner   = this;
          sym.Type    = SYM_SRC_EXP | SYM_TP_UNK;
          sym.Index   = 0;
          sym.NameOff = 0;
          Add( sym );
       }
     X_CATCH
     X_END
     if (Functions) _Del(Functions);

     if ( Count() != OldNumber )
       FILELog( "Loaded %6d exports for " REG_FMT " [%s]", Count()-OldNumber, Handle, Name.Text() );
#undef RVA
}

void SymModule::LoadImports( Off2RVA_t rva,PHModuleInformation mi )
  {  PIMAGE_DATA_DIRECTORY dd;
     PIMAGE_SECTION_HEADER ps;
     DWORD                 n,i;
     Symbol                sym;
     int                   OldNumber;

     for ( n = 0; n < (DWORD)mi->Header->FileHeader.NumberOfSections; n++ ) {
       ps = &mi->Sections[n];
       for( i = 0; (dd=FindDataDirectory( mi->Header,ps,i )) != NULL; i++ )
         if ( i == IMAGE_DIRECTORY_ENTRY_IMPORT )
           goto Work;
     }
     return;

#define RVA( off ) rva( (DWORD)mi->LoadBase, (DWORD)off, ps )

Work:
     OldNumber = Count();
     X_TRY
       PIMAGE_IMPORT_DESCRIPTOR it;
       PIMAGE_THUNK_DATA        td;
       HMODULE                  mod;
       CONSTSTR                 mnm;
       DWORD                    nmOff;

       for( it = (PIMAGE_IMPORT_DESCRIPTOR)RVA(dd->VirtualAddress); it->Name; it++ ) {
         mnm = (CONSTSTR)RVA( it->Name );
         if ( !mnm || !mnm[0] )
           continue;

         mod   = GetModuleHandle( mnm );
         nmOff = strLen(mnm)+2;

         for( td = (PIMAGE_THUNK_DATA)RVA(it->OriginalFirstThunk); td->u1.Ordinal; td++ ) {

           if ( IMAGE_SNAP_BY_ORDINAL(td->u1.Ordinal) ) {
             if ( (n=td->u1.Ordinal&0xFFFFUL) == 0 )
               continue;

             sym.Addr  = (DWORD) (mod ? GetProcAddress( mod,(LPCSTR)n ) : NULL);
             SNprintf( sym.Name,sizeof(sym.Name),"%s::ORD.%d",mnm,n );
           } else {
             PIMAGE_IMPORT_BY_NAME in = (PIMAGE_IMPORT_BY_NAME)RVA(td->u1.AddressOfData);

             sym.Addr = (DWORD) (mod ? GetProcAddress( mod,(LPCSTR)in->Name ) : NULL );
             SNprintf( sym.Name,sizeof(sym.Name),"%s::",mnm );
             DemangleName( (CONSTSTR)in->Name, sym.Name+nmOff, sizeof(sym.Name)-nmOff );
           }

           sym.Owner   = this;
           sym.Type    = SYM_SRC_IMP | SYM_TP_UNK;
           sym.Index   = 0;
           sym.NameOff = nmOff;
           Add( sym );
         }
       }
     X_CATCH
     X_END
#undef RVA
     if ( Count() > OldNumber )
       FILELog( "Loaded %6d imports for " REG_FMT " [%s]",Count()-OldNumber, Handle, Name.Text() );
}

enum {
  ST_NONE,
  ST_SEG,
  ST_NAME,
  ST_GCC
};

void SymModule::LoadLocals( void )
  {  char   str[ 10000 ],
            symbol[ 10000 ];
     char  *m,*tmp;
     FILE  *f;
     int    OldNumber;
     int    stage = ST_NONE;
     DWORD  dw;
     SIZE_T dw1;
     DWORD  gcc_base=0;
     BOOL   gcc_base_read=FALSE;
     Symbol sym;
     BOOL   ulink_map = FALSE;
     SIZE_T Segments[ MAX_SECTIONS ];

#if 1
     StrCpy( str, PathName.c_str(), sizeof(str) );
     ChangeFileExt( str,"map" );
#else
     StrCpy( str, "Bug\\Attach\\Cleaner.map", sizeof(str) );
#endif

     f = fopen( str,"r" );
     if (!f) {
       return;
     }

     OldNumber = Count();

     for ( dw = 0; dw < MAX_SECTIONS; dw++ )
       Segments[dw] = MAX_RVA;

     while( fgets(str,sizeof(str),f) && !feof(f) ) {
       if ( !str[0] ) continue;

       for ( m = str+strlen(str)-1; strchr("\n\r\t \b",*m) != NULL; *m-- = 0 )
         if ( m == str ) break;

       for ( m = str; *m && isspace(*m); m++ );
       if ( !m[0] ) continue;

       if ( strLen(m) <= 4 ) continue;

       if ( m[4] == ':' ||
            m[0] == '0' && m[1] == 'x' ) {
         switch( stage ) {
           case  ST_SEG: if ( sscanf( m,"%04X:" REG_FMT,&dw,&dw1 ) == 2 &&
                              dw < MAX_SECTIONS ) {

                           if ( Segments[dw] == MAX_RVA || dw1 < Segments[dw] )
                             Segments[dw] = dw1;

                           Log(( "Seg [%s]: [%d]=" REG_FMT,m,dw,Segments[dw] ));
                         }
                      break;

           case ST_NAME: if ( sscanf( m,"%04X:" REG_FMT,&dw,&dw1 ) == 2 &&
                              dw < MAX_SECTIONS && dw >= 1 ) {

                           while( *m && isspace(*m) ) m++;
                           if ( !m[0] ) break;

                           while( *m && !isspace(*m) ) m++;
                           if ( !m[0] ) break;

                           while( *m && isspace(*m) ) m++;
                           if ( !m[0] ) break;

                           StrCpy( symbol,m,sizeof(symbol) );
                           //Strip mangled names
                           if ( IsMangledName(symbol) && (m=strchr(symbol,' ')) != NULL )
                             *m = 0;

                           //Remove VC dummy symbols
                           if ( !DemangleName2( symbol, sizeof(symbol) ) &&
                                ( symbol[0] == '@' || (symbol[0] == '?' && symbol[4] == '@' ) ) )
                             break;

                           //Remove comments after VC name
                           for( m = symbol; m[1]; m++ )
                             if ( m[0] == ' ' && m[1] == ' ' ) {
                               *m = 0;
                               break;
                             }

                           Log(( "Sym[%s]",symbol ));
                           StrCpy( sym.Name,symbol,sizeof(sym.Name) );

                           if ( ulink_map )
                             sym.Addr  = ((DWORD)LoadBase) - ImageBase + dw1;
                            else
                             sym.Addr  = ((DWORD)LoadBase) + Sections[dw-1] + dw1;

                           sym.Owner = this;
                           sym.Type  = SYM_SRC_MAP | SYM_TP_UNK;
                           sym.Index = 0;

                           Log(( "Sym %08X:" REG_FMT " = " REG_FMT " \"%s\"",dw,dw1,sym.Addr,sym.Name ));
                           Add( sym );
                         }
                      break;

#pragma message("must check format")
           case  ST_GCC: if ( sscanf( m,"0x%08X",&dw ) == 1 ) {
                           int InBraces;
                           //Skip spaces
                           for( m += 10; *m && isspace(*m); m++ );

                           if (StrCmp(m,"vtable for ",11,FALSE) == 0 || (m[0]=='0' && m[1]=='x'))
                             break;

                           dw-=gcc_base;

                           //Get name
                           for( dw1 = 0, InBraces = 0; dw1 < sizeof(sym.Name)-1 && *m; dw1++,m++ ) {
                              if ( *m == '(' ) InBraces = 1;
                              else if ( *m == ')' ) InBraces = 0;
                              else if ( !InBraces && isspace(*m) ) break;

                              sym.Name[dw1] = *m;
                           }
                           sym.Name[dw1] = 0;

                           //Skip spaces
                           while( *m && isspace(*m) ) m++;

                           //Not a name
                           if ( *m && (*m == '=' || *m == '('))
                             break;

                           //Demangle
                           if ( !DemangleName2( sym.Name, sizeof(sym.Name) ) &&
                                sym.Name[0] == '@' )
                             break;

                           sym.Addr  = ((DWORD)LoadBase) - ImageBase + dw;
                           sym.Owner = this;
                           sym.Type  = SYM_SRC_MAP | SYM_TP_UNK;
                           sym.Index = 0;
                           Log(( "Sym %08X = " REG_FMT " \"%s\"",dw,sym.Addr,sym.Name ));
                           Add( sym );
                         }
                      break;
         }
       } else {

         BOOL skip = StrCmp(m,"Detailed ",9,FALSE) == 0;
         if ( !skip &&
              StrCmp(m,"Address ",8,FALSE) == 0 ) {
           tmp = m+8;
           while( *tmp && isspace(*tmp) ) tmp++;
           if ( !tmp[0] ) continue;

           skip = StrCmp(tmp,"Publics by Name",  15, FALSE) == 0;
         }

       //Skipped parts
         if ( skip ) {
           Log(( "Set stage to: %d->%d",stage,ST_NONE ));
           stage = ST_NONE;
         } else
       //Segments
         if ( StrCmp(m,"Start ",6,FALSE) == 0 ) {
           Log(( "Set stage to: %d->%d",stage,ST_SEG ));
           stage = ST_SEG;
         } else
       //ULINK names
         if ( StrCmp(m,"Address            Publics by Value",35,FALSE) == 0 ) {
           Log(( "Set stage to: %d->%d",stage,ST_NAME));
           ulink_map = TRUE;
           stage     = ST_NAME;
         } else
       //GCC names  (format: %08X \w+)
         if ( StrCmp(m,"Archive member included because",    31,FALSE) == 0 ) {
           Log(( "Set stage to: %d->%d",stage,ST_GCC));
           stage = ST_GCC;
         } else
         if ( !gcc_base_read && stage == ST_GCC && StrCmp(m,".text",           5,FALSE) == 0 )
         {
           tmp = m+5;
           while( *tmp && isspace(*tmp) ) tmp++;
           if ( !(tmp[0]=='0' && tmp[1]=='x') ) continue;
#pragma message("must check format")
           sscanf( m,"0x%08X",&gcc_base );
           gcc_base_read=TRUE;
         } else
       //Values section
         if ( StrCmp(m,"Address         Publics by Value",   32,FALSE) == 0 ||  //BCB1, BCB5, VC6, Intel
              StrCmp(m,"Address Publics by Value",           24,FALSE) == 0 ||  //BCB6
              stage != ST_GCC
              ) {
           Log(( "Set stage to: %d->%d",stage,ST_NAME));
           stage = ST_NAME;
         }
       }
     }
     fclose(f);

     if ( Count() > OldNumber )
       FILELog( "Loaded %6d symbols for " REG_FMT " [%s]",Count()-OldNumber, Handle, Name.Text() );
}
//---------------------------------------------------------------------------
SymDisasm::SymDisasm( SIZE_T MainAddr )
  {
    Sign        = DASM_SYM_LESS;
    Flags       = DISASM_HEXOUT | DISASM_DO_SIZE;
    MainAddress = MainAddr;
}

BOOL SymDisasm::Unassemble( void )
  {
      Comments           = "";
      instruction_length = 0;

      if ( Position() < MainAddress )
        Sign = DASM_SYM_LESS;
       else
      if ( Position() == MainAddress )
        Sign = DASM_SYM_CURRENT;
       else
      if ( Position() > MainAddress )
        Sign = DASM_SYM_MORE;

 return ::Unassemble(this);
}

void SymDisasm::Operand( SIZE_T& addr,char& Sign,int& opSize )
  {  PSymbol sym;
     SIZE_T  cur;
     char    str[MAX_PATH_SIZE],*m;
     BYTE    ch;

     if ( Comments.Length() || IS_FLAG(CurrentFlags,DISASM_FL_REFADD) )
       return;

     if ( Sign ) {
       if ( Sign == '+' )
         cur = (DWORD)( Position() + addr );
        else
         cur = (DWORD)( Position() - addr );
     } else
       cur = addr;

     sym    = LocateSymbol( cur,0 );
     str[0] = 0;
     m      = str;

     if ( (CurrentFlags&(DISASM_FL_CMP|DISASM_FL_REF|DISASM_FL_REFADD|DISASM_FL_MATH|DISASM_FL_BIT)) != 0 ||
          (CurrentFlags&(DISASM_FL_MOV)) != 0 && LO_WORD(addr) == 0 )
       SET_FLAG( CurrentFlags,DISASM_FL_DSMALL );

//Numbers
     if ( (CurrentFlags&(DISASM_FL_CMP|DISASM_FL_BIT|DISASM_FL_ADD|DISASM_FL_MUL)) != 0 ) {
        if (Sign) *m++ = Sign;
        m += Sprintf( m,"%d \"",addr );
        ch = (BYTE)LO_BYTE( LO_WORD(addr) ); *m++ = _isprint(ch) ? ch : '.';
        ch = (BYTE)HI_BYTE( LO_WORD(addr) ); *m++ = _isprint(ch) ? ch : '.';
        ch = (BYTE)LO_BYTE( HI_WORD(addr) ); *m++ = _isprint(ch) ? ch : '.';
        ch = (BYTE)HI_BYTE( HI_WORD(addr) ); *m++ = _isprint(ch) ? ch : '.';
        *m++ = '\"';
        *m = 0;
     } else
//Code
     if ( IS_FLAG(CurrentFlags,DISASM_FL_CODE) ) {

       m += Sprintf( m,"%c",(cur < Position()) ? DASM_SYM_UP : DASM_SYM_DOWN );

#if 1
       if ( IS_FLAG( CurrentFlags,DISASM_FL_ABS ) ) {
         Sign = 0;
         addr = cur;
         m  += Sprintf( m,"%s",HexDigit(DWORD(cur-Position()),' ',TRUE) );
       } else {
         m  += Sprintf( m, REG_FMT ,cur );
       }
#else
       Sign = 0;
       addr = cur;
       m   += Sprintf( m,"%s",HexDigit(DWORD(cur-Position()),' ',TRUE) );
#endif

       if ( sym ) {
         m += Sprintf( m," %s::%s",sym->Owner->Name.Text(),sym->Name );
         if ( sym->Addr != cur )
           Sprintf( m,"%s",HexDigit(DWORD(cur-sym->Addr),'+',TRUE) );
       }

       CLR_FLAG( CurrentFlags,DISASM_FL_DSMALL );
     } else /*CODE*/
//Data
     if ( !Sign &&
          ( (CurrentFlags&(DISASM_FL_REF|DISASM_FL_REFADD)) == DISASM_FL_REF || IS_FLAG(CurrentFlags,DISASM_FL_DATA) ) ) {
       if ( sym && sym->Addr == cur ) {
         Sign = 0;
         Sprintf( m,"%c%s %s::%s",
                  (cur < Position()) ? DASM_SYM_UP : DASM_SYM_DOWN,
                  HexDigit(DWORD(cur-Position()),' ',TRUE),
                  sym->Owner->Name.Text(),sym->Name );

         CLR_FLAG( CurrentFlags,DISASM_FL_DSMALL );
       } else
  //String
       if ( !Sign && !_IsBadStringPtr((LPCTSTR)cur,16) && *((LPCTSTR)cur) != 0 ) {
         char ss[ 17 ];
         StrCpy( ss,(LPCTSTR)cur,16 );
         SNprintf( str,sizeof(str), "\"%s\"", Str2Text(ss) );
         CLR_FLAG( CurrentFlags,DISASM_FL_DSMALL );
       } else
  //Dump
       if ( !Sign && CheckReadable( (LPVOID)cur,1 ) ) {
         m += Sprintf( m,"%c",(cur < Position()) ? DASM_SYM_UP : DASM_SYM_DOWN );

         int  n,i;
         BYTE b;

         for ( n = 0; n < 16; n++ ) {
           if ( !CheckReadable( (LPBYTE)cur+n,1 ) )
             break;
           m += Sprintf( m," %02X",*((LPBYTE)cur+n) );
         }
         for ( i = n; i < 16; i++,m += 3 )
           strcpy( m,"   " );

         *m++ = ' ';

         for ( i = 0; i < n; i++ ) {
           b = ((LPBYTE)cur)[i];
           *m++ = _isprint(b) ? b : '.' ;
         }

         for ( i = n; i < 16; i++ )
           *m++ = '.';

         *m = 0;
         CLR_FLAG( CurrentFlags,DISASM_FL_DSMALL );
       }
     }/*DATA*/

     Comments = str;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void RTL_CALLBACK unloadAtExit(void);

static demangler_t get_demangler(bool unload = false)
{
    static demangler_t demangler;
    static HMODULE     m;
    static AbortProc   aProc;

    if ( unload ) {  // call at exit
      if ( m > (HMODULE)1 ) FreeLibrary(m);
      m = NULL;
      demangler = NULL;
      if (aProc) {
        aProc();
        aProc = NULL;
      }
      return NULL;
    }

    if ( m == NULL )
    {
      char s[MAX_PATH];
      char *p;
      if (GetModuleFileName((HMODULE)&__ImageBase, s, MAX_PATH))
      {
      	p=strrchr(s,'\\');
      	if(p)
      	{
          strcpy(p+1,
#ifndef _WIN64
                     "demangle32.dll"
#else
                     "demangle64.dll"
#endif
                     );
          m = LoadLibrary(s);
      	}
      }
      if (!m)
        goto no_demangler;

      if((demangler = (demangler_t)GetProcAddress( m, LPCSTR(1) )) == NULL) {
        FreeLibrary(m);
no_demangler:
        m = (HMODULE)1;
      } else {
        aProc = AtExit(unloadAtExit);
      }
    }
    return demangler;
}

//----------------------------------
static void RTL_CALLBACK unloadAtExit(void)
{
    get_demangler(true);
}

//---------------------------------------------------------------------------
HDECLSPEC BOOL MYRTLEXP DemangleName2( char *InOut, size_t OutSize )
{
    demangler_t  demangler;
    char         cnm[4096];

    if(   !InOut
       || OutSize < 4
       || (demangler = get_demangler()) == NULL
       || demangler(NULL, 0, InOut, DEM_MASK) <= 0
       || demangler(cnm, (unsigned)sizeof(cnm), InOut, DEM_MASK) < 0) return FALSE;

    size_t len = strlen(cnm);
    if(len < OutSize) memcpy(InOut, cnm, len+1);
    else {
      memcpy(InOut, cnm, len = OutSize - 4);
      memcpy(InOut + len, "...", 4);
    }

    return TRUE;
}

//---------------------------------------------------------------------------
HDECLSPEC BOOL MYRTLEXP DemangleName( CONSTSTR MangledName, char *OutputBuffer,
                                      size_t OutputBufferSize )
{
    if(!OutputBuffer || !MangledName) return FALSE;
    if(strlen(MangledName) >= OutputBufferSize) {
      StrCpy(OutputBuffer, MangledName, (int)OutputBufferSize);
      return FALSE;
    }
    return DemangleName2(strcpy(OutputBuffer, MangledName), OutputBufferSize);
}

//---------------------------------------------------------------------------
HDECLSPEC BOOL MYRTLEXP IsMangledSymbol( CONSTSTR nm )
{
    if(nm) switch(nm[0]) {
      case 'W': case 'T':
        if(nm[1] == '?') {  // watcom
      case '?':
      case '@':
          return TRUE;
        }
      default:
        break;
    }
    return FALSE;
}

//---------------------------------------------------------------------------
HDECLSPEC BOOL MYRTLEXP IsMangledName( CONSTSTR nm )
{
    demangler_t  demangler;

    if(nm) {
      if(IsMangledSymbol(nm)) return TRUE;

      if(nm[0] == '_' && (demangler = get_demangler()) != NULL)
        return demangler(NULL, 0, nm, DEM_MASK) > 0;
    }
    return FALSE;
}

//---------------------------------------------------------------------------
PSymbol MYRTLEXP LocateModuleSymbol( HMODULE Module,SIZE_T addr,DWORD Flags )
  {  PSymModule mod;

    HSYMInitialize();
    mod = Module ? Modules->Find(Module) : NULL;

    if ( !mod && Module ) {
      PHModuleInformation mi = GetModuleInformation(Module);
      if ( !mi ) return NULL;

      mod = new SymModule(mi,NULL,Flags);
      if ( !mod->Handle && !mod->LoadBase ) {
        delete mod;
        return NULL;
      }
      mod = Modules->Add( mod );
    }
 return mod ? mod->Find(addr) : NULL;
}

PSymbol MYRTLEXP LocateModuleSymbolBase( LPVOID Base,SIZE_T addr,DWORD Flags )
  {  PSymModule mod;

    HSYMInitialize();

    mod = Base ? Modules->FindBase(Base) : NULL;

    if ( !mod && Base ) {
      PHModuleInformation mi = GetModuleInformationBase(Base);
      if ( !mi ) {
        Log(( "!Module for " REG_FMT, Base ));
        return NULL;
      }

      mod = new SymModule(mi,NULL,Flags);
      if ( !mod->Handle && !mod->LoadBase ) {
        Log(( "!SymbolModule" ));
        delete mod;
        return NULL;
      }
      mod = Modules->Add( mod );
    }
 return mod ? mod->Find(addr) : NULL;
}

PSymbol MYRTLEXP LocateSymbol( SIZE_T addr,DWORD AutoLoad )
  {  MEMORY_BASIC_INFORMATION mbi;

     if ( !addr || !VirtualQuery((LPVOID)addr,&mbi,sizeof(mbi)) )
       return NULL;
 return LocateModuleSymbolBase( (HMODULE)mbi.AllocationBase,addr,AutoLoad );
}
//---------------------------------------------------------------------------
BOOL MYRTLEXP AddModuleSymbol( HMODULE Module,DWORD Flags,PSymbol sym )
  {  PSymModule mod;
     PSymbol    p;

    if ( !Module || !sym || !sym->Addr || !sym->Name[0] )
      return FALSE;

    HSYMInitialize();
    mod = Modules->Find(Module);

    if ( !mod && Module ) {
      PHModuleInformation mi = GetModuleInformation(Module);
      if ( !mi ) return FALSE;

      mod = new SymModule(mi,NULL,Flags);
      if ( !mod->Handle && !mod->LoadBase ) {
        delete mod;
        return FALSE;
      }
      mod = Modules->Add( mod );
    }

    if ( mod ) {
      p = mod->Find( sym->Addr );
      if (p)
        MemMove( p,sym,sizeof(*p) );
       else
        mod->Add( *sym );
      mod->SortAddr();
      return TRUE;
    } else
      return FALSE;
}

PSymModule MYRTLEXP ManualAddModule( PHModuleInformation mi,Off2RVA_t rva,DWORD Flags )
  {  PSymModule mod;

     if ( !mi ) return FALSE;

     HSYMInitialize();
     mod = Modules->FindBase(mi->LoadBase);
     if ( mod )
       return mod;

     mod = new SymModule(mi,rva,Flags);
     if ( !mod->Handle && !mod->LoadBase ) {
       delete mod;
       return NULL;
     }

 return Modules->Add( mod );
}

BOOL MYRTLEXP AddModuleSymbolBase( LPVOID Base,DWORD Flags,PSymbol sym )
  {  PSymModule mod;
     PSymbol    p;

    if ( !Base || !sym || !sym->Addr || !sym->Name[0] )
      return FALSE;

    HSYMInitialize();
    mod = Modules->FindBase(Base);

    if ( !mod )
      mod = ManualAddModule( GetModuleInformationBase(Base),NULL,Flags );

    if ( mod ) {
      p = mod->Find( sym->Addr );
      if (p)
        MemMove( p,sym,sizeof(*p) );
       else
        mod->Add( *sym );
      mod->SortAddr();
      return TRUE;
    } else
      return FALSE;
}

BOOL MYRTLEXP AddSymbol( SIZE_T addr,DWORD Flags,PSymbol sym )
  {  MEMORY_BASIC_INFORMATION mbi;

     if ( !addr || !VirtualQuery((LPVOID)addr,&mbi,sizeof(mbi)) )
       return FALSE;
 return AddModuleSymbolBase( (HMODULE)mbi.AllocationBase,Flags,sym );
}

BOOL MYRTLEXP IsSystemModule( HMODULE m )
  {  char str[ MAX_PATH_SIZE ];

     str[GetModuleFileName(m,str,sizeof(str))] = 0;
 return IsSystemModule(str);
}

BOOL MYRTLEXP IsSystemModule( CONSTSTR m )
  {
     InitSysPath();
 return SysPathSize && StrCmp(SysPath,m,SysPathSize-1,FALSE) == 0;
}
//---------------------------------------------------------------------------
HDECLSPEC void MYRTLEXP MakeSymDisasm( HPrintProc_t print,SymDisasm& ui )
  {  PSymbol   sym,sym1;
     SIZE_T    addr;
     BOOL      eof;
     BYTE      b,ch;
     DWORD     n;
     char      str[ DISASM_CMDSIZE*2 + 10 ];
     char      buff[ 500 ];

    buff[0] = 0;

    for( addr = ui.Position(); ui.Unassemble(); addr += ui.instruction_length ) {

//Header
     sym = LocateSymbol( addr,0 );
     if ( sym && sym->Addr == addr )
       BufferedPrintf( buff,sizeof(buff),print,";\n;; %s::%s\n;\n",sym->Owner->Name.c_str(),sym->Name );

//Position
     if ( ui.Sign )
       BufferedPrintf( buff,sizeof(buff),print,"%c",ui.Sign );

     BufferedPrintf( buff,sizeof(buff),print,REG_FMT,addr );

//Check for "dup (X)"
     b = ui.Code();
     for ( n = 1; n < ui.instruction_length; n++ )
       if ( ui.CodeBuffer[n] != b )
         break;

     if ( n == ui.instruction_length ) {
       for( ; 1; n++ ) {
         eof = FALSE;
         ch = ui.getbyte(&eof);
         if ( eof )
           break;
         if ( ch != b ) {
           ui.returnbyte(ch);
           break;
         }
         sym1 = LocateSymbol( addr+n,0 );
         if ( sym1 && sym1 != sym && sym1->Addr == addr+n  )
           break;
       }

       if ( n > 2 || b == 0x90 ) {
         //Code
         BufferedPrintf( buff,sizeof(buff),print,"%*c%-7s %02Xh",10*3,' ', "db", b );
         BufferedPrintf( buff,sizeof(buff),print," dup (%d)\n",n );
         //Correct addr
         addr += n - ui.instruction_length;
         continue;
       } else
         for( ; n > ui.instruction_length; n-- )
           ui.returnbyte(0);
     }

//Dump
      for ( DWORD n = 0; n < 10; n++ )
        if ( n < ui.instruction_length )
          BufferedPrintf( buff,sizeof(buff),print," %02X",ui.CodeBuffer[n] );
         else
          BufferedPrintf( buff,sizeof(buff),print,"   " );

//OP and comments
      if ( ui.RightBuffer[0] )
        Sprintf( str,"%-7s %s, %s",ui.CommandBuffer,ui.LeftBuffer,ui.RightBuffer );
       else
        Sprintf( str,"%-7s %s",ui.CommandBuffer,ui.LeftBuffer );

      if ( ui.Comments.Length() )
        BufferedPrintf( buff,sizeof(buff),print,"%-30s ;%s",str,ui.Comments.c_str() );
       else
        BufferedPrintf( buff,sizeof(buff),print,"%s",str );

      BufferedPrintf( buff,sizeof(buff),print,"\n" );
    }
}

#endif
