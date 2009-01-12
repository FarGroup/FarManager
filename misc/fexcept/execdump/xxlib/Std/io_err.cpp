#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__HWIN32__)
  static CONSTSTR _OSErrorS( DWORD err )
    {  static char *WinEBuff = NULL;

      if ( WinEBuff ) LocalFree(WinEBuff);
      if ( FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,err,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (LPTSTR) &WinEBuff,0,NULL ) == 0 ) return "Unknown error";

      char *m;
      while( (m=strchr(WinEBuff,'\n')) != NULL ) *m = 0;
      while( (m=strchr(WinEBuff,'\r')) != NULL ) *m = 0;

   return WinEBuff;
  }
  #define _FIO_SETERRORN(v)   SetLastError( v )
  #define _FIO_ERRORN         __FIO_ERRORN()

  DWORD __FIO_ERRORN( void )
    {  DWORD err = GetLastError();

       if ( err != ERROR_SUCCESS )
         return err;

       static const DWORD errs[] = {

       #if defined(__BORLAND)
         ERROR_SUCCESS,                 // EZERO      0     Error 0
         ERROR_INVALID_FUNCTION,        // EINVFNC    1     Invalid function number
         ERROR_FILE_NOT_FOUND,          // ENOENT     2     No such file or directory
         ERROR_PATH_NOT_FOUND,          // ENOPATH    3     Path not found
         ERROR_TOO_MANY_OPEN_FILES,     // EMFILE     4     Too many open files
         ERROR_ACCESS_DENIED,           // EACCES     5     Permission denied
         ERROR_INVALID_HANDLE,          // EBADF      6     Bad file number
         ERROR_INVALID_BLOCK,           // ECONTR     7     Memory blocks destroyed
         ERROR_NOT_ENOUGH_MEMORY,       // ENOMEM     8     Not enough core
         ERROR_INVALID_BLOCK,           // EINVMEM    9     Invalid memory block address
         ERROR_BAD_ENVIRONMENT,         // EINVENV   10     Invalid environment
         ERROR_BAD_FORMAT,              // EINVFMT   11     Invalid format
         ERROR_INVALID_ACCESS,          // EINVACC   12     Invalid access code
         ERROR_INVALID_DATA,            // EINVDAT   13     Invalid data
         ERROR_GEN_FAILURE,             // EFAULT    14     Unknown error
         ERROR_IO_DEVICE,               // ENODEV    15     No such device
         ERROR_CURRENT_DIRECTORY,       // ECURDIR   16     Attempt to remove CurDir
         ERROR_NOT_SAME_DEVICE,         // ENOTSAM   17     Not same device
         ERROR_NO_MORE_FILES,           // ENMFILE   18     No more files
         ERROR_INVALID_PARAMETER,       // EINVAL    19     Invalid argument
         ERROR_BAD_ARGUMENTS,           // E2BIG     20     Arg list too long
         ERROR_BAD_FORMAT,              // ENOEXEC   21     Exec format error
         ERROR_NOT_SAME_DEVICE,         // EXDEV     22     Cross-device link
         ERROR_TOO_MANY_OPEN_FILES,     // ENFILE    23     Too many open files
         ERROR_CHILD_NOT_COMPLETE,      // ECHILD    24     No child process
         ERROR_SERIAL_NO_DEVICE,        // ENOTTY    25     UNIX - not MSDOS
         ERROR_SEEK_ON_DEVICE,          // ETXTBSY   26     UNIX - not MSDOS
         TYPE_E_SIZETOOBIG,             // EFBIG     27     UNIX - not MSDOS
         ERROR_DISK_FULL,               // ENOSPC    28     No space left on device
         ERROR_SEEK,                    // ESPIPE    29     Illegal seek
         ERROR_FILE_READ_ONLY,          // EROFS     30     Read-only file system
         ERROR_TOO_MANY_LINKS,          // EMLINK    31     UNIX - not MSDOS
         ERROR_BROKEN_PIPE,             // EPIPE     32     Broken pipe
         ERROR_BAD_ARGUMENTS,           // EDOM      33     Math argument
         TYPE_E_SIZETOOBIG,             // ERANGE    34     Result too large
         ERROR_ALREADY_EXISTS,          // EEXIST    35     File already exists
         ERROR_LOCK_VIOLATION,          // EDEADLOCK 36     Locking violation
         ERROR_ACCESS_DENIED,           // EPERM     37     Operation not permitted
         ERROR_FILE_NOT_FOUND,          // ESRCH     38     UNIX - not MSDOS
         ERROR_INVALID_AT_INTERRUPT_TIME,// EINTR     39     Interrupted function call
         ERROR_IO_DEVICE,               // EIO       40     Input/output error
         ERROR_NOT_FOUND,               // ENXIO     41     No such device or address
         ERROR_CONNECTION_UNAVAIL,      // EAGAIN    42     Resource temporarily unavailable
         WSAEWOULDBLOCK,                // ENOTBLK   43     UNIX - not MSDOS
         ERROR_BUSY,                    // EBUSY     44     Resource busy
         ERROR_DIRECTORY,               // ENOTDIR   45     UNIX - not MSDOS
         ERROR_DIRECTORY,               // EISDIR    46     UNIX - not MSDOS
         0,                             // EUCLEAN   47     UNIX - not MSDOS
         ERROR_INVALID_NAME,            // ENAMETOOLONG 48  Filename too long
         ERROR_DIR_NOT_EMPTY            // ENOTEMPTY 49     Directory not empty
       #else
       #if defined(__SYMANTEC)
         #error "Define errors mapping"
       #else
       #if defined(__MSOFT) || defined(__INTEL)
         ERROR_SUCCESS,            // EZERO           0
         ERROR_ACCESS_DENIED,      // EPERM           1
         ERROR_FILE_NOT_FOUND,     // ENOENT          2
         ERROR_PATH_NOT_FOUND,     // ESRCH           3
         ERROR_INVALID_AT_INTERRUPT_TIME,// EINTR     4
         ERROR_IO_DEVICE,          // EIO             5
         ERROR_NOT_FOUND,          // ENXIO           6
         ERROR_BAD_ARGUMENTS,      // E2BIG           7
         ERROR_BAD_FORMAT,         // ENOEXEC         8
         ERROR_INVALID_HANDLE,     // EBADF           9
         ERROR_CHILD_NOT_COMPLETE, // ECHILD          10
         ERROR_CONNECTION_UNAVAIL, // EAGAIN          11
         ERROR_NOT_ENOUGH_MEMORY,  // ENOMEM          12
         ERROR_ACCESS_DENIED,      // EACCES          13
         ERROR_GEN_FAILURE,        // EFAULT          14
         ERROR_BUSY,               // EBUSY           16
         ERROR_ALREADY_EXISTS,     // EEXIST          17
         ERROR_NOT_SAME_DEVICE,    // EXDEV           18
         ERROR_IO_DEVICE,          // ENODEV          19
         ERROR_DIRECTORY,          // ENOTDIR         20
         ERROR_DIRECTORY,          // EISDIR          21
         ERROR_INVALID_PARAMETER,  // EINVAL          22
         ERROR_TOO_MANY_OPEN_FILES,// ENFILE          23
         ERROR_TOO_MANY_OPEN_FILES,// EMFILE          24
         ERROR_SERIAL_NO_DEVICE,   // ENOTTY          25
         TYPE_E_SIZETOOBIG,        // EFBIG           27
         ERROR_DISK_FULL,          // ENOSPC          28
         ERROR_SEEK,               // ESPIPE          29
         6009L/*ERROR_FILE_READ_ONLY*/, // EROFS           30
         ERROR_TOO_MANY_LINKS,     // EMLINK          31
         ERROR_BROKEN_PIPE,        // EPIPE           32
         ERROR_BAD_ARGUMENTS,      // EDOM            33
         TYPE_E_SIZETOOBIG,        // ERANGE          34
         ERROR_LOCK_VIOLATION,     // EDEADLK         36
         0, // 37
         ERROR_INVALID_NAME,       // ENAMETOOLONG    38
         ERROR_LOCK_VIOLATION,     // ENOLCK          39
         ERROR_NO_SYSTEM_RESOURCES,// ENOSYS          40
         ERROR_DIR_NOT_EMPTY,      // ENOTEMPTY       41
         0 // EILSEQ          42
       #else
       #if defined(__DMC)
         ERROR_SUCCESS,            // EZERO           0
         ERROR_ACCESS_DENIED,      // EPERM           1
         ERROR_FILE_NOT_FOUND,     // ENOENT          2
         ERROR_PATH_NOT_FOUND,     // ESRCH           3
         ERROR_INVALID_AT_INTERRUPT_TIME,// EINTR     4
         ERROR_IO_DEVICE,          // EIO             5
         ERROR_NOT_FOUND,          // ENXIO           6
         ERROR_BAD_ARGUMENTS,      // E2BIG           7
         ERROR_BAD_FORMAT,         // ENOEXEC         8
         ERROR_INVALID_HANDLE,     // EBADF           9
         ERROR_CHILD_NOT_COMPLETE, // ECHILD          10
         ERROR_CONNECTION_UNAVAIL, // EAGAIN          11
         ERROR_NOT_ENOUGH_MEMORY,  // ENOMEM          12
         ERROR_ACCESS_DENIED,      // EACCES          13
         ERROR_GEN_FAILURE,        // EFAULT          14
         ERROR_BUSY,               // EBUSY           16
         ERROR_ALREADY_EXISTS,     // EEXIST          17
         ERROR_NOT_SAME_DEVICE,    // EXDEV           18
         ERROR_IO_DEVICE,          // ENODEV          19
         ERROR_DIRECTORY,          // ENOTDIR         20
         ERROR_DIRECTORY,          // EISDIR          21
         ERROR_INVALID_PARAMETER,  // EINVAL          22
         ERROR_TOO_MANY_OPEN_FILES,// ENFILE          23
         ERROR_TOO_MANY_OPEN_FILES,// EMFILE          24
         ERROR_SERIAL_NO_DEVICE,   // ENOTTY          25
         TYPE_E_SIZETOOBIG,        // EFBIG           27
         ERROR_DISK_FULL,          // ENOSPC          28
         ERROR_SEEK,               // ESPIPE          29
         ERROR_FILE_READ_ONLY,     // EROFS           30
         ERROR_TOO_MANY_LINKS,     // EMLINK          31
         ERROR_BROKEN_PIPE,        // EPIPE           32
         ERROR_BAD_ARGUMENTS,      // EDOM            33
         TYPE_E_SIZETOOBIG,        // ERANGE          34
         ERROR_LOCK_VIOLATION,     // EDEADLK         36
         0, // 37
         ERROR_INVALID_NAME,       // ENAMETOOLONG    38
         ERROR_LOCK_VIOLATION,     // ENOLCK          39
         ERROR_NO_SYSTEM_RESOURCES,// ENOSYS          40
         ERROR_DIR_NOT_EMPTY,      // ENOTEMPTY       41
         0 // EILSEQ          42
       #else
         #error "Unknown platform"
       #endif
       #endif
       #endif
       #endif
       };

       if ( errno >= 0 || errno < ARRAY_SIZE(errs) )
         return errs[ errno ];
        else
         return ERROR_SUCCESS;
  }

