/*
clipboard.cpp

Работа с буфером обмена.

*/

/* Revision: 1.11 01.03.2004 $ */

/*
Modify:
  01.03.2004 SVS
    ! Обертки FAR_OemTo* и FAR_CharTo* вокруг одноименных WinAPI-функций
      (задел на будущее + править впоследствии только 1 файл)
  21.04.2003 SVS
    ! Учтем состояние, когда AnsiMode=1 - в этом случае не "корежим"
      передаваемые в/из клипьорда данные (осталось докрутить редактор)
      Для этого: редактор должен вызывать InternalCopyToClipboard и
      InternalPasteFromClipboard/InternalPasteFromClipboardEx
  10.01.2003 SVS
    + Врапим функции работы с клипбордом на свои и... эмулируем работу
      этого самого клипборда в нужные моменты (глобальная переменная
      UsedInternalClipboard)
  12.08.2002 SVS
    - Bug-Pasting in file attributes (FAR 1.70b4)
  02.06.2002 SVS
    ! Если в CopyToClipboard переданы пустая строка, то это верный
      признак того, что очищаем клипборд.
  29.04.2002 SVS
    ! немного const
  16.10.2001 SVS
    ! Применим макрос UnicodeToOEM для прозрачности понимания действий
  27.09.2001 IS
    - Левый размер при использовании strncpy
  25.06.2001 IS
    ! Внедрение const
  06.05.2001 DJ
    ! перетрях #include
  23.01.2001 SVS
    ! Изменения в PasteFromClipboardEx в надежде на устранение падения
      при попытке вставить охрененный кусок кода в диалоге :-)
    - Опять же - утечка памяти в PasteFromClipboardEx :-((((
  22.12.2000 SVS
    + Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"

/* ------------------------------------------------------------ */
// CF_OEMTEXT CF_TEXT CF_UNICODETEXT CF_HDROP
static HGLOBAL hInternalClipboard[5]={0};
static UINT    uInternalClipboardFormat[5]={0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF};
static BOOL    OppenedClipboard=FALSE;

#ifndef UNICODE
static UINT WINAPI FAR_RegisterClipboardFormat(LPCSTR lpszFormat)
{
  if(UsedInternalClipboard)
  {
    if(!strcmp(lpszFormat,FAR_VerticalBlock))
    {
      return 0xFEB0;
    }
    return 0;
  }
  return RegisterClipboardFormatA(lpszFormat);
}
#else
static UINT WINAPI FAR_RegisterClipboardFormat(LPCWSTR lpszFormat)
{
  if(UsedInternalClipboard)
  {
    if(!wcscmp(lpszFormat,FAR_VerticalBlock))
    {
      return 0xFEB0;
    }
    return 0;
  }
  return RegisterClipboardFormatW(lpszFormat);
}
#endif // !UNICODE


static BOOL WINAPI FAR_OpenClipboard(HWND hWndNewOwner)
{
  if(UsedInternalClipboard)
  {
    if(!OppenedClipboard)
    {
      OppenedClipboard=TRUE;
      return TRUE;
    }
    return FALSE;
  }
  return OpenClipboard(hWndNewOwner);
}

static BOOL WINAPI FAR_CloseClipboard(VOID)
{
  if(UsedInternalClipboard)
  {
    if(OppenedClipboard)
    {
      OppenedClipboard=FALSE;
      return TRUE;
    }
    return FALSE;
  }
  return CloseClipboard();
}

BOOL WINAPI FAR_EmptyClipboard(VOID)
{
  if(UsedInternalClipboard)
  {
    if(OppenedClipboard)
    {
      for(int I=0; I < sizeof(hInternalClipboard)/sizeof(hInternalClipboard[0]); ++I)
        if(hInternalClipboard[I])
        {
          GlobalFree(hInternalClipboard[I]);
          hInternalClipboard[I]=0;
          uInternalClipboardFormat[I]=0xFFFF;
        }
      return TRUE;
    }
    return FALSE;
  }
  return EmptyClipboard();
}

