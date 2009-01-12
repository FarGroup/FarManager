#ifndef __MY_HSTREAM
#define __MY_HSTREAM

/** @defgroup Stream Stream IO
    @{

    Classes defined:
      - HStream
      - HOStream      Block stream IO
      - HFileOStream
      - HIStream
      - HFileIStream
 ****************************************************/
CLASS( HStream )
};

CLASSBASE( HOStream, public HStream )
    HDataPtr BuffData;
  protected:
    LPBYTE rBuff;
    DWORD  rSize,rCount;

  protected:
    virtual BOOL WriteCompleted( void ) = 0; //Write `rCount` from `rBuff`. Ignore rCount==0.
    virtual void doReset( void );
    virtual BOOL doAssigned( void );

  protected:
    HOStream( DWORD BufferSize );

  public:
    virtual ~HOStream();

    BOOL   Write( LPVOID buff,DWORD sz );
    BOOL   Write( CONSTSTR Format,... );
    BOOL   WriteV( CONSTSTR Format,va_list al );
    DWORD  ReadySize( void );
    void   Flush( void );
    void   Reset( void );
    BOOL   Assigned( void );
};

CLASSBASE( HIStream, public HStream )
  protected:
    LPBYTE rBuff;
    LPBYTE rTail;
    DWORD  rSize,rCount;

  protected:
    virtual BOOL ReadCompleted( void ) = 0;   //Read up to `rSize` to rBuff;
                                              //Ignore rSize=0;
                                              //Set rCount to number of readed
    virtual void doReset( void );
    virtual BOOL doAssigned( void );

  protected:
    HIStream( DWORD bsz );

  public:
    virtual ~HIStream();

    DWORD  Read( LPVOID buff,DWORD sz );
    DWORD  ReadySize( void );
    void   Reset( void );
    BOOL   Assigned( void );
};
/****************************************************
   [hs_file.cpp]

   HFileOStream
   HFileIStream
 ****************************************************/
#define DEF_FILEIO_BSIZE 32000

CLASSBASE( HFileOStream, public HOStream )
  public:
    int File;

  protected:
    virtual BOOL WriteCompleted( void );
    virtual void doReset( void );
    virtual BOOL doAssigned( void );

  public:
    HFileOStream( DWORD bsz = DEF_FILEIO_BSIZE );
    HFileOStream( CONSTSTR fnm,DWORD bsz = DEF_FILEIO_BSIZE );
    HFileOStream( int fn,DWORD bsz = DEF_FILEIO_BSIZE );
    ~HFileOStream();

    BOOL Assign( CONSTSTR FileName );
    BOOL Assign( int nFile );             // >= O_WRONLY mode
    void Close( void );
};

CLASSBASE( HFileIStream, public HIStream )
  public:
    int File;

  protected:
    virtual BOOL ReadCompleted( void );
    virtual void doReset( void );
    virtual BOOL doAssigned( void );

  public:
    HFileIStream( DWORD bsz = DEF_FILEIO_BSIZE );
    HFileIStream( CONSTSTR fnm,DWORD bsz = DEF_FILEIO_BSIZE );
    HFileIStream( int fn,DWORD bsz = DEF_FILEIO_BSIZE );
    ~HFileIStream();

    BOOL Assign( CONSTSTR FileName );
    BOOL Assign( int nFile );             // >= O_RDONLY mode
    void Close( void );
    BOOL FEOF( void );
};

/**@}*/
#endif
