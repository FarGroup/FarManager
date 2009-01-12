#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

class iFile : public LZHFileIStream {
    FileBlockReadyCB CB;
  protected:
    virtual BOOL BlockReady( PLZHStreamBlock Block,LPBYTE OriginalBlock,LPBYTE PackedBlock ) {
      return CB ? CB(this,Block,OriginalBlock,PackedBlock) : TRUE; }
  public:
    iFile( DWORD flags,FileBlockReadyCB cb ) : LZHFileIStream(flags) { CB = cb; }
};

class oFile : public LZHFileOStream {
    FileBlockReadyCB CB;
  protected:
    virtual BOOL BlockReady( PLZHStreamBlock Block,LPBYTE OriginalBlock,LPBYTE PackedBlock ) {
      return CB ? CB(this,Block,OriginalBlock,PackedBlock) : TRUE; }
  public:
    oFile( DWORD flags,FileBlockReadyCB cb ) : LZHFileOStream(flags) { CB = cb; }
};


/****************************************************
   LZHFileReadHeader
 ****************************************************/
BOOL MYRTLEXP LZHFileReadHeader( CONSTSTR szInFile, PLZHFileHeaderName fh )
  {  iFile s( 0,NULL );

     if ( !s.Assign(szInFile) )
       return FALSE;

     if ( FIO_READ( s.File,fh,sizeof(LZHFileHeader) ) != sizeof(LZHFileHeader) )
       return FALSE;

     if ( ((DWORD)fh->OriginalFileNameSize) > MAX_PATH_SIZE ) {
       FIO_SETERRORN( ERROR_MORE_DATA );
       return FALSE;
     }

     if ( FIO_READ(s.File,&fh->OriginalFileName[1],fh->OriginalFileNameSize) != fh->OriginalFileNameSize )
       return FALSE;

     fh->OriginalFileName[1+fh->OriginalFileNameSize] = 0;

 return TRUE;
}

/****************************************************
   LZHFileCompress
 ****************************************************/
BOOL MYRTLEXP LZHFileCompress( CONSTSTR szInFile, CONSTSTR szOutFile,DWORD flags,FileBlockReadyCB cb )
  {  LZHFileHeaderName  fh;
     oFile              os( flags,cb );
     BYTE               buff[ 1001 ];
     int                File,
                        sz;

     if ( !GetFileTimes( szInFile,
                         &fh.OriginalFileTime_Create,
                         &fh.OriginalFileTime_Write,
                         &fh.OriginalFileTime_Access ) ||
          (fh.OriginalFileSize=FileLength(szInFile)) == MAX_DWORD )
       return FALSE;

     if ( !os.Assign(szOutFile) )
       return FALSE;

     fh.OriginalFileNameSize = (BYTE)( strlen( FName(szInFile) )-1 );
     StrCpy( fh.OriginalFileName, szInFile, MAX_PATH_SIZE );

     sz = sizeof(LZHFileHeader) + fh.OriginalFileNameSize;
     if ( FIO_WRITE( os.File,&fh,sz ) != sz )
       return FALSE;

     File = FIO_OPEN( szInFile,O_RDONLY|O_BINARY );
     if ( File == -1 )
       return FALSE;

     BOOL rc = TRUE;
     while( (sz=FIO_READ(File,buff,sizeof(buff)-1)) > 0 )
       if ( !os.Write(buff,sz) ) {
         rc = FALSE;
         break;
       }
     FIO_CLOSE( File );

 return rc;
}

BOOL MYRTLEXP LZHFileDecompress( CONSTSTR szInFile, CONSTSTR szOutFile,DWORD flags,FileBlockReadyCB cb )
  {  LZHFileHeaderName fh;
     iFile             s( flags,cb );

     if ( !s.Assign(szInFile) )
       return FALSE;

     if ( FIO_READ( s.File,&fh,sizeof(LZHFileHeader) ) != sizeof(LZHFileHeader) )
       return FALSE;

     if ( ((DWORD)fh.OriginalFileNameSize) > MAX_PATH_SIZE ) {
       FIO_SETERRORN( ERROR_MORE_DATA );
       return FALSE;
     }

     if ( FIO_READ(s.File,&fh.OriginalFileName[1],fh.OriginalFileNameSize) != fh.OriginalFileNameSize )
       return FALSE;
     fh.OriginalFileName[1+fh.OriginalFileNameSize] = 0;

     if (!szOutFile)
       szOutFile = fh.OriginalFileName;

     BYTE buff[ 1001 ];
     int  File = FIO_OPEN( szOutFile,O_WRONLY|O_BINARY ),
          sz;

     if ( File == -1 ) {
       File = FIO_CREAT( szOutFile,FIO_DEF_ATTR );
       if ( File == -1 )
         return FALSE;
     }

     BOOL rc = TRUE;
     while( !s.FEOF() )
       if ( (sz=s.Read( buff,sizeof(buff)-1 )) <= 0 ||
            FIO_WRITE(File,buff,sz) != sz ) {
         rc = FALSE;
         break;
       }
      FIO_TRUNC( File,FIO_TELL(File) );
      FIO_CLOSE( File );

 return rc;
}
