#include "ftpincl.hpp"

extern struct PluginStartupInfo Info;
// Signal Handlers

char* Connection::hookup(char *host, int port)
{
  register struct hostent *hp = 0;
  int len;
  static char hostnamebuf[80];

  bzero((char *)&hisctladdr, sizeof (hisctladdr));
  hisctladdr.sin_addr.s_addr = inet_addr(host);
  if (hisctladdr.sin_addr.s_addr != -1)
  {
    hisctladdr.sin_family = AF_INET;
    strncpy(hostnamebuf, host, sizeof(hostnamebuf));
  }
  else
  {
    hp = gethostbyname(host);
    if (hp == NULL || brk_flag)
    {
abrtr:
      code = -1;
      return(NULL);
    }
    hisctladdr.sin_family = hp->h_addrtype;
    bcopy(hp->h_addr_list[0],(caddr_t)&hisctladdr.sin_addr, hp->h_length);
    strncpy(hostnamebuf, hp->h_name, sizeof(hostnamebuf));
  }
  hostname = hostnamebuf;
  hisctladdr.sin_port = port;

  for( ; ; ) {
    if((cmd_peer = socket(hisctladdr.sin_family, SOCK_STREAM, 0)) < 0) goto abrtr;
    if(connect(cmd_peer, (struct sockaddr *)&hisctladdr, sizeof (hisctladdr))
       >= 0) break;
    if(brk_flag) goto abrtr;
    if (hp && hp->h_addr_list[1])
    {
      int oerrno = errno;

//      fprintf(stderr, "ftp: connect to address %s: ",
//        inet_ntoa(hisctladdr.sin_addr));
      errno = oerrno;
      hp->h_addr_list++;
      bcopy(hp->h_addr_list[0],(caddr_t)&hisctladdr.sin_addr, hp->h_length);
//      fprintf(stdout, "Trying %s...\n",
//        inet_ntoa(hisctladdr.sin_addr));
      closesocket(cmd_peer);
      cmd_peer = -1;
      continue;
    }
    code = -1;
    goto bad;
  }
  len = sizeof (myctladdr);
  if (getsockname(cmd_peer, (struct sockaddr *)&myctladdr, &len) < 0)
  {
    code = -1;
    goto bad;
  }
  cin = cout = cmd_peer;
//  if (verbose)
//    printf("Connected to %s.\n", hostname);
  if (getreply(0) > 2) {  /* read startup message from server */
    closesocket(cin);
    cmd_peer = -1;
    cin = 0;
    code = -1;
    goto bad;
  }
#ifdef SO_OOBINLINE
  {
    int on = 1;
    setsockopt(cmd_peer, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on));
  }
#endif SO_OOBINLINE

  return (hostname);
bad:
  closesocket(cmd_peer);
  cmd_peer = -1;
  return ((char *)0);
}


int Connection::login(char *host)
{
  char tmp[80];
  char *user, *pass, *acct=NULL;
  int n, aflag = 0;

  user = *UserName ? UserName:"anonymous";

  ConnectMessage(1);
  n = command("USER %s", user);
  if (n == CONTINUE)
  {
    pass = UserPassword;
    ConnectMessage(2);
    n = command("PASS %s", pass);
  }
  if (n == CONTINUE)
  {
    aflag++;
    acct = "";
    n = command("ACCT %s", acct);
  }
  if (n != COMPLETE)
  {
    SetLastError(ERROR_INTERNET_LOGIN_FAILURE);
    return (0);
  }
  if (!aflag && acct != NULL)
    command("ACCT %s", acct);
  return (1);
}

void Connection::cmdabort()
{
  abrtflag++;
  if (ptflag)
    longjmp(ptabort,1);
}

/*VARARGS1*/
int Connection::command(char *fmt, ...)
{
  va_list ap;
  int r;

  abrtflag = 0;
  if (cout == NULL)
  {
//    perror ("No control connection for command");
    code = -1;
    return (0);
  }
  {
    char buffer[1024];

    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);
    va_end(ap);
    strcat(buffer,"\r\n");
    fputsSocket(buffer, cout);
  }
  cpend = 1;
  r = getreply(!strcmp(fmt, "QUIT"));
  while ((strcmp(fmt,"PWD")==0 && (strchr(reply_string,'\"')==NULL ||
         !isdigit(reply_string[0]) || !isdigit(reply_string[1]) ||
         !isdigit(reply_string[2]) || !isspace(reply_string[3]))) && !brk_flag)
    r = getreply(FALSE);
  return(r);
}


