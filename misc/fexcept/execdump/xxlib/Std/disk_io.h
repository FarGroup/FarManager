#ifndef __DISK_IO
#define __DISK_IO

#if defined(__cplusplus)
/*********************************
        FILE_ENUM
 *********************************/
STRUCT( FILE_ENUM )
    MyString  Path,oldDir,FPath;

  #if defined(__GNUC__)
    dirent   *ent;
    DIR      *dir;
  #else
  #if defined(__QNX__)
    dirent   *ent;
    DIR      *dir;
  #else
  #if defined(__REALDOS__) || defined(__HWIN16__)
    struct ffblk f;
  #else
  #if defined(__PROTDOS__)
    struct FIND f;
  #else
  #if defined(__HWIN32__)
    HANDLE          fHandle;
    WIN32_FIND_DATA f;
  #else
  #error ERR_PLATFORM
  #endif //Win32
  #endif //ProtDOS
  #endif //RealDOS
  #endif //QNX
  #endif //GNUC

  BOOL  MYRTLEXP OpenDir( CONSTSTR path = NULL );
  char *MYRTLEXP FName( void );
  char *MYRTLEXP PathName( void );
  BOOL  MYRTLEXP NextFile( void );
  void  MYRTLEXP CloseDir( void );
};

/*********************************
        HotPathEntry
 *********************************/
STRUCT( FileAttrEntry )
  const char* Name;
  int   Value;
  char  CharDescr;
};

STRUCT( HotPathEntry )
  MyString Path,
           Label,
           Description;

  HotPathEntry( void ) {}
  HotPathEntry( const MyString& p,const MyString& l,const MyString& d ) { Path = p; Label = l; Description = d; }
};

typedef MyArray<PHotPathEntry> *PHotPathArray;
typedef MyArray<PHotPathEntry> HotPathArray;
#endif

#if defined( __QNX__ )
  HDECLSPEC BOOL QnxCmpFile( char *f1, char *f2, int len = -1 );
  #define CMP_CHAR( a,b )     ((a)==(b))
  #define CMP_FILE( a,b )     QnxCmpFile( a,b )
  #define CMPN_FILE( a,b,l )  QnxCmpFile( a,b,l )
#else
#if defined( __GNUC__ )
  #define CMP_CHAR( a,b )     ((a)==(b))
  #define CMP_FILE( a,b )     ( StrCmp(a,b) == 0 )
  #define CMPN_FILE( a,b,l )  ( StrCmp(a,b,l) == 0 )
#else
#if defined( __HDOS__ )
  #define CMP_CHAR( a,b )    (toupper(a)==toupper(b))
  #define CMP_FILE( a,b )    (StrCmpI(a,b) == 0)
  #define CMPN_FILE( a,b,l ) (StrNCmpI(a,b,l) == 0)
#else
#if defined( __HWIN16__ )
  #define CMP_CHAR( a,b )    (tolower(a) == tolower(b))
  #define CMP_FILE( a,b )    (StrCmpI(a,b) == 0)
  #define CMPN_FILE( a,b,l ) (StrNCmpI(a,b,l) == 0)
#else
#if defined( __HWIN32__ )
  #define CMP_CHAR( a,b )    (CharLower( (LPTSTR)MK_DWORD(0,a) ) == CharLower( (LPTSTR)MK_DWORD(0,b) ))
  #define CMP_FILE( a,b )    (StrCmpI(a,b) == 0)
  #define CMPN_FILE( a,b,l ) (StrNCmpI(a,b,l) == 0)
#else
  #error ERR_PLATFORM
#endif//WIN32
#endif//WIN16
#endif//DOS
#endif//QNX
#endif//GNUC

/*********************************
        FileEntry `Flags`
 *********************************/
enum _FileEntryFlags {
 FF_SETTED        = 0x10,
 FF_SELECTED      = 0x01,
 FF_FULLPATH      = 0x02,
 FF_VIRTUALFILE   = 0x04,
 FF_USED          = 0x08   //Used in update path
};

#define FE_MAXDATE       11

#if defined(__cplusplus)
/*********************************
        FileEntry
 *********************************/
PRECLASS( DirEntry );

STRUCT( FileEntry )
  void FillData( char *fname,FILE_ENUM *fe );  //Fill data members after setup Path & Name
 public:
  struct stat Info;
  ATTR_TYPE   allAttr;
  PDirEntry   Parent;
  DWORD       Flags;
  MyString    _Name,
              _Path;
