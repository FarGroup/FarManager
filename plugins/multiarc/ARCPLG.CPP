/*
Modify:
  19.02.2001 AS
   ! правка высвобождения списка PluginsData в деструкторе ArcPlugins
*/


ArcPlugins::ArcPlugins(const char *ModuleName)
{
  char PluginsFolder[NM],*NamePtr;

  PluginsData=NULL;
  PluginsCount=0;

  strcpy(PluginsFolder,ModuleName);
  if ((NamePtr=strrchr(PluginsFolder,'\\'))==NULL)
    return;

  strcpy(NamePtr,"\\Formats\\");
  FSF.FarRecursiveSearch(PluginsFolder,"*.fmt",
                    (FRSUSERFUNC)LoadFmtModules,FRS_RECUR,this);
}

int WINAPI ArcPlugins::LoadFmtModules(const WIN32_FIND_DATA *fdata,
                                     const char *PluginName,
                                     ArcPlugins *plugins)
{
  struct PluginItem CurPlugin;
  //char *Ptr;
  if(fdata->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
    return TRUE;

  HMODULE hModule=LoadLibrary(PluginName);
  if (hModule!=NULL)
  {
    struct PluginItem *NewPluginsData=(struct PluginItem *)realloc(plugins->PluginsData,
                            sizeof(struct PluginItem)*(plugins->PluginsCount+1));
    if (NewPluginsData==NULL)
    {
      FreeLibrary(hModule);
      return FALSE;
    }
    plugins->PluginsData=NewPluginsData;
    strcpy(CurPlugin.ModuleName,fdata->cFileName);
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
    CurPlugin.pGetSFXPos=(PLUGINGETSFXPOS)GetProcAddress(hModule,"GetSFXPos");

    if (CurPlugin.pLoadFormatModule)
      CurPlugin.pLoadFormatModule(PluginName);

    if (CurPlugin.pSetFarInfo)
      CurPlugin.pSetFarInfo(&Info);

    memcpy(&plugins->PluginsData[plugins->PluginsCount],&CurPlugin,sizeof(struct PluginItem));
    plugins->PluginsCount++;
  }
  return TRUE;
}

ArcPlugins::~ArcPlugins()
{
  for (int I=0;I<PluginsCount;I++)
    FreeLibrary(PluginsData[I].hModule);

  if(PluginsData)
    free(PluginsData);
}

int ArcPlugins::IsArchive(char *Name,const unsigned char *Data,int DataSize)
{
  char *ExtPtr=strrchr(Name,'.');
  /* $ 27.07.2001 AA */
  //DWORD MinSFXSize=-1;
  DWORD MinSFXSize=0xFFFFFFFF;
  /* 27.07.2001 AA $ */
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
        if(CurSFXSize <= MinSFXSize)
        {
          MinSFXSize=CurSFXSize;
          TrueArc=I;
        }
      }
  return(TrueArc);
}


BOOL ArcPlugins::OpenArchive(int PluginNumber,char *Name,int *Type)
{
  *Type=0; //$ AA 12.11.2001
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