static HANDLE WINAPI FAR_GetClipboardData(UINT uFormat)
{
  if(UsedInternalClipboard)
  {
    if(OppenedClipboard)
    {
      int I;
      for(I=0; I < sizeof(hInternalClipboard)/sizeof(hInternalClipboard[0]); ++I)
      {
        if(uInternalClipboardFormat[I] != 0xFFFF && uInternalClipboardFormat[I] == uFormat)
        {
          return hInternalClipboard[I];
        }
      }
    }
    return (HANDLE)NULL;
  }
  return GetClipboardData(uFormat);
}

static UINT WINAPI FAR_EnumClipboardFormats(UINT uFormat)
{
  if(UsedInternalClipboard)
  {
    if(OppenedClipboard)
    {
      int I;
      for(I=0; I < sizeof(hInternalClipboard)/sizeof(hInternalClipboard[0]); ++I)
      {
        if(uInternalClipboardFormat[I] != 0xFFFF && uInternalClipboardFormat[I] == uFormat)
        {
          return I+1 < sizeof(hInternalClipboard)/sizeof(hInternalClipboard[0])?uInternalClipboardFormat[I+1]:0;
        }
      }
    }
    return 0;
  }
  return EnumClipboardFormats(uFormat);
}

static HANDLE WINAPI FAR_SetClipboardData(UINT uFormat,HANDLE hMem)
{
  if(UsedInternalClipboard)
  {
    if(OppenedClipboard)
    {
      int I;
      for(I=0; I < sizeof(hInternalClipboard)/sizeof(hInternalClipboard[0]); ++I)
      {
        if(!hInternalClipboard[I])
        {
          hInternalClipboard[I]=hMem;
          uInternalClipboardFormat[I]=uFormat;
          return hMem;
        }
      }
    }
    return (HANDLE)NULL;
  }
  return SetClipboardData(uFormat,hMem);
}


/* ------------------------------------------------------------ */
int WINAPI CopyToClipboard(const char *Data)
{
  return InternalCopyToClipboard(Data,0);
}

int InternalCopyToClipboard(const char *Data,int AnsiMode)
{
  long DataSize;
  if (!FAR_OpenClipboard(NULL))
    return(FALSE);
  FAR_EmptyClipboard();
  if (Data!=NULL && (DataSize=strlen(Data))!=0)
  {
    HGLOBAL hData;
    void *GData;
    int BufferSize=DataSize+1;
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,DataSize+1);
        GlobalUnlock(hData);
        FAR_SetClipboardData(CF_OEMTEXT,(HANDLE)hData);
      }
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,DataSize+1);
        if(!AnsiMode)
          FAR_OemToChar((LPCSTR)GData,(LPTSTR)GData);
        GlobalUnlock(hData);
        FAR_SetClipboardData(CF_TEXT,(HANDLE)hData);
      }
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
      if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize*2))!=NULL)
        if ((GData=GlobalLock(hData))!=NULL)
        {
          MultiByteToWideChar(((AnsiMode)?CP_ACP:CP_OEMCP),0,Data,-1,(LPWSTR)GData,BufferSize);
          GlobalUnlock(hData);
          FAR_SetClipboardData(CF_UNICODETEXT,(HANDLE)hData);
        }
  }
  FAR_CloseClipboard();
  return(TRUE);
}


int CopyFormatToClipboard(const char *Format,char *Data)
{
  int FormatType=FAR_RegisterClipboardFormat(Format);
  if (FormatType==0)
    return(FALSE);

  long DataSize;
  if (Data!=NULL && (DataSize=strlen(Data))!=0)
  {
    HGLOBAL hData;
    void *GData;
    if (!FAR_OpenClipboard(NULL))
      return(FALSE);
    int BufferSize=DataSize+1;
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,BufferSize);
        GlobalUnlock(hData);
        FAR_SetClipboardData(FormatType,(HANDLE)hData);
      }
    FAR_CloseClipboard();
  }
  return(TRUE);
}


char* WINAPI PasteFromClipboard(void)
{
  return InternalPasteFromClipboard(0);
}

