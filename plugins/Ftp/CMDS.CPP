/*
 * FTP User Program -- Command Routines.
 */

#include "ftpincl.hpp"
#include "pathnames.hpp"

/*
 * Connect to peer server and
 * auto-login, if possible.
 */
int Connection::setpeer(int argc, char *argv[])
{
  char *host;

  if (connected)
  {
    code = -1;
    return(FALSE);
  }
  if (argc < 2)
  {
    sprintf(line,"open %s",argv[0]);
    makeargv();
    argc = margc;
    argv = margv;
  }
  if (argc > 3)
  {
    code = -1;
    return(FALSE);
  }
  if (argc > 2)
  {
    portnum = atoi(argv[2]);
    if (portnum <= 0)
    {
      code = -1;
      return(FALSE);
    }
    portnum = htons(portnum);
  }

  if (!hookup(argv[1], portnum))
  {
    code = -1;
    return(FALSE);
  }

  connected = 1;
  if (autologin && !login(argv[1]))
    return(FALSE);

  return(TRUE);
}


struct  types
{
  char  *t_name;
  char  *t_mode;
  int t_type;
  char  *t_arg;
} types[] = {
  { "ascii",  "A",  TYPE_A, 0 },
  { "binary", "I",  TYPE_I, 0 },
  { "image",  "I",  TYPE_I, 0 },
  { "ebcdic", "E",  TYPE_E, 0 },
  { "tenex",  "L",  TYPE_L, 0 },
  0
};

/*
 * Set transfer type.
 */
void Connection::settype(int argc, char *argv[])
{
  register struct types *p;
  int comret;

  types[4].t_arg=bytename;

  if (argc!=2)
  {
    code = -1;
    return;
  }
  for (p = types; p->t_name; p++)
    if (strcmp(argv[1], p->t_name) == 0)
      break;
  if (p->t_name == 0)
  {
    code = -1;
    return;
  }
  if ((p->t_arg != NULL) && (*(p->t_arg) != '\0'))
    comret = command ("TYPE %s %s", p->t_mode, p->t_arg);
  else
    comret = command("TYPE %s", p->t_mode);
  if (comret == COMPLETE)
    type = p->t_type;
}

/*
 * Set binary transfer type.
 */
/*VARARGS*/
void Connection::setbinary()
{
  stype[1] = "binary";
  settype(2, stype);
}

/*
 * Set ascii transfer type.
 */
/*VARARGS*/
void Connection::setascii()
{
  stype[1] = "ascii";
  settype(2, stype);
}

/*
 * Set tenex transfer type.
 */
/*VARARGS*/
void Connection::settenex()
{
  stype[1] = "tenex";
  settype(2, stype);
}

/*
 * Set ebcdic transfer type.
 */
/*VARARGS*/
void Connection::setebcdic()
{
  stype[1] = "ebcdic";
  settype(2, stype);
}


/*
 * Send a single file.
 */
void Connection::put(int argc, char *argv[])
{
  char *cmd;
  int loc = 0;
  char *oldargv1, *oldargv2;

  if (argc == 2)
  {
    argc++;
    argv[2] = argv[1];
    loc++;
  }
  if (argc < 3)
  {
    code = -1;
    return;
  }
  oldargv1 = argv[1];
  oldargv2 = argv[2];
  cmd = (argv[0][0] == 'a') ? "APPE" : ((sunique) ? "STOU" : "STOR");
  sendrequest(cmd,argv[1],argv[2],argv[1]!=oldargv1 || argv[2]!=oldargv2);
}


void Connection::reget(int argc, char *argv[])
{
  getit(argc, argv, 1, "r+");
}

void Connection::get(int argc, char *argv[])
{
  getit(argc, argv, 0, restart_point ? "r+" : "w" );
}

/*
 * Receive one file.
 */
int Connection::getit(int argc, char *argv[], int restartit, char *mode)
{
  int loc = 0;
  char *oldargv1, *oldargv2;

  if (argc == 2)
  {
    argc++;
    argv[2] = argv[1];
    loc++;
  }
  if (argc < 3)
  {
    code = -1;
    return (0);
  }
  oldargv1 = argv[1];
  oldargv2 = argv[2];
  if (restartit)
  {
    struct stat stbuf;
    int ret;

    ret = stat(argv[2], &stbuf);
    if (restartit == 1)
    {
      if (ret < 0)
        return (0);
      restart_point = stbuf.st_size;
    }
    else
    {
      if (ret == 0)
      {
        int overbose;

        overbose = verbose;
        verbose = -1;
        if (command("MDTM %s", argv[1]) == COMPLETE)
        {
          int yy, mo, day, hour, min, sec;
          struct tm *tm;
          verbose = overbose;
          sscanf(reply_string,"%*s %04d%02d%02d%02d%02d%02d",
                 &yy, &mo, &day, &hour, &min, &sec);
          tm = gmtime(&stbuf.st_mtime);
          tm->tm_mon++;
          if (tm->tm_year > yy%100)
            return (1);
          else if (tm->tm_year == yy%100) {
            if (tm->tm_mon > mo)
              return (1);
          } else if (tm->tm_mon == mo) {
            if (tm->tm_mday > day)
              return (1);
          } else if (tm->tm_mday == day) {
            if (tm->tm_hour > hour)
              return (1);
          } else if (tm->tm_hour == hour) {
            if (tm->tm_min > min)
              return (1);
          } else if (tm->tm_min == min) {
            if (tm->tm_sec > sec)
              return (1);
          }
        }
        else
        {
//          printf("%s\n", reply_string);
          verbose = overbose;
          return (0);
        }
      }
    }
  }

  recvrequest("RETR", argv[2], argv[1], mode,
      argv[1] != oldargv1 || argv[2] != oldargv2);
  restart_point = 0;
  return (0);
}


