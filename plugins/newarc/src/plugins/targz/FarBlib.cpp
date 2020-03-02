#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include "FarBlib.hpp"

bool BlibOk=false;
static HINSTANCE hBlib=NULL;

PBZCLOSE pbzclose=NULL;
PBZERROR pbzerror=NULL;
PBZOPEN pbzopen=NULL;
PBZREAD pbzread=NULL;

void InitBlib(char *lib)
{
  if(hBlib) CloseBlib();
  hBlib=LoadLibrary(lib);
  if(hBlib)
  {
    pbzclose=(PBZCLOSE)GetProcAddress(hBlib,"BZ2_bzclose@4");
    pbzerror=(PBZERROR)GetProcAddress(hBlib,"BZ2_bzerror@8");
    pbzopen=(PBZOPEN)GetProcAddress(hBlib,"BZ2_bzopen@8");
    pbzread=(PBZREAD)GetProcAddress(hBlib,"BZ2_bzread@12");
    if(pbzclose&&pbzerror&&pbzopen&&pbzread) BlibOk=true;
  }
}

void CloseBlib(void)
{
  FreeLibrary(hBlib);
  hBlib=NULL;
  pbzclose=NULL;
  pbzerror=NULL;
  pbzopen=NULL;
  pbzread=NULL;
  BlibOk=false;
}
