/*
clipboard.cpp

Работа с буфером обмена.

*/

/* Revision: 1.08 12.08.2002 $ */

/*
Modify:
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

int WINAPI CopyToClipboard(const char *Data)
{
  long DataSize;
  if (!OpenClipboard(NULL))
    return(FALSE);
  EmptyClipboard();
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
        SetClipboardData(CF_OEMTEXT,(HANDLE)hData);
      }
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,DataSize+1);
        OemToChar((LPCSTR)GData,(LPTSTR)GData);
        GlobalUnlock(hData);
        SetClipboardData(CF_TEXT,(HANDLE)hData);
      }
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
      if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize*2))!=NULL)
        if ((GData=GlobalLock(hData))!=NULL)
        {
          MultiByteToWideChar(CP_OEMCP,0,Data,-1,(LPWSTR)GData,BufferSize);
          GlobalUnlock(hData);
          SetClipboardData(CF_UNICODETEXT,(HANDLE)hData);
        }
  }
  CloseClipboard();
  return(TRUE);
}


int CopyFormatToClipboard(const char *Format,char *Data)
{
  int FormatType=RegisterClipboardFormat(Format);
  if (FormatType==0)
    return(FALSE);

  long DataSize;
  if (Data!=NULL && (DataSize=strlen(Data))!=0)
  {
    HGLOBAL hData;
    void *GData;
    if (!OpenClipboard(NULL))
      return(FALSE);
    int BufferSize=DataSize+1;
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,BufferSize);
        GlobalUnlock(hData);
        SetClipboardData(FormatType,(HANDLE)hData);
      }
    CloseClipboard();
  }
  return(TRUE);
}


char* WINAPI PasteFromClipboard(void)
{
  HANDLE hClipData;
  if (!OpenClipboard(NULL))
    return(NULL);
  int Unicode=FALSE;
  int Format=0;
  int ReadType=CF_OEMTEXT;
  while ((Format=EnumClipboardFormats(Format))!=0)
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
  if ((hClipData=GetClipboardData(Unicode ? CF_UNICODETEXT:ReadType))!=NULL)
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
        UnicodeToOEM((LPCWSTR)ClipAddr,ClipText,BufferSize);
      else
        if (ReadType==CF_TEXT)
          CharToOem(ClipAddr,ClipText);
        else
          strcpy(ClipText,ClipAddr);
    GlobalUnlock(hClipData);
  }
  CloseClipboard();
  return(ClipText);
}


// max - без учета символа конца строки!
char* WINAPI PasteFromClipboardEx(int max)
{
  char *ClipText=NULL;
  HANDLE hClipData;
  int Unicode=FALSE;
  int Format=0;
  int ReadType=CF_OEMTEXT;

  if (!OpenClipboard(NULL))
    return(NULL);

  while ((Format=EnumClipboardFormats(Format))!=0)
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

  if ((hClipData=GetClipboardData(Unicode ? CF_UNICODETEXT:ReadType))!=NULL)
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
        UnicodeToOEM((LPCWSTR)ClipAddr,ClipText,BufferSize);
      else
      {
        if (ReadType==CF_TEXT)
        {
          strncpy(ClipText,ClipAddr,BufferSize);
          CharToOem(ClipText,ClipText);
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
  CloseClipboard();
  return(ClipText);
}

char* PasteFormatFromClipboard(const char *Format)
{
  int FormatType=RegisterClipboardFormat(Format);
  if (FormatType==0)
    return(NULL);
  if (!OpenClipboard(NULL))
    return(NULL);
  HANDLE hClipData;
  char *ClipText=NULL;
  if ((hClipData=GetClipboardData(FormatType))!=NULL)
  {
    char *ClipAddr=(char *)GlobalLock(hClipData);
    int BufferSize=strlen(ClipAddr)+1;
    ClipText=new char[BufferSize];
    if (ClipText!=NULL)
      strcpy(ClipText,ClipAddr);
    GlobalUnlock(hClipData);
  }
  CloseClipboard();
  return(ClipText);
}
