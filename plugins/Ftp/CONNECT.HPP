int sleep(int time);
int gettimeofday(struct timeval *timenow, struct timezone *tz);
int fgetcSocket(int s);
char *fprintfSocket(int s, char *format, ...);
char *fputsSocket(char *format, int s);
int fputcSocket(int s, char putChar);
char *getpass (const char * prompt);

extern int CheckForEsc();
extern void ConnectMessage(int Msg,char *HostName=NULL);

#define NM 260

struct comvars
{
  int connect;
  char name[MAXHOSTNAMELEN];
  struct sockaddr_in mctl;
  struct sockaddr_in hctl;
  FILE *in;
  FILE *out;
  int tpe;
  int cpnd;
  int sunqe;
  int runqe;
};

class Connection
{
  private:
    void ginit(char **agargv);
    void collect(register char *as);
    void acollect(register char *as);
    void sort();
    void expand(char *as);
    void matchdir(char *pattern);
    int execbrc(char *p, char *s);
    int match(char *s, char *p);
    int amatch(register char *s, register char *p);
    void Gmatch(register char *s, register char *p);
    void Gcat(register char *s1, register char *s2);
    void addpath(char c);
    void rscan(register char **t, int (*f)());
    int letter(register char c);
    int digit(register char c);
    int any(register int c, register char *s);
    int blklen(register char **av);
    char** blkcpy(char **oav, register char **bv);
    char* strspl(register char *cp, register char *dp);
    char** copyblk(register char **v);
    char* strend(register char *cp);
    int gethdir(char *home);
    char* hookup(char *host, int port);
    int login(char *host);
    void cmdabort();
    int command(char *fmt, ...);
    int getreply(int expecteof);
    int empty(struct fd_set *mask, int sec);
    void abortsend();
    void sendrequest(char *cmd, char *local, char *remote, int printnames);
    void abortrecv();
    void recvrequest(char *cmd, char *local, char *remote, char *mode,int printnames);
    int initconn();
    int dataconn(char *mode);
    void psabort(int flag);
    void pswitch(int flag);
    void abortpt();
    void proxtrans(char *cmd, char *local, char *remote);
    void reset();
    int setpeer(int argc, char *argv[]);
    void settype(int argc, char *argv[]);
    void setbinary();
    void setascii();
    void settenex();
    void setebcdic();
    void put(int argc, char *argv[]);
    void reget(int argc, char *argv[]);
    void get(int argc, char *argv[]);
    int getit(int argc, char *argv[], int restartit, char *mode);
    void setport();
    void cd(int argc, char *argv[]);
    void deleteFile(int argc, char *argv[]);
    void renamefile(int argc, char *argv[]);
    void ls(int argc, char *argv[]);
    int user(int argc, char **argv);
    void pwd();
    void makedir(int argc, char *argv[]);
    void removedir(int argc, char *argv[]);
    void quote(int argc, char *argv[]);
    void site(int argc, char *argv[]);
    void do_chmod(int argc, char *argv[]);
    void do_umask(int argc, char *argv[]);
    void idle(int argc, char *argv[]);
    void rmthelp(int argc, char *argv[]);
    void quit();
    void disconnect();
    void account(int argc,char **argv);
    void proxabort();
    void doproxy(int argc, char *argv[]);
    void setcase();
    void setcr();
    void setsunique();
    void setrunique();
    void cdup();
    void restart(int argc, char *argv[]);
    void syst();
    void sizecmd(int argc, char *argv[]);
    void modtime(int argc, char *argv[]);
    void rmtstatus(int argc, char *argv[]);
    void newer(int argc, char *argv[]);
    void lostpeer();
    struct cmd *getcmd(register char *name);
    void makeargv();
    char *slurpstring();
    void InitCmdTab();
    void ExecCmdTab(struct cmd *c,int argc,char *argv[]);
    void EncodeLine(char *Line);

    struct cmd cmdtab[47];
    jmp_buf ptabort;
    int ptabflg;
    struct comvars proxstruct, tmpstruct;
    jmp_buf sendabort;
    jmp_buf recvabort;
    char reply_string[BUFSIZ];    /* last line of previous reply */
    struct  sockaddr_in hisctladdr;
    struct  sockaddr_in data_addr;
    int cmd_peer, data_peer, brk_flag;
    int abrtflag;
    int ptflag;
    int allbinary;
    struct  sockaddr_in myctladdr;
    off_t restart_point;
    int cin, cout;
    char *hostname;
    char *stype[3];
    char *mname;
    jmp_buf abortprox;
    int portnum;
    int autologin;
    int slrflag;

    /* Lot's of options... */
    /*
     * Options and other state info.
     */
    int sendport;   /* use PORT cmd for each data connection */
    int verbose;    /* print messages coming back from server */
    int connected;  /* connected to server */
    int fromatty;   /* input is from a terminal */
    int proxy;      /* proxy server connection active */
    int proxflag;   /* proxy connection exists */
    int sunique;    /* store files on server with unique name */
    int runique;    /* store local files with unique name */
    int code;       /* return/reply code for ftp command */
    int crflag;     /* if 1, strip car. rets. on ascii gets */
    char  pasv[64];   /* passive port for proxy data connection */
    char  *altarg;    /* argv[1] with no shell-like preprocessing  */

    int type;     /* file transfer type */
    int stru;     /* file transfer structure */
    char  formname[32];   /* name of file transfer format */
    int form;     /* file transfer format */
    int mode;     /* file transfer mode */
    char  bytename[32];   /* local byte size in ascii */
    int bytesize;   /* local byte size in binary */

    char  line[512];    /* input line buffer */
    char  *stringbase;    /* current scan point in line buffer */
    char  argbuf[512];    /* argument storage buffer */
    char  *argbase;   /* current storage point in arg buffer */
    int margc;      /* count of arguments on input line */
    char  *margv[20];   /* args parsed from input line */
    int     cpend;                  /* flag: if != 0, then pending server reply */
    int mflag;      /* flag: if != 0, then active multi command */
    int options;    /* used during socket creation */

    void ResetOutput();
    void AddOutput(char *Data,int Size);

    int ErrorCode;

    struct
    {
      char DirName[NM];
      char *Listing;
      int ListingSize;
    } ListCache[16];
    int ListCachePos;

    char UserName[128];
    char UserPassword[128];
    int SocketStartup;

    char *Output;
    int OutputSize;
    int OutputPos;

    int LastUsedTableNum;
  public:
    Connection();
    ~Connection();
    int Init(const char *Host,const char *User,const char *Password);
    int ProcessCommand(char *LineToProcess);
    int GetOutput(char *Line,int MaxLength);
    int GetReply(char *Line,int MaxLength);
    void CacheReset();
    int CacheGet();
    void CacheAdd();
    int GetExitCode();
    void SetTable(int Table) {TableNum=Table;};
    int ParseDirLine(WIN32_FIND_DATA *FindData);

    static void ResetLastReply();
    static void GetLastReply(char *Buf1,char *Buf2);

    char CurDir[NM];
    int (*GetCallback)(int Size);
    int (*PutCallback)(int Size);

    char SystemInfo[512];
    int SystemInfoFilled;

    char StartReply[512];

    int FindAllFiles;
    int TableNum;

    int PassiveMode;
    void AbortAllRequest(int brkFlag);
};
