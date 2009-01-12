#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#include "HSend.h"

#if defined(__HWIN__)

#include <Std/HThread.h>
#include <Std/HEvent.h>

#if 0
  #define  PROC( v )
  #define  Log( v )
#else
  #define  PROC( v )  INProc __proc v ;
  #define  Log( v )   INProc::Say v
#endif

// ---------------------------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------------------------
// Number of seconds unused clients live in clients cache
#define CLIENT_TTL   60  //Secs

// Minimum read\write timeout
#define MIN_TIMEOUT  100UL  //MSecs

// ---------------------------------------------------------------------------------
// METHODS
// ---------------------------------------------------------------------------------
#define USE_ASYNC     1
#define USE_FILEASYNC 1
#if defined(USE_ASYNC)
  #define PIPE_OVERLAPPED FILE_FLAG_OVERLAPPED
#else
  #define PIPE_OVERLAPPED 0
#endif

// ---------------------------------------------------------------------------------
// LOCALS
// ---------------------------------------------------------------------------------
static CONSTSTR GetCompName( void )
  {  static char buff[ MAX_PATH_SIZE ] = "";

     if ( buff[0] )
       return buff;

     DWORD sz = sizeof(buff);
     if ( !GetComputerName( buff, &sz ) )
       HAbort( "!GetComputerName" );
 return buff;
}

static BOOL _FWrite( HANDLE f, LPCVOID msg, DWORD sz, OVERLAPPED *os, DWORD tmout )
  {
#if defined(USE_FILEASYNC)
 return FIOWrite( f, msg, sz, os, tmout );
#else
 return FIOWrite( f, msg, sz, NULL, INFINITE );
#endif
}

static BOOL _FRead( HANDLE f, LPVOID msg, DWORD sz, OVERLAPPED *os, DWORD tmout )
  {
#if defined(USE_FILEASYNC)
 return FIORead( f, msg, sz, os, tmout );
#else
 return FIORead( f, msg, sz, NULL, INFINITE );
#endif
}

// ---------------------------------------------------------------------------------
// TYPES
// ---------------------------------------------------------------------------------
/* Used to query nameserver for name
   The back name of requestor mailslot must be created as:
     "\\%s\mailslot\srr-%8X.%08X", CName, Id1, Id2
*/
LOCALSTRUCT( HIRequest )
  DWORD Size;                   //sizeof(HIRequest)
  DWORD Id1, Id2;               //request process id`s
  char  CName[ 100 ];           //Computer network name
};

/* Used for reply by nameserver.
   Contains valid network name of pipe.
*/
LOCALSTRUCT( HIReply )
  DWORD Size;                   //sizeof(HIReply)
  char  Name[ MAX_PATH_SIZE ];  //Full network of nameserver pipe ready for clients
};

LOCALSTRUCT( HInfoPipe )
    int          Index;           //Index in array (any nonzero value)
    DWORD        BaseThreadId;    //Id of callers thread
    HANDLE       hMail;           //Hnadle of information mailslot
    PathString   Name;            //Name of info mailslot
    HManualEvent Event;           //Exit event
    HThread      Thread;          //Info thread

  private:
    static void idPipeNameProc( PHInfoPipe ptr );

  public:
    HInfoPipe( CONSTSTR nm );
    ~HInfoPipe();

    BOOL Create( void );
    void Close( void );
};

/* Server-Client channel.
   Contains opened server pipe handle.
   Used for perform Send
*/
LOCALSTRUCT( HPipeChannel )
  PathString NetName;        //Network name (as requested in NameLocate)
  PathString PipeName;       //Name of connected pipe
  PathString ClientName;     //Name of client pipe for send data back
  time_t     CreateTime;     //Time of last operation (used to delete old clients)
  pid_t      Pid;            //Id in array (any unique nonzero)
  LPVOID     Local;          //INT: HLocalPipe*

  HPipeChannel( void ) { Pid = 0; }
};

LOCALCLASSBASE( HChannels, public MyRefArray<HPipeChannel> )
  public:
    pid_t  StartPid;

  public:
    HChannels( void ) { StartPid = 1; }

    PHPipeChannel LocateNet( CONSTSTR nm );
    PHPipeChannel LocatePipe( CONSTSTR nm );
    PHPipeChannel Locate( pid_t pid );
    PHPipeChannel Create( CONSTSTR lnm, CONSTSTR nnm );
};

