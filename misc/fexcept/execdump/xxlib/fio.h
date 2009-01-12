#ifndef __MY_FILE_IO
#define __MY_FILE_IO

//#define DEBUG_FIO 1

class HFile;
typedef class HFile *PHFile;

HDECLSPEC BOOL MYRTLEXP HCopyFile( HFile& dest, HFile& src, DWORD sz );
HDECLSPEC int  MYRTLEXP HWriteText( HFile& dest, CONSTSTR fmt, va_list a );

template <class F,class T> bool FileRead( F& f,T& b )        { return f.Read( &b,sizeof(T) ) == sizeof(T); }
template <class F,class T> bool FileWrite( F& f,const T& b ) { return f.Write( &b,sizeof(T) ) == sizeof(T); }
/****************************************************************
   HFile
  ***************************************************************/
class HFile {
  protected:
   int Mode;

#if defined(DEBUG_FIO)
    void SayPos( CONSTSTR from, sDWORD to ) { printf( "%-15s by %9d to %9d\n", from ,to, Tell() ); }
#else
    #define SayPos( from, to )
#endif
  public:
    virtual BOOL  Seek( sDWORD sz, int mode = SEEK_SET ) = 0;
    virtual DWORD Read( LPVOID b, DWORD sz )             = 0;
    virtual DWORD Write( LPCVOID b, DWORD sz )           = 0;
    virtual DWORD Tell( void )                           = 0;
    virtual BOOL  EoF( void )                            = 0;
    virtual void  Close( void )                          = 0;

    int   GetMode( void )                     { return Mode; }
    WORD  GetC( void )                        { BYTE ch; return FileRead(*this,ch) ? ch : MAX_WORD; }
    BOOL  GetS( char *buff, int sz );
    DWORD GetSize( void )                     { DWORD sz, pos = Tell(); Seek(0,SEEK_END); sz = Tell(); Seek(pos); return sz; }
    int   printf( CONSTSTR msg,... )          { int rc; va_list a; va_start( a,msg ); rc = HWriteText(*this,msg,a); va_end( a ); return rc; }
    int   vprintf( CONSTSTR msg,va_list a )   { return HWriteText(*this,msg,a); }
};

inline BOOL HFile::GetS( char *buff, int sz )
  {
     for( ; sz > 0; sz--,buff++ ) {
       WORD ch = GetC();
       if ( ch == MAX_WORD ) return false;

       *buff = (char)ch;
       if ( *buff == '\n' || *buff == '\r' ) break;
     }
     *buff = 0;
 return true;
}

#define FIOFile_DEF O_BINARY

/****************************************************************
   FIOFile
  ***************************************************************/
class FIOFile : public HFile {
   int  Handle;
   BOOL Existed;

  public:
    FIOFile( void )                       { Existed = FALSE;  Handle = -1; Mode = 0; }
    FIOFile( CONSTSTR fnm )               { Handle = -1; Open(fnm); }
    FIOFile( CONSTSTR fnm, int mode )     { Handle = -1; Open(fnm,mode); }
    FIOFile( int fh, int mode )           { Handle = -1; Open(fh,mode); }
    ~FIOFile()                            { Close(); }

    virtual BOOL  Seek( sDWORD sz, int mode = SEEK_SET ) { return FIO_SEEK(Handle,sz,mode) != -1; }
    virtual DWORD Tell( void )                          { return (DWORD)FIO_TELL(Handle); }
    virtual DWORD Read( LPVOID b, DWORD sz )            { return (DWORD)FIO_READ(Handle,b,sz); }
    virtual DWORD Write( LPCVOID b, DWORD sz )          { return (DWORD)FIO_WRITE(Handle,b,sz); }
    virtual BOOL  EoF( void )                           { return eof(Handle) != 0; }
    virtual void  Close( void )                         { if (Handle != -1) { Trunk(); if (!Existed) FIO_CLOSE(Handle); Handle = -1; } }

