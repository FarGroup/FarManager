/*
Modify:
  19.02.2001 AS
   ! правка высвобождения списка PluginsData в деструкторе ArcPlugins
*/

ArcPlugins::ArcPlugins(char *ModuleName)
{
  PluginsData=NULL;
  PluginsCount=0;
  char PluginsFolder[NM],PluginsMask[NM],*NamePtr;
  strcpy(PluginsFolder,ModuleName);
  if ((NamePtr=strrchr(PluginsFolder,'\\'))==NULL)
    return;
  strcpy(NamePtr,"\\Formats\\");
  FSF.sprintf(PluginsMask,"%s*.fmt",PluginsFolder);
  HANDLE FindHandle;
  WIN32_FIND_DATA fdata;
  int Done=((FindHandle=FindFirstFile(PluginsMask,&fdata))==INVALID_HANDLE_VALUE);
  while (!Done)
  {
    char PluginName[NM];
    FSF.sprintf(PluginName,"%s%s",PluginsFolder,fdata.cFileName);
    HMODULE hModule=LoadLibrary(PluginName);
    if (hModule!=NULL)
    {
      struct PluginItem *NewPluginsData=(struct PluginItem *)realloc(PluginsData,sizeof(*PluginsData)*(PluginsCount+1));
      if (NewPluginsData==NULL)
        break;
      PluginsData=NewPluginsData;
      struct PluginItem CurPlugin;
      strcpy(CurPlugin.ModuleName,fdata.cFileName);
      CurPlugin.hModule=hModule;
      CurPlugin.Flags=0;
      CurPlugin.pLoadFormatModule=(PLUGINLOADFORMATMODULE)GetProcAddress(hModule,"LoadFormatModule");
      CurPlugin.pIsArchive=(PLUGINISARCHIVE)GetProcAddress(hModule,"IsArchive");
      CurPlugin.pOpenArchive=(PLUGINOPENARCHIVE)GetProcAddress(hModule,"OpenArchive");
      CurPlugin.pGetArcItem=(PLUGINGETARCITEM)GetProcAddress(hModule,"GetArcItem");
      CurPlugin.pCloseArchive=(PLUGINCLOSEARCHIVE)GetProcAddress(hModule,"CloseArchive");
      CurPlugin.pGetFormatName=(PLUGINGETFORMATNAME)GetProcAddress(hModule,"GetFormatName");
      CurPlugin.pGetDefaultCommands=(PLUGINGETDEFAULTCOMMANDS)GetProcAddress(hModule,"GetDefaultCommands");
      CurPlugin.pSetFarInfo=(PLUGINSETFARINFO)GetProcAddress(hModule,"SetFarInfo");
      CurPlugin.pGetSFXPos=(PLUGINGetSFXPos)GetProcAddress(hModule,"GetSFXPos");

      if (CurPlugin.pLoadFormatModule)
        CurPlugin.pLoadFormatModule(PluginName);

      if (CurPlugin.pSetFarInfo)
        CurPlugin.pSetFarInfo(&Info);

      PluginsData[PluginsCount]=CurPlugin;
      PluginsCount++;
    }
    Done=!FindNextFile(FindHandle,&fdata);
  }
  FindClose(FindHandle);
}


ArcPlugins::~ArcPlugins()
{
  for (int I=0;I<PluginsCount;I++)
    FreeLibrary(PluginsData[I].hModule);
//  delete PluginsData;
/* $ 19.02.2001 AS
*/
  delete [] PluginsData;
/* AS $*/
}


