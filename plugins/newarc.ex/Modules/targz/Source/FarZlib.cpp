#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include "FarZlib.hpp"

bool ZlibOk=false;
static HINSTANCE hZlib=NULL;

PGZCLOSE pgzclose=NULL;
PGZREAD pgzread=NULL;
PGZOPEN pgzopen=NULL;
PGZSEEK pgzseek=NULL;
PGZERROR pgzerror=NULL;

void InitZlib(char *lib)
{
  if(hZlib) CloseZlib();
  hZlib=LoadLibrary(lib);
  if(hZlib)
  {
    pgzclose=(PGZCLOSE)GetProcAddress(hZlib,"gzclose");
    pgzread=(PGZREAD)GetProcAddress(hZlib,"gzread");
    pgzopen=(PGZOPEN)GetProcAddress(hZlib,"gzopen");
    pgzseek=(PGZSEEK)GetProcAddress(hZlib,"gzseek");
    pgzerror=(PGZERROR)GetProcAddress(hZlib,"gzerror");
    if(pgzclose&&pgzread&&pgzopen&&pgzseek&&pgzerror) ZlibOk=true;
  }
}

void CloseZlib(void)
{
  FreeLibrary(hZlib);
  hZlib=NULL;
  pgzclose=NULL;
  pgzread=NULL;
  pgzopen=NULL;
  pgzseek=NULL;
  pgzerror=NULL;
  ZlibOk=false;
}
