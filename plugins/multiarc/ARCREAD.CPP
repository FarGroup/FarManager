#include "plugin.hpp"
#include "fmt.hpp"
#include "multiarc.hpp"
#include "marclng.hpp"

PluginClass::PluginClass(int ArcPluginNumber)
{
  *ArcName=0;
  *CurDir=0;
  ArcData=NULL;
  ArcDataCount=0;
  PluginClass::ArcPluginNumber=ArcPluginNumber;
  memset(&CurArcInfo,0,sizeof(struct ArcInfo));
  DizPresent=FALSE;
}


PluginClass::~PluginClass()
{
  FreeArcData();
}

void PluginClass::FreeArcData()
{
  if(ArcData)
  {
    for (int I=0;I<ArcDataCount;I++)
    {
      if (ArcData[I].Description!=NULL)
        delete ArcData[I].Description;
      if(ArcData[I].UserData)
        free((void *)ArcData[I].UserData);
    }
    free (ArcData);
    ArcData=NULL;
  }
  ArcDataCount=0;
}

int PluginClass::ReadArchive(char *Name)
{
  FreeArcData();
  DizPresent=FALSE;

  HANDLE ArcFindHandle;
  ArcFindHandle=FindFirstFile(Name,&ArcFindData);
  FindClose(ArcFindHandle);

  if (ArcFindHandle==INVALID_HANDLE_VALUE)
    return FALSE;

  if (!ArcPlugin->OpenArchive(ArcPluginNumber,Name,&ArcPluginType))
    return FALSE;

  strcpy(ArcName,Name);

  if (strchr(FSF.PointToName(ArcName),'.')==NULL)
    strcat(ArcName,".");

  memset(&ItemsInfo,0,sizeof(ItemsInfo));
  memset(&CurArcInfo,0,sizeof(CurArcInfo));
  TotalSize=PackedSize=0;

  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);

  DWORD StartTime=GetTickCount();//clock();
  int WaitMessage=FALSE;
  int AllocatedCount=0;
  int GetItemCode;

  while (1)
  {
    struct PluginPanelItem CurArcData;
    struct ArcItemInfo CurItemInfo;
    memset(&CurArcData,0,sizeof(CurArcData));
    memset(&CurItemInfo,0,sizeof(CurItemInfo));
    GetItemCode=ArcPlugin->GetArcItem(ArcPluginNumber,&CurArcData,&CurItemInfo);

    if (GetItemCode!=GETARC_SUCCESS)
      break;

    if ((ArcDataCount & 0x1f)==0)
    {
      if (CheckForEsc())
      {
        FreeArcData();
        ArcPlugin->CloseArchive(ArcPluginNumber,&CurArcInfo);
        Info.RestoreScreen(NULL);
        Info.RestoreScreen(hScreen);
        return -1;
      }

      if (GetTickCount()-StartTime>1000)
      {
        char FilesMsg[100];
        char NameMsg[NM];
        FSF.sprintf(FilesMsg,GetMsg(MArcReadFiles),ArcDataCount);
        const char *MsgItems[]={GetMsg(MArcReadTitle),GetMsg(MArcReading),NameMsg,FilesMsg};
        FSF.TruncPathStr(strncpy(NameMsg,Name,sizeof(NameMsg)-1),MAX_WIDTH_MESSAGE);
        Info.Message(Info.ModuleNumber,WaitMessage ? FMSG_KEEPBACKGROUND:0,NULL,MsgItems,
                   sizeof(MsgItems)/sizeof(MsgItems[0]),0);
        WaitMessage=TRUE;
      }
    }

    if (*CurItemInfo.Description)
    {
      CurArcData.Description=new char[strlen(CurItemInfo.Description)+1];
      if (CurArcData.Description)
        strcpy(CurArcData.Description,CurItemInfo.Description);
      DizPresent=TRUE;
    }

    if (strcmp(ItemsInfo.HostOS,CurItemInfo.HostOS)!=0)
      strcpy(ItemsInfo.HostOS,(*ItemsInfo.HostOS?GetMsg(MSeveralOS):CurItemInfo.HostOS));

    CurArcData.Flags=0;
    ItemsInfo.Solid|=CurItemInfo.Solid;
    ItemsInfo.Comment|=CurItemInfo.Comment;
    ItemsInfo.Encrypted|=CurItemInfo.Encrypted;

    if (CurItemInfo.Encrypted)
      CurArcData.Flags|=F_ENCRYPTED;

    if (CurItemInfo.DictSize>ItemsInfo.DictSize)
      ItemsInfo.DictSize=CurItemInfo.DictSize;

    if (CurItemInfo.UnpVer>ItemsInfo.UnpVer)
      ItemsInfo.UnpVer=CurItemInfo.UnpVer;

    *CurArcData.FindData.cAlternateFileName=0;

    char *NamePtr=CurArcData.FindData.cFileName;
    for (int I=0; *NamePtr ;I++, NamePtr++)
      if (*NamePtr=='/')
        *NamePtr='\\';

    CurArcData.UserData=0;

    NamePtr=CurArcData.FindData.cFileName;
    char *EndPos=NamePtr;
    while(*EndPos == '.') EndPos++;
    if(*EndPos == '\\')
      while(*EndPos == '\\') EndPos++;
    else
      EndPos=NamePtr;
    if(EndPos != NamePtr)
    {
      char *Pref=(char *)malloc((int)(EndPos-NamePtr)+1);
      if(Pref)
      {
        memcpy(Pref,NamePtr,(int)(EndPos-NamePtr));
        Pref[(int)(EndPos-NamePtr)]=0;
        CurArcData.UserData=(DWORD)Pref;
      }
    }

    if (EndPos!=CurArcData.FindData.cFileName)
      memmove(CurArcData.FindData.cFileName,EndPos,strlen(EndPos)+1);

    int Length=strlen(CurArcData.FindData.cFileName);

    if (Length>0 && (CurArcData.FindData.cFileName[Length-1]=='\\'))
    {
      CurArcData.FindData.cFileName[Length-1]=0;
      CurArcData.FindData.dwFileAttributes|=FILE_ATTRIBUTE_DIRECTORY;
    }

    struct PluginPanelItem *NewArcData=ArcData;

    if (ArcDataCount>=AllocatedCount)
    {
      AllocatedCount=AllocatedCount+256+AllocatedCount/4;
      NewArcData=(PluginPanelItem *)realloc(ArcData,AllocatedCount*sizeof(*ArcData));
    }

    if (NewArcData==NULL)
      break;

    TotalSize+=(((__int64)CurArcData.FindData.nFileSizeHigh)<<32)|(__int64)CurArcData.FindData.nFileSizeLow;
    PackedSize+=(((__int64)CurArcData.PackSizeHigh)<<32)|(__int64)CurArcData.PackSize;


    ArcData=NewArcData;
    ArcData[ArcDataCount]=CurArcData;
    ArcDataCount++;
  }

  if (ArcDataCount>0)
    ArcData=(PluginPanelItem *)realloc(ArcData,ArcDataCount*sizeof(*ArcData));

  ArcPlugin->CloseArchive(ArcPluginNumber,&CurArcInfo);

  if(GetItemCode != GETARC_EOF && GetItemCode != GETARC_SUCCESS)
  {
    switch(GetItemCode)
    {
      case GETARC_BROKEN:
        GetItemCode=MBadArchive;
        break;

      case GETARC_UNEXPEOF:
        GetItemCode=MUnexpEOF;
        break;

      case GETARC_READERROR:
        GetItemCode=MReadError;
        break;
    }

    char NameMsg[NM];
    const char *MsgItems[]={GetMsg(MError),NameMsg,GetMsg(GetItemCode),GetMsg(MOk)};
    FSF.TruncPathStr(strncpy(NameMsg,Name,sizeof(NameMsg)-1),MAX_WIDTH_MESSAGE);
    Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
  }

  Info.RestoreScreen(NULL);
  Info.RestoreScreen(hScreen);
  return TRUE;
}


