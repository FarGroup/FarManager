#ifndef __MY_SOCKET_LOWLEVEL
#define __MY_SOCKET_LOWLEVEL

/*******************************************************************
   SOCK default timeouts
 *******************************************************************/
enum _SOCK_Constants {
 SOCK_FULLTIMEOUT         = 60000,                              //Timeout for global operation (used in server for keep client alive)
 SOCK_MAXTIMEOUT          = 15000,                              //Timeout for operation marked as infinite (for one simple oeration)
 SOCK_SENDUNSEND          = 15000,                              //WIN socket linger value (wait on closesocket while all data has been send)
 SOCK_IOBUFFSIZE          = 1024*10,                            //Size of IO buffer setted by default
 SOCK_FULLCOUNT           = (SOCK_FULLTIMEOUT/SOCK_MAXTIMEOUT)  //Count of retries while client must send data or will be dropped
};

/*******************************************************************
     OS specific
 *******************************************************************/
#if defined(__HWIN__)
  typedef char FAR       *SOCK_DATATYPE;
  typedef int             ioctl_val;
#endif

#if defined(__QNX__)
  typedef int             SOCKET;
  typedef char           *SOCK_DATATYPE;
  #define INVALID_SOCKET  (-1)
  typedef u_long          ioctl_val;
  #define ioctlsocket     ioctl
#endif

#if defined(__GNUC__)
  typedef int             SOCKET;
  typedef char           *SOCK_DATATYPE;
  #define INVALID_SOCKET  (-1)
  typedef u_long          ioctl_val;
  #define ioctlsocket     ioctl
#endif

enum scResults {
  scrError,
  scrOk,
  scrTimeout
};

/*******************************************************************
     Procedures
 *******************************************************************/
HDECLSPEC SOCKET    MYRTLEXP scCreate( void );                                // Create stream socket and set it options
HDECLSPEC BOOL      MYRTLEXP scClose( SOCKET s );                             // Close STREAM socket
HDECLSPEC BOOL      MYRTLEXP scValid( SOCKET sock );                          // Check if handle is valid socket handle
HDECLSPEC DWORD     MYRTLEXP scError( void );                                 // Last socket operation error
HDECLSPEC void      MYRTLEXP scSetError( DWORD err );                         // Set --`--
HDECLSPEC scResults MYRTLEXP scISRead( SOCKET sock, DWORD timeout );
HDECLSPEC scResults MYRTLEXP scISWrite( SOCKET sock, DWORD timeout );
HDECLSPEC BOOL      MYRTLEXP scFlushUnread( SOCKET sock );

HDECLSPEC BOOL      MYRTLEXP scSend( SOCKET sock,LPCVOID _data,DWORD dsz, HANDLE Period = NULL );
HDECLSPEC BOOL      MYRTLEXP scRecv( SOCKET sock,LPVOID _data,DWORD dsz, HANDLE Period = NULL );
/* !!Notes:
    Period - OPT res of PRPeriodCreate
    If period is set and it ens before wait of complete
    transfer FALSE returns and ERROR_TIMEOUT is set
*/

HDECLSPEC SOCKET    MYRTLEXP scServerConnect( CONSTSTR HostName );            // Rets socket connected to host
HDECLSPEC SOCKET    MYRTLEXP scServerSetup( CONSTSTR HostName );              // Setup server socket to port

/* Accept connection to server socket
   Val:
     timeout - number of ms of INFINITE
   Ret:
     NULL           - timeout
     INVALID_SOCKET - error
     value          - client socket
*/
HDECLSPEC SOCKET    MYRTLEXP scAccept( SOCKET ServerSocket, DWORD timeout );

#endif
