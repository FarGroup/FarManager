#include "fake.hpp"
#include <windows.h>
#include <winsock.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <dir.h>
#include <dos.h>

#define L_SET SEEK_SET
#define L_INCR SEEK_CUR
#define caddr_t void *

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

#ifdef NOVFPRINTF
#define vfprintf(a,b,c) _doprnt(b,c,a)
#endif

#ifdef sun
/* FD_SET wasn't defined until 4.0. its a cheap test for uid_t  presence */
#ifndef FD_SET
#define	NBBY	8		/* number of bits in a byte */
/*
 * Select uses bit masks of file descriptors in longs.
 * These macros manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here
 * should be >= NOFILE (param.h).
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	256
#endif

typedef long  fd_mask;

#define NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))

typedef int uid_t;
typedef int gid_t;
#endif
#endif

/* The following defines are from ftp.h and telnet.h from bsd.h */
/* All relevent copyrights below apply.                         */

#define	IAC	255
#define	DONT	254
#define	DO	253
#define	WONT	252
#define	WILL	251
#define	SB	250
#define	GA	249
#define	EL	248
#define	EC	247
#define	AYT	246
#define	AO	245
#define	IP	244
#define	BREAK	243
#define	DM	242
#define	NOP	241
#define	SE	240
#define EOR     239
#define	ABORT	238
#define	SUSP	237
#define	xEOF	236


#define MAXPATHLEN 255
#define TYPE_A 'A'
#define TYPE_I 'I'
#define TYPE_E 'E'
#define TYPE_L 'L'

#define PRELIM		1
#define COMPLETE	2
#define CONTINUE	3
#define TRANSIENT	4

#define	MODE_S		1
#define	MODE_B		2
#define	MODE_C		3

#define	STRU_F		1
#define	STRU_R		2
#define	STRU_P		3

#define SIGQUIT 1
#define SIGPIPE 2
#define SIGALRM 3


#define	FORM_N		1
#define	FORM_T		2
#define	FORM_C		3


/*
 * Format of command table.
 */
struct cmd {
	char	*c_name;	/* name of command */
	char	c_conn;		/* must be connected to use command */
	char	c_proxy;	/* proxy server may execute */
};