#else

  static CONSTSTR _OSErrorS( int err )
    {
    return strerror( err );
  }

  #define _FIO_ERRORN         errno
  #define _FIO_SETERRORN(v)   errno = v
#endif

static char ErrBuff[1000];
static CONSTSTR MyErrors[] = {
 /* ERROR_USER */                 ErrBuff,

 /* ERROR_SUCCESS */             "The operation completed successfully",
 /* ERROR_INVALID_PARAMETER */   "The parameter is incorrect",
 /* ERROR_CALL_NOT_IMPLEMENTED */"This function is not supported on this system",
 /* WAIT_TIMEOUT */              "The wait operation timed out",
 /* ERROR_NOT_CONNECTED */       "Operation on non connected pipe",
 /* ERROR_CRC */                 "Data block integrity check fail",
 /* ERROR_CANCELLED*/            "Operation cancelled by use",
 /* ERROR_MORE_DATA */           "Need more data to complete operatio",

//DB
 /* ERROR_DB_CURSOR */           "Bad DB cursor",
 /* ERROR_DB_FETCH */            "Error in DB fetch",
 /* ERROR_DB_OOB */              "DB move out put bounds",

//IO
 /* ERROR_CLOSED */              "Already closed",
 /* ERROR_CONNCLOSED */          "Operation on already closed pipe",
 /* ERROR_TRANSFER */            "Error transfer data",
 /* ERROR_HEADER */              "Broken data in structured header",
 /* ERROR_MAGIC */               "Bad ID magic value",
 /* ERROR_OOB */                 "Out of bounds",
 /* ERROR_SERVERSIDE */          "Server side error",
 /* ERROR_SERVERSIDEUNK */       "Unknown server side error",
 /* ERROR_RANGE */               "Value out of possible range",
 /* ERROR_DATA */                "Data block contains unexpected data",
 /* ERROR_TOOSMALL */            "Specified buffer too small",
  NULL
};

