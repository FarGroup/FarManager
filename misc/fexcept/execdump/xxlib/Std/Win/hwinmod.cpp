#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__HWIN32__)
#include <winnt.h>

#if 1
  #define  PROC( v )
  #define  Log( v )
#else
  #define  PROC( v )  INProc __proc v ;
  #define  Log( v )   INProc::Say v
#endif

#define ABORT                 Log
//#define ABORT( v )          HAbort v

#define MAP_ABORT(v)          { ABORT( v ); break; }
#define QNT_ERROR( v )        { ABORT( v ); return FALSE; }
#define TH_ABORT(v)           { ABORT( v ); return FALSE; }

BOOL AllowSymAPI = TRUE;

/****************************************************
   NT process enumeration
 ****************************************************/
typedef struct {
 DWORD  unused[2];
 LPVOID ImageBase;
 DWORD  ImageSize;
 DWORD  Flags;
 USHORT Index;
 USHORT unused2;
 USHORT LoadCount;
 USHORT ModuleNameOffset;
 CHAR   ImageName[ 256 ];
} ModuleInformation, *LPModuleInformation;

typedef struct {
  DWORD             ModulesCount;
  ModuleInformation mi[1];
} ModuleInformationArray, *LPModuleInformationArray;

typedef struct {
  DWORD                    nonInteresed[12];
  LPModuleInformationArray ModuleInformation;
  DWORD                    nonInteresed2[11];
} DebugBuffer, *LPDebugBuffer;

#define PDI_MODULES 1

typedef LPDebugBuffer (WINAPI *RtlCreateQueryDebugBuffer_t)( DWORD,BOOL );
typedef BOOL          (WINAPI *RtlQueryProcessDebugInformation_t)( DWORD ProcessId,DWORD Index,LPDebugBuffer Info );
typedef BOOL          (WINAPI *RtlDestroyQueryDebugBuffer_t)( LPDebugBuffer Info );

/****************************************************
   NT process enumeration
 ****************************************************/
BOOL MYRTLEXP QueryModules_NT( DWORD ProcessId, EnumModuleCB cb,LPVOID Ptr )
  {  PROC(( "QueryModules_NT","id:%08X",ProcessId ))
     RtlCreateQueryDebugBuffer_t       pCreat;
     RtlQueryProcessDebugInformation_t pQuery;
     RtlDestroyQueryDebugBuffer_t      pDestroy;

     HMODULE hnt = GetModuleHandle("ntdll");
     if (!hnt)
       QNT_ERROR(( "!ntdll" ))

     pCreat   = (RtlCreateQueryDebugBuffer_t)      GetProcAddress(hnt, "RtlCreateQueryDebugBuffer");
     pQuery   = (RtlQueryProcessDebugInformation_t)GetProcAddress(hnt, "RtlQueryProcessDebugInformation");
     pDestroy = (RtlDestroyQueryDebugBuffer_t)     GetProcAddress(hnt, "RtlDestroyQueryDebugBuffer");

     if ( !pCreat || !pQuery || !pDestroy )
       QNT_ERROR(( "!GetFunctions" ))

     LPDebugBuffer pb = pCreat(0, FALSE);
     if(!pb)
       return FALSE; //HAbort( "!GetBuffer" );

     if ( pQuery( ProcessId,PDI_MODULES,pb) ) {
       pDestroy(pb);
       QNT_ERROR(( "!QueryInfo" ))
     }

     LPModuleInformation pMod   = pb->ModuleInformation->mi;
     DWORD               modCnt = pb->ModuleInformation->ModulesCount;

     for( DWORD i = 0; i < modCnt; i++, pMod++) {
         Log(( "ImageBase: %08X ImageSize: %08X Flags: %08X Index: %d LoadCount: %04X ModuleNameOffset: %04X\n"
               "\tImageName: [%s] (HMODULE: %08X)",
               pMod->ImageBase, pMod->ImageSize, pMod->Flags, pMod->Index, pMod->LoadCount,
               pMod->ModuleNameOffset, pMod->ImageName,
               GetModuleHandle(pMod->ImageName) ));

       cb( pMod->ImageName, pMod->ImageBase,
           GetModuleHandleA(pMod->ImageName),
           Ptr );
     }

 return TRUE;
}

/****************************************************
   Toolhelp process enumeration
 ****************************************************/
