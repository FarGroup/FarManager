#ifndef __FARBLIB_HPP__
#define __FARBLIB_HPP__

#include "bzlib.h"

typedef void (WINAPI *PBZCLOSE)(BZFILE *b);
typedef const char *(WINAPI *PBZERROR)(BZFILE *b,int *errnum);
typedef BZFILE *(WINAPI *PBZOPEN)(const char *path,const char *mode);
typedef int (WINAPI *PBZREAD)(BZFILE *b,void *buf,int len);

extern PBZCLOSE pbzclose;
extern PBZERROR pbzerror;
extern PBZOPEN pbzopen;
extern PBZREAD pbzread;

extern void InitBlib(char *lib);
extern void CloseBlib(void);

extern bool BlibOk;

#endif
