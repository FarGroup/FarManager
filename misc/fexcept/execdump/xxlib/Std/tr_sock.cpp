#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/** @page Client-Server socket transfer interface.
    @ingroup StdLib

    This set of functions designed to perform hi-level universal cached data
    transfer between server and client.

    The usage schema is:
      - create server side using PRServerSetup function;
      - connect to sserver using PRServerLocate function;
      - perform any bidirectional transfers between connected sides using PRSend and PRRecv functions;
      - close handles on both ends using PRCloseHandle;

    This functions caches transfered data, because of this:
      - the transfer speed independent of one transferred data packet length;
      - to really send data you need to switch handle to receive mode of use PRFlushCache manually;

    The size of cache used for transfer data is hardcoded to SOCK_IOBUFFSIZE value.
    This value is a 5K size.
*/
#if defined(DOXYGEN)
/** @brief Current transfer direction.
*/
enum SC_Transfer {
   SCT_Read,       ///< The connection handle is waiting for data.
   SCT_Write,      ///< The connection handle is writing data to other connection size.
   SCT_None        ///< The transfer dirrection is undefined.
};

/** @brief Structure contains information about connection handle.
    This structure used by PRGetState function to get connection handle information.
*/
STRUCT( PRSockState )
  SOCKET       OSHandle;      ///< OS socket handle used by connection.
  CONSTSTR     Addr;          ///< Address was used to create connection.
  SC_Transfer  TDirection;    ///< Current transfer dirrection.
  DWORD        CachedSize;    ///< Number of unsend bytes currently stored in connection cache.
};
#endif

#if 1
  #define  PROC( v )
  #define  Log( v )
#else
  #define  PROC( v )  INProc __proc v ;
  #define  Log( v )   INProc::Say v
#endif

#if defined(__QNX__) || defined(__HWIN__)
// !! Sockets implemented only for QNX and Windows

/*******************************************************************
     DATA
 *******************************************************************/
#define SOCK_MAGIC     MK_ID('S','c','l','\x0' )      // Client
#define SOCK_SRV_MAGIC MK_ID('S','s','r','\x0' )      // Server listen
#define SOCK_CLT_MAGIC MK_ID('S','s','c','\x0' )      // Server recver

//- Base of all sock holders
STRUCTBASE( PRSockBase, public HSafeObject )
    SOCKET       Handle;
    char        *Addr;
    SC_Transfer  TState;
    char         Cache[ SOCK_IOBUFFSIZE+1 ];
    DWORD        CSize;
  protected:
    virtual void Destroy( void );
  public:
    PRSockBase( DWORD Magic,SOCKET h = INVALID_SOCKET ) : HSafeObject( Magic ) { TState = SCT_None; Addr = NULL; Handle = h; CSize = 0; }

            void    Close( void );
            BOOL    ISRead( HANDLE Period = NULL );
            BOOL    ISWrite( HANDLE Period = NULL );
            BOOL    DOSend( LPCBYTE p,DWORD sz, HANDLE Period );
            BOOL    FlushCache( void );
    virtual void    UseHandle( void )     {}
    virtual void    ReleaseHandle( void ) {}
};
//- Client-side socket
STRUCTBASE( PRClientSock, public PRSockBase )
   CRITICAL_SECTION cs;
  protected:
    virtual void Destroy( void );
  public:
    PRClientSock( void );

    virtual void    UseHandle( void );
    virtual void    ReleaseHandle( void );
    static  PPRClientSock Cvt( LPVOID h ) { return (PPRClientSock)HSafeObject::Convert(h,sizeof(PRClientSock),SOCK_MAGIC); }
};
//- Server-side one connection socket
STRUCTBASE( PRServerClientSock, public PRSockBase )
  public:
    PRServerClientSock( SOCKET h ) : PRSockBase( SOCK_CLT_MAGIC,h ) {}
    static PPRServerClientSock Cvt( LPVOID h ) { return (PPRServerClientSock)HSafeObject::Convert(h,sizeof(PRServerClientSock),SOCK_CLT_MAGIC); }
};
//- Server side main socket
STRUCTBASE( PRServerMainSock, public PRSockBase )
  public:
    PRServerMainSock( void ) : PRSockBase( SOCK_SRV_MAGIC ) {}

    static PPRServerMainSock Cvt( LPVOID h ) { return (PPRServerMainSock)HSafeObject::Convert(h,sizeof(PRServerMainSock),SOCK_SRV_MAGIC); }
};
/*******************************************************************
     LOCAL CLASSES
 *******************************************************************/
void PRSockBase::Destroy( void )
  {
/*-*/Log(( "prClose %p %d",this,Handle ));
    StrFree( Addr );
    Close();
}
void PRSockBase::Close( void )
  {
    FlushCache();
    scClose(Handle);
    Handle = INVALID_SOCKET;
    TState = SCT_None;
}