/* Per thread listened pipe. Each thread creates and uses own item.
   Used for perform Receive.
   Can be connected with any single HPipeChannel
   'Remote' and 'Remote->Local' allways correctly set after succ Receive
*/
LOCALSTRUCT( HLocalPipe )
  HANDLE        Thread;     //Caller thread
  pid_t         Pid;        //Unique nonzero
  PHPipeChannel Remote;     //Ptr to local channel (after succ receive)
  HANDLE        hPipe;      //Local listening pipe

  HLocalPipe( void );
  ~HLocalPipe();
};

LOCALCLASSBASE( HPipes, public MyArray<PHInfoPipe> )
  public:
    int StartID;

  public:
    HPipes( void ) { StartID = 1; }

    PHInfoPipe Locate( CONSTSTR nm );
    PHInfoPipe Locate( pid_t pid );
};

LOCALCLASSBASE( HLocals, public MyArray<PHLocalPipe> )
  public:
    pid_t  StartPid;

  public:
    HLocals( void ) { StartPid = 1; }

    PHLocalPipe LocateThread( void );
};

// ---------------------------------------------------------------------------------
// Local DATA
// ---------------------------------------------------------------------------------
//Statistics
static int nNames = 0,
           nPipes = 0,
           nFiles = 0,
           nConnect = 0;
//Local arrays
static PHPipes    Pipes    = NULL;
static PHChannels Channels = NULL;
static PHLocals   Locals   = NULL;

//At exit local shutdowner
static AbortProc  OldAbort = NULL;
static BOOL       OnExitSet = FALSE;

//Used for every destructive op in any local array
static CRITICAL_SECTION cs;

static void FreeLocals( void )
  {
     if ( Locals ) {
       Log(( "HPIPE: Free locals" ));
       EnterCriticalSection( &cs );
       delete Channels; Channels = NULL;
       delete Locals;   Locals   = NULL;
       delete Pipes;    Pipes    = NULL;
       LeaveCriticalSection( &cs );
       DeleteCriticalSection( &cs );
     }
}

static void RTL_CALLBACK CloseLocals( void )
  {
     FreeLocals();
     if ( OldAbort ) OldAbort();
}

static void CreateLocals( void )
  {
    if ( Locals ) return;
    Log(( "HPIPE: Create locals" ));
    InitializeCriticalSection( &cs );
    Locals   = new HLocals;
    Pipes    = new HPipes;
    Channels = new HChannels;

    if ( !OnExitSet ) {
      OldAbort  = AtExit( CloseLocals );
      OnExitSet = TRUE;
    }
}

/* Force creation local listener for current thread
*/
static void ForceLocal( void )
  {
     CreateLocals();
     Locals->LocateThread();
}

// ---------------------------------------------------------------------------------
// Local classes
// ---------------------------------------------------------------------------------
HLocalPipe::HLocalPipe( void )
  {
    Thread = GetCurrentThread();
    Pid    = 0;
    Remote = NULL;
    hPipe  = NULL;
}

HLocalPipe::~HLocalPipe()
  {
    if ( hPipe ) {
      CloseHandle( hPipe );
      hPipe = NULL;
      nPipes--;
    }
}

HInfoPipe::HInfoPipe( CONSTSTR nm )
    : Event(FALSE)
  {
    Index        = 0;
    BaseThreadId = GetCurrentThreadId();
    hMail        = NULL;
    if ( *nm == '/' ) nm++;
    Name.printf( "\\\\.\\mailslot\\%s", nm );
}

HInfoPipe::~HInfoPipe()
  {
    Close();
}

