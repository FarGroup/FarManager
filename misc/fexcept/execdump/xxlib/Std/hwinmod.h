#ifndef __MY_H_WIN_MODULE
#define __MY_H_WIN_MODULE

#if defined(__HWIN32__)

LOCALSTRUCT( HModuleInformation )
  char                  FileName[ MAX_PATH_SIZE ];
  LPVOID                LoadBase;
  HMODULE               Module;
  PIMAGE_NT_HEADERS     Header;
  PIMAGE_SECTION_HEADER Sections;
};

/*
   Searches an image filename.
   Returns full image pathname.
   Scans currently loaded images for given ProcessId of for current process.

   Returns "" in name not found.
*/
HDECLSPEC CONSTSTR            MYRTLEXP GetModuleFileName( HMODULE Module,DWORD ProcessId = 0 );
HDECLSPEC CONSTSTR            MYRTLEXP GetModuleFileNameBase( LPVOID ModuleAllocationBase,DWORD ProcessId = 0 );
HDECLSPEC CONSTSTR            MYRTLEXP GetModuleFileNameAddr( LPVOID AddrFromModule,DWORD ProcessId = 0 );

/*
   Gets access to PE image of loaded module by it HMODULE from process enviropment or by image
   base load address.

   Rets NULL if module not found.
*/
HDECLSPEC PHModuleInformation MYRTLEXP GetModuleInformation( HMODULE Module,DWORD ProcessId = 0 );
HDECLSPEC PHModuleInformation MYRTLEXP GetModuleInformationBase( LPVOID ModuleAllocationBase,DWORD ProcessId = 0 );
HDECLSPEC PHModuleInformation MYRTLEXP GetModuleInformationAddr( LPVOID AddrFromModule,DWORD ProcessId = 0 );

/*
    Opens PE file for RO and returns NULL or ptr to PIMAGE_NT_HEADERS.
*/
HDECLSPEC PIMAGE_NT_HEADERS   MYRTLEXP OpenPEMapping( CONSTSTR FileName, HANDLE& hFile, HANDLE& hMapping, LPVOID& hAddr );
HDECLSPEC void                MYRTLEXP ClosePEMapping( HANDLE& hFile, HANDLE& hMapping, LPVOID& hAddr );

/*
    Enumerate PE data diretories belongs to section.

    Set Index to 0 on start and call this procedure until it returns not NULL to
    enumerate all data directories for target section.

    You must increment index after each successfull find.
    For example:
      for( i = 0; (dd=FindDataDirectory( Header,ps,i )) != NULL; i++ )
        ...
*/
HDECLSPEC PIMAGE_DATA_DIRECTORY MYRTLEXP FindDataDirectory( PIMAGE_NT_HEADERS Header,
                                                            PIMAGE_SECTION_HEADER ps,
                                                            DWORD& Index );
HDECLSPEC BOOL AllowSymAPI;

typedef void (MYRTLEXP *EnumModuleCB)( CONSTSTR FileName,LPVOID Base, HMODULE hm,LPVOID Ptr );

HDECLSPEC BOOL MYRTLEXP QueryModules_NT( DWORD ProcessId, EnumModuleCB cb,LPVOID Ptr );
HDECLSPEC BOOL MYRTLEXP QueryModules_TH( DWORD ProcessId, EnumModuleCB cb,LPVOID Ptr );

#endif //WIN32
#endif
