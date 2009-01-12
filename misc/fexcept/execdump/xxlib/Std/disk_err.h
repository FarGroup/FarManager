#ifndef __MY_DISK_ERRORS
#define __MY_DISK_ERRORS

/**
   - MAX_ERRNUM Maximum OS error number
   - ERROR_STD  Spcial value to call SetError but does not change
                currently set error value (values itsef mean nothing);
   - ERROR_USER User defined error value;
                Must be the MAX_ERRNUM value (The first value in string
                list with spacialerror strings).
                Contains free style string setted by "SetError(char*)";
   - ERROR_xxx  Other non-OS error values;
                Must have values starting at ERROR_USER+1 without holes;
                Determine one special string each;
*/

// ---- MAXIMUM SYSTEM ERRORS
#if defined(__HWIN__)
  #define MAX_ERRNUM  20000 // End of WinSock Error Codes
#else
#if defined(__HDOS__)
  #define MAX_ERRNUM  50    // ENOTEMPTY
#else
#if defined(__QNX__)
  #define MAX_ERRNUM  1013  // EDSTFAULT
#else
#if defined(__GNUC__)
  #define MAX_ERRNUM  ELAST
#else
#if defined(__TEC32__)
  #define MAX_ERRNUM  ERRMAX
#else
  #error ERR_PLATFORM
#endif //TEC32
#endif //GNUC
#endif //QNX
#endif //HDOS
#endif //HWIN

#if !defined(EZERO)
  #define EZERO         0             /*0    Error 0                          */
#endif
#if !defined(EINVFNC)
  #define EINVFNC       EPERM         /*1    Invalid function number          */
#endif
#if !defined(ENOFILE)
  #define ENOFILE       ENOENT        /*2    File not found                   */
#endif
#if !defined(ENOPATH)
  #define ENOPATH       ESRCH         /*3    Path not found                   */
#endif
#if !defined(ECONTR)
  #define ECONTR        E2BIG         /*7    Memory blocks destroyed          */
#endif
#if !defined(EINVMEM)
  #define EINVMEM       EBADF         /*9    Invalid memory block address     */
#endif
#if !defined(EINVENV)
  #define EINVENV       ECHILD        /*10    Invalid environment              */
#endif
#if !defined(EINVFMT)
  #define EINVFMT       EAGAIN        /*11    Invalid format                   */
#endif
#if !defined(EINVACC)
  #define EINVACC       ENOMEM        /*12    Invalid access code              */
#endif
#if !defined(EINVDAT)
  #define EINVDAT       EACCES        /*13    Invalid data                     */
#endif
#if !defined(EINVDRV)
  #define EINVDRV       15            /*15    Invalid drive specified          */
#endif
#if !defined(ECURDIR)
  #define ECURDIR       EBUSY         /*16    Attempt to remove CurDir         */
#endif
#if !defined(ENOTSAM)
  #define ENOTSAM       EEXIST        /*17    Not same device                  */
#endif
#if !defined(ENMFILE)
  #define ENMFILE       EXDEV         /*18    No more files                    */
#endif
#if !defined(ETXTBSY)
  #define ETXTBSY       26            /*26    UNIX - not MSDOS                 */
#endif
#if !defined(EDEADLOCK)
  #define EDEADLOCK     EDEADLK       /*36    Locking violation                */
#endif
#if !defined(ENOTBLK)
  #define ENOTBLK       43            /*43    UNIX - not MSDOS                 */
#endif
#if !defined(EUCLEAN)
  #define EUCLEAN       47            /*47    UNIX - not MSDOS                 */
#endif

#if defined(__HUNIX__)
  #define WSAEWOULDBLOCK          EWOULDBLOCK
  #define WSAEINPROGRESS          EINPROGRESS
  #define WSAEALREADY             EALREADY
  #define WSAENOTSOCK             ENOTSOCK
  #define WSAEDESTADDRREQ         EDESTADDRREQ
  #define WSAEMSGSIZE             EMSGSIZE
  #define WSAEPROTOTYPE           EPROTOTYPE
  #define WSAENOPROTOOPT          ENOPROTOOPT
  #define WSAEPROTONOSUPPORT      EPROTONOSUPPORT
  #define WSAESOCKTNOSUPPORT      ESOCKTNOSUPPORT
  #define WSAEOPNOTSUPP           EOPNOTSUPP
  #define WSAEPFNOSUPPORT         EPFNOSUPPORT
  #define WSAEAFNOSUPPORT         EAFNOSUPPORT
  #define WSAEADDRINUSE           EADDRINUSE
  #define WSAEADDRNOTAVAIL        EADDRNOTAVAIL
  #define WSAENETDOWN             ENETDOWN
  #define WSAENETUNREACH          ENETUNREACH
  #define WSAENETRESET            ENETRESET
  #define WSAECONNABORTED         ECONNABORTED
  #define WSAECONNRESET           ECONNRESET
  #define WSAENOBUFS              ENOBUFS
  #define WSAEISCONN              EISCONN
  #define WSAENOTCONN             ENOTCONN
  #define WSAESHUTDOWN            ESHUTDOWN
  #define WSAETOOMANYREFS         ETOOMANYREFS
  #define WSAETIMEDOUT            ETIMEDOUT
  #define WSAECONNREFUSED         ECONNREFUSED
  #define WSAELOOP                ELOOP
  #define WSAENAMETOOLONG         ENAMETOOLONG
  #define WSAEHOSTDOWN            EHOSTDOWN
  #define WSAEHOSTUNREACH         EHOSTUNREACH
  #define WSAENOTEMPTY            ENOTEMPTY
  #define WSAEPROCLIM             EPROCLIM
  #define WSAEUSERS               EUSERS
  #define WSAEDQUOT               EDQUOT
  #define WSAESTALE               ESTALE
  #define WSAEREMOTE              EREMOTE

  #define SOCKET_ERROR            -1

  #define ERROR_ACCESS_DENIED     EACCES