void HInfoPipe::idPipeNameProc( PHInfoPipe p )
  {  HIRequest     req, old;
     HIReply       rep;
     DWORD         dw;

     //Fill reply
     rep.Size = sizeof(rep);
     SNprintf( rep.Name, sizeof(rep.Name),
               "\\\\%s\\pipe\\%08X.%08X",
               GetCompName(), GetCurrentProcessId(), p->BaseThreadId );

     do{
       //SYNC read of mailslot request
       if ( !ReadFile( p->hMail, &req, sizeof(req), &dw, NULL ) )
         continue;
        else
       if ( dw != sizeof(req) || req.Size != sizeof(req) )
         continue;
        else
       //Folter out all mailslot duplicates from all network interfaces
       if ( memcmp( &req, &old, sizeof(req) ) == 0 )
         continue;

       //Remember last recv structure (for ignore the same requests)
       memcpy( &old, &req, sizeof(req) );

       //Full network mailslot name used received info
       char destNM[ MAX_PATH_SIZE ];
       SNprintf( destNM, sizeof(destNM),
                 "\\\\%s\\mailslot\\srr-%08X.%08X",
                 req.CName, req.Id1, req.Id2 );

       //Connect to client mailslot
       HANDLE h = CreateFile( destNM,
                              GENERIC_READ|GENERIC_WRITE,
                              FILE_SHARE_READ|FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL );
       if ( !h || h == INVALID_HANDLE_VALUE )
         continue;

       //Reply nameserver name
       WriteFile( h, &rep, sizeof(rep), &dw, NULL );

       //Down client connection
       CloseHandle( h );

     }while( !p->Event.IsSet() );
}

/* Creates net-wide listening mailslot for registered name.
   Run listener thread.
*/
BOOL HInfoPipe::Create( void )
  {  PROC(( "HInfoPipe::Create", "%s", Name.c_str() ))

     do{
       hMail = CreateMailslot( Name.c_str(), 0, 50, NULL );
       if ( !hMail || hMail == INVALID_HANDLE_VALUE ) {
         Log(( "!create mail" ));
         break;
       }
       nNames++;

       Event.Reset();
       if ( !Thread.Create( (ThreadProc_t)idPipeNameProc, this ) ) {
         Log(( "!create thread" ));
         break;
       }

       return TRUE;
     }while(0);

     Close();
 return FALSE;
}

void HInfoPipe::Close( void )
  {
     Event.Set();
     Thread.Wait();
     Thread.Close();
     if ( hMail ) {
       CloseHandle(hMail);
       hMail = NULL;
       nNames--;
     }
}

/* Look for altready registered names.
   Check all items fro existing and remove items with down listeners.
*/
PHInfoPipe HPipes::Locate( CONSTSTR nm )
  {
     if ( *nm == '/' ) nm++;

     EnterCriticalSection( &cs );
     for( int n = 0; n < Count(); n++ ) {
       PHInfoPipe p = Item(n);

       if ( !p->Thread.isRun() ) {
         Log(( "Thread %08X(%s) is down. Delete pipe item", p->Thread.Id, p->Name.c_str() ));
         DeleteNum( n );
         n--;
       } else
       if ( StrCmp( p->Name.c_str() + 14 /*\\.\mailslot\*/,
                    nm ) == 0 )
          return p;
     }
     LeaveCriticalSection( &cs );

 return NULL;
}

/* Find listener by unique id.
   Remove down listeners.
*/
PHInfoPipe HPipes::Locate( pid_t pid )
  {  PHInfoPipe rc = NULL;

     EnterCriticalSection( &cs );
     for( int n = 0; n < Count(); n++ ) {
       PHInfoPipe p = Item(n);

       if ( !p->Thread.isRun() ) {
         Log(( "Thread %08X(%s) is down. Delete pipe item", p->Thread.Id, p->Name.c_str() ));
         DeleteNum( n );
         n--;
       } else
       if ( p->Index == pid ) {
         rc = p;
         break;
       }
     }
     LeaveCriticalSection( &cs );
 return rc;
}