    int   Open( CONSTSTR fnm )            { Close(); Existed = FALSE; Handle = FIO_OPEN(fnm,Mode = FIOFile_DEF|O_RDONLY); return Handle != -1; }
    int   Open( CONSTSTR fnm, int mode )  { Close(); Existed = FALSE; Handle = FIO_OPEN(fnm,Mode = FIOFile_DEF|mode);     return Handle != -1; }
    int   Open( int fh, int mode )        { Close(); Existed = TRUE;  Mode = mode; Handle = fh;                           return Handle != -1; }
    int   Create( CONSTSTR fnm, int attr ){ Close(); Handle = FIO_CREAT(fnm,attr); Mode = O_RDWR;                         return Handle != -1; }

    BOOL  Trunk( void )                   { return FIO_TRUNC(Handle,Tell()); }
    BOOL  Trunk( DWORD sz )               { return FIO_TRUNC(Handle,sz); }

    int operator !()                      { return Handle == -1; }
    operator int()                        { return Handle; }
};

#if defined(__HWIN32__)
#define NOFILE INVALID_HANDLE_VALUE

/****************************************************************
   WINFile
  ***************************************************************/
class WINFile : public HFile {
   HANDLE Handle;
   BOOL   Self;

  private:
    HANDLE DoOpen( CONSTSTR fnm,int mode );
    HANDLE DoCreate( CONSTSTR fnm,DWORD mode );
    DWORD  MapSeek( int mode );
    DWORD  MapMode( int mode );

  public:
    WINFile( void )                        { Handle = NOFILE; Mode = 0; }
    WINFile( CONSTSTR fnm )                { Handle = NOFILE; Open(fnm); }
    WINFile( CONSTSTR fnm, int mode )      { Handle = NOFILE; Open(fnm,mode); }
    WINFile( HANDLE f )                    { Handle = NOFILE; Open(f); }
    ~WINFile()                             { Close(); }

    virtual BOOL  Seek( sDWORD sz, int mode = SEEK_SET ) { return SetFilePointer(Handle,sz,NULL,MapSeek(mode)) != MAX_DWORD; }
    virtual DWORD Tell( void )                           { return SetFilePointer(Handle,0,NULL,FILE_CURRENT); }
    virtual DWORD Read( LPVOID b, DWORD sz );
    virtual DWORD Write( LPCVOID b, DWORD sz );
    virtual BOOL  EoF( void );
    virtual void  Close( void )            { if (Handle != NOFILE) { Trunk(); if (Self) CloseHandle(Handle); Handle = NOFILE; } }

    int   Open( CONSTSTR fnm )             { Close(); Self = TRUE;  Handle = DoOpen(fnm,O_RDONLY);  return Handle != NOFILE; }
    int   Open( CONSTSTR fnm, int mode )   { Close(); Self = TRUE;  Handle = DoOpen(fnm,mode);      return Handle != NOFILE; }
    int   Open( HANDLE f )                 { Close(); Self = FALSE; Handle = f;                     return Handle != NOFILE; }
    int   Create( CONSTSTR fnm, DWORD attr){ Close(); Self = TRUE;  Handle = DoCreate(fnm,attr);    return Handle != NOFILE; }

    BOOL  Trunk( void )                    { return (BOOL)SetEndOfFile(Handle); }
    BOOL  Trunk( DWORD sz )                { Seek( (sDWORD)sz); return Trunk(); }

#if defined(__HAS_INT64__)
    BOOL    Seek64( __int64 sz )           { return Seek64(sz,SEEK_SET); }
    BOOL    Seek64( __int64 sz, int mode ) { LONG hi = (LONG)(sz >> 32); return SetFilePointer(Handle,(LONG)(sz & MAX_DWORD),&hi,MapSeek(mode)) != MAX_DWORD; }
    BOOL    Trunk64( __int64 sz )          { Seek64(sz); return Trunk(); }
    __int64 Tell64( void )                 { LONG hi, sz = SetFilePointer(Handle,0,&hi,FILE_CURRENT); return sz == ((LONG)MAX_DWORD) ? (-1) : ( (((__int64)hi)<< 32) | sz); }
#endif

