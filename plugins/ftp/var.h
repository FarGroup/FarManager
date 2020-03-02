#ifndef __FAR_PLUGIN_FTP_VARS
#define __FAR_PLUGIN_FTP_VARS

/* The following defines are from ftp.h and telnet.h from bsd.h */
/* All relevent copyrights below apply.                         */

#define ffIAC     255
#define ffDONT    254
#define ffDO      253
#define ffWONT    252
#define ffWILL    251
#define ffIP      244
#define ffDM      242

#define ffEOF     0x0236000


#define RPL_OK             0
#define RPL_PRELIM         1
#define RPL_COMPLETE       2
#define RPL_CONTINUE       3
#define RPL_TRANSIENT      4
#define RPL_BADPASS        5
#define RPL_ERROR          -1
#define RPL_TRANSFERERROR  -2
#define RPL_TIMEOUT        3000

#endif