/* Find per-thread receive item.
   Check thread for close and remove it.

   If infor for current thread not found, create listening
   pipe and add new item.
*/
PHLocalPipe HLocals::LocateThread( void )
  {  PROC(( "HLocals::LocateThread", NULL ))
     PHLocalPipe p;
     HANDLE      h;

     PHLocalPipe rc = NULL;
     EnterCriticalSection( &cs );
     for( int n = 0; n < Count(); n++ ) {
       p = Item(n);
       if ( WaitForSingleObject( p->Thread,0 ) != WAIT_TIMEOUT ) {
         Log(( "Thread %p(%d) is down. Delete local pipe item", p->Thread, p->Pid ));
         DeleteNum(n);
         n--;
       } else
       if ( p->Thread == GetCurrentThread() ) {
         Log(( "Local pipe is found %p(%d)", p->Thread, p->Pid ));
         rc = p;
         break;
       }
     }
     LeaveCriticalSection( &cs );
     if ( rc ) return rc;

     PathString nm;
     nm.printf( "\\\\.\\pipe\\%08X.%08X", GetCurrentProcessId(), GetCurrentThreadId() );
     Log(( "Create pipe %s", nm.c_str() ));

     h = CreateNamedPipe( nm.c_str(),
                          PIPE_OVERLAPPED|PIPE_ACCESS_DUPLEX,
                          PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT,
                          PIPE_UNLIMITED_INSTANCES,
                          0, 0, 1000, NULL );
     if ( !h || h == INVALID_HANDLE_VALUE ) {
       Log(( "!create pipe" ));
       return NULL;
     }
     nPipes++;

     EnterCriticalSection( &cs );
       p = Add( new HLocalPipe );
       p->Pid   = StartPid++;
       p->hPipe = h;
     LeaveCriticalSection( &cs );

     Log(( "rc=%p(%d)", p->Thread, p->Pid ));

 return p;
}

/* Find by reristered global name
*/
PHPipeChannel HChannels::LocateNet( CONSTSTR nm )
  {  PHPipeChannel p;
     for( int n = 0; n < Count(); n++ ) {
       p = Item(n);
       if ( StrCmp( p->NetName.c_str(),nm ) == 0 )
         return p;
     }
 return NULL;
}

/* Find by remote pipe name
   Check TTL and remove old items.
*/
PHPipeChannel HChannels::LocatePipe( CONSTSTR nm )
  {  PHPipeChannel p, rc = NULL;
     time_t        ctm = time(NULL);

     EnterCriticalSection( &cs );
     for( int n = 0; n < Count(); n++ ) {
       p = Item(n);
       if ( ctm-p->CreateTime > CLIENT_TTL ) {
         Log(( "HPIPE: Delete channel by TTL (pid: %d)", p->Pid ));
         DeleteNum( n );
         n--;
       } else
       if ( StrCmp( p->PipeName.c_str(),nm ) == 0 ) {
         p->CreateTime = ctm;
         rc = p;
         break;
       }
     }
     LeaveCriticalSection( &cs );

 return rc;
}

/* Find by unique id.
*/
PHPipeChannel HChannels::Locate( pid_t pid )
  {  PHPipeChannel p, rc = NULL;

     EnterCriticalSection( &cs );
     for( int n = 0; n < Count(); n++ ) {
       p = Item(n);
       if ( p->Pid == pid ) {
         rc = p;
         break;
       }
     }
     LeaveCriticalSection( &cs );
 return rc;
}

/* Create client-server channel item.
   lnm - global network name
   nnm - reote pipe name
*/
PHPipeChannel HChannels::Create( CONSTSTR lnm, CONSTSTR nnm )
  {  HPipeChannel *p, it;

     Log(( "HChannels::Create: lnm [%s], nm [%s]", lnm, nnm ));
     it.NetName    = lnm;
     it.PipeName   = nnm;
     it.CreateTime = time(NULL);

     EnterCriticalSection( &cs );
     p = Add( it );
     p->Pid = StartPid++;
     LeaveCriticalSection( &cs );

 return p;
}

// ---------------------------------------------------------------------------------
// INTERFACE
// ---------------------------------------------------------------------------------
int MYRTLEXP HPipeNameAttach( CONSTSTR nm )
  {  PROC(( "HPipeNameAttach", "%s", nm ))
     PHInfoPipe p;

     CreateLocals();

     if ( (p=Pipes->Locate(nm)) != NULL ) {
       Log(( "Already exist: %d", p->Index ));
       return p->Index;
     }

     p = new HInfoPipe(nm);

     if ( p->Create() ) {
       EnterCriticalSection( &cs );
         Pipes->Add( p );
         p->Index = Pipes->StartID++;
       LeaveCriticalSection( &cs );

       ForceLocal();
       Log(( "rc=%d", p->Index ));
       return p->Index;
     }

     DWORD err = FIO_ERRORN;
     delete p;
     FIO_SETERRORN( err );
     Log(( "rc=-1" ));
 return -1;
}

