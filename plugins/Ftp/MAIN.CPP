/*
 * FTP User Program -- Command Interface.
 */
#include "ftpincl.hpp"

#if defined(sun) && !defined(FD_SET)
typedef int uid_t;
#endif

int Connection::Init(const char *Host,const char *User,const char *Password)
{
  register char *cp;
  char homedir[MAXPATHLEN];
  WORD wVerReq;
  int argc;
  char *argv[1];

  WSADATA WSAData;
  struct  servent *sp;    /* service spec for tcp/ftp */

  strcpy(UserName,User);
  strcpy(UserPassword,Password);
  argv[0]=(char *)Host;
  argc=1;

  _fmode = O_BINARY; // This causes an error somewhere.

  wVerReq = MAKEWORD(1,1);

  if (WSAStartup(wVerReq, &WSAData) != 0)
    return(FALSE);
  SocketStartup=TRUE;

	sp = getservbyname("ftp", "tcp");
  if (sp == 0)
  {
//    fprintf(stderr, "ftp: ftp/tcp: unknown service\n");
    return(FALSE);
	}

  portnum = sp->s_port;

	autologin = 1;
  fromatty = 1; // Strengthen this test
  /*
	 * Set up defaults for FTP.
	 */
  type = TYPE_A;
  form = FORM_N;
  mode = MODE_S;
  stru = STRU_F;
  strcpy(bytename, "8"), bytesize = 8;
	if (fromatty)
		verbose++;
  cpend = 0;  /* no pending replies */
	proxy = 0;	/* proxy not active */

//  crflag = 1; /* strip c.r. on ascii gets */
  crflag=0;

  if (argc > 0)
    if (!setpeer(argc, argv))
      return(FALSE);
  return(TRUE);
}


void Connection::lostpeer()
{
  if (connected) {
    AbortAllRequest(0);
    cout = NULL;
    connected = 0;
  }
  pswitch(1);
  if (connected) {
    AbortAllRequest(0);
    cout = NULL;
    connected = 0;
  }
  proxflag = 0;
  pswitch(0);
}

/*
 * Command parser.
 */
int Connection::ProcessCommand(char *LineToProcess)
{
	register struct cmd *c;

  ResetOutput();
  strcpy(line,LineToProcess);

  makeargv();
  if (margc == 0)
    return(FALSE);
  c = getcmd(margv[0]);
  if (c==(struct cmd *)-1 || c==0)
    return(FALSE);
  if (c->c_conn && !connected)
  {
    SetLastError(ERROR_INTERNET_CONNECTION_ABORTED);
    return(FALSE);
  }
  code=0;
  ErrorCode=0;
  ExecCmdTab(c, margc, margv);
  return(GetExitCode());
}


struct cmd *Connection::getcmd(register char *name)
{
	register char *p, *q;
	register struct cmd *c, *found;
	register int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
  for (c = cmdtab; (p = c->c_name)!=NULL; c++) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return (c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return ((struct cmd *)-1);
	return (found);
}

/*
 * Slice a string up into argc/argv.
 */


void Connection::makeargv()
{
	char **argp;
	margc = 0;
	argp = margv;
	stringbase = line;		/* scan from first of buffer */
	argbase = argbuf;		/* store from first of buffer */
	slrflag = 0;
  while ((*argp++ = slurpstring())!=0)
		margc++;
}

/*
 * Parse string into argbuf;
 * implemented with FSM to
 * handle quoting and strings
 */
char *Connection::slurpstring()
{
	int got_one = 0;
	register char *sb = stringbase;
	register char *ap = argbase;
	char *tmp = argbase;		/* will return this if token found */

	if (*sb == '!' || *sb == '$') {	/* recognize ! as a token for shell */
		switch (slrflag) {	/* and $ as token for macro invoke */
			case 0:
				slrflag++;
				stringbase++;
				return ((*sb == '!') ? "!" : "$");
				/* NOTREACHED */
			case 1:
				slrflag++;
				altarg = stringbase;
				break;
			default:
				break;
		}
	}

S0:
	switch (*sb) {

	case '\0':
		goto OUT1;

	case ' ':
	case '\t':
		sb++; goto S0;

	default:
		switch (slrflag) {
			case 0:
				slrflag++;
				break;
			case 1:
				slrflag++;
				altarg = sb;
				break;
			default:
				break;
		}
		goto S1;
	}

S1:
	switch (*sb) {

	case ' ':
	case '\t':
	case '\0':
		goto OUT1;	/* end of token */

	case '\\':
		sb++; goto S2;	/* slurp next character */

	case '"':
		sb++; goto S3;	/* slurp quoted string */

	default:
		*ap++ = *sb++;	/* add character to token */
		got_one = 1;
		goto S1;
	}

S2:
	switch (*sb) {

	case '\0':
		goto OUT1;

	default:
		*ap++ = *sb++;
		got_one = 1;
		goto S1;
	}

S3:
	switch (*sb) {

	case '\0':
		goto OUT1;

	case '"':
		sb++; goto S1;

	default:
		*ap++ = *sb++;
		got_one = 1;
		goto S3;
	}

OUT1:
	if (got_one)
		*ap++ = '\0';
	argbase = ap;			/* update storage pointer */
	stringbase = sb;		/* update scan pointer */
	if (got_one) {
		return(tmp);
	}
	switch (slrflag) {
		case 0:
			slrflag++;
			break;
		case 1:
			slrflag++;
			altarg = (char *) 0;
			break;
		default:
			break;
	}
	return((char *)0);
}