#if !defined( _INC_TOOLHELP32 )
#define MAX_MODULE_NAME32 255
#define TH32CS_SNAPMODULE 0x00000008
typedef struct tagMODULEENTRY32
{
    DWORD   dwSize;
    DWORD   th32ModuleID;       // This module
    DWORD   th32ProcessID;      // owning process
    DWORD   GlblcntUsage;       // Global usage count on the module
    DWORD   ProccntUsage;       // Module usage count in th32ProcessID's context
    BYTE  * modBaseAddr;        // Base address of module in th32ProcessID's context
    DWORD   modBaseSize;        // Size in bytes of module starting at modBaseAddr
    HMODULE hModule;            // The hModule of this module in th32ProcessID's context
    char    szModule[MAX_MODULE_NAME32 + 1];
    char    szExePath[MAX_PATH];
} MODULEENTRY32, *LPMODULEENTRY32;
#endif

typedef HANDLE (WINAPI *CreateToolhelp32Snapshot_t)(DWORD dwFlags,DWORD th32ProcessID);
typedef BOOL   (WINAPI *Module32First_t)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
typedef BOOL   (WINAPI *Module32Next_t)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

BOOL MYRTLEXP QueryModules_TH( DWORD ProcessId, EnumModuleCB cb,LPVOID Ptr )
  {  PROC(( "QueryModules_TH","id:%08X",ProcessId ))

    static HMODULE                    hk32 = NULL;
    static CreateToolhelp32Snapshot_t pCreate;
    static Module32First_t            pFirst;
    static Module32Next_t             pNext;

    if ( !hk32 ) {
      hk32 = GetModuleHandle("kernel32");
      if ( !hk32 )
        hk32 = (HMODULE)INVALID_HANDLE_VALUE;

      pCreate = (CreateToolhelp32Snapshot_t)GetProcAddress(hk32, "CreateToolhelp32Snapshot");
      pFirst  = (Module32First_t)GetProcAddress(hk32, "Module32First");
      pNext   = (Module32Next_t)GetProcAddress(hk32, "Module32Next");

      if ( !pCreate || !pFirst || !pNext )
        hk32 = (HMODULE)INVALID_HANDLE_VALUE;
    }
    if ( hk32 == (HMODULE)INVALID_HANDLE_VALUE )
      TH_ABORT(( "!kernel" ))

    MODULEENTRY32 m;
    HANDLE        h;

    h = pCreate( TH32CS_SNAPMODULE, ProcessId );
    if ( h == INVALID_HANDLE_VALUE )
      TH_ABORT(( "!CreateSnapshoot" ))

    m.dwSize = sizeof(m);

    if ( !pFirst(h,&m) ) {
      CloseHandle(h);
      TH_ABORT(( "!FirstEnum" ))
    }

    do {
      cb( m.szExePath, m.modBaseAddr, m.hModule,Ptr );
    }while(pNext(h, &m));

    if (GetLastError() != ERROR_NO_MORE_FILES) {
      CloseHandle(h);
      TH_ABORT(( "enumerr %s",FIO_ERROR ))
    }

    CloseHandle(h);

 return TRUE;
}

/****************************************************
   Module array
 ****************************************************/
LOCALSTRUCTBASE( HModuleInfo, public HModuleInformation )
//Mapping info
  HANDLE   hFile;
  HANDLE   hMapping;
  LPVOID   hAddress;

  MYRTLEXP HModuleInfo( PIMAGE_NT_HEADERS h ) {
    Header   = h;
    Sections = h ? IMAGE_FIRST_SECTION(h) : NULL;
  }
  MYRTLEXP ~HModuleInfo( void ) {
    ClosePEMapping( hFile,hMapping,hAddress );
    Header   = NULL;
    Sections = NULL;
  }

  BOOL isTheSameBase( LPVOID imBase ) { return LoadBase == imBase; }
  BOOL isTheSameHMod( HMODULE hm )    { return Module == hm; }
};

LOCALCLASSBASE( HModuleArray, public MyArray<PHModuleInfo> )
  public:
    DWORD ProcessId;

  public:
    MYRTLEXP HModuleArray( DWORD id ) {
        Log(("HModuleArray::HModuleArray id:%08X",id ));
        ProcessId = id;
        if ( ProcessId == 0 ) ProcessId = GetCurrentProcessId();
    }

    void Rescan( void );

    DEF_SCANER( PHModuleInfo, FindBase, LPVOID,  { return Item->isTheSameBase(Data); } )
    DEF_SCANER( PHModuleInfo, FindHMod, HMODULE, { return Item->isTheSameHMod(Data); } )
};