// ---------------------------------------------------------------------------------
int MYRTLEXP HPipeNameDetach( int pid )
  {  PHInfoPipe p;

     CreateLocals();

     if ( (p=Pipes->Locate(pid)) == NULL ) {
       FIO_SETERRORN( ERROR_INVALID_PARAMETER );
       return -1;
     }

     EnterCriticalSection( &cs );
     Pipes->Delete( p );
     LeaveCriticalSection( &cs );
 return 0;
}

// ---------------------------------------------------------------------------------
pid_t MYRTLEXP HPipeNameLocate( CONSTSTR nm )
  {  PROC(( "HPipeNameLocate", "%s", nm ))
     PathString    Name;
     HIRequest     req;
     HIReply       rep;
     DWORD         dw;
     PHPipeChannel p;

     CreateLocals();

     if ( (p=Channels->LocateNet(nm)) != NULL ) {
       p->CreateTime = time(NULL);
       Log(( "Channel already exist: pid=%d", p->Pid ));
       return p->Pid;
     }

     HANDLE h,m;

     do{
       m = NULL;

       if ( *nm == '/' ) nm++;
       Name.printf( "\\\\*\\mailslot\\%s", nm );
       Log(( "Create mailslot [%s]", Name.c_str() ));

       h = CreateFile( Name.c_str(),
                       GENERIC_READ|GENERIC_WRITE,
                       FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, 0, NULL );
       if ( !h || h == INVALID_HANDLE_VALUE ) {
         Log(( "!create mailslot" ));
         break;
       }

       req.Size = sizeof(req);
       req.Id1  = GetCurrentProcessId();
       req.Id2  = GetCurrentThreadId();
       TStrCpy( req.CName, GetCompName() );

       SNprintf( rep.Name, sizeof(rep.Name), "\\\\.\\mailslot\\srr-%08X.%08X", req.Id1, req.Id2 );
       Log(( "Create SRR mailslot [%s]", rep.Name ));

       m = CreateMailslot( rep.Name, 0, 1000, NULL );
       if ( !m || m == INVALID_HANDLE_VALUE ) {
         Log(( "!create SRR mailslot" ));
         break;
       }

       if ( !WriteFile( h,&req,sizeof(req),&dw,NULL) || dw != sizeof(req) ) {
         Log(( "!write request" ));
         break;
       }

       if ( !ReadFile( m,&rep,sizeof(rep),&dw,NULL) || dw != sizeof(rep) ) {
         Log(( "!read request" ));
         break;
       }

       if ( rep.Size == sizeof(rep) ) {
         p = Channels->Create( nm, rep.Name );
         Log(( "rc=%d", p ? p->Pid : (-1) ));
         return p ? p->Pid : (-1);
       }

     }while(0);

     dw = FIO_ERRORN;
     if ( m && m != INVALID_HANDLE_VALUE ) CloseHandle(m);
     if ( h && h != INVALID_HANDLE_VALUE ) CloseHandle(h);
     FIO_SETERRORN( dw );

     Log(( "rc=-1" ));
 return -1;
}

