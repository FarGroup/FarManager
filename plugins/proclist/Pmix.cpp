#include "Proclist.hpp"
#include "Proclng.hpp"

TCHAR *GetMsg(int MsgId)
{
  return (TCHAR*)Info.GetMsg(Info.ModuleNumber,MsgId);
}

void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,
                    int ItemsNumber)
{
  for (int i=0;i<ItemsNumber;i++)
  {
    Item[i].Type=Init[i].Type;
    Item[i].X1=Init[i].X1;
    Item[i].Y1=Init[i].Y1;
    Item[i].X2=Init[i].X2;
    Item[i].Y2=Init[i].Y2;
    Item[i].Focus=Init[i].Focus;
    Item[i].History=(const TCHAR *)Init[i].Selected;
    Item[i].Flags=Init[i].Flags;
    Item[i].DefaultButton=Init[i].DefaultButton;
#ifdef UNICODE
    Item[i].MaxLen=0;
#endif
#ifndef UNICODE
    if ((DWORD_PTR)Init[i].Data<2000)
      lstrcpy(Item[i].Data,GetMsg((unsigned int)(DWORD_PTR)Init[i].Data));
    else
      lstrcpy(Item[i].Data,Init[i].Data);
#else
    if ((DWORD_PTR)Init[i].Data<2000)
      Item[i].PtrData = GetMsg((unsigned int)(DWORD_PTR)Init[i].Data);
    else
      Item[i].PtrData = Init[i].Data;
#endif
  }
}

void ConvertDate(const FILETIME& ft,TCHAR *DateText,TCHAR *TimeText)
{
  if (ft.dwHighDateTime==0 && ft.dwLowDateTime==0)
  {
    if (DateText!=NULL)
      *DateText=0;
    if (TimeText!=NULL)
      *TimeText=0;
    return;
  }

  SYSTEMTIME st;
  FILETIME ct;

  FileTimeToLocalFileTime(&ft,&ct);
  FileTimeToSystemTime(&ct,&st);

  if (TimeText!=NULL)
      GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, 0, TimeText, MAX_DATETIME);

  if (DateText!=NULL)
      GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, 0, DateText, MAX_DATETIME);
}

int WinError(TCHAR* pSourceModule)
{
  TCHAR* lpMsgBuf; BOOL bAllocated = FALSE;
  DWORD dwLastErr = GetLastError();
  FormatMessage( pSourceModule ?
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE :
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      pSourceModule ? GetModuleHandle(pSourceModule): NULL, dwLastErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR)&lpMsgBuf, 0, NULL );
  TCHAR ErrBuf[32];
  if(lpMsgBuf)
    bAllocated = TRUE;
  else {
      FSF.sprintf(ErrBuf, _T("Error 0x%x"), dwLastErr);
      lpMsgBuf = ErrBuf;
  }
#ifndef UNICODE
  CharToOem(lpMsgBuf, lpMsgBuf);
#endif

  static const TCHAR* items[]={0,0,0,0};
  items[0] = GetMsg(MError); items[3] = GetMsg(MOk);

  if(lstrlen(lpMsgBuf) > 64)
      for(int i=lstrlen(lpMsgBuf)/2; i<lstrlen(lpMsgBuf); i++)
          if(lpMsgBuf[i]==_T(' ')) { lpMsgBuf[i] = _T('\n'); break; }
  items[1] = _tcstok(lpMsgBuf,_T("\r\n"));
  items[2] = _tcstok(NULL,_T("\r\n")); if(!items[2]) items[2] = items[3];
  int rc = Message(FMSG_WARNING,0,items,(int)(ArraySize(items) - (items[2]==items[3])));
  if(bAllocated) LocalFree( lpMsgBuf );
  return rc;
}

#ifndef UNICODE
OemString::OemString(LPCSTR pAnsi)
{
    pStr = new char[lstrlen(pAnsi)+1];
    CharToOem(pAnsi, pStr);
}

OemString::OemString(LPCWSTR pWide)
{
    int nBytes = lstrlenW(pWide)+1;
    pStr = new char[nBytes];
    WideCharToMultiByte(CP_OEMCP, 0, pWide, -1, pStr, nBytes, 0, 0);
}
#endif