int Connection::getreply(int expecteof)
{
  register int c, n;
  register int dig;
  register char *cp;
  int originalcode = 0, continuation = 0;
  int pflag = 0;
  char *pt = pasv;

  for (;;) {
    dig = n = code = 0;
    cp = reply_string;
    while ((c = fgetcSocket(cin)) != '\n') {
      if (c == IAC) {     /* handle telnet commands */
        switch (c = fgetcSocket(cin)) {
        case WILL:
        case WONT:
          c = fgetcSocket(cin);
          fprintfSocket(cout, "%c%c%c",IAC,DONT,c);
          break;
        case DO:
        case DONT:
          c = fgetcSocket(cin);
          fprintfSocket(cout, "%c%c%c",IAC,WONT,c);
          break;
        default:
          break;
        }
        continue;
      }
      dig++;
      if (c == EOF)
      {
        if (expecteof)
        {
          code = 221;
          return (0);
        }
        lostpeer();
        code = 421;
        return(4);
      }
//      if (c != '\r' && (verbose > 0 || (verbose > -1 && n == '5' && dig > 4)))
//      {
//        if (proxflag && (dig == 1 || dig == 5 && verbose == 0))
//          printf("%s:",hostname);
//        putchar(c);
//      }
      if (dig < 4 && isdigit(c))
        code = code * 10 + (c - '0');
      if (!pflag && code == 227)
        pflag = 1;
      if (dig > 4 && pflag == 1 && isdigit(c))
        pflag = 2;
      if (pflag == 2) {
        if (c != '\r' && c != ')')
          *pt++ = c;
        else {
          *pt = '\0';
          pflag = 3;
        }
      }
      if (dig == 4 && c == '-') {
        if (continuation)
          code = 0;
        continuation++;
      }
      if (n == 0)
        n = c;
      if (cp < &reply_string[sizeof(reply_string) - 1])
        *cp++ = c;
    }
//    if (verbose > 0 || verbose > -1 && n == '5')
//    {
//      putchar(c);
//      fflush (stdout);
//    }
    *cp = '\0';
    if (*StartReply==0)
      strncpy(StartReply,reply_string,sizeof(StartReply));

    extern char LastReply1[200],LastReply2[200];

    strncpy(LastReply1,LastReply2,sizeof(LastReply1));
    strncpy(LastReply2,reply_string,sizeof(LastReply2));
    if (continuation && code != originalcode)
    {
      if (originalcode == 0)
        originalcode = code;
      continue;
    }
    if (n != '1')
      cpend = 0;
    if (code == 421 || originalcode == 421)
      lostpeer();
    return (n - '0');
  }
}


int Connection::empty(struct fd_set *mask, int sec)
{
  struct timeval t;

  t.tv_sec = (long) sec;
  t.tv_usec = 0;
  return(select(32, mask, (struct fd_set *) 0, (struct fd_set *) 0, &t));
}


void Connection::abortsend()
{
  mflag = 0;
  abrtflag = 0;
  longjmp(sendabort, 1);
}

#define HASHBYTES 1024

