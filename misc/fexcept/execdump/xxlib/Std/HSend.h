#ifndef __MY_PIPE_SEND
#define __MY_PIPE_SEND

#if defined( __HWIN__ )
  STRUCT( HPipeStat )
    int Names;      //Number of items in names array
    int Channels;   //Number of currently known channels
    int Threads;    //Number of listener threads
    int NNames,     //Number of registered unique names
        NPipes,     //Number of pipes open
        NFiles,     //Number of files open
        NConnect;   //Number of connected clients
  };

/*  HPipeSend
    Sends message to known nameserver, wait and receive servers reply.
    If server expects less data than 'ssz' send only part of data.
    If server reply less data than 'rsz' receive only part of data.
    Ret:
     rsz  - number of really received bytes
     -1   - on error
*/
  HDECLSPEC int    MYRTLEXP HPipeSend( pid_t pid, LPCVOID smsg, LPVOID rmsg, DWORD ssz, DWORD rsz,DWORD tmout = INFINITE );

/*  HPipeReply
    Send reply to clietn prev connected by Receive.
    If 'pid' is not connected to client return -1.
    If client expects less data than 'sz' only part of data will be send.
    If 'tmout' is set to 0 it used only for check available connection. Data transfers
    uses MIN_TIMEOUT instead of zero.
    If 'tmout' is set to INFINITE will locks until error or connection made.
    In INFINITE mode, if connection made but transfers error reached -1 will be returned.
    Ret:
     pid  - connection id, can be used for Reply and|or Send data back.
     -1   - on error
*/
  HDECLSPEC int    MYRTLEXP HPipeReply( pid_t pid, LPCVOID msg, DWORD sz,DWORD tmout = INFINITE );

/*  HPipeReceive
    Wait for connection and receive client request.
    If client sends less data than 'sz' fill only part of data.
    If 'tmout' is set to 0 it used only for check available connection. Data transfers
    uses MIN_TIMEOUT instead of zero.
    If 'tmout' is set to INFINITE will locks until error or connection made.
    In INFINITE mode, if connection made but transfers error reached -1 will be returned.
    Ret:
     pid  - connection id, can be used for Reply and|or Send data back.
     -1   - on error
*/
  HDECLSPEC pid_t  MYRTLEXP HPipeReceive( pid_t pid, LPVOID msg, DWORD ssz,DWORD tmout = INFINITE );

/*  HPipeNameLocate
    Look for network name, create commenication channel and returns id of channel.
    Ret:
     channel id - nonzero, on success
     -1         - on error
*/
  HDECLSPEC pid_t  MYRTLEXP HPipeNameLocate( CONSTSTR nm );

/*  HPipeNameAttach
    Registers global net-wide name.
    Ret:
     registered ID
     -1 on error
*/
  HDECLSPEC int    MYRTLEXP HPipeNameAttach( CONSTSTR nm );

/*  HPipeNameDetach
    Unregisters global net-wide name.
    Ret:
     0  on success
     -1 on error
*/
  HDECLSPEC int    MYRTLEXP HPipeNameDetach( int pid );

/* Fill statistics structure
*/
  HDECLSPEC void   MYRTLEXP HPipeGetStats( PHPipeStat st );

/* Close all internal data used.
*/
  HDECLSPEC void MYRTLEXP HPipeCloseConnections( void );

  #define IO_Send                HPipeSend
  #define IO_Receive             HPipeReceive
  #define IO_Creceive( p, m, s ) HPipeReceive( p,m,s,0 )
  #define IO_Reply               HPipeReply
  #define IO_NameAttach          HPipeNameAttach
  #define IO_NameLocate          HPipeNameLocate
  #define IO_NameDetach          HPipeNameDetach

  //INTRxxx on QNX are signal-safe functions which are mapped to regular ones on HWIN
  #define INTRSend               IO_Send
  #define INTRReply              IO_Reply
  #define INTRReceive            IO_Receive
#else
#if defined(__QNX__)
  #define IO_NameAttach( nm )  qnx_name_attach( 0, nm )
  #define IO_NameLocate( nm)   qnx_name_locate( 0, nm, 0, NULL )
  #define IO_NameDetach( pid ) qnx_name_detach( 0, pid )
#endif
#endif

#endif
