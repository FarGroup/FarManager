#include "proclist.hpp"
#include "proclng.hpp"

char *GetMsg(int MsgId)
{
  return (char*)Info.GetMsg(Info.ModuleNumber,MsgId);
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
    Item[i].Selected=Init[i].Selected;
    Item[i].Flags=Init[i].Flags;
    Item[i].DefaultButton=Init[i].DefaultButton;
    if ((unsigned int)Init[i].Data<2000)
      lstrcpy(Item[i].Data,GetMsg((unsigned int)Init[i].Data));
    else
      lstrcpy(Item[i].Data,Init[i].Data);
  }
}

/*
int LocalStricmp(char *Str1,char *Str2)
{
  char AnsiStr1[8192],AnsiStr2[8192];
  OemToChar(Str1,AnsiStr1);
  OemToChar(Str2,AnsiStr2);
  CharLower(AnsiStr1);
  CharLower(AnsiStr2);
  return(stricmp(AnsiStr1,AnsiStr2));
}
*/

/*
void AddEndSlash(char *Path)
{
  int Length=lstrlen(Path);
  if (Length==0 || Path[Length-1]!='\\' && Path[Length-1]!='/')
    lstrcat(Path,"\\");
}
*/

void ConvertDate(const FILETIME& ft,char *DateText,char *TimeText)
{
/*  static int DateFormat,DateSeparator,TimeSeparator;
  static int Init=FALSE;
  if (!Init)
  {
    DateFormat=GetDateFormat();
    DateSeparator=GetDateSeparator();
    TimeSeparator=GetTimeSeparator();
    Init=TRUE;
  }
*/
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
  {
      GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, 0, TimeText, MAX_DATETIME);
  /*
    char *Letter="";
    if (TimeLength==6)
    {
      Letter=(st.wHour<12) ? "a":"p";
      if (st.wHour>12)
        st.wHour-=12;
      if (st.wHour==0)
        st.wHour=12;
    }
    if (TimeLength<7)
      FSF.sprintf(TimeText,"%02d%c%02d%s",st.wHour,TimeSeparator,st.wMinute,Letter);
    else
    {
      char FullTime[100];
      FSF.sprintf(FullTime,"%02d%c%02d%c%02d.%03d",st.wHour,TimeSeparator,
              st.wMinute,TimeSeparator,st.wSecond,st.wMilliseconds);
      FSF.sprintf(TimeText,"%.*s",TimeLength,FullTime);
    }*/
  }

  if (DateText!=NULL)
  {
      GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, 0, DateText, MAX_DATETIME);
/*
    st.wYear%=100;
    switch(DateFormat)
    {
      case 0:
        FSF.sprintf(DateText,"%02d%c%02d%c%02d",st.wMonth,DateSeparator,st.wDay,DateSeparator,st.wYear);
        break;
      case 1:
        FSF.sprintf(DateText,"%02d%c%02d%c%02d",st.wDay,DateSeparator,st.wMonth,DateSeparator,st.wYear);
        break;
      default:
        FSF.sprintf(DateText,"%02d%c%02d%c%02d",st.wYear,DateSeparator,st.wMonth,DateSeparator,st.wDay);
        break;
    }*/
  }
}

/*
int GetDateFormat()
{
  char Info[100];
  GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IDATE,Info, sizeof(Info));
  return(FSF.atoi(Info));
}

int GetDateSeparator()
{
  char Info[100];
  GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SDATE,Info, sizeof(Info));
  return(*Info);
}


int GetTimeSeparator()
{
  char Info[100];
  GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_STIME,Info, sizeof(Info));
  return(*Info);
}
*/

int WinError(char* pSourceModule, BOOL bDown)
{
  char* lpMsgBuf; BOOL bAllocated = FALSE;
  DWORD dwLastErr = GetLastError();
  FormatMessage( pSourceModule ?
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE :
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      pSourceModule ? GetModuleHandle(pSourceModule): NULL, dwLastErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR)&lpMsgBuf, 0, NULL );
  char ErrBuf[32];
  if(lpMsgBuf)
    bAllocated = TRUE;
  else {
      FSF.sprintf(ErrBuf, "Error 0x%x", dwLastErr);
      lpMsgBuf = ErrBuf;
  }
  CharToOem(lpMsgBuf, lpMsgBuf);

  static const char* items[]={0,0,0,0};
  items[0] = GetMsg(MError); items[3] = GetMsg(MOk);

  if(lstrlen(lpMsgBuf) > 64)
      for(int i=lstrlen(lpMsgBuf)/2; i<lstrlen(lpMsgBuf); i++)
          if(lpMsgBuf[i]==' ') { lpMsgBuf[i] = '\n'; break; }
  items[1] = strtok(lpMsgBuf,"\r\n");
  items[2] = strtok(NULL,"\r\n"); if(!items[2]) items[2] = items[3];
  int rc = Message(bDown ? FMSG_WARNING|FMSG_DOWN : FMSG_WARNING,
        0,items,sizeof(items)/sizeof(*items) - (items[2]==items[3]));
  if(bAllocated) LocalFree( lpMsgBuf );
  return rc;
}

OemString::OemString(LPCSTR pAnsi)
{
    pStr = new char[lstrlen(pAnsi)+1];
    //if(!Opt.AnsiOutput)
    CharToOem(pAnsi, pStr);
}

OemString::OemString(LPCWSTR pWide)
{
    int nBytes = lstrlenW(pWide)+1;
    pStr = new char[nBytes];
    //if(!Opt.AnsiOutput)
    WideCharToMultiByte(CP_OEMCP, 0, pWide, -1, pStr, nBytes, 0, 0);
    //else
}
