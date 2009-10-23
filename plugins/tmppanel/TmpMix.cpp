/*
TMPMIX.CPP

Temporary panel miscellaneous utility functions

*/

#include "TmpPanel.hpp"

const TCHAR *GetMsg(int MsgId)
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
    PItem->History=0;
    PItem->DefaultButton=0;
#ifdef UNICODE
    PItem->MaxLen=0;
#endif
#ifndef UNICODE
    lstrcpy(PItem->Data,PInit->Data!=-1 ? GetMsg(PInit->Data) : "");
#else
    PItem->PtrData = PInit->Data!=-1 ? GetMsg(PInit->Data) : L"";
#endif
  }
}

void FreePanelItems(PluginPanelItem *Items, DWORD Total)
{
  if(Items){
    for (DWORD I=0;I<Total;I++) {
      if (Items[I].Owner)
        free (Items[I].Owner);
#ifdef UNICODE
      if (Items[I].FindData.lpwszFileName)
        free ((wchar_t*)Items[I].FindData.lpwszFileName);

      if (Items[I].FindData.lpwszAlternateFileName)
        free ((wchar_t*)Items[I].FindData.lpwszAlternateFileName);

#endif
    }
    free (Items);
  }
}

TCHAR *ParseParam(TCHAR *& str)
{
  TCHAR* p=str;
  TCHAR* parm=NULL;
  if(*p==_T('|')){
    parm=++p;
    p=_tcschr(p,_T('|'));
    if(p){
      *p=_T('\0');
      str=p+1;
      FSF.LTrim(str);
      return parm;
    }
  }
  return NULL;
}

void GoToFile(const TCHAR *Target, BOOL AnotherPanel)
{
#ifndef UNICODE
  int FCTL_SetPanelDir = AnotherPanel?FCTL_SETANOTHERPANELDIR:FCTL_SETPANELDIR;
  int FCTL_GetPanelInfo = AnotherPanel?FCTL_GETANOTHERPANELINFO:FCTL_GETPANELINFO;
  int FCTL_RedrawPanel = AnotherPanel?FCTL_REDRAWANOTHERPANEL:FCTL_REDRAWPANEL;
#define _PANEL_HANDLE  INVALID_HANDLE_VALUE
#else
#define FCTL_SetPanelDir  FCTL_SETPANELDIR
#define FCTL_GetPanelInfo FCTL_GETPANELINFO
#define FCTL_RedrawPanel  FCTL_REDRAWPANEL
  HANDLE  _PANEL_HANDLE = AnotherPanel?PANEL_PASSIVE:PANEL_ACTIVE;
#endif

  PanelRedrawInfo PRI;
  PanelInfo PInfo;
  int pathlen;

  const TCHAR *p = FSF.PointToName(const_cast<TCHAR*>(Target));
  StrBuf Name(lstrlen(p)+1);
  lstrcpy(Name,p);
  pathlen=(int)(p-Target);
  StrBuf Dir(pathlen+1);
  if (pathlen)
    memcpy(Dir.Ptr(),Target,pathlen*sizeof(TCHAR));
  Dir[pathlen]=_T('\0');

  FSF.Trim(Name);
  FSF.Trim(Dir);
  FSF.Unquote(Name);
  FSF.Unquote(Dir);

  if (*Dir.Ptr())
  {
#ifndef UNICODE
    Info.Control(_PANEL_HANDLE,FCTL_SetPanelDir,Dir.Ptr());
#else
    Info.Control(_PANEL_HANDLE,FCTL_SetPanelDir,0,(LONG_PTR)Dir.Ptr());
#endif
  }

#ifndef UNICODE
  Info.Control(_PANEL_HANDLE,FCTL_GetPanelInfo,&PInfo);
#else
  Info.Control(_PANEL_HANDLE,FCTL_GetPanelInfo,0,(LONG_PTR)&PInfo);
#endif

  PRI.CurrentItem=PInfo.CurrentItem;
  PRI.TopPanelItem=PInfo.TopPanelItem;

  for(int J=0; J < PInfo.ItemsNumber; J++)
  {

#ifndef UNICODE
#define FileName PInfo.PanelItems[J].FindData.cFileName
#else
#define FileName (PPI?PPI->FindData.lpwszFileName:NULL)
    PluginPanelItem* PPI=(PluginPanelItem*)malloc(Info.Control(_PANEL_HANDLE,FCTL_GETPANELITEM,J,0));
    if(PPI)
    {
      Info.Control(_PANEL_HANDLE,FCTL_GETPANELITEM,J,(LONG_PTR)PPI);
    }
#endif

    if(!FSF.LStricmp(Name,FSF.PointToName(FileName)))
#undef FileName
    {
      PRI.CurrentItem=J;
      PRI.TopPanelItem=J;
#ifdef UNICODE
      free(PPI);
#endif
      break;
    }
#ifdef UNICODE
    free(PPI);
#endif
  }
#ifndef UNICODE
  Info.Control(_PANEL_HANDLE,FCTL_RedrawPanel,&PRI);
#else
  Info.Control(_PANEL_HANDLE,FCTL_RedrawPanel,0,(LONG_PTR)&PRI);
#endif
#undef _PANEL_HANDLE
#undef FCTL_SetPanelDir
#undef FCTL_GetPanelInfo
#undef FCTL_RedrawPanel
}

void WFD2FFD(WIN32_FIND_DATA &wfd, FAR_FIND_DATA &ffd)
{
  ffd.dwFileAttributes=wfd.dwFileAttributes;
  ffd.ftCreationTime=wfd.ftCreationTime;
  ffd.ftLastAccessTime=wfd.ftLastAccessTime;
  ffd.ftLastWriteTime=wfd.ftLastWriteTime;
#ifndef UNICODE
  ffd.nFileSizeHigh=wfd.nFileSizeHigh;
  ffd.nFileSizeLow=wfd.nFileSizeLow;
#else
  ffd.nFileSize = wfd.nFileSizeHigh;
  ffd.nFileSize <<= 32;
  ffd.nFileSize |= wfd.nFileSizeLow;
#endif
#ifndef UNICODE
  ffd.dwReserved0=wfd.dwReserved0;
  ffd.dwReserved1=wfd.dwReserved1;
#else
  ffd.nPackSize = 0;
#endif
#ifndef UNICODE
  lstrcpy(ffd.cFileName,wfd.cFileName);
  lstrcpy(ffd.cAlternateFileName,wfd.cAlternateFileName);
#else
  ffd.lpwszFileName = wcsdup(wfd.cFileName);
  ffd.lpwszAlternateFileName = NULL;  // wcsdup(wfd.cAlternateFileName);
#endif
}

#ifdef UNICODE
wchar_t* NtPath(const wchar_t* path, wchar_t* buf) {
  int l = lstrlen(path);
  if (l > 4 && path[0] == L'\\' && path[1] == L'\\')
  {
    if ((path[2] == L'?' || path[2] == L'.') && path[3] == L'\\')
    {
      lstrcpy(buf, path);
    }
    else
    {
      lstrcpy(buf, L"\\\\?\\UNC\\");
      lstrcat(buf, path + 2);
    }
  }
  else
  {
    lstrcpy(buf, L"\\\\?\\");
    lstrcat(buf, path);
  }
  return buf;
}
#endif
