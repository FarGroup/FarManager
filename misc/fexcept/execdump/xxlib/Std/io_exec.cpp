#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__REALDOS__)
  //[sc_sys.c]
  extern "C" int SC_System( CONSTSTR cmdline );
#endif

BOOL MYRTLEXP SafeExecuteString( CONSTSTR str,char *msg )
  {  BOOL res;

   if (msg) fprintf(stdout,msg);
    res = ExecuteString( str );
   if (msg) fprintf( stdout,EOL );
   fflush( stdout ); fflush( stderr ); fflush( stdin );
 return res;
}

BOOL MYRTLEXP ExecuteString( CONSTSTR str )
  {
#if defined(__REALDOS__)
  return SC_System( str ) != -1;
#else
#if defined(__HWIN32__)
     STARTUPINFO si = { sizeof(STARTUPINFO), NULL, NULL, NULL,
                        0,0,0,0,0,0,0,
                        STARTF_USESTDHANDLES,  //Flags
                        0,0,NULL,
                        GetStdHandle(STD_INPUT_HANDLE),
                        GetStdHandle(STD_OUTPUT_HANDLE),
                        GetStdHandle(STD_OUTPUT_HANDLE)
                      };
     PROCESS_INFORMATION pi;
     char   *m = getenv("COMSPEC");
     char    s[ MAX_CMD_SIZE+1 ] = "";

     if ( m )
       strcpy( s,m );
      else
       strcpy( s,"command.com" );
     strcat( s," /c " );
     strncat( s,str,MAX_CMD_SIZE ); s[ MAX_CMD_SIZE ] = 0;

     if ( !CreateProcess( NULL,
                          s,
                          NULL,NULL,
                          TRUE,
                          0,
                          NULL,NULL,
                          &si,&pi ) ) return FALSE;
     WaitForSingleObject( pi.hProcess,INFINITE );
 return TRUE;
#else
 return system( str ) >= 0;
#endif  //WIN32
#endif  //REALDOS
}