/** @brief Returns text description for error code.
    @param Error Optional error code to describe.
    @return Constant string with error description.

    If error code set to FIO_ERROR_NONE function returns description for
    last RTL error.
*/
CONSTSTR MYRTLEXP GetIOErrorS_( FIO_ERR_TYPE Error, BOOL decode )
  {
    if ( Error == FIO_ERROR_NONE || Error == ERROR_STD )
      Error = _FIO_ERRORN;

    if ( ((unsigned)Error) < MAX_ERRNUM ) {
#if defined(__HWIN32__)
      if ( decode ) {
        static char errorbuf[ MAX_PATH_SIZE ];
        CharToOem( _OSErrorS(Error),errorbuf );
        return errorbuf;
      }
#endif
      return _OSErrorS(Error);
    }

    if ( Error == ERROR_USER )
      return MyErrors[0];

    Error -= MAX_ERRNUM;
    if ( Error < ARRAY_SIZE(MyErrors) )
      return MyErrors[ Error ];

 return "Unknown error";
}

/** @brief Retuns last RTL error code.
    @return Last RTL or extended RTL error code.

    Function can return extended error codes (ERROR_xxx).
*/
FIO_ERR_TYPE MYRTLEXP GetIOErrorN( void )
  {
 return _FIO_ERRORN;
}