LOCALCLASSBASE( HProcessArray, public MyArray<PHModuleArray> )
  public:
    MYRTLEXP HProcessArray( void ) {}

    DEF_SCANER( PHModuleArray, FindId, DWORD, { return Item->ProcessId == Data; } )
};

static void MYRTLEXP idEnum( CONSTSTR FileName,LPVOID LoadBase, HMODULE hm,LPVOID Ptr );

//+ DATA
static PHProcessArray Processes = NULL;
static AbortProc      aProc;

static void RTL_CALLBACK idHWinExit( void )
  {
    delete Processes;
    Processes = NULL;
    if ( aProc )
      aProc();
}

static void MYRTLEXP HWINInitialize( void )
  {
    if ( !Processes ) {
      Processes = new HProcessArray;
      aProc = AtExit( idHWinExit );
    }
}

/****************************************************
   Module array
 ****************************************************/
void HModuleArray::Rescan( void )
  {
    DeleteAll();

    if ( !QueryModules_TH( ProcessId,idEnum,this ) )
      if ( VERSION_WIN_NT && AllowSymAPI )
        QueryModules_NT( ProcessId,idEnum,this );
}

void MYRTLEXP idEnum( CONSTSTR FileName,LPVOID LoadBase, HMODULE hm,LPVOID Ptr )
  {  PHModuleArray ma = (PHModuleArray)Ptr;
     PHModuleInfo      p;
     HANDLE            hFile,
                       hMapping;
     LPVOID            hAddr;
     PIMAGE_NT_HEADERS fh;

     Log(( "idEnum [%s] l:%08X m:%08X",FileName,LoadBase,hm ));

     if ( !FileName[0] ) HAbort( "!FileName" );
     if ( !LoadBase )    HAbort( "!LoadBase for [%s]",FileName );
     if ( !hm)           HAbort( "!HModule for [%s]",FileName );

     fh = OpenPEMapping( FileName,hFile,hMapping,hAddr );
     if ( !fh ) {
       Log(( "Enumerating non PE module [%s]",FileName ));
       return;
     }

     p = ma->Add( new HModuleInfo(fh) );

     StrCpy( p->FileName,FileName,sizeof(p->FileName) );
     p->LoadBase  = LoadBase;
     p->Module    = hm;
     p->hAddress  = hAddr;
     p->hFile     = hFile;
     p->hMapping  = hMapping;
}

/****************************************************
   ModuleInformation
 ****************************************************/
static PHModuleInfo FindInfo_HMod( HMODULE m,DWORD ProcessId )
  {  PROC(( "FindInfo_HMod","%08X id:%08X",m,ProcessId ))

     PHModuleArray ma;
     PHModuleInfo  mi;

     if ( !ProcessId ) ProcessId = GetCurrentProcessId();
     if ( !m )         m         = GetModuleHandle(NULL);
     Log(( "mod: %08X id:%08X",m,ProcessId ));

     HWINInitialize();
     ma = Processes->FindId( ProcessId );
     if ( !ma ) {
       ma = Processes->Add( new HModuleArray(ProcessId) );
       Log(( "Add new HModuleArray %08X",ProcessId ));
     }

     mi = ma->FindHMod( m );
     if ( !mi ) {
       ma->Rescan();
       mi = ma->FindHMod(m);
     }

 return mi;
}

static PHModuleInfo FindInfo_Base( LPVOID b,DWORD ProcessId )
  {  PROC(( "FindInfo_Base","%08X id:%08X",b,ProcessId ))
     PHModuleArray ma;
     PHModuleInfo  mi;

     if ( !b )
       return NULL; //HAbort( "Query info for module by zero load base!" );

     if ( !ProcessId ) ProcessId = GetCurrentProcessId();
     Log(( "base: %08X id:%08X",b,ProcessId ));

     HWINInitialize();
     ma = Processes->FindId( ProcessId );
     if ( !ma ) {
       ma = Processes->Add( new HModuleArray(ProcessId) );
       Log(( "Add new HModuleArray" ));
     }

     mi = ma->FindBase(b);
     if ( !mi ) {
       ma->Rescan();
       mi = ma->FindBase(b);
     }

 return mi;
}

CONSTSTR MYRTLEXP GetModuleFileName( HMODULE m,DWORD ProcessId /*=0*/ )
  {  PHModuleInfo mi = FindInfo_HMod( m,ProcessId );

 return mi ? mi->FileName : "";
}