    int operator !()                       { return Handle == NOFILE; }
    operator int()                         { return Handle != NOFILE; }
};

inline DWORD WINFile::Read( LPVOID b, DWORD sz )
  {  DWORD rd;
     if ( ReadFile(Handle,b,sz,&rd,NULL) )
       return rd;
      else
       return MAX_DWORD;
}

inline DWORD WINFile::Write( LPCVOID b, DWORD sz )
  { DWORD rd;

    if ( WriteFile(Handle,b,sz,&rd,NULL) )
      return rd;
     else
      return MAX_DWORD;
}

inline DWORD WINFile::MapSeek( int mode ) {
  if ( mode == SEEK_CUR ) return FILE_CURRENT; else
  if ( mode == SEEK_END ) return FILE_END; else
   return FILE_BEGIN;
}

inline DWORD  WINFile::MapMode( int mode )
  {  DWORD wm = 0;
     if ( IS_FLAG(mode,O_RDONLY) ) wm |= GENERIC_READ;
     if ( IS_FLAG(mode,O_WRONLY) ) wm |= GENERIC_WRITE;
 return wm;
}

inline HANDLE WINFile::DoOpen( CONSTSTR fnm,int mode )
  {
    Mode = mode;
 return CreateFile( fnm, MapMode(mode), FILE_SHARE_READ, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL );
}

inline HANDLE WINFile::DoCreate( CONSTSTR fnm,DWORD att )
  {
    Mode = O_RDWR;
 return CreateFile( fnm, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW,
                    att | FILE_FLAG_RANDOM_ACCESS, NULL );
}

inline BOOL WINFile::EoF( void )
  {  DWORD pos = Tell(),
           end = SetFilePointer(Handle,0,NULL,FILE_BEGIN);
 return pos >= end;
}

typedef class MAPFileRO *PMAPFileRO;
HDECLSPEC BOOL MYRTLEXP OpenMAPFileRO( PMAPFileRO mf, CONSTSTR fnm,int mode );

/****************************************************************
   MAPFileRO
  ***************************************************************/
class MAPFileRO : public HFile {
  public:
    HANDLE Handle,
           Map;
    LPBYTE View;
    DWORD  MaxSize,
           Position;

    friend BOOL MYRTLEXP OpenMAPFileRO( PMAPFileRO mf, CONSTSTR fnm,int mode );

  private:
    void DoClose( void );
    void Init( void ) { Handle = Map = NULL; View = NULL; MaxSize = Position = 0; Mode = O_RDONLY; }

  public:
    MAPFileRO( void )                        { Init(); }
    MAPFileRO( CONSTSTR fnm )                { Init(); Open(fnm); }
    MAPFileRO( CONSTSTR fnm, int mode )      { Init(); Open(fnm,mode); }
    ~MAPFileRO()                             { Close(); }

    virtual BOOL  Seek( sDWORD sz, int mode = SEEK_SET );
    virtual DWORD Tell( void )                          { return Position; }
    virtual DWORD Read( LPVOID b, DWORD sz );
    virtual DWORD Write( LPCVOID b, DWORD sz )          { return MAX_DWORD; }
    virtual BOOL  EoF( void )                           { return Position >= MaxSize; }
    virtual void  Close( void )                         { if (Handle != NOFILE) { DoClose(); Handle = NOFILE; } }

    int    Open( CONSTSTR fnm )            { return Open(fnm,O_RDONLY); }
    int    Open( CONSTSTR fnm, int mode );
    LPBYTE Mapped( void )                  { return View; }

    int operator !()                       { return Handle == NOFILE; }
    operator int()                         { return Handle != NOFILE; }
};

inline int MAPFileRO::Open( CONSTSTR fnm, int mode )
  {
    Close();
    return OpenMAPFileRO(this,fnm,mode);
}