void Connection::sendrequest(char *cmd, char *local, char *remote, int printnames)
{
  if (type==TYPE_A)
    restart_point=0;

  FILE *fin;
  int dout = 0;
  int (*closefunc)(FILE *);
  char buf[BUFSIZ], *bufp;
  long bytes = 0, hashbytes = HASHBYTES;
  long FileRestartPoint=restart_point;
  register int c, d;
  struct stat st;
  struct timeval start, stop;
  char *mode;

  if (proxy)
  {
    proxtrans(cmd, local, remote);
    return;
  }
  closefunc = NULL;
  mode = "w";
  if (setjmp(sendabort))
  {
    while (cpend)
    {
      getreply(0);
    }
    if (data_peer >= 0)
    {
      closesocket(data_peer);
      data_peer = -1;
    }
    code = -1;
    return;
  }
  if (strcmp(local, "-") == 0)
    fin = stdin;
  else
    if (*local == '|')
    {
      fin = _popen(local + 1, "r");
      if (fin == NULL)
      {
        code = -1;
        return;
      }
      closefunc = _pclose;
    }
    else
    {
      fin = fopen(local, "r");
      if (fin == NULL)
      {
        code = -1;
        return;
      }
      closefunc = fclose;
      if (fstat(fileno(fin), &st) < 0 || (st.st_mode&S_IFMT) != S_IFREG)
      {
//        fprintf(stdout, "%s: not a plain file.\n", local);
        fclose(fin);
        code = -1;
        return;
      }
    }
  if (initconn())
  {
    code = -1;
    if (closefunc != NULL)
      (*closefunc)(fin);
    return;
  }
  if (setjmp(sendabort))
    goto abort;

  if (restart_point && (strcmp(cmd, "STOR") == 0 || strcmp(cmd, "APPE") == 0))
  {
    if (fseek(fin, (long) restart_point, 0) < 0)
    {
      restart_point = 0;
      if (closefunc != NULL)
        (*closefunc)(fin);
      return;
    }
    if (strcmp(cmd, "APPE") != 0)
      if (command("REST %ld", (long) restart_point)!= CONTINUE)
      {
        restart_point = 0;
        if (closefunc != NULL)
          (*closefunc)(fin);
        return;
      }
    restart_point = 0;
    mode = "r+";
  }
  if(PassiveMode)
  {
    dout = dataconn(mode);
    if(dout == NULL)
      goto abort;
  }
  if (remote) {
    if (command("%s %s", cmd, remote) != PRELIM)
    {
      if (closefunc != NULL)
        (*closefunc)(fin);
      return;
    }
  }
  else
    if (command("%s", cmd) != PRELIM)
    {
      if (closefunc != NULL)
        (*closefunc)(fin);
      return;
    }
  if(!PassiveMode)
  {
    dout = dataconn(mode);
    if (dout == NULL)
        goto abort;
  }
  (void) gettimeofday(&start, (struct timezone *)0);

  switch (type)
  {
  case TYPE_I:
  case TYPE_L:
  case TYPE_A:
    errno = d = 0;
    if (PutCallback)
      PutCallback(FileRestartPoint);
    while ((c = read(fileno(fin), buf, sizeof (buf))) > 0)
    {
      bytes += c;
      for (bufp = buf; c > 0; c -= d, bufp += d)
      {

#ifdef DEBUGSLOW
        Sleep(100);
#endif
        if ((d = send(dout, bufp, c, 0)) <= 0)
          break;
      }
      if (PutCallback && bytes > 0)
        if (!PutCallback(bytes+FileRestartPoint))
        {
          ErrorCode=ERROR_CANCELLED;
          goto abort;
        }
    }
    if (PutCallback && bytes > 0)
      PutCallback(bytes+FileRestartPoint);
//    if (c < 0)
//      perror(local);
    if (d <= 0 && bytes>0)
    {
//      if (d == 0)
//        fprintf(stderr, "netout: write returned 0?\n");
//      else
//        if (errno != EPIPE)
//          perror("netout");
      ErrorCode=ERROR_DISK_FULL;
      bytes = -1;
    }
    break;
/*  case TYPE_A:
    if (PutCallback)
      PutCallback(FileRestartPoint);
    while ((c = getc(fin)) != EOF)
    {
      if (c == '\n')
      {
//        while (hash && (bytes >= hashbytes))
//        {
//          putchar('#');
//          fflush(stdout);
//          hashbytes += HASHBYTES;
//        }
      }
      fputcSocket(dout, c);
      bytes++;
      if (PutCallback && (bytes & 0x3ff))
        if (!PutCallback(bytes+FileRestartPoint))
        {
          ErrorCode=ERROR_CANCELLED;
          goto abort;
        }
    }
    if (PutCallback && bytes > 0)
      PutCallback(bytes+FileRestartPoint);
//    if (hash)
//    {
//      if (bytes < hashbytes)
//        putchar('#');
//      putchar('\n');
//      fflush(stdout);
//    }
//    if (ferror(fin))
//      perror(local);
    break;
*/
  }
  gettimeofday(&stop, (struct timezone *)0);
  if (closefunc != NULL)
    (*closefunc)(fin);
  if(data_peer != -1)
  {
    shutdown(data_peer, 1/*SD_SEND*/);
    if(getreply(0) > 2) {
      ErrorCode = ERROR_WRITE_FAULT;
      code = -1;
    }
    closesocket(data_peer);
    data_peer = -1;
  } else getreply(0);
  return;
abort:
   gettimeofday(&stop, (struct timezone *)0);
  if (!cpend)
  {
    code = -1;
    return;
  }
  if(data_peer != -1) {
    closesocket(data_peer);
    data_peer = -1;
  }
  getreply(0);
  code = -1;
  if (closefunc != NULL && fin != NULL)
    (*closefunc)(fin);
}


