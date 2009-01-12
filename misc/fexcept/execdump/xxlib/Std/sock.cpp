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

#if defined(__QNX__) || defined(__HWIN__)
//! Sockets implemented only for QNX and Windows

#if defined(__HWIN__)
static AbortProc aProc;
static void RTL_CALLBACK __scDestroy( void )
  {
    WSACleanup();
    if (aProc) aProc();
}
#endif

static BOOL MYRTLEXP scInit( void )
  {  static BOOL inited = FALSE;
     if (inited) return TRUE;
     inited = TRUE;

#if defined(__HWIN__)
     aProc = AtExit( __scDestroy );

     WORD    wVersionRequested;
     WSADATA wsaData;
     int     err;
     wVersionRequested = MAKEWORD( 1, 0 );
     err = WSAStartup( wVersionRequested, &wsaData );
     if ( err != 0 ) {
       FIO_SETERROR( "Error start WSA: Can not find version required" );
       return FALSE;
     }

     if ( LOBYTE( wsaData.wVersion ) != 1 || HIBYTE( wsaData.wVersion ) != 0 ) {
       WSACleanup();
       FIO_SETERROR( "Error init WSA: Reports unsupported version: %d.%d",
                     LOBYTE( wsaData.wVersion ), HIBYTE( wsaData.wVersion ) );
       return FALSE;
     }
#endif
 return TRUE;
}

SOCKET MYRTLEXP scCreate( void )
  {  SOCKET s;
     DWORD  err;
     int    on = 1;

    if ( !scInit() )
      return INVALID_SOCKET;

    s = socket(AF_INET,SOCK_STREAM,0);
    if ( !scValid(s) )
      return INVALID_SOCKET;

    do{
#if defined(__HWIN__)
      if ( setsockopt( s,SOL_SOCKET,SO_DONTLINGER,(SOCK_DATATYPE)&on,sizeof(on) ) != 0 ) break;
#endif
      if ( setsockopt( s,SOL_SOCKET,SO_DONTROUTE,(SOCK_DATATYPE)&on,sizeof(on) ) != 0 ) break;

      if ( setsockopt( s,IPPROTO_TCP,TCP_NODELAY,(SOCK_DATATYPE)&on,sizeof(on) ) != 0 ) break;

#ifdef SO_OOBINLINE
      if ( setsockopt(s, SOL_SOCKET, SO_OOBINLINE, (SOCK_DATATYPE)&on, sizeof(on)) != 0 ) break;
#endif
      //ioctl_val ul = 1;
      //if ( ioctlsocket( s,FIONBIO,&ul ) != 0 ) break;

      return s;
    }while(0);

    err = scError();
     scClose(s);
    scSetError(err);

 return INVALID_SOCKET;
}

BOOL MYRTLEXP scClose( SOCKET sock )
  {  BOOL  res;
     DWORD err = scError();

     shutdown(sock,2);
#if defined(__QNX__)
     res = close(sock) == 0;
#endif

#if defined(__HWIN__)
     res = closesocket(sock) == 0;
#endif

/*-*/Log(("scClose %d",sock ));
     scSetError(err);
 return res;
}

DWORD MYRTLEXP scError( void )
  {
#if defined(__QNX__)
 return (DWORD)errno;
#endif

#if defined(__HWIN16__)
 return (DWORD)errno;
#endif

#if defined(__HWIN32__)
 return GetLastError();
#endif
}

void MYRTLEXP scSetError( DWORD err )
  {
#if defined(__QNX__)
 errno = (int)err;
#endif

#if defined(__HWIN16__)
 errno = (int)err;
#endif

#if defined(__HWIN32__)
 SetLastError( err );
#endif
}

BOOL MYRTLEXP scValid( SOCKET sock )
  {
#if defined(__QNX__)
  return sock >= 0;
#endif

#if defined(__HWIN__)
  return sock != INVALID_SOCKET && sock != ((SOCKET)SOCKET_ERROR);
#endif
}

SOCKET MYRTLEXP scServerConnect( CONSTSTR name )
  {  SOCKET       sock;
     sockaddr_in  server;
     hostent     *he;
     const char  *m;
     u_short      port;
     char         str[256];
     size_t       n;

     if ( !scInit() )
       return INVALID_SOCKET;

     if ( !name ||
          (m=StrChr(name,':')) == NULL ||
          (port=(u_short)AtoI( m+1 )) == 0 )
       return INVALID_SOCKET;

     n = Min( sizeof(str)-1, size_t(m-name) );
     strncpy( str, name, n );
     str[n] = 0;

     if ( (he=gethostbyname(str)) == NULL )
       return INVALID_SOCKET;

     sock = scCreate();
     if ( !scValid(sock) )
       return INVALID_SOCKET;

     MemMove( &server.sin_addr,he->h_addr,he->h_length );
     server.sin_family      = AF_INET;
     server.sin_port        = htons(port);
     if ( connect(sock,(struct sockaddr *)&server, sizeof(server)) != 0 ) {
       scClose(sock);
       return INVALID_SOCKET;
     } else
       return sock;
}