// ---------------------------------------------------------------------------------
int MYRTLEXP HPipeSend( pid_t pid, LPCVOID smsg, LPVOID rmsg, DWORD ssz, DWORD rsz,DWORD tmout )
  {  PHPipeChannel p;
     DWORD         expectedSize;
     HANDLE        hFile;
     OVERLAPPED   os;
#if defined(USE_ASYNC)
     HManualEvent ev( FALSE );
     memset( &os, 0, sizeof(os) );
     os.hEvent = ev.Handle;
     ev.Reset();
#endif

     PROC(( "Send", "pid: %d, %p, %p, %d, %d, tm: %d", pid, smsg, rmsg, ssz, rsz, tmout ));
     CreateLocals();

     if ( (p=Channels->Locate(pid)) == NULL ) {
       FIO_SETERRORN( ERROR_NOT_CONNECTED );
       return -1;
     }
     p->CreateTime = time(NULL);

Reconnect:
   //Create IO pipe for single SRR transaction
     for( ; 1; Sleep(1) ) {
       hFile = CreateFile( p->PipeName.c_str(),
                           GENERIC_READ|GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           PIPE_OVERLAPPED,
                           NULL );
       if ( hFile && hFile != INVALID_HANDLE_VALUE )
         break;

       //Client does not create name yet or does not call receive
       if ( FIO_ERRORN == ERROR_FILE_NOT_FOUND ) {
         Log(( "SND: client not found yet - do loop" ));
         continue;
       }

       if ( FIO_ERRORN != ERROR_PIPE_BUSY ) {
         Log(( "SND: Channel error! Delete item (%s,%s,%s) pid:%d",
               p->NetName.c_str(), p->PipeName.c_str(), p->ClientName.c_str(), p->Pid ));
         DWORD err = FIO_ERRORN;
         EnterCriticalSection( &cs );
           Channels->Delete( p );
         LeaveCriticalSection( &cs );
         FIO_SETERRORN( err );
         return -1;
       }
     }
     nFiles++;

     int rc = -1;
     do{
     //Send back info
       HIReply info;
       info.Size = sizeof(info);
       SNprintf( info.Name, sizeof(info.Name),
                 "\\\\%s\\pipe\\%08X.%08X",
                 GetCompName(), GetCurrentProcessId(), GetCurrentThreadId() );
       if ( !_FWrite( hFile, &info, sizeof(info), &os, tmout ) ) {
         Log(( "SND: !write info" ));
         break;
       }

     //Recv max possible receive size
       if ( !_FRead( hFile,  &expectedSize, sizeof(expectedSize), &os, tmout ) ) {
         Log(( "SND: !read exp size" ));
         break;
       }

     //Server signals to reconnect
       if ( !expectedSize ) {
         Log(( "SND: signalled to reconnect" ));
         CloseHandle( hFile );
         goto Reconnect;
       }

     //Min sizes
       ssz = Min(expectedSize,ssz);

     //Send actual receive size
       if ( !_FWrite( hFile, &ssz, sizeof(ssz), &os, tmout ) ) {
         Log(( "SND: !write act size" ));
         break;
       }

     //Send receive data
       if ( !_FWrite( hFile, smsg, ssz, &os, tmout ) ) {
         Log(( "SND: !write data" ));
         break;
       }

     //Read reply data size
       if ( !_FRead( hFile,  &expectedSize, sizeof(expectedSize), &os, tmout ) ) {
         Log(( "SND: !read reply data" ));
         break;
       }
       rsz = Min(expectedSize,rsz);

     //Write actual reply data size
       if ( !_FWrite( hFile, &rsz, sizeof(rsz), &os, tmout ) ) {
         Log(( "SND: !write act reply size" ));
         break;
       }

     //Read reply result
       if ( !_FRead( hFile, rmsg, rsz, &os, tmout ) ) {
         Log(( "SND: !read reply" ));
         break;
       }

       rc = rsz;
     }while( 0 );

     CloseHandle( hFile );
     nFiles--;

     Log(( "SND: rc=%d", rc ));
 return rc;
}

