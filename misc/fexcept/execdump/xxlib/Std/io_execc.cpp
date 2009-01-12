#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__GNUC__) || defined(__QNX__)
LOCALSTRUCT( ReadedData )
  BOOL           nl;
  MyString       str;
  PCTStringArray arr;
  BOOL           showOut;
};

BOOL RTL_CALLBACK idReaded( char *buff,int sz,PReadedData data )
  {
     for( ; sz--; buff++ )
       switch( *buff ){
         case '\r':
         case '\n': if ( data->nl ) break;
                    data->nl = TRUE;

                    if (data->arr)
                      data->arr->Add( new MyString(data->str) );

                    if ( data->showOut )
                      fprintf( stdout,"%s" EOL,data->str.Text() );

                    data->str = "";
                break;

           default: data->nl = FALSE;
                    data->str.Add( *buff );
       }

 return TRUE;
}
#endif

BOOL MYRTLEXP ExecuteCommandOut( CONSTSTR cmd,PCTStringArray arr,BOOL showOut )
  {
#if defined(__GNUC__) || defined(__QNX__)
    ReadedData data;
    data.nl      = FALSE;
    data.arr     = arr;
    data.showOut = showOut;

    if (arr)
      arr->DeleteAll();

    if ( ExecuteCommandCB(cmd,NULL,(HBufferProc_t)idReaded,&data) == INVALID_PID )
      return FALSE;

    if ( data.str.Length() > 0 && arr )
      arr->Add( new MyString(data.str) );

  return TRUE;
#else
#if defined(__HDOS__) || defined(__HWIN__)
    char fl[ MAX_PATH_SIZE ];
    char ex[ MAX_PATH_SIZE ];

    SNprintf( fl, sizeof(fl), "%s$exec$.tmp", GetTmpDir() );
    SNprintf( ex, sizeof(ex), "\"%s\" >\"%s\"", cmd, fl );

    if ( !SafeExecuteString( ex ) ) {
      DeleteFile( fl );
      return FALSE;
    }

    if ( arr )
      if ( ReadTextFile( fl,arr ) == NULL ) {
        DeleteFile( fl );
        return FALSE;
      }
    DeleteFile( fl );

    if ( arr && showOut )
      for ( int n = 0; n < arr->Count(); n++ )
        fprintf( stdout,"%s" EOL,arr->Item(n)->Text() );

  return TRUE;
#else
#error ERR_PLATFORM
#endif
#endif
}

#if defined(__GNUC__) || defined(__QNX__)
static void RTL_CALLBACK idNullExit( void )
  {
}

pid_t MYRTLEXP ExecuteCommandCB( CONSTSTR cmd,HANDLE TimeoutPeriod,HBufferProc_t CallBack,LPVOID Param )
  {  pid_t pid;
     int   fd[2];

    if( pipe(fd) == -1 ||
        (pid=fork()) == -1 )
      return INVALID_PID;

    if( pid == 0 ) {
      /* This is the child process.
       * Move read end of the pipe to stdin ( 0 ),
       * close any extraneous file descriptors,
       * then use exec to 'become' the command.
       */
      AtExit( idNullExit );
      close( fd[FIO_IO_READ] );
      dup2( fd[FIO_IO_WRITE], FIO_IO_WRITE );
      dup2( fd[FIO_IO_WRITE], FIO_IO_ERROR );
      BOOL rc = SafeExecuteString(cmd,"");
      close( fd[FIO_IO_WRITE] );
      exit( rc==FALSE );
    }

    close( fd[FIO_IO_WRITE] );

    char    buffer[80];
    int     inLen;
    int     status;
    timeval timeout;

    timeout.tv_sec  = 0;
    timeout.tv_usec = 10;

    do{
       fd_set  readfds;
       fd_set  excptfds;

       if ( TimeoutPeriod && PRPeriodEnd(TimeoutPeriod) ) {
         FIO_SETERRORN( WAIT_TIMEOUT );
         break;
       }

       if ( !status ) {
         int rc;
         status = waitpid( pid, &rc, WNOHANG ) == -1 &&
                  (WIFEXITED(rc) || WIFSIGNALED(rc));
       }

       FD_ZERO(&readfds);
       FD_ZERO(&excptfds);
       FD_SET(fd[FIO_IO_READ], &readfds);
       FD_SET(fd[FIO_IO_READ], &excptfds);

       inLen = select( fd[FIO_IO_READ]+1, &readfds, NULL, &excptfds, &timeout );

       if ( inLen == 0 )
         continue;

       if ( inLen == SOCKET_ERROR ||
            inLen != 0 && FD_ISSET(fd[FIO_IO_READ],&excptfds) ) {
         break;
       }

       inLen = read( fd[FIO_IO_READ],buffer,sizeof(buffer) );
       if ( inLen )
         if ( !CallBack(buffer,inLen,Param) ) {
           FIO_SETERRORN( ERROR_CANCELLED );
           break;
         }

       if ( !inLen && status )
         return 0;

    }while( 1 );

//Close pipe
    close( fd[FIO_IO_READ] );

//Check process ends
    if ( status )
      return 0;
     else
      return pid;
}
#else
#if defined(__HWIN32__)