/** @brief Sets RTL or extended error code.
    @param ErrorValue Error code to set. This value can not be ERROR_USER value.
*/
void MYRTLEXP SetErrorN( FIO_ERR_TYPE ErrorValue )
  {
/*
    if ( ErrorValue == ERROR_STD )
      ; //Ignore set already setted value
     else
    if ( ErrorValue == ERROR_USER )
      ; //Ignore set user error
     else
*/
      _FIO_SETERRORN( ErrorValue );
}

/** @brief Sets extended RTL error.
    @param Format printf-like format string with following parameters.

    This function accept error string and set error value to ERROR_USER.
*/
void MYRTLEXP_PT SetError( CONSTSTR Format, ... )
  {  va_list a;
     char    _ErrBuff[ sizeof(ErrBuff) ];

    va_start( a, Format );
      VSNprintf( _ErrBuff,sizeof(_ErrBuff),Format,a );
    va_end( a );

    memcpy( ErrBuff, _ErrBuff, sizeof(ErrBuff) );
    _FIO_SETERRORN( ERROR_USER );
}
/** @brief Sets extended RTL error.
    @param Format printf-like format string with following parameters.

    This function accept error string and set error value to ERROR_USER.
*/
void MYRTLEXP SetErrorV( CONSTSTR Format, va_list a )
  {  char _ErrBuff[ sizeof(ErrBuff) ];

    VSNprintf( _ErrBuff,sizeof(_ErrBuff),Format,a );
    memcpy( ErrBuff, _ErrBuff, sizeof(ErrBuff) );
    _FIO_SETERRORN( ERROR_USER );
}
