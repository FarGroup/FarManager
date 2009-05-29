#ifndef __MY_SYMBOLS
#define __MY_SYMBOLS

//---------------------------------------------------------------------------
enum _SYM_SymbolType {
 SYM_SRC_MAP   = 0x00010000UL,  //Source of symbol
 SYM_SRC_IMP   = 0x00020000UL,
 SYM_SRC_EXP   = 0x00040000UL,

 SYM_TP_UNK    = 0x00000000UL,  //Type of symbol
 SYM_TP_CODE   = 0x00000001UL,
 SYM_TP_DATA   = 0x00000002UL,

 SYM_TP_MASK   = 0x0000FFFFUL,
 SYM_SRC_MASK  = 0xFFFF0000UL
};

PRECLASS( SymModule );

STRUCT( Symbol )
  SIZE_T     Addr;                 //Symbol address
  DWORD      Index;                //Index in base module array
  DWORD      Type;                 //Set of SYM_xxx
  DWORD      NameOff;              //Offset of symbol name of `Name` (only if symbol imported)
  char       Name[ MAX_PATH_SIZE ];
  PSymModule Owner;
};

//---------------------------------------------------------------------------
#define MAX_SECTIONS  20

CLASSBASE( SymArray, public MyRefArray<Symbol> )
  public:
    LPVOID     LoadBase;
    SIZE_T     ImageBase;
    SIZE_T     Sections[ MAX_SECTIONS ];
  protected:
    DEF_QSORT( Symbol, SortAddrINT, ; )
  public:
    SymArray( void );

    DEF_BSEARCH( Symbol, FindAddr, ; )

    PSymbol Find( SIZE_T addr );
    void    SortAddr( void );
};

//---------------------------------------------------------------------------
typedef SIZE_T (RTL_CALLBACK *Off2RVA_t)( SIZE_T LoadBase, DWORD Offset, PIMAGE_SECTION_HEADER Section );

CLASSBASE( SymModule, public SymArray )
  public:
    PathString PathName;
    PathString Name;
    HMODULE    Handle;
  protected:
    void    LoadImports( Off2RVA_t rva,PHModuleInformation ModuleInformation );
    void    LoadExports( Off2RVA_t rva,PHModuleInformation ModuleInformation );
    void    LoadLocals( void );
  public:
    SymModule( PHModuleInformation mi, Off2RVA_t rva = NULL,DWORD Flags = STK_LOADEXPORTS|STK_LOADLOCALS );
};

//---------------------------------------------------------------------------
STRUCTBASE( SymDisasm, public UnassembeInfo )
    PathString Comments;
    char       Sign;
    SIZE_T     MainAddress;

    SymDisasm( SIZE_T MainAddr );

    virtual void  Operand( SIZE_T& addr,char& useSign,int& opSize );
    virtual SIZE_T Position( void )  = 0;
    virtual BOOL  Unassemble( void );
};

//---------------------------------------------------------------------------
HDECLSPEC PSymbol    MYRTLEXP LocateModuleSymbol( HMODULE Module,SIZE_T addr,DWORD Flags /*STK_xxx*/ );
HDECLSPEC PSymbol    MYRTLEXP LocateModuleSymbolBase( LPVOID Module,SIZE_T addr,DWORD Flags /*STK_xxx*/ );
HDECLSPEC PSymbol    MYRTLEXP LocateSymbol( SIZE_T addr,DWORD Flags /*STK_xxx*/ );

HDECLSPEC PSymModule MYRTLEXP ManualAddModule( PHModuleInformation mi,Off2RVA_t rva /*=NULL*/,DWORD Flags );

HDECLSPEC BOOL       MYRTLEXP AddModuleSymbol( HMODULE Module,DWORD Flags /*STK_xxx*/,PSymbol sym );
HDECLSPEC BOOL       MYRTLEXP AddModuleSymbolBase( LPVOID ModuleBase,DWORD Flags /*STK_xxx*/,PSymbol sym );
HDECLSPEC BOOL       MYRTLEXP AddSymbol( SIZE_T addr,DWORD Flags /*STK_xxx*/,PSymbol sym );

HDECLSPEC BOOL       MYRTLEXP DemangleName2( char *InOut, size_t OutSize );
HDECLSPEC BOOL       MYRTLEXP DemangleName( CONSTSTR MangledName,char *OutputBuffer,size_t OutputBufferSize );
HDECLSPEC BOOL       MYRTLEXP IsMangledName( CONSTSTR nm );
HDECLSPEC BOOL       MYRTLEXP IsMangledSymbol( CONSTSTR nm );
HDECLSPEC BOOL       MYRTLEXP IsSystemModule( HMODULE Module );
HDECLSPEC BOOL       MYRTLEXP IsSystemModule( CONSTSTR ModulePathName );
HDECLSPEC void       MYRTLEXP MakeSymDisasm( HPrintProc_t print,SymDisasm& ui );

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

#endif