#if defined(__QNX__) || defined(__GNUC__)
  MyString    _Link,
              _MountOn,_MountTo;
#endif

  FileEntry( void );
  FileEntry( PDirEntry owner );
  FileEntry( PDirEntry owner,FileEntry& p );
  FileEntry( PDirEntry owner,CONSTSTR path );
  FileEntry( PDirEntry owner,FILE_ENUM *fe );

  FileEntry& operator=( FileEntry& p );

  void Assign( PDirEntry owner,FileEntry& p );
  void Assign( PDirEntry owner,CONSTSTR path );
  void Assign( PDirEntry owner,FILE_ENUM *fe );
  void AssignSize( long sz );
  void AssignPath( const MyString& p );

  MyString   PathName( void );          //FULL PathName
  MyString   ShortPathName( void );     //FULL PathName with short name
  char      *Path( void );              //Path only
  char      *NameExt( void )            { return _Name.Text(); }
  MyString   Name( void );              //Name only
  MyString   Ext( void );               //Ext only
  MyString   ShortNameExt( void );      //Short version (acc in Windows)
  MyString   ShortName( void );
  MyString   ShortExt( void );
  long       Size( void );
  time_t     Time( void );
  time_t     ModifyTime( void );
  time_t     AccessTime( void );
  int        Owner( void );
  int        Attr( void );
  int        Group( void );
  int        User( void );
  MyString   Link( void );          // if file link, name of link f.e. /bin/Ansi.qnx -> "/bin/Ansi"
  MyString   MountOn( void );       // name of device file mount on f.e. "/dev/hd0t77"
  MyString   MountTo( void );       // mounted dir f.e. "/" for "/dev/hd0t77"

  void       SetTime( time_t cr, time_t ac, time_t mod );
  void       SetAttr( ATTR_TYPE v );
  void       SetGroupUser( int user,int group );

  MyString   sSize( char delimiter = 0,int divider = 1,CONSTSTR fmt = NULL ); //NULL means "%lu"
  char      *sTime( void );
  char      *sDate( void );
  char      *sModifyTime( void );
  char      *sModifyDate( void );
  char      *sAccessTime( void );
  char      *sAccessDate( void );
  char      *sOwner( void );
  char      *sGroup( void );
  char      *sUser( void );
  char      *sAttr( void );

  BOOL       Executable( void );
  BOOL       Directory( void );
  BOOL       Setted( void ) { return IS_FLAG(Flags,FF_SETTED); }
  char       GetTypeChar( void );
};
#endif

#if defined(__cplusplus)
/*********************************
          DirEntry
 *********************************/
#define SEL_PATTERN  ((char*)0xFFFF)

typedef BOOL (*OpenDirProc)( long count );

CLASSBASE( DirEntry, public MyArray<PFileEntry> )
 public:
  MyString Path;

 public:
  DirEntry( BOOL allocBig = TRUE );

  BOOL isRealPath( void ) { return Path[0] != ':'; }

  BOOL OpenDir( CONSTSTR path,OpenDirProc callBack = NULL,CONSTSTR filter = NULL,CONSTSTR exfilter = NULL );
  int  IndexOf(const char* fname);
  int  SelCount( void );
  BOOL Assign( DirEntry& from, BOOL selected );
  BOOL Assign( DirEntry& from, PFileEntry p );
};
#endif

#if !defined(CT_EVN_CFG)
  #define CT_ENV_CFG "PNC"
#endif
#if !defined(CT_EVN_CFG1)
  #define CT_ENV_CFG1 "POPA"
#endif
/*********************************
             Data
 *********************************/
HDECLSPEC FileAttrEntry  MYRTLEXP_PT FileAttributes[];
HDECLSPEC pchar          MYRTLEXP_PT CTCfgOverride;    //Pointer to overrided Popa`s base path
                                        //Assign here `strdup`-ed string pointer

enum cfpTypes {
  cfpPATH,
  cfpNAME,
  cfpNAMEEXT,
  cfpEXT
};