/*
 * Toggle PORT cmd use before each data connection.
 */
/*VARARGS*/
void Connection::setport()
{
  sendport = !sendport;
  code = sendport;
}


/*
 * Set current working directory
 * on remote machine.
 */
void Connection::cd(int argc, char *argv[])
{
  if (argc==2)
    if (command("CWD %s", argv[1]) == ERROR && code == 500)
      command("XCWD %s", argv[1]);
}


/*
 * Delete a single file.
 */
void Connection::deleteFile(int argc, char *argv[])
{
  if (argc < 2)
  {
    code = -1;
    return;
  }
  command("DELE %s", argv[1]);
}


/*
 * Rename a remote file.
 */
void Connection::renamefile(int argc, char *argv[])
{
  if (argc < 3)
  {
    code = -1;
    return;
  }
  if (command("RNFR %s", argv[1]) == CONTINUE)
    command("RNTO %s", argv[2]);
}

/*
 * Get a directory listing
 * of remote files.
 */
void Connection::ls(int argc, char *argv[])
{
  char *cmd;

  if (argc < 2)
    argc++, argv[1] = NULL;
  if (argc < 3)
    argc++, argv[2] = "-";
  if (argc > 3)
  {
    code = -1;
    return;
  }
  cmd = argv[0][0] == 'n' ? "NLST" : "LIST";
  recvrequest(cmd, argv[2], argv[1], "w", 0);
}


/*
 * Send new user information (re-login)
 */
int Connection::user(int argc, char **argv)
{
  char acct[80];
  int n, aflag = 0;

  if (argc<2 || argc > 4)
  {
    code = -1;
    return (0);
  }
  n = command("USER %s", argv[1]);
  if (n == CONTINUE)
  {
    if (argc < 3 )
      argv[2] = UserPassword, argc++;
    n = command("PASS %s", argv[2]);
  }
  if (n == CONTINUE)
  {
    if (argc < 4)
    {
      *acct=0;
      argv[3] = acct;
      argc++;
    }
    n = command("ACCT %s", argv[3]);
    aflag++;
  }
  if (n != COMPLETE)
  {
    SetLastError(ERROR_INTERNET_LOGIN_FAILURE);
    return (0);
  }
  if (!aflag && argc == 4) {
    command("ACCT %s", argv[3]);
  }
  return (1);
}

/*
 * Print working directory.
 */
/*VARARGS*/
void Connection::pwd()
{
  int oldverbose = verbose;

  /*
   * If we aren't verbose, this doesn't do anything!
   */
  verbose = 1;
  if (command("PWD") == ERROR && code == 500)
    command("XPWD");
  verbose = oldverbose;
}

/*
 * Make a directory.
 */
void Connection::makedir(int argc, char *argv[])
{
  if (argc < 2)
  {
    code = -1;
    return;
  }
  if (command("MKD %s", argv[1]) == ERROR && code == 500)
    command("XMKD %s", argv[1]);
}

/*
 * Remove a directory.
 */
void Connection::removedir(int argc, char *argv[])
{
  if (argc < 2)
  {
    code = -1;
    return;
  }
  if (command("RMD %s", argv[1]) == ERROR && code == 500)
    command("XRMD %s", argv[1]);
}

/*
 * Send a line, verbatim, to the remote machine.
 */
void Connection::quote(int argc, char *argv[])
{
  int i;
  char buf[BUFSIZ];

  if (argc < 2)
  {
    code = -1;
    return;
  }
  strcpy(buf, argv[1]);
  for (i = 2; i < argc; i++)
  {
    strcat(buf, " ");
    strcat(buf, argv[i]);
  }
  if (command(buf) == PRELIM)
    while (getreply(0) == PRELIM);
}

/*
 * Send a SITE command to the remote machine.  The line
 * is sent almost verbatim to the remote machine, the
 * first argument is changed to SITE.
 */

void Connection::site(int argc, char *argv[])
{
  int i;
  char buf[BUFSIZ];

  if (argc < 2)
  {
    code = -1;
    return;
  }
  strcpy(buf, "SITE ");
  strcat(buf, argv[1]);
  for (i = 2; i < argc; i++)
  {
    strcat(buf, " ");
    strcat(buf, argv[i]);
  }
  if (command(buf) == PRELIM)
    while (getreply(0) == PRELIM);
}