inline void MAPFileRO::DoClose( void )
  {
    if (View)   UnmapViewOfFile( View ); View = NULL;
    if (Map)    CloseHandle( Map );      Map  = NULL;
    if (Handle) CloseHandle(Handle);     Handle = NOFILE;
}

inline BOOL MAPFileRO::Seek( sDWORD sz, int mode )
  {
    switch( mode ) {
      case SEEK_SET: if ( ((DWORD)sz) > MaxSize )
                       return false;
                     Position = sz;
                     SayPos( "SeekSET", sz );
                     return true;

      case SEEK_CUR: if ( Position+sz > MaxSize )
                       return false;
                     Position += sz;
                     SayPos( "SeekCUR", sz );
                     return true;

      case SEEK_END: sz = Abs(sz);
                     if ( ((DWORD)sz) > MaxSize )
                       return false;
                     Position = MaxSize - sz;
                     SayPos( "SeekEND", sz );
                     return true;
    }

 return false;
}

inline DWORD MAPFileRO::Read( LPVOID b, DWORD sz )
  {
     if ( Position+sz > MaxSize )
       return 0;

     CopyMemory( b, View+Position, sz );
     Position += sz;
     SayPos( "Read", sz );

 return sz;
}

#undef NOFILE
#endif // HWIN Mapped file R\O

#define FIO_THROW THROW

/****************************************************************
   HThrowFileFile
  ***************************************************************/
class HThrowFile {
    PHFile File;

  private:
    void ChFile( void ) { if (!File) FIO_THROW( "File is not assigned" ); }

  public:
    HThrowFile( void )     : File(NULL) {}
    HThrowFile( PHFile f ) : File(f)    {}
    HThrowFile( HFile& f ) : File(&f)   {}

    void Assign( PHFile f ) { File = f; }
    void Assign( HFile& f ) { File = &f; }

    void  Seek( sDWORD sz, int mode = SEEK_SET ) { ChFile(); if ( !File->Seek(sz,mode) ) Throw( "Error set file pointer to %d",sz ); }
    void  Read( LPVOID b, DWORD sz )             { ChFile(); if ( File->Read(b,sz) != sz ) Throw( "Error reading %d bytes", sz ); }
    void  Write( LPCVOID b, DWORD sz )           { ChFile(); if ( File->Write(b,sz) != sz ) Throw( "Error writing %d bytes", sz ); }
    DWORD Tell( void )                           { ChFile(); return File->Tell(); }
    BOOL  EoF( void )                            { ChFile(); return File->EoF(); }
    void  Close( void )                          { ChFile(); File->Close(); }
    int   GetMode( void )                        { ChFile(); return File->GetMode(); }
    BYTE  GetC( void )                           { ChFile(); WORD ch = File->GetC(); if (ch == MAX_WORD) Throw( "Error reading char" ); return (BYTE)ch; }
    void  GetS( char *buff, int sz )             { ChFile(); if ( !File->GetS(buff,sz) ) Throw( "Error reading string" ); }
    DWORD GetSize( void )                        { ChFile(); return File->GetSize(); }
    void  printf( CONSTSTR msg,... )             { va_list a; va_start( a,msg ); vprintf(msg,a); va_end( a ); }
    void  vprintf( CONSTSTR msg,va_list a )      { ChFile(); if ( *msg && !File->vprintf(msg,a) ) Throw( "Error writing formatted string" ); }

#if defined(HAS_INLINE_ENUM)
    template <class T> void TRead( T& b )        { Read( &b,sizeof(T) ); }
    template <class T> void TWrite( const T& b ) { Write( &b,sizeof(T) ); }
#endif

    void  Throw( CONSTSTR msg, ... ) {
      va_list  a;
      char     str[ 100 ];
      va_start( a,msg );
        VSNprintf( str, sizeof(str), msg, a );
      va_end( a );
      if ( File )
        FIO_THROW( Message( "%s at %d file offset", str, File->Tell() ) )
       else
        FIO_THROW( Message( "%s and file is not open", str ) )
    }
};

#endif