#if defined(__HDOS__)
enum DriveTypes {
  DRIVE_ERROR     = 0x00, /* Bad drive */
  DRIVE_FIXED     = 0x01, /* Fixed drive */
  DRIVE_REMOVABLE = 0x02, /* Removeable (floppy) drive */
  DRIVE_REMOTE    = 0x03, /* Remote (network) drive */
  DRIVE_CDROM     = 0x04, /* MSCDEX V2.00+ driven CD-ROM drive */
  DRIVE_DBLSPACE  = 0x05, /* DoubleSpace compressed drive */
  DRIVE_SUBST     = 0x06, /* SUBST'ed drive */
  DRIVE_RAMDISK   = 0x08  /* RAM drive */
};

HDECLSPEC DriveTypes GetDiskType( BYTE Drive );              // Rets specified drive type
HDECLSPEC UINT       GetDriveType( LPCTSTR lpRootPathName ); // --`--
HDECLSPEC int        GetFloppyCount( void );                 // Rets floppy disk count
#endif

/*********************************
   Dirs
 *********************************/
HDECLSPEC BOOL       MYRTLEXP SetCurDir( CONSTSTR path );
HDECLSPEC CONSTSTR   MYRTLEXP GetCurDir( void );
HDECLSPEC MyString   MYRTLEXP MakeRootDir( CONSTSTR path );                     //Expand root path from full path
HDECLSPEC CONSTSTR   MYRTLEXP GetTmpDir( void );                                //Get ENV
HDECLSPEC CONSTSTR   MYRTLEXP GetStartupDir( HANDLE h = NULL );
HDECLSPEC CONSTSTR   MYRTLEXP LocalFile( CONSTSTR FileName,CONSTSTR basePath = NULL );
HDECLSPEC BOOL       MYRTLEXP CreateFullDirectory( char *nm );

/*********************************
   Path`s
 *********************************/
HDECLSPEC pchar      MYRTLEXP FixSlashChars( char *str,char CharChangeTo = SLASH_CHAR );

#if defined(__cplusplus)
HDECLSPEC MyString   MYRTLEXP FindFilePath( CONSTSTR path,CONSTSTR sep,CONSTSTR fname );
HDECLSPEC MyString   MYRTLEXP DelLastSlash( const MyString& path, char Slash = SLASH_CHAR );
HDECLSPEC void       MYRTLEXP DelLastSlash( PathString& path, char Slash = SLASH_CHAR );
HDECLSPEC MyString   MYRTLEXP AddLastSlash( MyString& path, char Slash = SLASH_CHAR );
HDECLSPEC void       MYRTLEXP AddLastSlash( PathString& path, char Slash = SLASH_CHAR );
#endif

HDECLSPEC pchar      MYRTLEXP DelLastSlash( char *path, char Slash = SLASH_CHAR );
HDECLSPEC pchar      MYRTLEXP AddLastSlash( char *path, char Slash = SLASH_CHAR );

HDECLSPEC BOOL       MYRTLEXP RelativeConvertable( CONSTSTR base, CONSTSTR path );
HDECLSPEC BOOL       MYRTLEXP IsAbsolutePath( CONSTSTR path );
HDECLSPEC pchar      MYRTLEXP MakeAbsolutePath( char *path );
#if defined(__cplusplus)
HDECLSPEC MyString   MYRTLEXP MakeRelativePath( CONSTSTR base, CONSTSTR path  );
HDECLSPEC MyString   MYRTLEXP MakeStartLocalPath( CONSTSTR fnm );            //Creates full pathname relative on startup path
HDECLSPEC MyString   MYRTLEXP MakeStartRelativePath( CONSTSTR path );        //Creates relative path based on startup path
#endif

#if defined(__cplusplus)
inline    MyString   MYRTLEXP MakeStartLocalPath( const MyString& fnm )      { return MakeStartLocalPath( fnm.c_str() ); }
inline    MyString   MYRTLEXP MakeStartRelativePath( const MyString& fnm )   { return MakeStartRelativePath(fnm.c_str()); }
#if defined(__VCL__)
inline    MyString   MYRTLEXP MakeStartLocalPath( const AnsiString& fnm )    { return MakeStartLocalPath( fnm.c_str() ); }
inline    MyString   MYRTLEXP MakeStartRelativePath( const AnsiString& fnm ) { return MakeStartRelativePath(fnm.c_str()); }
#endif
#endif
/*******************************************************************
   Pathname
 *******************************************************************/