int PluginClass::GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  HANDLE ArcFindHandle;
  WIN32_FIND_DATA NewArcFindData;
  ArcFindHandle=FindFirstFile(ArcName,&NewArcFindData);
  FindClose(ArcFindHandle);

  if (ArcFindHandle==INVALID_HANDLE_VALUE)
    return FALSE;

  if (CompareFileTime(&NewArcFindData.ftLastWriteTime,&ArcFindData.ftLastWriteTime)!=0 ||
      NewArcFindData.nFileSizeLow!=ArcFindData.nFileSizeLow || ArcData==NULL)
  {
	BOOL ReadArcOK=FALSE;
	DWORD size = Info.AdvControl(Info.ModuleNumber,ACTL_GETPLUGINMAXREADDATA,(void *)0);
	HANDLE h=CreateFile(ArcName, FILE_READ_DATA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (INVALID_HANDLE_VALUE!=h)
	{
		unsigned char *Data=new unsigned char[size];
		DWORD read;
		if (Data && ReadFile(h, Data, size, &read, 0))
		{
			DWORD SFXSize;
			if (ArcPlugin->IsArchive(ArcPluginNumber, ArcName, Data, read, &SFXSize))
			{
				ReadArcOK=ReadArchive(ArcName);
			}
		}
		delete[] Data;
		CloseHandle(h);
	}
	if (!ReadArcOK) return FALSE;
  }

  int CurDirLength=strlen(CurDir);
  *pPanelItem=NULL;
  *pItemsNumber=0;
  int AlocatedItemsNumber=0;

  for (int I=0;I<ArcDataCount;I++)
  {
    char Name[NM];
    PluginPanelItem CurItem=ArcData[I];
    BOOL Append=FALSE;
    strcpy(Name,CurItem.FindData.cFileName);

    if (Name[0]=='\\')
      Append=TRUE;

    if (Name[0]=='.' && (Name[1]=='\\' || Name[1]=='.' && Name[2]=='\\'))
      Append=TRUE;

    if (!Append &&
         strlen(Name)>static_cast<size_t>(CurDirLength) &&
         strnicmp(Name,CurDir,CurDirLength)==0 &&
         (CurDirLength==0 || Name[CurDirLength]=='\\'))
    {
      char *StartName,*EndName;
      StartName=Name+CurDirLength+(CurDirLength!=0);

      if ((EndName=strchr(StartName,'\\'))!=NULL)
      {
        *EndName=0;
        CurItem.FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
        CurItem.FindData.nFileSizeLow=CurItem.PackSize=0;
      }

      strcpy(CurItem.FindData.cFileName,StartName);
      Append=TRUE;

      if (CurItem.FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        for (int J=0; J < *pItemsNumber; J++)
          if ((*pPanelItem)[J].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            if (stricmp(CurItem.FindData.cFileName,(*pPanelItem)[J].FindData.cFileName)==0)
              Append=FALSE;
      }
    }

    if (Append)
    {
      PluginPanelItem *NewPanelItem=*pPanelItem;
      if (*pItemsNumber>=AlocatedItemsNumber)
      {
        AlocatedItemsNumber=AlocatedItemsNumber+256+AlocatedItemsNumber/4;
        NewPanelItem=(PluginPanelItem *)realloc(*pPanelItem,AlocatedItemsNumber*sizeof(PluginPanelItem));

        if (NewPanelItem==NULL)
          break;

        *pPanelItem=NewPanelItem;
      }
      NewPanelItem[*pItemsNumber]=CurItem;
      (*pItemsNumber)++;
    }
  }
  if (*pItemsNumber>0)
    *pPanelItem=(PluginPanelItem *)realloc(*pPanelItem,*pItemsNumber*sizeof(PluginPanelItem));
  return TRUE;
}


void PluginClass::FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber)
{
  if(PanelItem) free(PanelItem);
}