void Connection::do_chmod(int argc, char *argv[])
{
  if (argc != 3)
  {
    code = -1;
    return;
  }
  command("SITE CHMOD %s %s", argv[1], argv[2]);
}


void Connection::do_umask(int argc, char *argv[])
{
  int oldverbose = verbose;

  verbose = 1;
  command(argc == 1 ? "SITE UMASK" : "SITE UMASK %s", argv[1]);
  verbose = oldverbose;
}


void Connection::idle(int argc, char *argv[])
{
  int oldverbose = verbose;

  verbose = 1;
  command(argc == 1 ? "SITE IDLE" : "SITE IDLE %s", argv[1]);
  verbose = oldverbose;
}

/*
 * Ask the other side for help.
 */
void Connection::rmthelp(int argc, char *argv[])
{
  int oldverbose = verbose;

  verbose = 1;
  command(argc == 1 ? "HELP" : "HELP %s", argv[1]);
  verbose = oldverbose;
}

/*
 * Terminate session and exit.
 */
/*VARARGS*/
void Connection::quit()
{
  if (connected)
    disconnect();
  pswitch(1);
  if (connected) {
    disconnect();
  }
}


/*
 * Terminate session, but don't exit.
 */
void Connection::disconnect()
{
  if (!connected)
    return;
  command("QUIT");
  cout = NULL;
  connected = 0;
  data_peer = -1;
}


void Connection::account(int argc,char **argv)
{
  char acct[50], *ap;

  if (argc > 1)
  {
    ++argv;
    --argc;
    (void) strncpy(acct,*argv,49);
    acct[49] = '\0';
    while (argc > 1)
    {
      --argc;
      ++argv;
      strncat(acct,*argv, 49-strlen(acct));
    }
    ap = acct;
  }
  else
  {
//    ap = getpass("Account:");
    ap="";
  }
  command("ACCT %s", ap);
}


void Connection::proxabort()
{
  if (!proxy) {
    pswitch(1);
  }
  if (connected) {
    proxflag = 1;
  }
  else {
    proxflag = 0;
  }
  pswitch(0);
  longjmp(abortprox,1);
}


void Connection::doproxy(int argc, char *argv[])
{
  register struct cmd *c;

  if (argc < 2)
  {
    code = -1;
    return;
  }
  c = getcmd(argv[1]);
  if (c == (struct cmd *) -1)
  {
//    printf("?Ambiguous command\n");
    code = -1;
    return;
  }
  if (c == 0)
  {
//    printf("?Invalid command\n");
    code = -1;
    return;
  }
  if (!c->c_proxy)
  {
//    printf("?Invalid proxy command\n");
    code = -1;
    return;
  }
  if (setjmp(abortprox))
  {
    code = -1;
    return;
  }
  pswitch(1);
  if (c->c_conn && !connected)
  {
    pswitch(0);
    code = -1;
    return;
  }
  ExecCmdTab(c, argc-1, argv+1);
  proxflag = (connected) ? 1 : 0;
  pswitch(0);
}


void Connection::setcr()
{
  crflag = !crflag;
  code = crflag;
}


void Connection::setsunique()
{
  sunique = !sunique;
  code = sunique;
}


void Connection::setrunique()
{
  runique = !runique;
  code = runique;
}

/* change directory to parent directory */
void Connection::cdup()
{
  if (command("CDUP") == ERROR && code == 500)
    command("XCUP");
}

/* restart transfer at specific point */
void Connection::restart(int argc, char *argv[])
{
  if (argc == 2)
    restart_point = atol(argv[1]);
}


/* show remote system type */
void Connection::syst()
{
  command("SYST");
}


/*
 * get size of file on remote machine
 */
void Connection::sizecmd(int argc, char *argv[])
{
  if (argc < 2)
  {
    code = -1;
    return;
  }
  command("SIZE %s", argv[1]);
}

/*
 * get last modification time of file on remote machine
 */
void Connection::modtime(int argc, char *argv[])
{
  int overbose;

  if (argc < 2)
  {
    code = -1;
    return;
  }
  overbose = verbose;
  verbose = -1;
  if (command("MDTM %s", argv[1]) == COMPLETE)
  {
    int yy, mo, day, hour, min, sec;
    sscanf(reply_string, "%*s %04d%02d%02d%02d%02d%02d", &yy, &mo,
           &day, &hour, &min, &sec);
    /* might want to print this in local time */
//    printf("%s\t%02d/%02d/%04d %02d:%02d:%02d GMT\n", argv[1],
//           mo, day, yy, hour, min, sec);
  }
  verbose = overbose;
}

/*
 * show status on remote machine
 */
void Connection::rmtstatus(int argc, char *argv[])
{
  command(argc > 1 ? "STAT %s" : "STAT" , argv[1]);
}

/*
 * get file if modtime is more recent than current file
 */
void Connection::newer(int argc, char *argv[])
{
  getit(argc, argv, -1, "w");
}