pid_t MYRTLEXP ExecuteCommandCB( CONSTSTR cmd,HANDLE TimeoutPeriod/*NULL*/,HBufferProc_t cb,LPVOID Param )
  {  STARTUPINFO         si;
     PROCESS_INFORMATION pi;
     SECURITY_ATTRIBUTES sa;
     HANDLE              hRead, hWrite;
     BYTE                buff[ 1024 ];
     char                SaveDir[ MAX_PATH_SIZE ];
     pid_t               rc = INVALID_PID;
     DWORD               dw;

     if ( !cb ) {
       FIO_SETERRORN( ERROR_INVALID_PARAMETER );
       return INVALID_PID;
     }

     sa.nLength              = sizeof(sa);
     sa.lpSecurityDescriptor = NULL;
     sa.bInheritHandle       = TRUE;

     if ( !CreatePipe( &hRead, &hWrite, &sa, 0 ) )
       return FALSE;

     memset(&pi,0,sizeof(pi));
     memset(&si,0,sizeof(si));
     SaveDir[0] = 0;

     do{
       dw = ExpandEnvironmentStrings( "%COMSPEC% /c ", (LPTSTR)buff, sizeof(buff) );
       if ( dw >= sizeof(buff)-1 ) {
         FIO_SETERRORN( ERROR_TOOSMALL );
         break;
       }
       dw--;
       dw = ExpandEnvironmentStrings( cmd, (LPTSTR)(buff+dw), sizeof(buff)-dw ) + dw;
       if ( dw >= sizeof(buff)-1 ) {
         FIO_SETERRORN( ERROR_TOOSMALL );
         break;
       }

       GetCurrentDirectory( sizeof(SaveDir), SaveDir);

       si.cb          = sizeof(si);
       si.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
       si.wShowWindow = SW_HIDE;
       si.hStdInput   = GetStdHandle(STD_INPUT_HANDLE);
       si.hStdOutput  = hWrite;
       si.hStdError   = hWrite;

       if ( !CreateProcess(NULL,(LPTSTR)buff,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi) )
         break;

       rc = (pid_t)pi.dwProcessId;
       CloseHandle( hWrite );     hWrite     = NULL;
       CloseHandle( pi.hThread ); pi.hThread = NULL;

       do{
         if ( !PeekNamedPipe( hRead, NULL, 0, NULL, &dw, NULL ) ) {
           if ( FIO_ERRORN == ERROR_HANDLE_EOF ||
                FIO_ERRORN == ERROR_BROKEN_PIPE )
             break;
            else {
             rc = INVALID_PID;
             break;
            }
         }

         if ( !dw ) {
           if ( TimeoutPeriod && PRPeriodEnd(TimeoutPeriod) ) {
             FIO_SETERRORN( ERROR_TIMEOUT );
             break;
           }
           Sleep(1);
         } else {
           if ( ReadFile( hRead, buff, Min( (DWORD)sizeof(buff), dw ), &dw, NULL ) ) {
             if ( dw ) {
               buff[dw] = 0;
               if ( !cb( (char*)buff, dw, Param) ) {
                 FIO_SETERRORN( ERROR_CANCELLED );
                 break;
               }
             }
           } else
           if ( FIO_ERRORN == ERROR_HANDLE_EOF ||
                FIO_ERRORN == ERROR_BROKEN_PIPE )
             break;
            else {
             rc = INVALID_PID;
             break;
            }
         }
       }while(1);

     }while(0);

     DWORD err = FIO_ERRORN;
       if ( rc != INVALID_PID &&
            WaitForSingleObject(pi.hProcess,0) != WAIT_TIMEOUT )
         rc = 0;

       CloseHandle( hRead );
       CloseHandle( hWrite );
       CloseHandle( pi.hProcess );
       CloseHandle( pi.hThread );

       if ( SaveDir[0] ) SetCurrentDirectory( SaveDir );
     FIO_SETERRORN( err );

 return rc;
}

#else
#if defined(__HDOS__)

#else
#if defined(__HWIN16__)

#else
  #error ERR_PLATFORM
#endif //WIN16
#endif //DOS
#endif //WIN32
#endif //QNX