CONSTSTR MYRTLEXP GetModuleFileNameBase( LPVOID b,DWORD ProcessId /*=0*/ )
  {  PHModuleInfo mi = FindInfo_Base(b,ProcessId);

 return mi ? mi->FileName : "";
}

CONSTSTR MYRTLEXP GetModuleFileNameAddr( LPVOID Addr,DWORD ProcessId /*=0*/ )
  {  MEMORY_BASIC_INFORMATION mbi;

     if ( !Addr || !VirtualQuery(Addr,&mbi,sizeof(mbi)) )
       return "";

 return GetModuleFileNameBase( mbi.AllocationBase,ProcessId );
}

PHModuleInformation MYRTLEXP GetModuleInformation( HMODULE b,DWORD ProcessId )
  {
 return FindInfo_HMod( b,ProcessId );
}

PHModuleInformation MYRTLEXP GetModuleInformationBase( LPVOID b,DWORD ProcessId )
  {
 return FindInfo_Base( b,ProcessId );
}

PHModuleInformation MYRTLEXP GetModuleInformationAddr( LPVOID Addr,DWORD ProcessId /*=0*/ )
  {  MEMORY_BASIC_INFORMATION mbi;

     if ( !Addr || !VirtualQuery(Addr,&mbi,sizeof(mbi)) )
       return NULL;

 return GetModuleInformationBase( mbi.AllocationBase );
}

/****************************************************
   PEMapping
 ****************************************************/
void MYRTLEXP ClosePEMapping( HANDLE& hFile, HANDLE& hMapping, LPVOID& hAddr )
  {
     if (hAddr)    UnmapViewOfFile(hAddr); hAddr    = NULL;
     if (hMapping) CloseHandle(hMapping);  hMapping = NULL;
     if (hFile)    CloseHandle(hFile);     hFile    = NULL;
}

PIMAGE_NT_HEADERS MYRTLEXP OpenPEMapping( CONSTSTR FileName, HANDLE& hFile, HANDLE& hMapping, LPVOID& hAddr )
  {  PIMAGE_NT_HEADERS fh;

     hAddr    = NULL;
     hMapping = NULL;
     hFile    = NULL;

     do{
       hFile = CreateFile( FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
       if ( hFile == INVALID_HANDLE_VALUE )
         MAP_ABORT(( "!Open [%s]",FileName ));

       if ( (hMapping=CreateFileMapping(hFile,NULL,PAGE_READONLY,0,0,NULL)) == NULL )
         MAP_ABORT(("!Createmapping" ));

       if ( (hAddr=MapViewOfFile(hMapping,FILE_MAP_READ,0,0,0 )) == NULL )
         MAP_ABORT(("!Map"));

       fh = (PIMAGE_NT_HEADERS)hAddr;
       if ( !CheckReadable(fh,sizeof(IMAGE_NT_HEADERS64)) ||
            ((PIMAGE_DOS_HEADER)fh)->e_magic != IMAGE_DOS_SIGNATURE )
         MAP_ABORT(("!Magic %08X",((PIMAGE_DOS_HEADER)fh)->e_magic));

       fh = (PIMAGE_NT_HEADERS)((DWORD)fh + ((PIMAGE_DOS_HEADER)fh)->e_lfanew);
       if ( !CheckReadable(fh,sizeof(IMAGE_NT_HEADERS64)) ||
            fh->Signature != IMAGE_NT_SIGNATURE && fh->Signature != IMAGE_VXD_SIGNATURE )
         MAP_ABORT(("!Signature %08X",fh->Signature));

       if ( !CheckReadable( IMAGE_FIRST_SECTION(fh),sizeof(IMAGE_SECTION_HEADER)*fh->FileHeader.NumberOfSections ) )
         MAP_ABORT(("!Sections"));

       return fh;

     }while(0);

     ClosePEMapping( hFile,hMapping,hAddr );

 return NULL;
}
/****************************************************
   PE manipulation
 ****************************************************/
PIMAGE_DATA_DIRECTORY MYRTLEXP FindDataDirectory( PIMAGE_NT_HEADERS Header, PIMAGE_SECTION_HEADER ps,DWORD& Index )
  {  PIMAGE_DATA_DIRECTORY dd;
     DWORD                 lo = ps->VirtualAddress,
                           hi = ps->VirtualAddress + ps->SizeOfRawData;

     for ( ; Index < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; Index++ ) {
       dd = &Header->OptionalHeader.DataDirectory[Index];
       if ( dd->VirtualAddress >= lo && dd->VirtualAddress <= hi )
         return dd;
     }
 return NULL;
}
#endif //WIN32