#endif

// ---- STD ERROR CONSTANTS
#define ERROR_STD                     (MAX_ERRNUM+0)
#define ERROR_USER                    (MAX_ERRNUM+1)

#if !defined(__HWIN32__)
  #define ERROR_SUCCESS               (MAX_ERRNUM +  1)  // The operation completed successfully.
  #define ERROR_INVALID_PARAMETER     (MAX_ERRNUM +  2)  // The parameter is incorrect.
  #define ERROR_CALL_NOT_IMPLEMENTED  (MAX_ERRNUM +  3)  // This function is not supported on this system.
  #define WAIT_TIMEOUT                (MAX_ERRNUM +  4)  // The wait operation timed out.
  #define ERROR_TIMEOUT               WAIT_TIMEOUT
  #define ERROR_NOT_CONNECTED         (MAX_ERRNUM +  5)  // Operation on non connected pipe.
  #define ERROR_CRC                   (MAX_ERRNUM +  6)  // Data block integrity check fail.
  #define ERROR_CANCELLED             (MAX_ERRNUM +  7)  // Operation cancelled by user
  #define ERROR_MORE_DATA             (MAX_ERRNUM +  8)  // Need more data to complete operation
#endif

//DB
  #define ERROR_DB_CURSOR             (MAX_ERRNUM +  9)  // Bad DB cursor
  #define ERROR_DB_FETCH              (MAX_ERRNUM + 10)  // Error in DB fetch
  #define ERROR_DB_OOB                (MAX_ERRNUM + 11)  // Error in DB fetch
//IO
  #define ERROR_CLOSED                (MAX_ERRNUM + 12)  // Already closed.
  #define ERROR_CONNCLOSED            (MAX_ERRNUM + 13)  // Operation on already closed pipe.
  #define ERROR_TRANSFER              (MAX_ERRNUM + 14)  // Error transfer data.
  #define ERROR_HEADER                (MAX_ERRNUM + 15)  // Broken data in structured header.
  #define ERROR_MAGIC                 (MAX_ERRNUM + 16)  // Bad ID magic value.
  #define ERROR_OOB                   (MAX_ERRNUM + 17)  // Out of bounds.
  #define ERROR_SERVERSIDE            (MAX_ERRNUM + 18)  // Server side error.
  #define ERROR_SERVERSIDEUNK         (MAX_ERRNUM + 19)  // Unknown server side error.
  #define ERROR_RANGE                 (MAX_ERRNUM + 20)  // Value out of possible range.
  #define ERROR_DATA                  (MAX_ERRNUM + 21)  // Data block contains unexpected data.
  #define ERROR_TOOSMALL              (MAX_ERRNUM + 22)  // Specified buffer too small

/* Universal error handlers
*/
#if defined(__HWIN32__)
  typedef DWORD FIO_ERR_TYPE;

  #define FIO_ERROR_NONE (MAX_DWORD-1)
#else
  typedef int FIO_ERR_TYPE;

  #define FIO_ERROR_NONE (-2)
#endif

HDECLSPEC FIO_ERR_TYPE MYRTLEXP    GetIOErrorN( void );
HDECLSPEC void         MYRTLEXP    SetErrorN( FIO_ERR_TYPE ErrorValue );
HDECLSPEC void         MYRTLEXP_PT SetError( CONSTSTR Format, ... );
HDECLSPEC void         MYRTLEXP    SetErrorV( CONSTSTR Format, va_list a );

HDECLSPEC CONSTSTR     MYRTLEXP    GetIOErrorS_( FIO_ERR_TYPE Error, BOOL decode );

#if defined(__HWIN32__) && (!defined(__HCONSOLE__) || defined(USEPACKAGES))
  #define FIO_ERROR    GetIOErrorS_( FIO_ERROR_NONE, FALSE )
#else
  #define FIO_ERROR    GetIOErrorS_( FIO_ERROR_NONE, TRUE )
#endif

#define FIO_ERRORN        GetIOErrorN()
#define FIO_SETERRORN(v)  SetErrorN( v )
#define FIO_SETERROR      SetError

#if defined(__cplusplus)
LOCALCLASS( HSaveError )
    FIO_ERR_TYPE Error;
  public:
    HSaveError( void ) { Error = FIO_ERRORN; }
    ~HSaveError()      { FIO_SETERRORN( Error ); }
};
#endif

#endif