HDECLSPEC pchar      MYRTLEXP MakePathName( char *path,CONSTSTR fname,char Slash = SLASH_CHAR );
#if defined(__cplusplus)
HDECLSPEC MyString   MYRTLEXP ChangeFilePart(const char* fpath,CONSTSTR newPart,cfpTypes chType, char Slash = SLASH_CHAR );
HDECLSPEC MyString   MYRTLEXP MakePathName( CONSTSTR path,CONSTSTR fname,char Slash = SLASH_CHAR ); //Make pathname from `path` & `name`
HDECLSPEC MyString   MYRTLEXP MakeFullPathName( const MyString& fname, const MyString& basePath );  //Make full path-ed filename
HDECLSPEC MyString   MYRTLEXP MakePathString( CONSTSTR path, int maxWidth );                 // inserts "/../" if length > maxWidth
HDECLSPEC MyString   MYRTLEXP GetLastDirName( const MyString& path, char Slash = SLASH_CHAR );      //ret`s last directory from path
HDECLSPEC MyString   MYRTLEXP GetFPath( const MyString& pathname, char Slash = SLASH_CHAR );        //ret`s path from full pathname
HDECLSPEC MyString   MYRTLEXP GetFName( const MyString& pathname, char Slash = SLASH_CHAR );        //ret`s name & ext from full pathname
HDECLSPEC MyString   MYRTLEXP GetFNameOnly( const MyString& pathname, char Slash = SLASH_CHAR );    //ret`s name from full pathname
HDECLSPEC MyString   MYRTLEXP GetFExtOnly( const MyString& pathname, char Slash = SLASH_CHAR );     //ret`s ext from full pathname
#endif
HDECLSPEC pchar      MYRTLEXP ChangeFileExt( char *path,CONSTSTR Ext, char Slash = SLASH_CHAR );
HDECLSPEC pchar      MYRTLEXP ChangeFileName( char *path,CONSTSTR Name, char Slash = SLASH_CHAR );

//Each returns self static local buffer
HDECLSPEC CONSTSTR   MYRTLEXP FNameOnly( CONSTSTR nm, char Slash = SLASH_CHAR );
HDECLSPEC CONSTSTR   MYRTLEXP FName( CONSTSTR nm, char Slash = SLASH_CHAR );
HDECLSPEC CONSTSTR   MYRTLEXP FPath( CONSTSTR nm, char Slash = SLASH_CHAR );
HDECLSPEC CONSTSTR   MYRTLEXP FExtOnly( CONSTSTR nm, char Slash = SLASH_CHAR );

/*******************************************************************
   Checking
 *******************************************************************/
#if defined(__cplusplus)
HDECLSPEC BOOL       MYRTLEXP RootDir( const MyString& path );
#endif
HDECLSPEC BOOL       MYRTLEXP PossibleFChar( char ch );                            //Ret TRUE if possible use `ch` in filename
/*******************************************************************
   File Cmp
 *******************************************************************/
HDECLSPEC BOOL       MYRTLEXP InPattern( CONSTSTR pattern,CONSTSTR fname );                    //check `fname` in pattern (patterns may be multiply - delimited with ",")
#if defined(__cplusplus)
HDECLSPEC BOOL       MYRTLEXP IsSameFile( const MyString& f1,const MyString& f2 );
HDECLSPEC BOOL       MYRTLEXP PathCmp( const MyString& path,const MyString& path1 );
HDECLSPEC BOOL       MYRTLEXP BasePath( const MyString& base, const MyString& path );          //Rets `TRUE` if `base` is a base path of `path`
HDECLSPEC BOOL       MYRTLEXP CmpFile( const MyString& pattern,const MyString& fname );        //Cmp ONE pattern & fname
#endif
/*******************************************************************
   conversions
 *******************************************************************/
#if defined(__cplusplus)
HDECLSPEC MyString   MYRTLEXP MakeInName( const MyString& source,PFileEntry file );
HDECLSPEC MyString   MYRTLEXP MakeOutName( const MyString& Dest,PFileEntry file );
HDECLSPEC MyString   MYRTLEXP MakePatternStr( const MyString& fname,const MyString& astr );  //Expand string with replace
#endif                                                                                       //"!." like symbols w pats of file
/*******************************************************************
   System
 *******************************************************************/
HDECLSPEC BOOL       MYRTLEXP ExecuteString( CONSTSTR str );
HDECLSPEC BOOL       MYRTLEXP SafeExecuteString( CONSTSTR str,char *msg = NULL );
#if defined(__cplusplus)
HDECLSPEC BOOL       MYRTLEXP ExecuteCommandOut( CONSTSTR str,PCTStringArray arr,BOOL showOut = NULL );
#endif

