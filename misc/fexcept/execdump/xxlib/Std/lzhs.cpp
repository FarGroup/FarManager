#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if 1
  #define  PROC( v )
  #define  Log( v )
#else
  #define  PROC( v )  INProc __proc v ;
  #define  Log( v )   INProc::Say v
#endif

static DWORD GetSize( DWORD flags )
  {
    if ( IS_FLAG(flags,LZH_F_BLOCK128) ) return 128000; else
    if ( IS_FLAG(flags,LZH_F_BLOCK64) ) return 64000; else
    if ( IS_FLAG(flags,LZH_F_BLOCK32) ) return 32000; else
    if ( IS_FLAG(flags,LZH_F_BLOCK16) ) return 16000; else
      return 8000;
}
/****************************************************
   LZHOStream
 ****************************************************/
LZHOStream::LZHOStream( DWORD flags )
    : HOStream( GetSize(flags) )
  {
    Flags      = flags;
    WritenSize = 0;
    cBuff      = new BYTE[ rSize+1 ];
}

LZHOStream::~LZHOStream()
  {
    delete[] cBuff;
}

void LZHOStream::doReset( void )
  {
    HOStream::doReset();
    WritenSize = 0;
}

BOOL LZHOStream::WriteCompleted( void )
  {
     if ( !rCount ) return TRUE;

     if ( !WritenSize ) {
       LZHStreamHeader sh;
       sh.Id        = LZH_STREAMID;
       sh.BlockSize = rSize;

       if ( !WriteBuffer(&sh,sizeof(sh)) )
         return FALSE;

       WritenSize += sizeof(sh);
     }

     LZHStreamBlock fb;
     DWORD          rSz;

     rSz = LZHCompress( rBuff,rCount,cBuff,rSize );
     if ( !rSz )
       return FALSE;

     fb.Id                = LZH_SBLOCKID;
     fb.OriginalBlockSize = rCount;
     fb.BlockSize         = rSz;
     if ( IS_FLAG(Flags,LZH_F_CRC) ) {
       fb.BlockCRC          = Crc32(0,cBuff,rSz);
       fb.OriginalBlockCRC  = Crc32(0,rBuff,rCount);
     } else {
       fb.BlockCRC          = 0;
       fb.OriginalBlockCRC  = 0;
     }

     if ( !BlockReady( &fb,rBuff,cBuff ) ) {
       FIO_SETERRORN( ERROR_CANCELLED );
       return FALSE;
     }

#if 0
     BYTE  tmp[ LZH_STREAMBLOCK+500 ];
     DWORD tsz;
     int   ttmp;
     tsz = LZHDecompress( cBuff,rSz,tmp,sizeof(tmp)-1 );
     if ( tsz != (int)rCount ||
          (ttmp=memcmp( tmp,rBuff,Min(tsz,(int)rCount) )) != 0 )
       HAbort( "Block does not match with compressed %d!=%d Cmp=%d\n",
               tsz,rCount,ttmp );
#endif

     if ( !WriteBuffer(&fb,sizeof(fb)) ||
          !WriteBuffer(cBuff,fb.BlockSize) )
       return FALSE;

     WritenSize  += rCount + sizeof(fb);
     rCount       = 0;

 return TRUE;
}

DWORD LZHOStream::CompleteSize( void ) { return WritenSize; }
BOOL  LZHOStream::BlockReady( PLZHStreamBlock /*Block*/,LPBYTE /*OriginalBlock*/,LPBYTE /*PackedBlock*/ ) { return TRUE; }

/****************************************************
   LZHFileOStream
 ****************************************************/
LZHFileOStream::LZHFileOStream( DWORD flags )
    : LZHOStream(flags)
  {
    File = -1;
}

LZHFileOStream::LZHFileOStream( CONSTSTR fnm,DWORD flags )
    : LZHOStream(flags)
  {
    File = -1;
    Assign( fnm );
}

