#include <windows.h>
#include "d:\lang\bc5\far\plugin.hpp"
#include "complng.hpp"
#include "compare.hpp"
#include "compmix.cpp"
#include "compreg.cpp"

#define FA_DIREC FILE_ATTRIBUTE_DIRECTORY

void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  strcpy(PluginRootKey,Info->RootKey);
  strcat(PluginRootKey,"\\AdvCompare");
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  Compare();
  return(INVALID_HANDLE_VALUE);
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=0;
  Info->DiskMenuStringsNumber=0;
  static char *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MCompare);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  Info->PluginConfigStringsNumber=0;
}


void Compare()
{
  struct InitDialogItem InitItems[]={
    DI_DOUBLEBOX,3,1,48,9,0,0,0,0,(char *)MCmpTitle,
    DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)MCompareSubfolders,
    DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MCompareTime,
    DI_CHECKBOX,9,4,0,0,0,0,0,0,(char *)MCompareLowPrecision,
    DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MCompareSize,
    DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MCompareContents,
    DI_TEXT,5,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,8,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,8,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  DialogItems[1].Selected=GetRegKey(HKEY_CURRENT_USER,"","ProcessSubfolders",0);
  DialogItems[2].Selected=GetRegKey(HKEY_CURRENT_USER,"","CompareTime",1);
  DialogItems[3].Selected=GetRegKey(HKEY_CURRENT_USER,"","LowPrecisionTime",1);
  DialogItems[4].Selected=GetRegKey(HKEY_CURRENT_USER,"","CompareSize",1);
  DialogItems[5].Selected=GetRegKey(HKEY_CURRENT_USER,"","CompareContents",0);
  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,52,11,"Contents",DialogItems,
                           sizeof(DialogItems)/sizeof(DialogItems[0]));
  if (ExitCode!=7)
    return;
  Opt.ProcessSubfolders=DialogItems[1].Selected;
  Opt.CompareTime=DialogItems[2].Selected;
  Opt.LowPrecisionTime=DialogItems[3].Selected;
  Opt.CompareSize=DialogItems[4].Selected;
  Opt.CompareContents=DialogItems[5].Selected;
  SetRegKey(HKEY_CURRENT_USER,"","ProcessSubfolders",Opt.ProcessSubfolders);
  SetRegKey(HKEY_CURRENT_USER,"","CompareTime",Opt.CompareTime);
  SetRegKey(HKEY_CURRENT_USER,"","LowPrecisionTime",Opt.LowPrecisionTime);
  SetRegKey(HKEY_CURRENT_USER,"","CompareSize",Opt.CompareSize);
  SetRegKey(HKEY_CURRENT_USER,"","CompareContents",Opt.CompareContents);

  struct PanelInfo AInfo,PInfo;
  Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&AInfo);
  Info.Control(INVALID_HANDLE_VALUE,FCTL_GETANOTHERPANELINFO,&PInfo);
  if (AInfo.PanelType!=PTYPE_FILEPANEL || PInfo.PanelType!=PTYPE_FILEPANEL ||
      (AInfo.Plugin || PInfo.Plugin) && (Opt.ProcessSubfolders || Opt.CompareContents))
  {
    char *MsgItems[]={GetMsg(MCmpTitle),GetMsg(MFilePanelsRequired),GetMsg(MOk)};
    Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
    return;
  }

  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);

  ShowMessage(NULL,NULL);

  for (int I=0;I<AInfo.ItemsNumber;I++)
    if (!Opt.ProcessSubfolders && (AInfo.PanelItems[I].FindData.dwFileAttributes & FA_DIREC))
      AInfo.PanelItems[I].Flags&=~PPIF_SELECTED;
    else
      AInfo.PanelItems[I].Flags|=PPIF_SELECTED;
  for (int I=0;I<PInfo.ItemsNumber;I++)
    if (!Opt.ProcessSubfolders && (PInfo.PanelItems[I].FindData.dwFileAttributes & FA_DIREC))
      PInfo.PanelItems[I].Flags&=~PPIF_SELECTED;
    else
      PInfo.PanelItems[I].Flags|=PPIF_SELECTED;

  for (int I=0;I<AInfo.ItemsNumber;I++)
    for (int J=0;J<PInfo.ItemsNumber;J++)
    {
      WIN32_FIND_DATA *AData,*PData;
      AData=&AInfo.PanelItems[I].FindData;
      PData=&PInfo.PanelItems[J].FindData;
      if (LocalStricmp(AData->cFileName,PData->cFileName)==0)
      {
        ShowMessage(AData->cFileName,PData->cFileName);
        int Equal=TRUE;
        if ((AData->dwFileAttributes & FA_DIREC)!=(PData->dwFileAttributes & FA_DIREC))
          Equal=FALSE;
        if (AData->dwFileAttributes & FA_DIREC)
          if (Opt.ProcessSubfolders && strcmp(AData->cFileName,"..")!=0)
          {
            char AName[NM],PName[NM];
            strcpy(AName,AInfo.CurDir);
            AddEndSlash(AName);
            strcat(AName,AData->cFileName);
            strcpy(PName,PInfo.CurDir);
            AddEndSlash(PName);
            strcat(PName,PData->cFileName);
            int CompareCode=CompareDirectories(AName,PName);
            if (CompareCode==1)
            {
              AInfo.PanelItems[I].Flags&=~PPIF_SELECTED;
              PInfo.PanelItems[J].Flags&=~PPIF_SELECTED;
              continue;
            }
            else
              if (CompareCode==-1)
              {
                I=AInfo.ItemsNumber;
                break;
              }
              else
                Equal=FALSE;
          }
          else
            Equal=FALSE;

        if (Opt.CompareSize && (AData->nFileSizeHigh!=PData->nFileSizeHigh ||
            AData->nFileSizeLow!=PData->nFileSizeLow))
          Equal=FALSE;


        if (Equal && Opt.CompareTime)
          if (Opt.LowPrecisionTime)
          {
            if (AData->ftLastWriteTime.dwLowDateTime!=PData->ftLastWriteTime.dwLowDateTime ||
                AData->ftLastWriteTime.dwHighDateTime!=PData->ftLastWriteTime.dwHighDateTime)
              Equal=FALSE;
          }
          else
          {
            WORD ADosDate,ADosTime,PDosDate,PDosTime;
            FileTimeToDosDateTime(&AData->ftLastWriteTime,&ADosDate,&ADosTime);
            FileTimeToDosDateTime(&PData->ftLastWriteTime,&PDosDate,&PDosTime);
            DWORD AFullDosTime,PFullDosTime;
            AFullDosTime=((DWORD)ADosDate<<16)+ADosTime;
            PFullDosTime=((DWORD)PDosDate<<16)+PDosTime;
            if (AFullDosTime!=PFullDosTime)
              Equal=FALSE;
          }

        if (Equal && Opt.CompareContents)
        {
          char AName[NM],PName[NM];
          strcpy(AName,AInfo.CurDir);
          AddEndSlash(AName);
          strcat(AName,AData->cFileName);
          strcpy(PName,PInfo.CurDir);
          AddEndSlash(PName);
          strcat(PName,PData->cFileName);
          if (!CompareContents(AName,PName))
            Equal=FALSE;
        }
        if (Equal)
        {
          AInfo.PanelItems[I].Flags&=~PPIF_SELECTED;
          PInfo.PanelItems[J].Flags&=~PPIF_SELECTED;
        }
      }
    }
  Info.RestoreScreen(hScreen);

  Info.Control(INVALID_HANDLE_VALUE,FCTL_SETSELECTION,&AInfo);
  Info.Control(INVALID_HANDLE_VALUE,FCTL_SETANOTHERSELECTION,&PInfo);
  Info.Control(INVALID_HANDLE_VALUE,FCTL_REDRAWPANEL,NULL);
  Info.Control(INVALID_HANDLE_VALUE,FCTL_REDRAWANOTHERPANEL,NULL);

}