BOOL PRSockBase::ISRead( HANDLE Period)
  {
    if ( !scValid(Handle) ) {
/*-*/Log(( "PRCH[%s]::!handle %d",&SMID,Handle ));
      TState = SCT_None;
      scSetError(WSAENOTSOCK);
      return FALSE;
    }

    if ( TState != SCT_Read &&
         !FlushCache() )
      return FALSE;

    if ( scISRead(Handle, Period ? PRPeriodLast(Period) : INFINITE) != scrOk )
      return FALSE;

    TState = SCT_Read;
 return TRUE;
}

BOOL PRSockBase::ISWrite( HANDLE Period )
  {
    if ( !scValid(Handle) ) {
/*-*/Log(( "PRCH[%s]::!handle %d",&SMID,Handle ));
      TState = SCT_None;
      return FALSE;
    }

    if ( TState == SCT_Write ) return TRUE;

    if ( scISWrite(Handle,Period ? PRPeriodLast(Period) : INFINITE) != scrOk ) {
/*-*/Log(( "PRCH[%s]::!iswrite h:%d",&SMID,Handle ));
      return FALSE;
    }
    TState = SCT_Write;
 return TRUE;
}

BOOL PRSockBase::FlushCache( void )
  {  BOOL res = (CSize)?scSend(Handle,Cache,CSize):TRUE;

     if ( !res ) {
/*-*/Log(( "PRCH[%s]: !flush %d",&SMID,CSize ));
     }
     CSize = 0;
 return res;
}

BOOL PRSockBase::DOSend( LPCBYTE p,DWORD sz, HANDLE Period )
  {  DWORD l;
     if ( !p || !sz ) return TRUE;
     if ( !scValid(Handle) ) return FALSE;

     l = Min( ((DWORD)(sizeof(Cache)-CSize)),sz );
     MemMove( Cache+CSize,p,l );

     CSize += l;
     sz    -= l;
     p     += l;

//Full
     if ( CSize == sizeof(Cache) ) {
       if ( !scSend( Handle, Cache, CSize, Period) ) {
         Log(( "PRcs[%s]:: !fcache %d %d (%d,%d)",&SMID,Handle,CSize,l,sz ));
         return FALSE;
       }

       CSize = 0;
       if ( sz >= sizeof(Cache) )
         return scSend(Handle,(SOCK_DATATYPE)p,sz,Period);
        else
         MemMove( Cache,p,CSize=sz );
     }
 return TRUE;
}
/*******************************************************************
   PRSock  client socket
 *******************************************************************/
PRClientSock::PRClientSock( void )
   : PRSockBase( SOCK_MAGIC )
  {
     InitializeCriticalSection( &cs );
}
void PRClientSock::Destroy( void )
  {
    EnterCriticalSection(&cs);
    DeleteCriticalSection(&cs);
    PRSockBase::Destroy();
}
void PRClientSock::UseHandle( void )
  {
    EnterCriticalSection( &cs );
}
void PRClientSock::ReleaseHandle( void )
  {
    LeaveCriticalSection( &cs );
}
/*******************************************************************
     Utils
 *******************************************************************/
static PPRSockBase CvtAny( HANDLE h )
  {  PPRClientSock       pcl;
     PPRServerClientSock pscl;
     PPRServerMainSock   ps;

//Client-side: close
     if ( (pcl=PRClientSock::Cvt((LPVOID)h)) != NULL )        return pcl;  else
//Server-side conversation: close
     if ( (pscl=PRServerClientSock::Cvt((LPVOID)h)) != NULL ) return pscl; else
//Serverside main: close
     if ( (ps=PRServerMainSock::Cvt((LPVOID)h)) != NULL )     return ps;   else
       return NULL;
}
/*******************************************************************
     INTERFACE IMPLEMENTATION
 *******************************************************************/
HANDLE MYRTLEXP PRServerSetup( CONSTSTR name )
  {
     PPRServerMainSock  p = new PRServerMainSock;
     if ( !p )
       return NULL;

     if ( !scValid(p->Handle=scServerSetup(name)) ) {
       HSafeObject::Release(p);
       return NULL;
     }

     p->Addr = StrDup( name );

 return (HANDLE)p;
}

HANDLE MYRTLEXP PRWaitClient( HANDLE srv,DWORD timeout )
  {  PPRServerMainSock p = PRServerMainSock::Cvt((LPVOID)srv);
     SOCKET            cl;

     if ( !srv || !p ) {
       scSetError(ERROR_INVALID_PARAMETER);
       return INVALID_HANDLE_VALUE;
     }

     cl = scAccept( p->Handle, timeout );

     //Err
     if ( cl == INVALID_SOCKET )
       return INVALID_HANDLE_VALUE;

     //Timeout
     if ( !cl )
       return NULL;

/*-*/Log(("prAccepted %d",cl ));
 return (HANDLE)(new PRServerClientSock( cl ));
}

