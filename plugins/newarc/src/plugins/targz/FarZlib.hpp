#ifndef __FARZLIB_HPP__
#define __FARZLIB_HPP__

#include "zlib.h"

typedef int (WINAPI *PGZCLOSE)(gzFile file);
typedef int (WINAPI *PGZREAD)(gzFile file, voidp buf, unsigned len);
typedef gzFile (WINAPI *PGZOPEN)(const char *path, const char *mode);
typedef z_off_t (WINAPI *PGZSEEK)(gzFile file,z_off_t offset, int whence);
typedef const char *(WINAPI *PGZERROR)(gzFile file, int *errnum);

extern PGZCLOSE pgzclose;
extern PGZREAD pgzread;
extern PGZOPEN pgzopen;
extern PGZSEEK pgzseek;
extern PGZERROR pgzerror;

extern void InitZlib(char *lib);
extern void CloseZlib(void);

extern bool ZlibOk;

#endif