SOCKET MYRTLEXP scServerSetup( CONSTSTR name )
  {  SOCKET srv;
     struct sockaddr_in addr;
     CONSTSTR m;

     if ( !scInit() )
       return INVALID_SOCKET;

     if ( !name || (m=StrChr( name,':' )) == NULL ) {
       SetError( "Invalid server name parameter" );
       return INVALID_SOCKET;
     }

     srv = socket(AF_INET,SOCK_STREAM,0);
     if ( !scValid(srv) )
       return INVALID_SOCKET;

     do{
       { //u_long on = 1;
         //ioctlsocket( srv, FIONBIO, &on );
       }
       { int on = 1;
         if (setsockopt( srv, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on) ) < 0)
           break;
       }

      addr.sin_family      = AF_INET;
      addr.sin_addr.s_addr = INADDR_ANY;
      addr.sin_port        = htons( (u_short)AtoI( m+1 ) );
      if ( bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        break;

      if ( listen(srv, 1) < 0 )
        break;

      return srv;
    }while(0);

    { HSaveError _err;
      scClose( srv );
    }

 return INVALID_SOCKET;
}

SOCKET MYRTLEXP scAccept( SOCKET srv, DWORD timeout )
  {  SOCKET Sock;

     switch( scISRead( srv,timeout ) ) {
       case scrTimeout: return NULL;

       case   scrError: return INVALID_SOCKET;
     }

     Sock = accept( srv, NULL, NULL );

 return scValid(Sock) ? Sock : INVALID_SOCKET;
}

scResults MYRTLEXP scISRead( SOCKET sock, DWORD timeout )
  {  fd_set  ready;
     fd_set  err;
     timeval to;
     int     res;

     if ( timeout == INFINITE ) {
       to.tv_usec = 0;
       to.tv_sec  = 10;
     } else
     if ( timeout <= 1 ) {
       to.tv_usec = 1;
       to.tv_sec  = 0;
     } else {
       to.tv_usec = (timeout % 1000) * 1000;
       to.tv_sec  = timeout / 1000;
     }

     do{
       FD_ZERO(&ready); FD_SET(sock, &ready);
       FD_ZERO(&err);   FD_SET(sock, &err);

       res = select(int(sock + 1),&ready,0,&err,&to);

       if ( res < 0 )
         return scrError;
        else
       if ( res == 0 ) {
         if ( timeout == INFINITE )
           continue;
          else {
           FIO_SETERRORN( ERROR_TIMEOUT );
           return scrTimeout;
         }
       } else
         return FD_ISSET(sock,&err) ? scrError : scrOk;

     }while( 1 );
}

scResults scISWrite( SOCKET sock, DWORD timeout )
  {  fd_set  ready;
     fd_set  err;
     timeval to;
     int     res;

     if ( timeout == INFINITE ) {
       to.tv_usec = 0;
       to.tv_sec  = 10;
     } else
     if ( timeout <= 1 ) {
       to.tv_usec = 1;
       to.tv_sec  = 0;
     } else {
       to.tv_usec = (timeout % 1000) * 1000;
       to.tv_sec  = timeout / 1000;
     }

     do{
       FD_ZERO(&ready); FD_SET(sock, &ready);
       FD_ZERO(&err);   FD_SET(sock, &err);

       res = select(int(sock + 1), 0, &ready, &err, &to);

       if ( res < 0 )
         return scrError;
        else
       if ( res == 0 ) {
         if ( timeout == INFINITE )
           continue;
          else {
           FIO_SETERRORN( ERROR_TIMEOUT );
           return scrTimeout;
         }
       } else
         return FD_ISSET(sock,&err) ? scrError : scrOk;

     }while( 1 );
}

BOOL MYRTLEXP scFlushUnread( SOCKET sock )
  {  char buff[ 100 ];

     while( 1 ) {
       if ( !scISRead(sock,FALSE) )
         return scError() == 0;

       if ( recv(sock,(SOCK_DATATYPE)buff,sizeof(buff)-1,0) <= 0 )
         return FALSE;
     }
}

BOOL MYRTLEXP scSend( SOCKET sock, LPCVOID _data, DWORD dsz, HANDLE Period )
  {  int sz;
     LPBYTE data = (LPBYTE)_data;

     do{
       if ( scISWrite(sock, Period ? PRPeriodLast(Period) : INFINITE) != scrOk )
         return FALSE;

       if ( (sz=send(sock,(SOCK_DATATYPE)data,dsz,0)) <= 0 )
         return FALSE;

       if ( Period && PRPeriodEnd(Period) ) {
         FIO_SETERRORN( ERROR_TIMEOUT );
         return FALSE;
       }

       data += sz;
       dsz  -= (DWORD)sz;
     }while( ((int)dsz) > 0 );

 return TRUE;
}

BOOL MYRTLEXP scRecv( SOCKET sock, LPVOID _data, DWORD dsz, HANDLE Period )
  {  int sz;
     LPBYTE data = (LPBYTE)_data;

     do{
       if ( scISRead(sock, Period ? PRPeriodLast(Period) : INFINITE) != scrOk )
         return FALSE;

       if ( (sz=recv(sock,(SOCK_DATATYPE)data,dsz,0)) <= 0 ) {
         return FALSE;
       }

       if ( Period && PRPeriodEnd(Period) ) {
         FIO_SETERRORN( ERROR_TIMEOUT );
         return FALSE;
       }

       data += sz;
       dsz -= (DWORD)sz;
     }while( ((int)dsz) > 0 );

 return TRUE;
}
#endif