int PluginClass::SetDirectory(const char *Dir,int OpMode)
{
  if (*Dir=='\\' && *(++Dir)==0)
  {
    *CurDir=0;
    return TRUE;
  }

  if (strcmp(Dir,"..")==0)
  {
    if (*CurDir==0)
      return FALSE;

    char *Slash=strrchr(CurDir,'\\');
    if (Slash!=NULL)
      *Slash=0;
    else
      *CurDir=0;
  }
  else
  {
    int Found=FALSE;
    int CurDirLength=strlen(CurDir);
    if (CurDirLength!=0)
      CurDirLength++;

    int NewDirLength=strlen(Dir);

    for (int I=0;I<ArcDataCount;I++)
    {
      char *CurName=ArcData[I].FindData.cFileName;

      if (strlen(CurName)>=static_cast<size_t>(CurDirLength+NewDirLength) &&
          LocalStrnicmp(CurName+CurDirLength,Dir,NewDirLength)==0)
      {
        char Ch=CurName[CurDirLength+NewDirLength];
        if (Ch=='\\' || Ch=='/' || Ch==0)
        {
          Found=TRUE;
          break;
        }
      }
    }

    if (!Found)
      return FALSE;

    if (*CurDir==0 || *Dir==0 || strchr(Dir,'\\')!=0)
      strcpy(CurDir,Dir);
    else
    {
      FSF.AddEndSlash(CurDir);
      strcat(CurDir,Dir);
    }
  }

  return TRUE;
}


