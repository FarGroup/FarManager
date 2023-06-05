#include "MultiArc.hpp"
#include "marclng.hpp"

#include <cstdlib>

typedef int (__cdecl *FCMP)(const void *, const void *);

ArcPlugins::ArcPlugins(const char *ModuleName)
{
  char PluginsFolder[NM],*NamePtr;

  PluginsData=NULL;
  PluginsCount=0;

  lstrcpy(PluginsFolder,ModuleName);
  if ((NamePtr=strrchr(PluginsFolder,'\\'))==NULL)
    return;

  lstrcpy(NamePtr,"\\Formats\\");
  FSF.FarRecursiveSearch(PluginsFolder,"*.fmt",
                    (FRSUSERFUNC)LoadFmtModules,FRS_RECUR,this);

  FSF.qsort(PluginsData,PluginsCount,sizeof(struct PluginItem),(FCMP)CompareFmtModules);
}

int __cdecl ArcPlugins::CompareFmtModules(const void *elem1, const void *elem2)
{
  const char *left = (((const PluginItem *)elem1)->ModuleName);
  const char *right = (((const PluginItem *)elem2)->ModuleName);
  DWORD try_left=(((const PluginItem *)elem1)->TryIfNoOther);
  DWORD try_right=(((const PluginItem *)elem2)->TryIfNoOther);

  if (try_left && !try_right)
    return 1;
  if (!try_left && try_right)
    return -1;

  return FSF.LStricmp(left,right);
}