// ---------------------------------------------------------------------------------
pid_t MYRTLEXP HPipeReceive( pid_t pid, LPVOID msg, DWORD sz,DWORD tmout )
  {  PHLocalPipe  p;
     OVERLAPPED   os;
     HManualEvent ev( FALSE );

     PROC(( "Receive", "pid: %d, %p[%d], %d", pid, msg, sz, tmout ));

     ForceLocal();
     if ( (p=Locals->LocateThread()) == NULL )
       return -1;

     memset( &os, 0, sizeof(os) );
     os.hEvent = ev.Handle;
     ev.Reset();

     do{
       if ( ConnectNamedPipe( p->hPipe,&os ) )
         break;

       if ( FIO_ERRORN == ERROR_PIPE_CONNECTED )
         break;

       if ( FIO_ERRORN == ERROR_IO_PENDING ) {
         DWORD dw;

         dw = WaitForSingleObject( ev.Handle, Max( MIN_TIMEOUT, tmout ) );
         if ( dw != WAIT_OBJECT_0 ) {
           DisconnectNamedPipe( p->hPipe );
           return -1;
         }

         if ( !GetOverlappedResult( p->hPipe,&os,&dw,TRUE ) ) {
           DisconnectNamedPipe( p->hPipe );
           return -1;
         }

         break;
       }

       DisconnectNamedPipe( p->hPipe );
       return -1;
     }while( 1 );

     HIReply info;

     if ( !tmout )
       tmout = INFINITE;
      else
     if ( tmout != INFINITE )
       tmout = Max( tmout, MIN_TIMEOUT );

     do{
     //Recv info
       if ( !_FRead( p->hPipe, &info, sizeof(info), &os, tmout ) ) {
         Log(( "RCV: !read info" ));
         break;
       }

     //Fill remote
       p->Remote = Channels->LocatePipe( info.Name );
       if ( pid ) {
         if ( p->Remote && p->Remote->Pid != pid ) {
           Log(( "RCV: connected remote pid is wrong %d instead %d", p->Remote->Pid, pid ));

           //Write reconnect signal
           DWORD tmp = 0;
           if ( !_FWrite( p->hPipe, &tmp, sizeof(tmp), &os, tmout ) ) {
             Log(( "RCV: !write size signal" ));
             break;
           }
           break;
         }
       }

       if ( !p->Remote )
         p->Remote = Channels->Create( "", info.Name );
        p->Remote->Local = p;

     //Write max receive size
       if ( !_FWrite( p->hPipe, &sz, sizeof(sz), &os, tmout ) ) {
         Log(( "RCV: !write recv size" ));
         break;
       }

     //Read actual size
       if ( !_FRead( p->hPipe, &sz, sizeof(sz), &os, tmout ) ) {
         Log(( "RCV: !read actual size" ));
         break;
       }

     //Read recv data
       if ( !_FRead( p->hPipe, msg, sz, &os, tmout ) ) {
         Log(( "RCV: !read recv data" ));
         break;
       }

       Log(( "RCV: rc=%d", p->Remote->Pid ));
       nConnect++;
       return p->Remote->Pid;
     }while(0);

     DisconnectNamedPipe( p->hPipe );
 return -1;
}

// ---------------------------------------------------------------------------------
int MYRTLEXP HPipeReply( pid_t pid, LPCVOID msg, DWORD sz,DWORD tmout )
  {  PHPipeChannel ch = Channels->Locate(pid);
     PHLocalPipe   p;
     int           rc;
     OVERLAPPED   os;
#if defined(USE_ASYNC)
     HManualEvent ev( FALSE );
     memset( &os, 0, sizeof(os) );
     os.hEvent = ev.Handle;
     ev.Reset();
#endif

     CreateLocals();

     if ( !ch || (p=(PHLocalPipe)ch->Local) == NULL ) {
       FIO_SETERRORN( ERROR_NOT_CONNECTED );
       return -1;
     }
     ch->CreateTime = time(NULL);

     PROC(( "Reply", NULL ));
     rc = -1;
     do{
     //Write max reply size
       if ( !_FWrite( p->hPipe, &sz, sizeof(sz), &os, tmout ) ) {
         Log(( "RPL: !write reply size" ));
         break;
       }

     //Read actual reply size
       if ( !_FRead( p->hPipe, &sz, sizeof(sz), &os, tmout ) ) {
         Log(( "RPL: !read reply size" ));
         break;
       }

     //Write reply data
       if ( !_FWrite( p->hPipe, msg, sz, &os, tmout ) ) {
         Log(( "RPL: !write reply data" ));
         break;
       }

       rc = 0;
     }while(0);

     DisconnectNamedPipe( p->hPipe );
     nConnect--;

     Log(( "RPL: rc=%d", rc ));
 return rc;
}

// ---------------------------------------------------------------------------------
void MYRTLEXP HPipeGetStats( PHPipeStat st )
  {
    st->Names    = Pipes    ? Pipes->Count()    : (-1);
    st->Channels = Channels ? Channels->Count() : (-1);
    st->Threads  = Locals   ? Locals->Count()   : (-1);
    st->NNames   = nNames;
    st->NPipes   = nPipes;
    st->NFiles   = nFiles;
    st->NConnect = nConnect;
}

// ---------------------------------------------------------------------------------
void MYRTLEXP HPipeCloseConnections( void )
  {
    FreeLocals();
}

#endif