void Connection::abortrecv()
{
  mflag = 0;
  abrtflag = 0;
  longjmp(recvabort, 1);
}


void Connection::recvrequest(char *cmd, char *local, char *remote, char *mode,
                             int printnames)
{
  if (type==TYPE_A)
    restart_point=0;

  FILE *fout;
  int din = 0;
  int (*closefunc)(FILE *);
  int oldverbose, oldtype = 0, is_retr, tcrflag, nfnd;
  char msg;
//  static char *buf; // Szurgot: Shouldn't this go SOMEWHERE?
  char buf[1024];
  int bufsize = 1024;
  long bytes = 0, hashbytes = HASHBYTES;
  struct fd_set mask;
  register int c, d;
  struct timeval start, stop;
  struct stat st;

  is_retr = strcmp(cmd, "RETR") == 0;
  if (proxy && is_retr)
  {
    proxtrans(cmd, local, remote);
    return;
  }
  closefunc = NULL;
  tcrflag = !crflag && is_retr;
  if (setjmp(recvabort))
  {
    while (cpend)
    {
      getreply(0);
    }
    if (data_peer >= 0)
    {
      closesocket(data_peer);
      data_peer = -1;
    }
    code = -1;
    return;
  }
  if (initconn())
  {
    code = -1;
    return;
  }
  if (setjmp(recvabort))
    goto abort;
  if (!is_retr)
  {
    if (type != TYPE_A && (allbinary == 0 || type != TYPE_I))
    {
      oldtype = type;
      oldverbose = verbose;
      verbose = 0;
      setascii();
      verbose = oldverbose;
    }
  }
  else
    if (restart_point)
    {
      if (command("REST %ld", (long) restart_point) != CONTINUE)
        return;
    }
  if(PassiveMode)
  {
    din = dataconn("r");
    if(din == NULL)
      goto abort;
  }
  if (remote)
  {
    if (command("%s %s", cmd, remote) != PRELIM)
    {
      if (oldtype)
      {
        verbose = 0;
        switch (oldtype)
        {
          case TYPE_I:
            setbinary();
            break;
          case TYPE_E:
            setebcdic();
            break;
          case TYPE_L:
            settenex();
            break;
        }
        verbose = oldverbose;
      }
      return;
    }
  }
  else
  {
    if (command("%s", cmd) != PRELIM)
    {
      if (oldtype)
      {
        verbose = 0;
        switch (oldtype)
        {
          case TYPE_I:
            setbinary();
            break;
          case TYPE_E:
            setebcdic();
            break;
          case TYPE_L:
            settenex();
            break;
        }
        verbose = oldverbose;
      }
      return;
    }
  }
  if(!PassiveMode)
  {
    din = dataconn("r");
    if (din == NULL)
      goto abort;
  }
  if (strcmp(local, "-") == 0)
    fout = stdout;
  else if (*local == '|')
  {
    fout = _popen(local + 1, "w");
    if (fout == NULL)
      goto abort;
    closefunc = _pclose;
  }
  else
  {
    fout = fopen(local, mode);
    if (fout == NULL)
      goto abort;
    closefunc = fclose;
  }
  gettimeofday(&start, (struct timezone *)0);
  switch (type) {

  case TYPE_A:
  case TYPE_I:
  case TYPE_L:
    if (restart_point && lseek(fileno(fout), (long) restart_point, L_SET) < 0)
    {
      if (closefunc != NULL)
        (*closefunc)(fout);
      return;
    }
    errno = d = 0;

    if (GetCallback)
      if (!GetCallback(restart_point))
      {
        ErrorCode=ERROR_CANCELLED;
        goto abort;
      }
    while ((c = recv(din, buf, bufsize, 0)) > 0)
    {
#ifdef DEBUGSLOW
      Sleep(100);
#endif
      if (fout==stdout)
        AddOutput(buf, c);
      else
        if (write(fileno(fout), buf, c) != c)
        {
          ErrorCode=GetLastError();
          goto abort;
        }
      bytes += c;
      if (GetCallback && bytes > 0)
        if (!GetCallback(bytes+restart_point))
        {
          ErrorCode=ERROR_CANCELLED;
          goto abort;
        }
    }
    if (GetCallback && bytes > 0)
      GetCallback(bytes+restart_point);
    break;
  }
  if (closefunc != NULL)
    (*closefunc)(fout);
  gettimeofday(&stop, (struct timezone *)0);
  if(data_peer != -1)
  {
    closesocket(data_peer);
    data_peer = -1;
  }
  getreply(0);
  if (oldtype)
  {
    verbose = 0;
    switch (oldtype)
    {
      case TYPE_I:
        setbinary();
        break;
      case TYPE_E:
        setebcdic();
        break;
      case TYPE_L:
        settenex();
        break;
    }
    verbose = oldverbose;
  }
  return;
abort:
/* abort using RFC959 recommended IP,SYNC sequence  */

  gettimeofday(&stop, (struct timezone *)0);
  if (oldtype)
  {
    verbose = 0;
    switch (oldtype)
    {
      case TYPE_I:
        setbinary();
        break;
      case TYPE_E:
        setebcdic();
        break;
      case TYPE_L:
        settenex();
        break;
    }
    verbose = oldverbose;
  }
  if (!cpend)
  {
    code = -1;
    return;
  }

  fprintfSocket(cout,"%c%c",IAC,IP);
  msg = IAC;
/* send IAC in urgent mode instead of DM because UNIX places oob mark */
/* after urgent byte rather than before as now is protocol            */
  if (send(cout,&msg,1,MSG_OOB) != 1)
    /*perror("abort")*/;
  fprintfSocket(cout,"%cABOR\r\n",DM);

//  fprintfSocket(cout,"ABOR\r\n");

  FD_ZERO(&mask);
  FD_SET(cin, &mask); // Need to correct this
  if (din)
  {
    FD_SET(din, &mask); // Need to correct this
  }

  if ((nfnd = empty(&mask,10)) <= 0)
  {
    if (nfnd < 0)
      /*perror("abort")*/;
    code = -1;
    lostpeer();
  }


/* Removed by me
  if (din && FD_ISSET(din, &mask)) {
    while ((c = recv(din, buf, bufsize, 0)) > 0)
      ;
  }
*/

  if ((c = getreply(0)) == ERROR && code == 552) { /* needed for nic style abort */
    if (data_peer >= 0)
    {
      closesocket(data_peer);
      data_peer = -1;
    }
    getreply(0);
  }

  getreply(0);

  code = -1;
  if (data_peer >= 0) {
    closesocket(data_peer);
    data_peer = -1;
  }
  if (closefunc != NULL && fout != NULL)
    (*closefunc)(fout);

  if (data_peer != -1)
  {
    closesocket(data_peer);
    data_peer = -1;
  }
}

