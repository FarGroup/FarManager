#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__QNX__)
static CONSTSTR Signals[] = {
"SIGHUP   hangup",
"SIGINT   interrupt",
"SIGQUIT  quit",
"SIGILL   illegal instruction (not reset when caught)",
"SIGTRAP  trace trap (not reset when caught)",
"SIGIOT   IOT instruction",
"SIGABRT  used by abort",
"SIGEMT   EMT instruction",
"SIGFPE   floating point exception",
"SIGKILL  kill (cannot be caught or ignored)",
"SIGBUS   bus error",
"SIGSEGV  segmentation violation",
"SIGSYS   bad argument to system call",
"SIGPIPE  write on pipe with no reader",
"SIGALRM  real-time alarm clock",
"SIGTERM  software termination signal from kill",
"SIGUSR1  user defined signal 1",
"SIGUSR2  user defined signal 2",
"SIGCHLD  death of child",
"SIGPWR   power-fail restart",
"SIGWINCH window change",
"SIGURG   urgent condition on I/O channel",
"SIGPOLL  System V name for SIGIO",
"SIGIO    Asynchronus I/O",
"SIGSTOP  sendable stop signal not from tty",
"SIGTSTP  stop signal from tty",
"SIGCONT  continue a stopped process",
"SIGTTIN  attempted background tty read",
"SIGTTOU  attempted background tty write",
"SIGDEV   Dev event"
};

CONSTSTR SignalName( int sig )
  {
   if ( sig >= SIGHUP && sig <= SIGDEV )
     return Signals[sig-SIGHUP];
    else
     return "unknown";
}
#endif