#ifndef __STD_LIB_ALL
#define __STD_LIB_ALL

/** @page Platform Platfom definitions

    Library uses next platform definitions:
      - __BORLAND
      - __BCB1__
      - __VCL__

      - __HWIN__
      - __HDOS__
      - __REALDOS__
      - __PROTDOS__

      - __MSWIN32__
      - __BCWIN32__
      - __MSOFT
      - __TEC32__
      - __SYMANTEC
      - __INTEL

      - __DEBUG__
      - __HCONSOLE__
*/
#include <Global/platform.h>

/***************************************
            STD HEADERS
 ***************************************/
#if defined(__GNUC__)
    #include <sys/param.h>
    #include <sys/stat.h>
    #include <glob.h>
    #include <term.h>               // __cur_term
    #include <termios.h>            // tcdrain
    #include <sys/types.h>          // time_t, struct tm, ...
    #include <sys/wait.h>           // wait
    #include <dirent.h>             // DIR, fsys_stat, ...
    #include <unistd.h>             // access, StdIO...
    #include <pwd.h>                // getpwuid
    #include <grp.h>                // getgrgid
    #include <semaphore.h>          // CRITICAL_SECTION
//SOCKET`s
    #include <netdb.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <sys/select.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
#else                                                                                             //!! REALDOS
#if defined(__QNX__)                                                                              //!! QNX
  #if !defined( CONSOLE_ONLY )
    #include <sys/qnxterm.h>        // term_xx
    #include <term.h>               // __cur_term
  #endif
    #include <termios.h>            // tcdrain
    #include <sys/types.h>          // time_t, struct tm, ...
    #include <sys/kernel.h>         // Yield
    #include <sys/wait.h>           // wait
    #include <sys/qioctl.h>         // qnx_ioctl
    #include <sys/dir.h>            // file IO
    #include <sys/fsys.h>           // fsys_get_mount_XXX
    #include <sys/proxy.h>          // qnx_proxy_attach
    #include <sys/kernel.h>         // Creceive
    #include <sys/irqinfo.h>        // qnx_hint_attach
    #include <sys/osinfo.h>         // qnx_osinfo
    #include <sys/name.h>           // qnx_net_alive
    #include <sys/vc.h>             // qnx_nidtostr
    #include <sys/psinfo.h>         // qnx_psinfo
    #include <dirent.h>             // DIR, fsys_stat, ...
    #include <unistd.h>             // access, StdIO...
    #include <pwd.h>                // getpwuid
    #include <grp.h>                // getgrgid
    #include <semaphore.h>          // CRITICAL_SECTION
    #include <sys/sched.h>          // getprio
    #include <sys/fd.h>             // qnx_fd_query
    #include <sys/sendmx.h>         // Sendfdmx
    #include <sys/io_msg.h>         // _io_select, _select_set
//SOCKET`s
    #include <netdb.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <sys/select.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
#else                                                                                             //!! REALDOS
#if defined(__REALDOS__)
    #include <dos.h>
    #include <dir.h>                // file IO
    #include <io.h>                 // struct ftime
    #include <direct.h>             // _chdrive
    #include <sys/timeb.h>          // ftime
#else
#if defined(__PROTDOS__)                                                                          //!! SC DOSX
    #include <dos.h>
    #include <dir.h>                // file IO
    #include <io.h>                 // struct ftime
    #include <direct.h>             // _chdrive
    #include <sound.h>              // sound_beep
    #include <signal.h>             // signal
#else
#if defined(__HWIN__)                                                                             //!! WIN32
    #include <windows.h>
    #include <direct.h>             // _chdir
    #include <dos.h>                // _argv
    #include <io.h>                 // struct ftime, SEEK_xxx
    #include <timeapi.h>
  #if !defined(__MSOFT) && !defined(__INTEL)
    #include <dir.h>                // file IO
  #endif
//SOCKET
    #include <winsock.h>
  #if defined(__VCL__)
    #include <vcl/vcl.h>
    #include <vcl/comctrls.hpp>
    #include <vcl/typinfo.hpp>
    #include <vcl/registry.hpp>
  #endif
