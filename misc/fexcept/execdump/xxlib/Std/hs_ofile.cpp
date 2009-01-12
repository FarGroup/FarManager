#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

PHIStream MYRTLEXP OpenStreamFile( CONSTSTR nm )
  {  DWORD dw;
     int   f = FIO_OPEN( nm,O_RDONLY );

     if ( f == -1 )
       return NULL;

//NON compressed
    if ( FIO_READ( f,&dw,sizeof(dw) ) != sizeof(dw) ) {
      FIO_CLOSE( f );
      return NULL;
    }
    FIO_CLOSE( f );

    if ( dw != LZH_FILEID ) {
      PHFileIStream fs = new HFileIStream;
      if ( !fs->Assign(nm) ) {
        delete fs;
        return NULL;
      }
      return fs;
    } else {

//Compressed
      PLZHFileIStream fs = new LZHFileIStream;
      if ( !fs->Assign(nm) ) {
        delete fs;
        return NULL;
      }
      return fs;
    }
}
