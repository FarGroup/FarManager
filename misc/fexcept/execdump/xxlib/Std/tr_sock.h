#ifndef __MY_SOCK_TRANSPOPORT
#define __MY_SOCK_TRANSPOPORT

enum SC_Transfer {
   SCT_Read,
   SCT_Write,
   SCT_None
};

STRUCT( PRSockState )
  SOCKET       OSHandle;
  CONSTSTR     Addr;
  SC_Transfer  TDirection;
  DWORD        CachedSize;
};

/** @brief Create and prepare server-side socket
    @param name  Name of server. Must have {[<host name>:]<port>} format.
    @return Ready server handle or NULL on error.
    @sa PRWaitClient
*/
HDECLSPEC HANDLE MYRTLEXP PRServerSetup( CONSTSTR name );

/** @brief Waits client connection on server handle.
    @param srv      Server handle created calling PRServerSetup function.
    @param infinite Indicate infinite client waiting or just check if clients
                    already waiting for connection.
    @return NULL - timeout
            INVALID_HANDLE_VALUE - on error
            Client handle - on succ
    @sa PRServerSetup
*/
HDECLSPEC HANDLE MYRTLEXP PRWaitClient( HANDLE srv,DWORD timeout ); //Ret: NULL - timeout, INVALID_HANDLE_VALUE - err

/** @brief Locate and connect to waiting server on network.
    @param name Name of server. For IP servers must have a {<host name>:<port>} format.
    @return Server connection handle or NULL on error.
*/
HDECLSPEC HANDLE MYRTLEXP PRServerLocate( CONSTSTR name );

/** @brief Attempt to restore lost connection with server.
    @param Client2Server Connection handle created by call to PRServerLocate function.
    @return Return TRUE if connection successfull.

    @note The handle must be valid.
    @note If connection still alive function close it and try to connect.
    @note If function fail the handle destroyed and can not be used any more.
*/
HDECLSPEC BOOL   MYRTLEXP PRClientReconnect( HANDLE Client2Server );

/** @brief Decrement connection handle usage counter.

    Every call to PRCloseHandle decrement handle usage.
    Handle will be freed only if usage counter reach zero.
*/
HDECLSPEC void   MYRTLEXP PRCloseHandle( HANDLE h );

/** @brief Increment connection handle usage counter.

    Every call to PRCloseHandle decrement handle usage.
    Handle will be freed only if usage counter reach zero.
*/
HDECLSPEC void   MYRTLEXP PRIncUsage( HANDLE h );

/** @brief Attempt to get monopoly access to connection handle (EnterCriticalSection).
    @param h Created connection handle.
    @return TRUE if attempt successfull.

    Only one piece of code may have monopoly access to handle at the same time.
    If somebody already has monopoly access to handle the function call fail.
    This function usefull if you wish to set specific conection options in
    enviropment with many handle users.

    This function acts exactly like EnterCriticalSection API function but
    for one handle.
*/
HDECLSPEC BOOL   MYRTLEXP PRHandleUse( HANDLE h );

/** @brief Free connection handle from monopoly access (LeaveCriticalSection).
    @param h Created connection handle.
    @return TRUE if handle was freed.

    Only one piece of code may have monopoly access to handle at the same time.
    If somebody already has monopoly access to handle the function call fail.
    This function usefull if you wish to set specific conection options in
    enviropment with many handle users.

    This function acts exactly like LeaveCriticalSection API function but
    for one handle.
*/
HDECLSPEC BOOL   MYRTLEXP PRHandleRelease( HANDLE h );

/** @brief Retrives OS-specific socket handle from connection handle.
    @param h Created connection handle.
    @return SOCKET handle or INVALID_SOCKET on error.
*/
HDECLSPEC SOCKET MYRTLEXP PROSHandle( HANDLE h );

/** @brief Send data throught connection handle.
    @param h     Create connection handle.
    @param Buff  Buffer of data to send.
    @param sz    number of bytes to send.
    @param Period  OPT PRPeriodCreate handle of period.
                   If period is set and ends before io FALSE
                   returns and ERROR_TIMEOUT is set

    @return TRUE if all buffer bytes was successfulle sended.

    You can use any connection handle in call to this function.
*/
HDECLSPEC BOOL   MYRTLEXP PRSend( HANDLE h, LPCVOID Buff, DWORD sz, HANDLE Pareiod = NULL );

/** @brief Receive data throught connection handle.
    @param h       Create connection handle.
    @param Buff    Buffer of data to send.
    @param sz      number of bytes to send.
    @param Period  OPT PRPeriodCreate handle of period.
                   If period is set and ends before io FALSE
                   returns and ERROR_TIMEOUT is set

    @return TRUE if all buffer bytes was successfulle received.

    You can use any connection handle in call to this function.

    @note this function designed to receive \a exactly amount of data.
*/
HDECLSPEC BOOL   MYRTLEXP PRRecv( HANDLE h,LPVOID Buff,DWORD sz, HANDLE Pareiod = NULL );

/** @brief Forces to send all cached data if any.
    @param h Connection handle.

    @return Returns TRUE if handle is valid and flushed successfully (or if there is no data to send).

    @note The connection handle flushes automatically when you call receive data functions.
    @note Because of cached data transfers, to avoid data lost use this function before call
          to raw API functions such as \b recv using OS socket handle.
*/
HDECLSPEC BOOL   MYRTLEXP PRFlushCache( HANDLE h );

/** @brief Query connection handle state.
    @param h Connection handle to query.
    @param b Address of PRSockState structure to fill handle state data with.
    @return TRUE if specified handle is valid.
*/
HDECLSPEC BOOL   MYRTLEXP PRGetState( HANDLE h,PPRSockState p );

#endif