int WINAPI ArcPlugins::LoadFmtModules(const WIN32_FIND_DATA *fdata,
                                     const char *PluginName,
                                     ArcPlugins *plugins)
{
  if(fdata->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
    return TRUE;

  struct PluginItem CurPlugin;
  HMODULE hModule=LoadLibrary(PluginName);

  if (hModule!=NULL)
  {
    lstrcpy(CurPlugin.ModuleName,fdata->cFileName);
    CurPlugin.hModule=hModule;
    CurPlugin.Flags=0;
    CurPlugin.TryIfNoOther=GetRegKey(HKEY_CURRENT_USER,"TryIfNoOther",CurPlugin.ModuleName,0);
    CurPlugin.pLoadFormatModule=(PLUGINLOADFORMATMODULE)GetProcAddress(hModule,"LoadFormatModule");
    CurPlugin.pIsArchive=(PLUGINISARCHIVE)GetProcAddress(hModule,"IsArchive");
    CurPlugin.pOpenArchive=(PLUGINOPENARCHIVE)GetProcAddress(hModule,"OpenArchive");
    CurPlugin.pGetArcItem=(PLUGINGETARCITEM)GetProcAddress(hModule,"GetArcItem");
    CurPlugin.pCloseArchive=(PLUGINCLOSEARCHIVE)GetProcAddress(hModule,"CloseArchive");
    CurPlugin.pGetFormatName=(PLUGINGETFORMATNAME)GetProcAddress(hModule,"GetFormatName");
    CurPlugin.pGetDefaultCommands=(PLUGINGETDEFAULTCOMMANDS)GetProcAddress(hModule,"GetDefaultCommands");
    CurPlugin.pSetFarInfo=(PLUGINSETFARINFO)GetProcAddress(hModule,"SetFarInfo");
    CurPlugin.pGetSFXPos=(PLUGINGETSFXPOS)GetProcAddress(hModule,"GetSFXPos");

    // обязательные!
    if(!(CurPlugin.pIsArchive &&
         CurPlugin.pOpenArchive &&
         CurPlugin.pGetArcItem /*&&
         CurPlugin.pGetFormatName &&
         CurPlugin.pGetDefaultCommands*/))
    {
      FreeLibrary(hModule);
      return TRUE;
    }

    if (CurPlugin.pLoadFormatModule)
      CurPlugin.pLoadFormatModule(PluginName);

    if (CurPlugin.pSetFarInfo)
    {
      // Дабы FMT не испортил оригинальный PluginStartupInfo дадим ему
      // временную "переменную"
      memcpy(&CurPlugin.Info,&Info,sizeof(struct PluginStartupInfo));
      memcpy(&CurPlugin.FSF, &FSF, sizeof(struct FarStandardFunctions));
      CurPlugin.Info.FSF=&CurPlugin.FSF;
      CurPlugin.pSetFarInfo(&CurPlugin.Info);
    }

    struct PluginItem *NewPluginsData=(struct PluginItem *)realloc(plugins->PluginsData,
                            sizeof(struct PluginItem)*(plugins->PluginsCount+1));
    if (NewPluginsData==NULL)
    {
      FreeLibrary(hModule);
      return FALSE; // при нехватке памяти остановим процесс поиска.
    }
    plugins->PluginsData=NewPluginsData;
    memcpy(&plugins->PluginsData[plugins->PluginsCount],&CurPlugin,sizeof(struct PluginItem));
    plugins->PluginsCount++;
  }
  return TRUE;
}

ArcPlugins::~ArcPlugins()
{
  for (int I=0;I<PluginsCount;I++)
    FreeLibrary(PluginsData[I].hModule);

  free(PluginsData);
}

int ArcPlugins::IsArchive(char *Name,const unsigned char *Data,int DataSize)
{
  DWORD MinSFXSize=0xFFFFFFFF;
  DWORD CurSFXSize;
  int TrueArc=-1;
  int I;

  for (I=0; I < PluginsCount; I++)
  {
    if (TrueArc>-1 && PluginsData[I].TryIfNoOther)
      break;
    if (IsArchive(I, Name, Data, DataSize, &CurSFXSize))
    {
      if(CurSFXSize <= MinSFXSize)
      {
        MinSFXSize=CurSFXSize;
        TrueArc=I;
      }
    }
  }
  return TrueArc;
}

BOOL ArcPlugins::IsArchive(int ArcPluginNumber, char *Name,const unsigned char *Data,int DataSize, DWORD* SFXSize)
{
  struct PluginItem *CurPluginsData=PluginsData+ArcPluginNumber;
  if (CurPluginsData->pIsArchive &&
    CurPluginsData->pIsArchive(Name,Data,DataSize))
  {
      if(CurPluginsData->pGetSFXPos)
        *SFXSize=CurPluginsData->pGetSFXPos();
      else
        *SFXSize=0;
    return TRUE;
  }
  return FALSE;
}

BOOL ArcPlugins::OpenArchive(int PluginNumber,char *Name,int *Type)
{
  *Type=0; //$ AA 12.11.2001
  if ((DWORD)PluginNumber < (DWORD)PluginsCount && PluginsData[PluginNumber].pOpenArchive)
    return PluginsData[PluginNumber].pOpenArchive(Name,Type);
  return FALSE;
}

int ArcPlugins::GetArcItem(int PluginNumber,struct PluginPanelItem *Item,
                           struct ArcItemInfo *Info)
{
  if ((DWORD)PluginNumber < (DWORD)PluginsCount && PluginsData[PluginNumber].pGetArcItem)
    return PluginsData[PluginNumber].pGetArcItem(Item,Info);
  return FALSE;
}


void ArcPlugins::CloseArchive(int PluginNumber,struct ArcInfo *Info)
{
  if ((DWORD)PluginNumber < (DWORD)PluginsCount && PluginsData[PluginNumber].pCloseArchive)
    PluginsData[PluginNumber].pCloseArchive(Info);
}


BOOL ArcPlugins::GetFormatName(int PluginNumber,int Type,char *FormatName,
                               char *DefaultExt)
{
  *FormatName=0;
  if ((DWORD)PluginNumber < (DWORD)PluginsCount && PluginsData[PluginNumber].pGetFormatName)
    return PluginsData[PluginNumber].pGetFormatName(Type,FormatName,DefaultExt);
  return FALSE;
}


BOOL ArcPlugins::GetDefaultCommands(int PluginNumber,int Type,int Command,
                                    char *Dest)
{
  *Dest=0;
  if ((DWORD)PluginNumber < (DWORD)PluginsCount && PluginsData[PluginNumber].pGetDefaultCommands)
    return PluginsData[PluginNumber].pGetDefaultCommands(Type,Command,Dest);
  return FALSE;
}
