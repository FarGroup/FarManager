/*
  MA_EMU.CPP

  MultiArc plugin emulator for debugging second-level plugin modules

  Copyrigth (c) 2001 FAR group
*/

/* $Revision: 1.1.1.1 $ */

/*
 Example:
   bcc32 -v ma_emu.cpp rar.cpp
   td32 ma_emu archive.rar
*/

#include "plugin.hpp"

#include <string.h>
#include <stdio.h>

#include "fmt.hpp"

char Buff[128*1024];

int main(int argc,char *argv[])
{
  FILE *fp;

  if(argc != 2)
    return 1;

  if((fp=fopen(argv[1],"rb")) == NULL)
    return 2;

  fread(Buff,sizeof(Buff),1,fp);
  fclose(fp);

/*
  LoadFormatModule("");
*/

/*
  {
    struct PluginStartupInfo startupInfo;
    SetFarInfo(&startupInfo);
  }
*/

  if(IsArchive(argv[1],(const unsigned char *)Buff,sizeof(Buff)))
  {
    int TypeArc;
    char FormatName[NM], DefaultExt[NM], Command[NM];

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