int ArcPlugins::IsArchive(char *Name,const unsigned char *Data,int DataSize)
{
  char *ExtPtr=strrchr(Name,'.');
  DWORD MinSFXSize=-1;
  DWORD CurSFXSize;
  int TrueArc=-1;
  if (ExtPtr!=NULL)
  {
    ExtPtr++;
    for (int I=0;I<PluginsCount;I++)
      for (int J=0;;J++)
      {
        char FormatName[200],DefExt[NM];
        if (!GetFormatName(I,J,FormatName,DefExt))
          break;
        if (stricmp(ExtPtr,DefExt)==0)
          if (PluginsData[I].pIsArchive(Name,Data,DataSize))
          {
            if(PluginsData[I].pGetSFXPos)
              CurSFXSize=PluginsData[I].pGetSFXPos();
            else
              CurSFXSize=0;
/*{
  char BBBB[100];
  FSF.sprintf(BBBB,"1) %s)\nCurSFXSize=%d\nMinSFXSize=%d\nOk",FormatName,CurSFXSize,MinSFXSize);
  Info.Message(Info.ModuleNumber,FMSG_ALLINONE,NULL,(const char **)BBBB,0,1);
}*/
            if(CurSFXSize <= MinSFXSize)
            {
              MinSFXSize=CurSFXSize;
              TrueArc=I;
            }
            break;
          }
      }
  }
  for (int I=0;I<PluginsCount;I++)
    if (PluginsData[I].pIsArchive)
      if (PluginsData[I].pIsArchive(Name,Data,DataSize))
      {
        if(PluginsData[I].pGetSFXPos)
          CurSFXSize=PluginsData[I].pGetSFXPos();
        else
          CurSFXSize=0;
/*{
  char BBBB[100];
  FSF.sprintf(BBBB,"%s\nCurSFXSize=%d\nMinSFXSize=%d\nOk",PluginsData[I].ModuleName,CurSFXSize,MinSFXSize);
  Info.Message(Info.ModuleNumber,FMSG_ALLINONE,NULL,(const char **)BBBB,0,1);
}   */
        if(CurSFXSize <= MinSFXSize)
        {
          MinSFXSize=CurSFXSize;
          TrueArc=I;
        }
      }
/*{
  char BBBB[100];
  FSF.sprintf(BBBB,"End: %s\nCurSFXSize=%d\nMinSFXSize=%d\nOk",PluginsData[I].ModuleName,CurSFXSize,MinSFXSize);
  Info.Message(Info.ModuleNumber,FMSG_ALLINONE,NULL,(const char **)BBBB,0,1);
} */
  return(TrueArc);
}


BOOL ArcPlugins::OpenArchive(int PluginNumber,char *Name,int *Type)
{
  if (PluginNumber>=0 && PluginsData[PluginNumber].pOpenArchive)
    return(PluginsData[PluginNumber].pOpenArchive(Name,Type));
  return(FALSE);
}

int ArcPlugins::GetArcItem(int PluginNumber,struct PluginPanelItem *Item,
                           struct ArcItemInfo *Info)
{
  if (PluginNumber>=0 && PluginsData[PluginNumber].pGetArcItem)
    return(PluginsData[PluginNumber].pGetArcItem(Item,Info));
  return(FALSE);
}


void ArcPlugins::CloseArchive(int PluginNumber,struct ArcInfo *Info)
{
  if (PluginNumber>=0 && PluginsData[PluginNumber].pCloseArchive)
    PluginsData[PluginNumber].pCloseArchive(Info);
}


BOOL ArcPlugins::GetFormatName(int PluginNumber,int Type,char *FormatName,
                               char *DefaultExt)
{
  *FormatName=0;
  if (PluginNumber>=0 && PluginNumber<PluginsCount && PluginsData[PluginNumber].pGetFormatName)
    return(PluginsData[PluginNumber].pGetFormatName(Type,FormatName,DefaultExt));
  return(FALSE);
}


BOOL ArcPlugins::GetDefaultCommands(int PluginNumber,int Type,int Command,
                                    char *Dest)
{
  *Dest=0;
  if (PluginNumber>=0 && PluginNumber<PluginsCount && PluginsData[PluginNumber].pGetDefaultCommands)
    return(PluginsData[PluginNumber].pGetDefaultCommands(Type,Command,Dest));
  return(FALSE);
}
