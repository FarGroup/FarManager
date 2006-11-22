/*
TMPMIX.CPP

Temporary panel miscellaneous utility functions

*/

#include "stdafx.h"

const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

void InitDialogItems(const MyInitDialogItem *Init,struct FarDialogItem *Item,
                    int ItemsNumber)
{
  int i;
  struct FarDialogItem *PItem=Item;
  const MyInitDialogItem *PInit=Init;
  for (i=0;i<ItemsNumber;i++,PItem++,PInit++)
  {
    PItem->Type=PInit->Type;
    PItem->X1=PInit->X1;
    PItem->Y1=PInit->Y1;
    PItem->X2=PInit->X2;
    PItem->Y2=PInit->Y2;
    PItem->Flags=PInit->Flags;
    PItem->Focus=0;
    PItem->Selected=0;
    PItem->DefaultButton=0;
    lstrcpy(PItem->Data,PInit->Data!=-1 ? GetMsg(PInit->Data) : "");
  }
}

void FreePanelItems(PluginPanelItem *Items, DWORD Total)
{
  if(Items){
    for (DWORD I=0;I<Total;I++)
      if (Items[I].Owner)
        my_free (Items[I].Owner);
    my_free (Items);
  }
}

char * my_strchr (const char * string,int ch)
{
  while (*string && *string != (char)ch)
    string++;
  if (*string == (char)ch)
    return((char *)string);
  return(NULL);
}

char * my_strrchr(const char * string,int ch)
{
  char *start = (char *)string;
  while (*string++);
  while (--string != start && *string != (char)ch);
  if (*string == (char)ch)
    return( (char *)string );
  return(NULL);
}

char *ParseParam(char *& str)
{
  char* p=str;
  char* parm=NULL;
  if(*p=='|'){
    parm=++p;
    p=my_strchr(p,'|');
    if(p){
      *p='\0';
      str=p+1;
      FSF.LTrim(str);
      return parm;
    }
  }
  return NULL;
}

void GoToFile(const char *Target, BOOL AnotherPanel)
{
  int FCTL_SetPanelDir = AnotherPanel?FCTL_SETANOTHERPANELDIR:FCTL_SETPANELDIR;
  int FCTL_GetPanelInfo = AnotherPanel?FCTL_GETANOTHERPANELINFO:FCTL_GETPANELINFO;
  int FCTL_RedrawPanel = AnotherPanel?FCTL_REDRAWANOTHERPANEL:FCTL_REDRAWPANEL;

  PanelRedrawInfo PRI;
  PanelInfo PInfo;
  char Name[NM], Dir[NM*5];
  int pathlen;

  lstrcpy(Name,FSF.PointToName(const_cast<char*>(Target)));
  pathlen=(int)(FSF.PointToName(const_cast<char*>(Target))-Target);
  if(pathlen)
    my_memcpy(Dir,Target,pathlen);
  Dir[pathlen]=0;

  FSF.Trim(Name);
  FSF.Trim(Dir);
  FSF.Unquote(Name);
  FSF.Unquote(Dir);

  if(*Dir)
    Info.Control(INVALID_HANDLE_VALUE,FCTL_SetPanelDir,&Dir);
  Info.Control(INVALID_HANDLE_VALUE,FCTL_GetPanelInfo,&PInfo);

  PRI.CurrentItem=PInfo.CurrentItem;
  PRI.TopPanelItem=PInfo.TopPanelItem;

  for(int J=0; J < PInfo.ItemsNumber; J++)
  {
    if(!FSF.LStricmp(Name,
       FSF.PointToName(PInfo.PanelItems[J].FindData.cFileName)))
    {
      PRI.CurrentItem=J;
      PRI.TopPanelItem=J;
      break;
    }
  }
  Info.Control(INVALID_HANDLE_VALUE,FCTL_RedrawPanel,&PRI);
}

void WFD2FFD(WIN32_FIND_DATA &wfd, FAR_FIND_DATA &ffd)
{
  ffd.dwFileAttributes=wfd.dwFileAttributes;
  ffd.ftCreationTime=wfd.ftCreationTime;
  ffd.ftLastAccessTime=wfd.ftLastAccessTime;
  ffd.ftLastWriteTime=wfd.ftLastWriteTime;
  ffd.nFileSizeHigh=wfd.nFileSizeHigh;
  ffd.nFileSizeLow=wfd.nFileSizeLow;
  ffd.dwReserved0=wfd.dwReserved0;
  ffd.dwReserved1=wfd.dwReserved1;
  lstrcpy(ffd.cFileName,wfd.cFileName);
  lstrcpy(ffd.cAlternateFileName,wfd.cAlternateFileName);
}