LZHFileOStream::LZHFileOStream( int fnm,DWORD flags )
    : LZHOStream(flags)
  {
    File = -1;
    Assign( fnm );
}

LZHFileOStream::~LZHFileOStream()
  {
    Close();
}

void LZHFileOStream::Close( void )
  {
    if ( File != -1 ) {
      Flush();
      FIO_TRUNC( File,FIO_TELL(File) );
      FIO_CLOSE( File );
    }
    Reset();
}

BOOL LZHFileOStream::Assign( CONSTSTR FileName )
  {
    Close();

    File = FIO_OPEN( FileName,O_WRONLY|O_BINARY );
    if ( File == -1 ) {
      File = FIO_CREAT( FileName,FIO_DEF_ATTR );
      if ( File == -1 )
        return FALSE;
    }

    DWORD dw = LZH_FILEID;

 return FIO_WRITE( File,&dw,sizeof(dw) ) == sizeof(dw);
}

BOOL LZHFileOStream::Assign( int nFile )
  {
    Close();

    File = nFile;
    if ( File == -1 )
      return FALSE;

    DWORD dw = LZH_FILEID;

 return FIO_WRITE( File,&dw,sizeof(dw) ) == sizeof(dw);
}

BOOL LZHFileOStream::WriteBuffer( LPVOID buff,int sz )
  {
  return File != -1 &&
         FIO_WRITE( File,buff,sz) == sz;
}

BOOL LZHFileOStream::doAssigned( void ) { return File != -1; }

/****************************************************
   LZHIStream
 ****************************************************/
LZHIStream::LZHIStream( DWORD flags )
    : HIStream( GetSize(flags) )
  {
    ReadedSize = 0;
    Flags      = flags;
    cBuff      = new BYTE[ rSize+1 ];
}

LZHIStream::~LZHIStream()
  {
    delete[] cBuff;
}

void LZHIStream::doReset( void )
  {
    HIStream::doReset();
    ReadedSize = 0;
}

BOOL LZHIStream::ReadCompleted( void )
  {
     if ( !ReadedSize ) {
       LZHStreamHeader sh;

       if ( !ReadBuffer(&sh,sizeof(sh)) )
         return FALSE;

       if ( sh.Id != LZH_STREAMID ) {
         FIO_SETERRORN( ERROR_HEADER );
         return FALSE;
       }
       if ( rSize < sh.BlockSize ) {
         delete[] rBuff;
         delete[] cBuff;
         rBuff  = new BYTE[ sh.BlockSize+1 ];
         cBuff  = new BYTE[ sh.BlockSize+500 ];
         rSize  = sh.BlockSize;
       }

       ReadedSize += sizeof(sh);
     }

     LZHStreamBlock fb;

     if ( !ReadBuffer(&fb,sizeof(fb)) )
       return FALSE;

     if ( fb.Id != LZH_SBLOCKID ) {
       FIO_SETERRORN( ERROR_HEADER );
       return FALSE;
     }
     if ( fb.BlockSize >= rSize ) {
       FIO_SETERRORN( ERROR_RANGE );
       return FALSE;
     }

     if ( !ReadBuffer(cBuff,fb.BlockSize) )
       return FALSE;

     rCount      = LZHDecompress( cBuff,fb.BlockSize,rBuff,rSize );
     ReadedSize += rCount + sizeof(fb);

     if ( IS_FLAG(Flags,LZH_F_CRC) )
       if ( fb.BlockCRC         && fb.BlockCRC         != Crc32(0,cBuff,fb.BlockSize) ||
            fb.OriginalBlockCRC && fb.OriginalBlockCRC != Crc32(0,rBuff,rCount) ) {
         FIO_SETERRORN( ERROR_CRC );
         return FALSE;
       }

     if ( !BlockReady( &fb,rBuff,cBuff ) ) {
       return FALSE;
     }

 return rCount == fb.OriginalBlockSize;
}