#define INVALID_PID ((pid_t)-1)

HDECLSPEC pid_t      MYRTLEXP ExecuteCommandCB( CONSTSTR cmd,HANDLE TimeoutPeriod,HBufferProc_t CallBack,LPVOID Param );
/*******************************************************************
   Actions
 *******************************************************************/
HDECLSPEC BOOL       MYRTLEXP MakeDir( CONSTSTR path );
HDECLSPEC BOOL       MYRTLEXP DeleteDir( CONSTSTR path );
HDECLSPEC DWORD      MYRTLEXP FileLength( CONSTSTR fname );
HDECLSPEC BOOL       MYRTLEXP RenameFile( CONSTSTR oldN,CONSTSTR newN );   // File (can move)
HDECLSPEC BOOL       MYRTLEXP CheckExist( CONSTSTR path, int mode );          // Dir, File
#if !defined(__HWIN32__)
HDECLSPEC BOOL       MYRTLEXP DeleteFile( CONSTSTR path );                    // File
#endif
HDECLSPEC BOOL       MYRTLEXP GetFileTimes( CONSTSTR fname,time_t *cr/*=NULL*/,time_t *wr/*=NULL*/,time_t *ac/*=NULL*/ );
HDECLSPEC BOOL       MYRTLEXP SetFileTimes( CONSTSTR fname,time_t cr/*=0*/,time_t wr/*=0*/,time_t ac/*=0*/ );
/*******************************************************************
   Extended Actions
 *******************************************************************/
HDECLSPEC BOOL       MYRTLEXP GetFileAttr( CONSTSTR fname,ATTR_TYPE& attr );
HDECLSPEC BOOL       MYRTLEXP SetFileAttr( CONSTSTR fname,ATTR_TYPE attr );
HDECLSPEC BOOL       MYRTLEXP isReadOnly( CONSTSTR fname );
HDECLSPEC BOOL       MYRTLEXP isDirectory( CONSTSTR fname );
HDECLSPEC BOOL       MYRTLEXP ClrReadOnly( CONSTSTR fname );
HDECLSPEC BOOL       MYRTLEXP SetReadOnly( CONSTSTR fname );
HDECLSPEC void       MYRTLEXP SetCurRights( char *fname,int rwState );
HDECLSPEC BOOL       MYRTLEXP MakeLink( char *src, char *dest );
HDECLSPEC BOOL       MYRTLEXP UnLink( char *lnk );
#if defined(__cplusplus)
HDECLSPEC void       MYRTLEXP SetAttrAs( const MyString& dest,PFileEntry source );
#endif
/*******************************************************************
   HI-end service
 *******************************************************************/
#if defined(__cplusplus)
HDECLSPEC BOOL           MYRTLEXP FindFileText( const MyString& fname,BYTE Bytes[],DWORD byteLen,BOOL CaseSens );
HDECLSPEC void           MYRTLEXP QueryHotPaths( PHotPathArray ar );                        // DOS-disks, QNX-mashines, UNIX-??
HDECLSPEC PCTStringArray MYRTLEXP ReadTextFile( CONSTSTR path, PCTStringArray ar = NULL ); // If arr=NULL create new array
HDECLSPEC BOOL           MYRTLEXP WriteTextFile( CONSTSTR path, PCTStringArray ar );        // If arr=NULL create new array
#endif
HDECLSPEC pchar          MYRTLEXP MakeCfgName( char *name );                                //Rets local buffer !!

/*******************************************************************
   HI-end service
 *******************************************************************/
HDECLSPEC FileType   MYRTLEXP QueryFileType( int f );    //Returns type of stdio file handle
HDECLSPEC FileType   MYRTLEXP QueryFileType( FILE *f );

/*******************************************************************
   Overlapped IO
 *******************************************************************/
#if defined(__HWIN32__)
/* Transfers whole sz bytes.
   Uses overlapped if specified
*/
HDECLSPEC BOOL MYRTLEXP FIOWrite( HANDLE f, LPCVOID msg, DWORD sz, OVERLAPPED *os /*NULL*/, DWORD tmout /*INFINITE*/ );
HDECLSPEC BOOL MYRTLEXP FIORead( HANDLE f, LPVOID msg, DWORD sz, OVERLAPPED *os /*NULL*/, DWORD tmout /*INFINITE*/ );
#endif

#endif