char* InternalPasteFromClipboard(int AnsiMode)
{
  HANDLE hClipData;
  if (!FAR_OpenClipboard(NULL))
    return(NULL);
  int Unicode=FALSE;
  int Format=0;
  int ReadType=CF_OEMTEXT;
  while ((Format=FAR_EnumClipboardFormats(Format))!=0)
  {
    if (Format==CF_UNICODETEXT && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
      Unicode=TRUE;
      break;
    }
    if (Format==CF_TEXT)
    {
      ReadType=CF_TEXT;
      break;
    }
    if (Format==CF_OEMTEXT)
      break;
  }
  char *ClipText=NULL;
  if ((hClipData=FAR_GetClipboardData(Unicode ? CF_UNICODETEXT:ReadType))!=NULL)
  {
    int BufferSize;
    char *ClipAddr=(char *)GlobalLock(hClipData);
    if (Unicode)
      BufferSize=lstrlenW((LPCWSTR)ClipAddr)+1;
    else
      BufferSize=strlen(ClipAddr)+1;

    ClipText=new char[BufferSize];
    if (ClipText!=NULL)
      if (Unicode)
      {
        if(AnsiMode)
          UnicodeToAscii((LPCWSTR)ClipAddr,ClipText,BufferSize);
        else
          UnicodeToOEM((LPCWSTR)ClipAddr,ClipText,BufferSize);
      }
      else
        if (ReadType==CF_TEXT)
        {
          if(!AnsiMode)
            FAR_CharToOem(ClipAddr,ClipText);
        }
        else
          strcpy(ClipText,ClipAddr);
    GlobalUnlock(hClipData);
  }
  FAR_CloseClipboard();
  return(ClipText);
}

char* WINAPI PasteFromClipboardEx(int max)
{
  return InternalPasteFromClipboardEx(max,0);
}

// max - без учета символа конца строки!
char* InternalPasteFromClipboardEx(int max,int AnsiMode)
{
  char *ClipText=NULL;
  HANDLE hClipData;
  int Unicode=FALSE;
  int Format=0;
  int ReadType=CF_OEMTEXT;

  if (!FAR_OpenClipboard(NULL))
    return(NULL);

  while ((Format=FAR_EnumClipboardFormats(Format))!=0)
  {
    if (Format==CF_UNICODETEXT && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
      Unicode=TRUE;
      break;
    }
    if (Format==CF_TEXT)
    {
      ReadType=CF_TEXT;
      break;
    }
    if (Format==CF_OEMTEXT)
      break;
  }

  if ((hClipData=FAR_GetClipboardData(Unicode ? CF_UNICODETEXT:ReadType))!=NULL)
  {
    int BufferSize;
    char *ClipAddr=(char *)GlobalLock(hClipData);
    if (Unicode)
      BufferSize=lstrlenW((LPCWSTR)ClipAddr)+1;
    else
      BufferSize=strlen(ClipAddr)+1;
    if ( BufferSize>max )
        BufferSize=max;

    ClipText=new char[BufferSize+2];
    if (ClipText!=NULL)
    {
      memset(ClipText,0,BufferSize+2);
      if (Unicode)
        if(AnsiMode)
          UnicodeToAscii((LPCWSTR)ClipAddr,ClipText,BufferSize);
        else
          UnicodeToOEM((LPCWSTR)ClipAddr,ClipText,BufferSize);
      else
      {
        if (ReadType==CF_TEXT)
        {
          strncpy(ClipText,ClipAddr,BufferSize);
          if(!AnsiMode)
            FAR_CharToOem(ClipText,ClipText);
          ClipText[BufferSize]=0;
        }
        else
        {
          strncpy(ClipText,ClipAddr,BufferSize);
          ClipText[BufferSize]=0;
        }
      }
    }
    GlobalUnlock(hClipData);
  }
  FAR_CloseClipboard();
  return(ClipText);
}

char* PasteFormatFromClipboard(const char *Format)
{
  int FormatType=FAR_RegisterClipboardFormat(Format);
  if (FormatType==0)
    return(NULL);
  if (!FAR_OpenClipboard(NULL))
    return(NULL);
  HANDLE hClipData;
  char *ClipText=NULL;
  if ((hClipData=FAR_GetClipboardData(FormatType))!=NULL)
  {
    char *ClipAddr=(char *)GlobalLock(hClipData);
    int BufferSize=strlen(ClipAddr)+1;
    ClipText=new char[BufferSize];
    if (ClipText!=NULL)
      strcpy(ClipText,ClipAddr);
    GlobalUnlock(hClipData);
  }
  FAR_CloseClipboard();
  return(ClipText);
}
