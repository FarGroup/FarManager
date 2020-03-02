/*
  MA_EMU.CPP

  MultiArc plugin emulator for debugging second-level plugin modules

  Copyrigth (c) 2001 FAR group
*/

/*
 Example:
   bcc32 -v ma_emu.cpp rar.cpp
   td32 ma_emu archive.rar
*/

#include <plugin.hpp>

#ifndef __STDIO_H
#include <stdio.h>
#endif
#ifndef __STDLIB_H
#include <stdlib.h>
#endif
#ifndef __STRING_H
#include <string.h>
#endif
#ifndef __STDARG_H
#include <stdarg.h>
#endif


#include "fmt.hpp"

char Buff[128*1024];

int WINAPI LStricmp(const char *Str1,const char *Str2)
{
  return stricmp(Str1,Str2);
}

int WINAPI LStrnicmp(const char *Str1,const char *Str2,int Num)
{
  return strnicmp(Str1,Str2,Num);
}

int WINAPI SPrintf(char *Buffer,const char *Format,...)
{
  int ret=0;
  if(Buffer && Format)
  {
    va_list argptr;
    va_start(argptr,Format);
    ret=vsprintf(Buffer,Format,argptr);
    va_end(argptr);
  }
  return ret;
}

char* WINAPI FarMkTemp(char *Dest, const char *Prefix)
{
  if(Dest)
  {
    char TempName[NM];
    strcpy(TempName,"FTMPXXXXXX");
    if (mktemp(TempName)!=NULL)
    {
      strcpy(Dest,strupr(TempName));
      return Dest;
    }
  }
  return NULL;
}


int main(int argc,char *argv[])
{
  FILE *fp;

  if(argc != 2)
    return 1;

  if((fp=fopen(argv[1],"rb")) == NULL)
    return 2;

  fread(Buff,sizeof(Buff),1,fp);
  fclose(fp);


  LoadFormatModule(argv[0]);

  struct PluginStartupInfo Info={0};
  Info.StructSize=sizeof(Info);
  FARSTANDARDFUNCTIONS  FSF={0};
  FSF.StructSize=sizeof(FARSTANDARDFUNCTIONS);
  Info.FSF=&FSF;
  FSF.LStricmp=LStricmp;
  FSF.LStrnicmp=LStrnicmp;
  FSF.sprintf=SPrintf;
  FSF.MkTemp=FarMkTemp;

  SetFarInfo(&Info);

  if(IsArchive(argv[1],(const unsigned char *)Buff,sizeof(Buff)))
  {
    int TypeArc;
    char FormatName[NM], DefaultExt[NM], Command[512];

    if(OpenArchive(argv[1],&TypeArc) != FALSE)
    {
      struct ArcInfo arcInfo;
      struct ArcItemInfo itemInfo={0};
      struct PluginPanelItem panelItem={0};

      //DWORD  SFXPos=GetSFXPos();
      GetFormatName(TypeArc,FormatName,DefaultExt);
      GetDefaultCommands(TypeArc,0,Command);

      while(GetArcItem(&panelItem,&itemInfo) == GETARC_SUCCESS)
      {
        printf("%-16s 0x%04X %10ld %10ld %d\n",
               panelItem.FindData.cFileName,
               panelItem.FindData.dwFileAttributes,
               panelItem.FindData.nFileSizeLow,
               panelItem.PackSize,
               itemInfo.DictSize);

        memset(&panelItem,0,sizeof(panelItem));
        memset(&itemInfo,0,sizeof(itemInfo));
      }

      memset(&arcInfo,0,sizeof(arcInfo));
      CloseArchive(&arcInfo);

      return 0;
    }
    return 4;
  }
  return 3;
}
