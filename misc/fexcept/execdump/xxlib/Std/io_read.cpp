#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

PCTStringArray MYRTLEXP ReadTextFile( CONSTSTR path, PCTStringArray ar )
  {  char     buffer[200];
     int      file,i, inLen;
     MyString str;
     BOOL     nl = FALSE;

    file = open( path,O_RDONLY );
    if ( file == -1 ) return NULL;
    if ( !ar ) ar = new CTStringArray;

    do{
      inLen = read( file,buffer,sizeof(buffer) );
      if ( inLen > 0 )
        for ( i = 0; i < inLen; i++ )
          switch( buffer[i] ){
            case '\r':
            case '\n': if ( nl ) break;
                       nl = TRUE;
                       ar->Add( new MyString(str) );
                       str = "";
                   break;
              default: nl = FALSE; str.Add( buffer[i] );
          }
    }while( inLen > 0 );
    if ( str.Length() > 0 ) ar->Add( new MyString(str) );
    close( file );
 return ar;
}