DWORD LZHIStream::CompleteSize( void ) { return ReadedSize; }
BOOL  LZHIStream::BlockReady( PLZHStreamBlock /*Block*/,LPBYTE /*OriginalBlock*/,LPBYTE /*PackedBlock*/ ) { return TRUE; }

/****************************************************
   LZHFileIStream
 ****************************************************/
LZHFileIStream::LZHFileIStream( DWORD flags )
    : LZHIStream(flags)
  {
    File = -1;
}

LZHFileIStream::LZHFileIStream( CONSTSTR fnm,DWORD flags )
    : LZHIStream(flags)
  {
    File = -1;
    Assign( fnm );
}

LZHFileIStream::LZHFileIStream( int fnm,DWORD flags )
    : LZHIStream(flags)
  {
    File = -1;
    Assign( fnm );
}

LZHFileIStream::~LZHFileIStream()
  {
    Close();
}

void LZHFileIStream::Close( void )
  {
    if ( File != -1 ) {
      FIO_CLOSE( File );
      File = -1;
    }
    Reset();
}

BOOL LZHFileIStream::Assign( CONSTSTR FileName )
  {
    Close();

    File = FIO_OPEN( FileName,O_RDONLY|O_BINARY );
    if ( File == -1 )
      return FALSE;

    DWORD dw;
    if ( FIO_READ( File,&dw,sizeof(dw) ) != sizeof(dw) )
      return FALSE;

    if ( dw != LZH_FILEID ) {
      FIO_SETERRORN( ERROR_MAGIC );
      return FALSE;
    }

 return TRUE;
}

BOOL LZHFileIStream::Assign( int nFile )
  {
    Close();

    File = nFile;
    if ( File == -1 )
      return FALSE;

    DWORD dw;
    if ( FIO_READ( File,&dw,sizeof(dw) ) != sizeof(dw) )
      return FALSE;

    if ( dw != LZH_FILEID ) {
      FIO_SETERRORN( ERROR_MAGIC );
      return FALSE;
    }

 return TRUE;
}

BOOL LZHFileIStream::ReadBuffer( LPVOID buff,int sz )
  {
     if ( File != -1 && FIO_READ(File,buff,sz) == sz ) {
       return TRUE;
     } else {
       return FALSE;
     }
}

BOOL LZHFileIStream::FEOF( void )
  {
 return ReadySize() == 0 &&
        (File == -1 || FIO_EOF(File));
}

BOOL LZHFileIStream::doAssigned( void ) { return File != -1; }

#if defined(__VCL__)
void MYRTLEXP HSaveStream( TStream *ControlStream, CONSTSTR fn, bool CompressContents )
  {  HOStream *out;

     if ( CompressContents )
       out = new LZHFileOStream( fn );
      else
       out = new HFileOStream( fn );

     __try{
       BYTE buff[ 512 ];
       DWORD sz;

       ControlStream->Position = 0;

       while( 1 ) {
         sz = Min( (size_t)(ControlStream->Size - ControlStream->Position), sizeof(buff) );
         if ( !sz )
           break;

         ControlStream->Read( buff, sz );
         if ( !out->Write( buff, sz ) )
           throw Exception( FIO_ERROR );
       }
     }__finally{
       delete out;
       ControlStream->Position = 0;
     }
}

void MYRTLEXP HLoadStream( TStream *ControlStream, CONSTSTR fn )
  {  HAutoPtr<HIStream> in( OpenStreamFile(fn) );

     if ( !in.Ptr() )
       return;

     BYTE  buff[ 512 ];
     DWORD sz,
           fsz;

     ControlStream->Position = 0;
     for( fsz = 0; (sz=in->Read(buff,sizeof(buff))) > 0; fsz += sz )
       ControlStream->Write( buff, sz );

     ControlStream->Size     = (int)fsz;
     ControlStream->Position = 0;
}
#endif