#else
#if defined(__TEC32__)
    #include <limits.h>
    #include <locale.h>
    #include <stddef.h>
#else
    #error ERR_PLATFORM
#endif
#endif //Win32
#endif //SC DOSX
#endif //MSDOS
#endif //QNX
#endif //UNIX

#include <signal.h>             // SIGxxx
#include <math.h>               // sqrt
#include <ctype.h>              // isprint
#include <stdio.h>              // Sprintf
#include <stdlib.h>             // atexit
#include <string.h>             // strXXX
#include <setjmp.h>             // exseptions
#include <assert.h>             // assert
#include <errno.h>              // errors
#include <stdarg.h>             // va_list
#include <time.h>               // timespec

#if !defined(__TEC32__)
  #include <sys/stat.h>           // stat
  #include <fcntl.h>              // O_RDWR,xxx
  #if !defined(__HUNIX__)
    #include <malloc.h>             // alloc,NULL
    #include <conio.h>              // getch
    #include <process.h>            // system,signal
  #endif
#endif

#if defined(__TEC32__)
  //Multi has memXXX functions in "string.h"
#else
  #if defined(__GNUC__)
    #include <memory.h>           // memXXXX
    #include <utime.h>        // utime
  #else
  #if defined(__MSOFT) || defined(__INTEL)   //MSOFT differ from other platforms
    #include <memory.h>           // memXXXX
    #include <sys/utime.h>        // utime
  #else
    #include <mem.h>              // memXXXX
    #include <utime.h>            // utime
  #endif
  #endif
#endif

/***************************************
              MY HEADERS
 ***************************************/
#include <Global/pack1.h>

//DEFINES and platform MAP`s
#include <Global/defs.h>

#if defined(__VCL__)
  #include <Global/vcl_defs.h>
#endif

#if defined(__TEC32__)
#include <Std/io_TEC32.h>
#endif

#include <Std/xxDefs.h>
#include <Std/disk_err.h>
#include <Std/disk_iof.h>

//STD, TYPES and CONSTS
#include <Std/m_enums.h>
#include <Std/m_types.h>
#include <Std/scankeys.h>
#include <Std/m_std.h>
#include <Std/m_assert.h>
#include <Std/chknew.h>
#include <Std/fielddef.h>

#if defined(__cplusplus)
  #include <Std/m_tmp.h>
  #include <Std/mutils.h>
  #include <Std/mclasses.h>
  #include <Std/dataptr.h>
#endif

#if defined(__cplusplus) && !defined(__TEC32__)
  #include <Std/disk_io.h>
#endif

#if defined(__cplusplus) && !defined(__TEC32__)
  #include <Std/threads.h>
  #include <Std/sock.h>
  #include <Std/tr_sock.h>
#endif

#if defined(__cplusplus)
  #include <Std/period.h>
#endif

#if defined(__cplusplus)
  #include <Std/htime.h>
  #include <Std/cs.h>
  #include <Std/SQLTypes.h>
  #include <Std/hstream.h>
  #include <Std/plog.h>
#endif

#if !defined(__TEC32__) && defined(__cplusplus)
  #include <Std/2asm.h>
  #include <Std/lzh.h>
  #include <Std/hvalue.h>
  #include <Std/hbtree.h>
  #include <Std/hdpool.h>
  #include <Std/htree.h>
  #include <Std/hregtree.h>
  #include <Std/HTreeCfg.h>
  #include <Std/fhold.h>
  #include <Std/parsr.h>
  #include <Std/xml.h>
#endif

#if defined(__cplusplus)
  #include <Std/hdbase.h>
  #include <Std/HThread.h>
  #include <Std/HEvent.h>
  #include <Std/HSend.h>
#endif

#if !defined(__TEC32__) && defined(__cplusplus)
  #include <Std/xxExcept.h>
  #include <Std/db_math.h>
#endif

#include <Global/pop.h>

#endif