int CompareContents(char *AName,char *PName)
{
  HANDLE hFileA=CreateFile(AName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (hFileA==INVALID_HANDLE_VALUE)
    return(FALSE);
  HANDLE hFileP=CreateFile(PName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (hFileP==INVALID_HANDLE_VALUE)
  {
    CloseHandle(hFileA);
    return(FALSE);
  }
  DWORD ReadSizeA,ReadSizeP;
  char ABuf[32768],PBuf[32768];
  int Equal=TRUE;
  while (1)
  {
    if (CheckForEsc() || !ReadFile(hFileA,ABuf,sizeof(ABuf),&ReadSizeA,NULL))
    {
      Equal=FALSE;
      break;
    }
    if (!ReadFile(hFileP,PBuf,sizeof(PBuf),&ReadSizeP,NULL))
    {
      Equal=FALSE;
      break;
    }
    if (ReadSizeA!=ReadSizeP)
    {
      Equal=FALSE;
      break;
    }
    if (ReadSizeA==0)
      break;
    if (memcmp(ABuf,PBuf,ReadSizeA)!=0)
    {
      Equal=FALSE;
      break;
    }
  }
  CloseHandle(hFileA);
  CloseHandle(hFileP);
  return(Equal);
}


int CompareDirectories(char *ADir,char *PDir)
{
  PluginPanelItem *PanelItemA,*PanelItemP;
  int ItemsNumberA,ItemsNumberP;
  if (!Info.GetDirList(ADir,&PanelItemA,&ItemsNumberA))
  {
    Info.FreeDirList(PanelItemA);
    return(-1);
  }
  if (!Info.GetDirList(PDir,&PanelItemP,&ItemsNumberP))
  {
    Info.FreeDirList(PanelItemA);
    Info.FreeDirList(PanelItemP);
    return(-1);
  }
  if (ItemsNumberA!=ItemsNumberP)
  {
    Info.FreeDirList(PanelItemA);
    Info.FreeDirList(PanelItemP);
    return(FALSE);
  }
  for (int I=0;I<ItemsNumberA;I++)
  {
    int Equal=FALSE;
    for (int J=0;J<ItemsNumberP;J++)
    {
      if (CheckForEsc())
        return(-1);
      WIN32_FIND_DATA *AData,*PData;
      AData=&PanelItemA[I].FindData;
      PData=&PanelItemP[J].FindData;
      if (LocalStricmp(AData->cFileName,PData->cFileName)==0)
      {
        ShowMessage(AData->cFileName,PData->cFileName);
        Equal=TRUE;
        if ((AData->dwFileAttributes & FA_DIREC)!=(PData->dwFileAttributes & FA_DIREC))
          Equal=FALSE;
        if (AData->dwFileAttributes & FA_DIREC)
          break;

        if (Opt.CompareSize && (AData->nFileSizeHigh!=PData->nFileSizeHigh ||
            AData->nFileSizeLow!=PData->nFileSizeLow))
          Equal=FALSE;
 
        if (Equal && Opt.CompareTime)
          if (Opt.LowPrecisionTime)
          {
            if (AData->ftLastWriteTime.dwLowDateTime!=PData->ftLastWriteTime.dwLowDateTime ||
                AData->ftLastWriteTime.dwHighDateTime!=PData->ftLastWriteTime.dwHighDateTime)
              Equal=FALSE;
          }
          else
          {
            WORD ADosDate,ADosTime,PDosDate,PDosTime;
            FileTimeToDosDateTime(&AData->ftLastWriteTime,&ADosDate,&ADosTime);
            FileTimeToDosDateTime(&PData->ftLastWriteTime,&PDosDate,&PDosTime);
            DWORD AFullDosTime,PFullDosTime;
            AFullDosTime=((DWORD)ADosDate<<16)+ADosTime;
            PFullDosTime=((DWORD)PDosDate<<16)+PDosTime;
            int D=AFullDosTime-PFullDosTime;
            if (D<-1 || D>1)
              Equal=FALSE;
          }
 
        if (Equal && Opt.CompareContents)
        {
          char AName[NM],PName[NM],*Slash;
          strcpy(AName,ADir);
          if ((Slash=strrchr(AName,'\\'))!=NULL)
            *(Slash+1)=0;
          strcat(AName,AData->cFileName);
          strcpy(PName,PDir);
          if ((Slash=strrchr(PName,'\\'))!=NULL)
            *(Slash+1)=0;
          strcat(PName,PData->cFileName);
          if (!CompareContents(AName,PName))
            Equal=FALSE;
        }
        if (Equal)
          break;
      }
    }
    if (!Equal)
    {
      Info.FreeDirList(PanelItemA);
      Info.FreeDirList(PanelItemP);
      return(FALSE);
    }
  }
  Info.FreeDirList(PanelItemA);
  Info.FreeDirList(PanelItemP);
  return(TRUE);
}


void ShowMessage(char *Name1,char *Name2)
{
  if (!Opt.CompareContents)
  {
    if (Name1==NULL)
    {
      char *MsgItems[]={"",GetMsg(MComparingFiles)};
      Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
    }
    return;
  }
  char *MsgItems[]={GetMsg(MCmpTitle),GetMsg(MComparing),"",GetMsg(MComparingWith),""};
  char TruncName1[NM],TruncName2[NM];

  memset(TruncName1,' ',40);
  memset(TruncName2,' ',40);
  TruncName1[40]=TruncName2[40]=0;
  if (Name1!=NULL)
  {
    int Length1=strlen(Name1);
    if (Length1<40)
      strncpy(TruncName1,Name1,strlen(Name1));
    else
    {
      strcpy(TruncName1,Name1);
      TruncStr(TruncName1,40);
    }
    int Length2=strlen(Name1);
    if (Length2<40)
      strncpy(TruncName2,Name2,strlen(Name2));
    else
    {
      strcpy(TruncName2,Name2);
      TruncStr(TruncName2,40);
    }
  }

  MsgItems[2]=TruncName1;
  MsgItems[4]=TruncName2;
  int Flags=(Name1==NULL) ? FMSG_LEFTALIGN:FMSG_LEFTALIGN|FMSG_KEEPBACKGROUND;
  Info.Message(Info.ModuleNumber,Flags,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
}
