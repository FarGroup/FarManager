#ifndef __INC_LZHCOMPRESS_H
#define __INC_LZHCOMPRESS_H

/** @ingroup Stream LZH compressed stream
    @{

    Classes defined:
     - LZHOStream
     - LZHFileOStream
     - LZHIStream
     - LZHFileIStream
*/
#define LZH_FILEID       MK_ID( 'L','Z','f','1' )
#define LZH_STREAMID     MK_ID( 'L','Z','s','1' )
#define LZH_SBLOCKID     MK_ID( 'B','L','s','r' )

enum _LZH_FileOptions {
 LZH_F_BLOCK128   = 0x00000001,
 LZH_F_BLOCK64    = 0x00000002,
 LZH_F_BLOCK32    = 0x00000004,
 LZH_F_BLOCK16    = 0x00000008,
 LZH_F_BLOCK8     = 0x00000010,
 LZH_F_CRC        = 0x00000020,

 LZH_F_DEFAULT    = LZH_F_BLOCK32
};

#include <Global/pack1.h>
STRUCT( LZHStreamHeader )
  DWORD Id;                 //LZH_STREAMID
  DWORD BlockSize;          //Size of block used to pack the file (in bytes)
};

STRUCT( LZHStreamBlock )
  DWORD Id;                 //LZH_SBLOCKID
  DWORD BlockSize;
  DWORD OriginalBlockSize;
  DWORD BlockCRC;
  DWORD OriginalBlockCRC;
};

STRUCT( LZHFileHeader )
  DWORD  OriginalFileSize;
  time_t OriginalFileTime_Create;
  time_t OriginalFileTime_Write;
  time_t OriginalFileTime_Access;
  BYTE   OriginalFileNameSize;
  char   OriginalFileName[1];
};
STRUCTBASE( LZHFileHeaderName, public LZHFileHeader )
  char   _name_place[ MAX_PATH_SIZE ];
};
#include <Global/pop.h>

/****************************************************
   LZHOStream
   LZHIStream
 ****************************************************/
CLASSBASE( LZHOStream, public HOStream )
    DWORD  Flags;
    LPBYTE cBuff;
    DWORD  WritenSize;

  protected:
    virtual BOOL WriteCompleted( void );
    virtual void doReset( void );
    //Self
    virtual BOOL WriteBuffer( LPVOID buff,int sz ) = 0;
    virtual BOOL BlockReady( PLZHStreamBlock Block,LPBYTE OriginalBlock,LPBYTE PackedBlock );

  public:
    LZHOStream( DWORD flags = LZH_F_DEFAULT );
    ~LZHOStream();

    DWORD  CompleteSize( void );
};

CLASSBASE( LZHIStream, public HIStream )
    DWORD  Flags;
    LPBYTE cBuff;
    DWORD  ReadedSize;

  protected:
    virtual BOOL ReadCompleted( void );
    virtual void doReset( void );
    //Self
    virtual BOOL ReadBuffer( LPVOID buff,int sz ) = 0;
    virtual BOOL BlockReady( PLZHStreamBlock Block,LPBYTE OriginalBlock,LPBYTE PackedBlock );

  public:
    LZHIStream( DWORD flags = LZH_F_DEFAULT );
    ~LZHIStream();

    DWORD  CompleteSize( void );
};

/****************************************************
   LZHFileOStream
   LZHFileIStream
 ****************************************************/
CLASSBASE( LZHFileOStream, public LZHOStream )
  public:
    int File;

  protected:
    virtual BOOL WriteBuffer( LPVOID buff,int sz );
    virtual BOOL doAssigned( void );

  public:
    LZHFileOStream( DWORD flags = LZH_F_DEFAULT );
    LZHFileOStream( CONSTSTR fnm,DWORD flags = LZH_F_DEFAULT );
    LZHFileOStream( int fn,DWORD flags = LZH_F_DEFAULT );
    ~LZHFileOStream();

    BOOL Assign( CONSTSTR FileName );
    BOOL Assign( int nFile );           //At least O_WRONLY mode
    void Close( void );
};

CLASSBASE( LZHFileIStream, public LZHIStream )
  public:
    int File;

  protected:
    virtual BOOL ReadBuffer( LPVOID buff,int sz );
    virtual BOOL doAssigned( void );

  public:
    LZHFileIStream( DWORD flags = LZH_F_DEFAULT );
    LZHFileIStream( CONSTSTR fnm,DWORD flags = LZH_F_DEFAULT );
    LZHFileIStream( int fn,DWORD flags = LZH_F_DEFAULT );
    ~LZHFileIStream();

    BOOL Assign( CONSTSTR FileName );
    BOOL Assign( int nFile );           //At least O_RDONLY mode
    BOOL FEOF( void );
    void Close( void );
};

/****************************************************
   Procedures
 ****************************************************/
/*  LZHCompress/LZHDecompress

    Process memory block.
    Rets number of bytes in result, 0 on error.
*/
HDECLSPEC DWORD          MYRTLEXP LZHCompress( void *pInBuffer, int nInBufLen, void *pOutBuf, int nOutBufLen );
HDECLSPEC DWORD          MYRTLEXP LZHDecompress( void *pInBuffer, int nInBufLen, void *pOutBuf, int nOutBufLen );

/*  LZHFileReadHeader

    Reads header of compressed file.
    Rets FALSE on error.
*/
HDECLSPEC BOOL           MYRTLEXP LZHFileReadHeader( CONSTSTR szInFile, PLZHFileHeaderName /*OUT*/fh );

/*  LZHFileCompress/LZHFileDecompress

    Process files.
    Each compressed file contains data of single file.
    Rets FALSE on any error.
*/
typedef BOOL (RTL_CALLBACK *FileBlockReadyCB)( PHStream Stream,PLZHStreamBlock Block,LPBYTE OriginalBlock,LPBYTE PackedBlock );

HDECLSPEC BOOL           MYRTLEXP LZHFileCompress( CONSTSTR szInFile, CONSTSTR szOutFile,
                                                   DWORD flags = LZH_F_DEFAULT,FileBlockReadyCB cb = NULL );
HDECLSPEC BOOL           MYRTLEXP LZHFileDecompress( CONSTSTR szInFile, CONSTSTR szOutFile,
                                                     DWORD flags = LZH_F_DEFAULT,FileBlockReadyCB cb = NULL );

/** @brief  Detects and create input stream for a file.
    @param  FileName pathname of file to open.
    @return handle of input stream.
    @retval NULL Error opening file or file is not a valid packed stream.

    [hs_ofile.cpp]
    Function determine type of file (plain or compressed) and assigns to it LZH of direct file input stream.
    You must manually delete handle it return.
*/
HDECLSPEC PHIStream MYRTLEXP OpenStreamFile( CONSTSTR FileName );

#if defined(__VCL__)
HDECLSPEC void MYRTLEXP HSaveStream( TStream *in, CONSTSTR fn, bool CompressContents );  //throws exceptions!
HDECLSPEC void MYRTLEXP HLoadStream( TStream *out, CONSTSTR fn );                        //throws exceptions!
#endif

/**@}*/
#endif