void PluginClass::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=OPIF_USEFILTER|OPIF_USESORTGROUPS|OPIF_USEHIGHLIGHTING|
              OPIF_ADDDOTS|OPIF_COMPAREFATTIME;
  Info->HostFile=ArcName;
  Info->CurDir=CurDir;

  char FormatName[100];
  static char Format[100],DefExt[NM],Title[NM];

  ArcPlugin->GetFormatName(ArcPluginNumber,ArcPluginType,FormatName,DefExt);
  FSF.sprintf(Format,GetMsg(MArcFormat),FormatName);
  Info->Format=Format;

  char NameTitle[NM*2];
  strncpy(NameTitle,FSF.PointToName(ArcName),sizeof(NameTitle)-1);

  {
    struct PanelInfo PInfo;
    if(::Info.Control((HANDLE)this,FCTL_GETPANELSHORTINFO,&PInfo))
    {     //TruncStr
      FSF.TruncPathStr(NameTitle,(PInfo.PanelRect.right-PInfo.PanelRect.left+1-(strlen(FormatName)+3+4)));
    }
  }

  FSF.sprintf(Title," %s:%s",FormatName,NameTitle);
    if (*CurDir)
  {
    strcat(Title,"\\");
    strcat(Title,CurDir);
  }
  strcat(Title," ");

  Info->PanelTitle=Title;

  static struct InfoPanelLine InfoLines[15];

  memset(InfoLines,0,sizeof(InfoLines));
  FSF.sprintf(InfoLines[0].Text,GetMsg(MInfoTitle),FSF.PointToName(ArcName));
  InfoLines[0].Separator=TRUE;
  FSF.sprintf(InfoLines[1].Text,GetMsg(MInfoArchive));
  strcpy(InfoLines[1].Data,FormatName);

  if (ItemsInfo.UnpVer!=0)
    FSF.sprintf(InfoLines[1].Data+strlen(InfoLines[1].Data)," %d.%d",
            ItemsInfo.UnpVer/256,ItemsInfo.UnpVer%256);

  if (*ItemsInfo.HostOS)
    FSF.sprintf(InfoLines[1].Data+strlen(InfoLines[1].Data),"/%s",ItemsInfo.HostOS);

  strcpy(InfoLines[2].Text,GetMsg(MInfoArcType));

  if (ItemsInfo.Solid)
    strcpy(InfoLines[2].Data,GetMsg(MInfoSolid));

  if (CurArcInfo.SFXSize)
  {
    if (*InfoLines[2].Data)
      strcat(InfoLines[2].Data," ");
    strcat(InfoLines[2].Data,GetMsg(MInfoSFX));
  }

  if (CurArcInfo.Volume)
  {
    if (*InfoLines[2].Data)
      strcat(InfoLines[2].Data," ");
    strcat(InfoLines[2].Data,GetMsg(MInfoVolume));
  }

  if (*InfoLines[2].Data==0)
    strcpy(InfoLines[2].Data,GetMsg(MInfoNormal));

  strcpy(InfoLines[3].Text,GetMsg(MInfoArcComment));
  strcpy(InfoLines[3].Data,CurArcInfo.Comment ? GetMsg(MInfoPresent):GetMsg(MInfoAbsent));
  strcpy(InfoLines[4].Text,GetMsg(MInfoFileComments));
  strcpy(InfoLines[4].Data,ItemsInfo.Comment ? GetMsg(MInfoPresent):GetMsg(MInfoAbsent));
  strcpy(InfoLines[5].Text,GetMsg(MInfoPasswords));
  strcpy(InfoLines[5].Data,ItemsInfo.Encrypted ? GetMsg(MInfoPresent):GetMsg(MInfoAbsent));
  strcpy(InfoLines[6].Text,GetMsg(MInfoRecovery));
  strcpy(InfoLines[6].Data,CurArcInfo.Recovery ? GetMsg(MInfoPresent):GetMsg(MInfoAbsent));
  strcpy(InfoLines[7].Text,GetMsg(MInfoLock));
  strcpy(InfoLines[7].Data,CurArcInfo.Lock ? GetMsg(MInfoPresent):GetMsg(MInfoAbsent));
  strcpy(InfoLines[8].Text,GetMsg(MInfoAuthVer));
  strcpy(InfoLines[8].Data,(CurArcInfo.Flags & AF_AVPRESENT) ? GetMsg(MInfoPresent):GetMsg(MInfoAbsent));
  strcpy(InfoLines[9].Text,GetMsg(MInfoDict));

  if (ItemsInfo.DictSize==0)
    strcpy(InfoLines[9].Data,"???");
  else
    FSF.sprintf(InfoLines[9].Data,"%d %s",ItemsInfo.DictSize,GetMsg(MInfoDictKb));

  strcpy(InfoLines[10].Text,GetMsg(MInfoChapters));
  if(CurArcInfo.Chapters)
    //FSF.sprintf(InfoLines[10].Data,"%d/%d",ItemsInfo.Chapter,CurArcInfo.Chapters);
    FSF.sprintf(InfoLines[10].Data,"%d",CurArcInfo.Chapters);
  else
    strcpy(InfoLines[10].Data,GetMsg(MInfoAbsent));

  strcpy(InfoLines[11].Text,GetMsg(MInfoTotalFiles));
  FSF.sprintf(InfoLines[11].Data,"%d",ArcDataCount);
  strcpy(InfoLines[12].Text,GetMsg(MInfoTotalSize));
  InsertCommas(TotalSize,InfoLines[12].Data);
  strcpy(InfoLines[13].Text,GetMsg(MInfoPackedSize));
  InsertCommas(PackedSize,InfoLines[13].Data);
  strcpy(InfoLines[14].Text,GetMsg(MInfoRatio));
  FSF.sprintf(InfoLines[14].Data,"%d%%",ToPercent(PackedSize,TotalSize));

  Info->InfoLines=InfoLines;
  Info->InfoLinesNumber=sizeof(InfoLines)/sizeof(InfoLines[0]);

  static char *DescrFiles[32],DescrFilesString[256];
  strcpy(DescrFilesString,Opt.DescriptionNames);
  int DescrFilesNumber=0;
  char *NamePtr=DescrFilesString;

  while (DescrFilesNumber<sizeof(DescrFiles)/sizeof(DescrFiles[0]))
  {
    while (__isspace(*NamePtr))
      NamePtr++;
    if (*NamePtr==0)
      break;
    DescrFiles[DescrFilesNumber++]=NamePtr;
    if ((NamePtr=strchr(NamePtr,','))==NULL)
      break;
    *(NamePtr++)=0;
  }

  Info->DescrFiles=DescrFiles;

  if (!Opt.ReadDescriptions || DizPresent)
    Info->DescrFilesNumber=0;
  else
    Info->DescrFilesNumber=DescrFilesNumber;

  static struct KeyBarTitles KeyBar;
  memset(&KeyBar,0,sizeof(KeyBar));
  KeyBar.ShiftTitles[1-1]="";
  KeyBar.AltTitles[6-1]=(char*)GetMsg(MAltF6);
  KeyBar.AltShiftTitles[9-1]=(char*)GetMsg(MAltShiftF9);
  Info->KeyBar=&KeyBar;
}