HANDLE MYRTLEXP PRServerLocate( CONSTSTR name )
  {
     PPRClientSock p = new PRClientSock;
     if ( !p )
       return NULL;

     p->Handle = scServerConnect(name);

/*-*/Log(("prConnected %d",p->Handle ));

     if ( !scValid(p->Handle) ) {
       HSafeObject::Release(p);
       return NULL;
     }

     p->Addr = StrDup( name );

 return (HANDLE)p;
}

void MYRTLEXP PRIncUsage( HANDLE h )
  {  PPRSockBase p = CvtAny( h );

     if ( p )
       HSafeObject::Use(p);
}

void MYRTLEXP PRCloseHandle( HANDLE h )
  {
    HSafeObject::Release( (PHSafeObject)h );
}

SOCKET MYRTLEXP PROSHandle( HANDLE h )
  {  PPRSockBase p = CvtAny( h );

     return p ? p->Handle : INVALID_SOCKET;
}

BOOL MYRTLEXP PRHandleUse( HANDLE h )
  {  PPRSockBase p = CvtAny( h );
     if ( !p ) return FALSE;
     p->UseHandle();
 return TRUE;
}

BOOL MYRTLEXP PRHandleRelease( HANDLE h )
  {  PPRSockBase p = CvtAny( h );

     if ( !p )
       return FALSE;

     p->ReleaseHandle();

 return TRUE;
}

BOOL MYRTLEXP PRClientReconnect( HANDLE Client2Server )
  {
     PPRClientSock p = PRClientSock::Cvt( (LPVOID)Client2Server );
     if ( !p ) {
       scSetError(ERROR_INVALID_PARAMETER);
       return FALSE;
     }

     do{
       p->Close();
       if ( !scValid(p->Handle=scServerConnect(p->Addr)) )
         break;
       return TRUE;
     }while(0);

     PRCloseHandle(Client2Server);

 return FALSE;
}

BOOL MYRTLEXP PRSend( HANDLE h, LPCVOID Buff, DWORD sz, HANDLE Period )
  {  PPRSockBase p = PRClientSock::Cvt( (LPVOID)h );
     if ( !p ) p = PRServerClientSock::Cvt( (LPVOID)h );

     if ( !p ) {
/*-*/Log(( "PRs:: !cvt %p",h ));
       return FALSE;
     }

     if ( !Buff || !sz ) {
/*-*/Log(( "PRs:: !param %p,%p[%d]",h,Buff,sz ));
       scSetError(ERROR_INVALID_PARAMETER);
       return FALSE;
     }

     if ( !p->ISWrite( Period ) ) {
/*-*/Log(( "PRs:: !iswrite" ));
       return FALSE;
     }

return p->DOSend( (LPCBYTE)Buff, sz, Period );
}

BOOL MYRTLEXP PRRecv( HANDLE h, LPVOID Buff, DWORD sz, HANDLE Period )
  {  PPRSockBase p = PRClientSock::Cvt( (LPVOID)h );
     if (!p) p = PRServerClientSock::Cvt( (LPVOID)h );

     if ( !p ) {
/*-*/Log(( "PRr:: !cvt %p",h ));
       return FALSE;
     }
     if ( !Buff || !sz ) {
/*-*/Log(( "PRr:: !param %p,%p[%d]",h,Buff,sz ));
       scSetError(ERROR_INVALID_PARAMETER);
       return FALSE;
     }

     if ( !p->ISRead(Period) )
       return FALSE;

return scRecv( p->Handle, (SOCK_DATATYPE)Buff, sz, Period );
}

BOOL MYRTLEXP PRFlushCache( HANDLE h )
  {  PPRSockBase p = PRClientSock::Cvt( (LPVOID)h );
     if ( !p ) p = PRServerClientSock::Cvt( (LPVOID)h );

     if (!p) {
       scSetError(ERROR_INVALID_PARAMETER);
       return FALSE;
     }

return p->FlushCache();
}

BOOL MYRTLEXP PRGetState( HANDLE h,PPRSockState b )
  {  PPRSockBase p = PRClientSock::Cvt( (LPVOID)h );

     if (!p) p = PRServerClientSock::Cvt( (LPVOID)h );

     if (!p) {
       scSetError(ERROR_INVALID_PARAMETER);
       return FALSE;
     }

     b->OSHandle   = p->Handle;
     b->Addr       = p->Addr;
     b->TDirection = p->TState;
     b->CachedSize = p->CSize;

  return TRUE;
}
#endif