/*
 * Need to start a listen on the data channel
 * before we send the command, otherwise the
 * server's connect may fail.
 */
int Connection::initconn()
{
  register char *p, *a;
  int result, len, tmpno = 0;
  int on = 1;

noport:
  data_addr = myctladdr;
  if (sendport)
    data_addr.sin_port = 0; /* let system pick one */
  if (data_peer != -1)
  {
    closesocket(data_peer);
    data_peer = -1;
  }
  if(brk_flag || (data_peer = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
//    perror("ftp: socket");
    if (tmpno)
      sendport = 1;
    return (1);
  }
  if (!sendport)
    if (setsockopt(data_peer, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof (on)) < 0)
    {
//      perror("ftp: setsockopt (reuse address)");
      goto bad;
    }

  if(!PassiveMode || !sendport)
  {
    if (bind(data_peer, (struct sockaddr *)&data_addr, sizeof (data_addr)) < 0)
    {
//      perror("ftp: bind");
      goto bad;
    }
    if (options & SO_DEBUG &&
        setsockopt(data_peer, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof (on)) < 0)
      /*perror("ftp: setsockopt (ignored)")*/;
    len = sizeof (data_addr);
    if (getsockname(data_peer, (struct sockaddr *)&data_addr, &len) < 0) {
//      perror("ftp: getsockname");
      goto bad;
    }
    if (listen(data_peer, 1) < 0)
      /*perror("ftp: listen")*/;
  }
  if (sendport)
  {
    result = PassiveMode ?
        command("PASV") : command("PORT %d,%d,%d,%d,%d,%d",
                                   data_addr.sin_addr.S_un.S_un_b.s_b1,
                                   data_addr.sin_addr.S_un.S_un_b.s_b2,
                                   data_addr.sin_addr.S_un.S_un_b.s_b3,
                                   data_addr.sin_addr.S_un.S_un_b.s_b4,
                                   (unsigned char)data_addr.sin_port,
                                   (unsigned char)(data_addr.sin_port>>8));
    if (result == ERROR && sendport == -1)
    {
      sendport = 0;
      tmpno = 1;
      goto noport;
    }
    if(result!=COMPLETE) return(1);
    if(PassiveMode) {
      extern char LastReply2[200];
      char *p;
      if(code != 227 || (p=strchr(LastReply2,'('))==NULL) return(1);
      unsigned a1, a2, a3, a4, p1, p2, n;
#define TOP(a) (a&~0xFF)
      if((n=sscanf(p, "(%u,%u,%u,%u,%u,%u)", &a1, &a2, &a3, &a4, &p1, &p2))
         !=6 || TOP(a1) || TOP(a2) || TOP(a3) ||
         TOP(a4) || TOP(p1) || TOP(p2)) return(1);
      data_addr.sin_addr.s_addr = (a4<<24) | (a3<<16) | (a2<<8) | a1;
      data_addr.sin_port = (p2<<8) | p1;
      if(!data_addr.sin_addr.s_addr || data_addr.sin_addr.s_addr == -1) return(1);
      if(data_addr.sin_addr.s_addr != hisctladdr.sin_addr.s_addr)
      {
        char Msg[200];
        sprintf(Msg,"[%s -> %s]",
                inet_ntoa(hisctladdr.sin_addr),inet_ntoa(data_addr.sin_addr));
        Info.Text(0,0,0x1f,Msg);
        Info.Text(0,0,0,NULL);
      }
    }
    return (0);
  }
  if (tmpno)
    sendport = 1;
  return (0);
bad:
  if(data_peer != -1)
  {
    closesocket(data_peer);
    data_peer = -1;
  }
  if (tmpno)
    sendport = 1;
  return (1);
}


int Connection::dataconn(char *mode)
{
  struct sockaddr_in from;
  int s;

  if(brk_flag) return(0);
  if(!PassiveMode)
  {
    int fromlen = sizeof(from);
    s = accept(data_peer, (struct sockaddr *) &from, &fromlen);
    if (s < 0) goto errdatacon;
    closesocket(data_peer);
    data_peer = s;
  }
  else
  {
    if(data_peer == -1)
    {
      char *MsgItems[]={"FTP","Internal error!","Ok"};
      Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
      return(NULL);
    }
    if(brk_flag ||
       connect(data_peer, (struct sockaddr *)&data_addr, sizeof(data_addr)))
    {
errdatacon:
      if(data_peer != -1)
      {
        closesocket(data_peer);
        data_peer = -1;
      }
      return (NULL);
    }
  }
  return (data_peer);
}


void Connection::psabort(int flag)
{
  abrtflag++;
}


void Connection::pswitch(int flag)
{
  struct comvars *ip, *op;

  abrtflag = 0;
  if (flag)
  {
    if (proxy)
      return;
    ip = &tmpstruct;
    op = &proxstruct;
    proxy++;
  }
  else
  {
    if (!proxy)
      return;
    ip = &proxstruct;
    op = &tmpstruct;
    proxy = 0;
  }
  ip->connect = connected;
  connected = op->connect;
  if (hostname)
  {
    strncpy(ip->name, hostname, sizeof(ip->name) - 1);
    ip->name[strlen(ip->name)] = '\0';
  }
  else
    ip->name[0] = 0;
  hostname = op->name;
  ip->hctl = hisctladdr;
  hisctladdr = op->hctl;
  ip->mctl = myctladdr;
  myctladdr = op->mctl;
  ip->in = (FILE *)cin; // What the hell am I looking at...?
  cin = (int)op->in;
  ip->out = (FILE *)cout; // Same again...
  cout = (int)op->out;
  ip->tpe = type;
  type = op->tpe;
  if (!type)
    type = 1;
  ip->cpnd = cpend;
  cpend = op->cpnd;
  ip->sunqe = sunique;
  sunique = op->sunqe;
  ip->runqe = runique;
  runique = op->runqe;
  abrtflag = 0;
}


void Connection::abortpt()
{
  ptabflg++;
  mflag = 0;
  abrtflag = 0;
  longjmp(ptabort, 1);
}


void Connection::proxtrans(char *cmd, char *local, char *remote)
{
  int tmptype, oldtype = 0, secndflag = 0, nfnd;
  char *cmd2;
  struct fd_set mask;

  if (strcmp(cmd, "RETR"))
    cmd2 = "RETR";
  else
    cmd2 = runique ? "STOU" : "STOR";
  if (command("PASV") != COMPLETE)
  {
//    printf("proxy server does not support third part transfers.\n");
    return;
  }
  tmptype = type;
  pswitch(0);
  if (!connected)
  {
//    printf("No primary connection\n");
    pswitch(1);
    code = -1;
    return;
  }
  if (type != tmptype)
  {
    oldtype = type;
    switch (tmptype)
    {
      case TYPE_A:
        setascii();
        break;
      case TYPE_I:
        setbinary();
        break;
      case TYPE_E:
        setebcdic();
        break;
      case TYPE_L:
        settenex();
        break;
    }
  }
  if (command("PORT %s", pasv) != COMPLETE)
  {
    switch (oldtype)
    {
      case 0:
        break;
      case TYPE_A:
        setascii();
        break;
      case TYPE_I:
        setbinary();
        break;
      case TYPE_E:
        setebcdic();
        break;
      case TYPE_L:
        settenex();
        break;
    }
    pswitch(1);
    return;
  }
  if (setjmp(ptabort))
    goto abort;
  if (command("%s %s", cmd, remote) != PRELIM)
  {
    switch (oldtype)
    {
      case 0:
        break;
      case TYPE_A:
        setascii();
        break;
      case TYPE_I:
        setbinary();
        break;
      case TYPE_E:
        setebcdic();
        break;
      case TYPE_L:
        settenex();
        break;
    }
    pswitch(1);
    return;
  }
//  sleep(2);
  pswitch(1);
  secndflag++;
  if (command("%s %s", cmd2, local) != PRELIM)
    goto abort;
  ptflag++;
  getreply(0);
  pswitch(0);
  getreply(0);
  switch (oldtype)
  {
    case 0:
      break;
    case TYPE_A:
      setascii();
      break;
    case TYPE_I:
      setbinary();
      break;
    case TYPE_E:
      setebcdic();
      break;
    case TYPE_L:
      settenex();
      break;
  }
  pswitch(1);
  ptflag = 0;
//  printf("local: %s remote: %s\n", local, remote);
  return;
abort:
  ptflag = 0;
  if (strcmp(cmd, "RETR") && !proxy)
    pswitch(1);
  else if (!strcmp(cmd, "RETR") && proxy)
    pswitch(0);
  if (!cpend && !secndflag) {  /* only here if cmd = "STOR" (proxy=1) */
    if (command("%s %s", cmd2, local) != PRELIM)
    {
      pswitch(0);
      switch (oldtype) {
        case 0:
          break;
        case TYPE_A:
          setascii();
          break;
        case TYPE_I:
          setbinary();
          break;
        case TYPE_E:
          setebcdic();
          break;
        case TYPE_L:
          settenex();
          break;
      }
      if (cpend)
      {
        char msg[2];

        fprintfSocket(cout,"%c%c",IAC,IP);
        *msg = IAC;
        *(msg+1) = DM;
        if (send(cout,msg,2,MSG_OOB) != 2)
          /*perror("abort")*/;
        fprintfSocket(cout,"ABOR\r\n");
        FD_ZERO(&mask);
//        FD_SET(fileno(cin), &mask); // Chris: Need to correct this
        if ((nfnd = empty(&mask,10)) <= 0) {
          if (nfnd < 0)
          {
//            perror("abort");
          }
          if (ptabflg)
            code = -1;
          lostpeer();
        }
        getreply(0);
        getreply(0);
      }
    }
    pswitch(1);
    if (ptabflg)
      code = -1;
    return;
  }
  if (cpend)
  {
    char msg[2];

    fprintfSocket(cout,"%c%c",IAC,IP);
    *msg = IAC;
    *(msg+1) = DM;
    if (send(cout,msg,2,MSG_OOB) != 2)
      /*perror("abort")*/;
    fprintfSocket(cout,"ABOR\r\n");
    FD_ZERO(&mask);
//    FD_SET(fileno(cin), &mask); // Chris: Need to correct this...
    if ((nfnd = empty(&mask,10)) <= 0) {
      if (nfnd < 0)
      {
//      perror("abort");
      }
      if (ptabflg)
        code = -1;
      lostpeer();
    }
    getreply(0);
    getreply(0);
  }
  pswitch(!proxy);
  if (!cpend && !secndflag) {  /* only if cmd = "RETR" (proxy=1) */
    if (command("%s %s", cmd2, local) != PRELIM) {
      pswitch(0);
      switch (oldtype) {
        case 0:
          break;
        case TYPE_A:
          setascii();
          break;
        case TYPE_I:
          setbinary();
          break;
        case TYPE_E:
          setebcdic();
          break;
        case TYPE_L:
          settenex();
          break;
      }
      if (cpend)
      {
        char msg[2];

        fprintfSocket(cout,"%c%c",IAC,IP);
        *msg = IAC;
        *(msg+1) = DM;
        if (send(cout,msg,2,MSG_OOB) != 2)
          /*perror("abort")*/;
        fprintfSocket(cout,"ABOR\r\n");
        FD_ZERO(&mask);
//        FD_SET(fileno(cin), &mask); // Chris:
        if ((nfnd = empty(&mask,10)) <= 0) {
          if (nfnd < 0)
          {
//          perror("abort");
          }
          if (ptabflg)
            code = -1;
          lostpeer();
        }
        (void) getreply(0);
        (void) getreply(0);
      }
      pswitch(1);
      if (ptabflg)
        code = -1;
      return;
    }
  }
  if (cpend) {
    char msg[2];

    fprintfSocket(cout,"%c%c",IAC,IP);
    *msg = IAC;
    *(msg+1) = DM;
    if (send(cout,msg,2,MSG_OOB) != 2)
      /*perror("abort")*/;
    fprintfSocket(cout,"ABOR\r\n");
    FD_ZERO(&mask);
//    FD_SET(fileno(cin), &mask); // Chris:
    if ((nfnd = empty(&mask,10)) <= 0) {
      if (nfnd < 0)
      {
//      perror("abort");
      }
      if (ptabflg)
        code = -1;
      lostpeer();
    }
    (void) getreply(0);
    (void) getreply(0);
  }
  pswitch(!proxy);
  if (cpend)
  {
    FD_ZERO(&mask);
//    FD_SET(fileno(cin), &mask); // Chris:
    if ((nfnd = empty(&mask,10)) <= 0) {
      if (nfnd < 0)
      {
//      perror("abort");
      }
      if (ptabflg)
        code = -1;
      lostpeer();
    }
    getreply(0);
    getreply(0);
  }
  if (proxy)
    pswitch(0);
  switch (oldtype) {
    case 0:
      break;
    case TYPE_A:
      setascii();
      break;
    case TYPE_I:
      setbinary();
      break;
    case TYPE_E:
      setebcdic();
      break;
    case TYPE_L:
      settenex();
      break;
  }
  pswitch(1);
  if (ptabflg)
    code = -1;
}


void Connection::reset()
{
  struct fd_set mask;
  int nfnd = 1;

  FD_ZERO(&mask);
  while (nfnd > 0) {
//    FD_SET(fileno(cin), &mask); // Chris
    if ((nfnd = empty(&mask,0)) < 0)
    {
//    perror("reset");
      code = -1;
      lostpeer();
    }
    else
      if (nfnd)
        getreply(0);
  }
}


void Connection::AbortAllRequest(int BrkFlag)
{
  if(data_peer != -1)
  {
    shutdown(data_peer, 2);
    closesocket(data_peer);
    data_peer = -1;
  }
  if(cmd_peer != -1)
  {
    shutdown(cmd_peer, 2);
    closesocket(cmd_peer);
    cmd_peer = -1;
  }
  if(BrkFlag) brk_flag = TRUE;
}
