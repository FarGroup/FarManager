/*
registry.cpp

Работа с registry

*/

/* Revision: 1.05 06.05.2001 $ */

/*
Modify:
  06.05.2001 DJ
    ! перетрях #include
  07.03.2001 IS
    + DeleteEmptyKey - удаление пустого ключа в том случае, если он не содержит
      никаких переменных и подключей. Возвращает TRUE при успехе.
  22.02.2001 SVS
    ! Для получения строки (GetRegKey) отработаем ситуацию с ERROR_MORE_DATA
      Если такая ситуация встретилась - получим сколько надо в любом случае
    + Проверки на корректность открытия ключа!
  15.09.2000 IS
    + Функция CheckRegValue(char *Key, char *ValueName) - возвращает FALSE,
      если указанная переменная не содержит данные или размер данных равен нулю
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "global.hpp"

static LONG CloseRegKey(HKEY hKey);
int CopyKeyTree(char *Src,char *Dest,char *Skip=NULL);
void DeleteFullKeyTree(char *KeyName);
static void DeleteKeyTreePart(char *KeyName);
static int DeleteCount;

static HKEY hRegRootKey=HKEY_CURRENT_USER;
static HKEY hRegCurrentKey=NULL;
static int RequestSameKey=FALSE;

void SetRegRootKey(HKEY hRootKey)
{
  hRegRootKey=hRootKey;
}


void UseSameRegKey()
{
  CloseSameRegKey();
  RequestSameKey=TRUE;
}


void CloseSameRegKey()
{
  if (hRegCurrentKey!=NULL)
  {
    RegCloseKey(hRegCurrentKey);
    hRegCurrentKey=NULL;
  }
  RequestSameKey=FALSE;
}


void SetRegKey(char *Key,char *ValueName,char *ValueData)
{
  HKEY hKey=CreateRegKey(Key);
  RegSetValueEx(hKey,ValueName,0,REG_SZ,(unsigned char *)ValueData,strlen(ValueData)+1);
  CloseRegKey(hKey);
}


void SetRegKey(char *Key,char *ValueName,DWORD ValueData)
{
  HKEY hKey=CreateRegKey(Key);
  RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
  CloseRegKey(hKey);
}


void SetRegKey(char *Key,char *ValueName,BYTE *ValueData,DWORD ValueSize)
{
  HKEY hKey=CreateRegKey(Key);
  RegSetValueEx(hKey,ValueName,0,REG_BINARY,ValueData,ValueSize);
  CloseRegKey(hKey);
}


/* $ 22.02.2001 SVS
  Для получения строки (GetRegKey) отработаем ситуацию с ERROR_MORE_DATA
  Если такая ситуация встретилась - получим сколько надо в любом случае
*/
int GetRegKey(char *Key,char *ValueName,char *ValueData,char *Default,DWORD DataSize)
{
  int ExitCode;
  HKEY hKey=OpenRegKey(Key);
  if(hKey) // надобно проверить!
  {
    DWORD Type,QueryDataSize=DataSize;
    ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(unsigned char *)ValueData,&QueryDataSize);
    if(ExitCode == ERROR_MORE_DATA) // если размер не подходящие...
    {
      char *TempBuffer=new char[QueryDataSize+1]; // ...то выделим сколько надо
      if(TempBuffer) // Если с памятью все нормально...
      {
        if((ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(unsigned char *)TempBuffer,&QueryDataSize)) == ERROR_SUCCESS)
          strncpy(ValueData,TempBuffer,DataSize); // скопируем сколько надо.
        delete[] TempBuffer;
      }
    }
    CloseRegKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    strcpy(ValueData,Default);
    return(FALSE);
  }
  return(TRUE);
}
/* SVS $ */


int GetRegKey(char *Key,char *ValueName,int &ValueData,DWORD Default)
{
  int ExitCode;
  HKEY hKey=OpenRegKey(Key);
  if(hKey)
  {
    DWORD Type,Size=sizeof(ValueData);
    ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(BYTE *)&ValueData,&Size);
    CloseRegKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    ValueData=Default;
    return(FALSE);
  }
  return(TRUE);
}


int GetRegKey(char *Key,char *ValueName,DWORD Default)
{
  int ValueData;
  GetRegKey(Key,ValueName,ValueData,Default);
  return(ValueData);
}


int GetRegKey(char *Key,char *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize)
{
  int ExitCode;
  HKEY hKey=OpenRegKey(Key);
  if(hKey)
  {
    DWORD Type,Required=DataSize;
    ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,ValueData,&Required);
    if(ExitCode == ERROR_MORE_DATA) // если размер не подходящие...
    {
      char *TempBuffer=new char[Required+1]; // ...то выделим сколько надо
      if(TempBuffer) // Если с памятью все нормально...
      {
        if((ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(unsigned char *)TempBuffer,&Required)) == ERROR_SUCCESS)
          memcpy(ValueData,TempBuffer,DataSize);  // скопируем сколько надо.
        delete[] TempBuffer;
      }
    }
    CloseRegKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    if (Default!=NULL)
      memcpy(ValueData,Default,DataSize);
    else
      memset(ValueData,0,DataSize);
    return(FALSE);
  }
  return(DataSize);
}


HKEY CreateRegKey(char *Key)
{
  if (hRegCurrentKey)
    return(hRegCurrentKey);
  HKEY hKey;
  DWORD Disposition;
  char FullKeyName[512];
  sprintf(FullKeyName,"%s%s%s",Opt.RegRoot,*Key ? "\\":"",Key);
  RegCreateKeyEx(hRegRootKey,FullKeyName,0,NULL,0,KEY_WRITE,NULL,
                 &hKey,&Disposition);
  if (RequestSameKey)
  {
    RequestSameKey=FALSE;
    hRegCurrentKey=hKey;
  }
  return(hKey);
}


HKEY OpenRegKey(char *Key)
{
  if (hRegCurrentKey)
    return(hRegCurrentKey);
  HKEY hKey;
  char FullKeyName[512];
  sprintf(FullKeyName,"%s%s%s",Opt.RegRoot,*Key ? "\\":"",Key);
  if (RegOpenKeyEx(hRegRootKey,FullKeyName,0,KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS,&hKey)!=ERROR_SUCCESS)
  {
    CloseSameRegKey();
    return(NULL);
  }
  if (RequestSameKey)
  {
    RequestSameKey=FALSE;
    hRegCurrentKey=hKey;
  }
  return(hKey);
}


void DeleteRegKey(char *Key)
{
  char FullKeyName[512];
  sprintf(FullKeyName,"%s%s%s",Opt.RegRoot,*Key ? "\\":"",Key);
  RegDeleteKey(hRegRootKey,FullKeyName);
}


void DeleteRegValue(char *Key,char *Value)
{
  HKEY hKey;
  char FullKeyName[512];
  sprintf(FullKeyName,"%s%s%s",Opt.RegRoot,*Key ? "\\":"",Key);
  if (RegOpenKeyEx(hRegRootKey,FullKeyName,0,KEY_WRITE,&hKey)==ERROR_SUCCESS)
  {
    RegDeleteValue(hKey,Value);
    CloseRegKey(hKey);
  }
}


void DeleteKeyRecord(char *KeyMask,int Position)
{
  char FullKeyName[512],NextFullKeyName[512],MaskKeyName[512];
  sprintf(MaskKeyName,"%s%s%s",Opt.RegRoot,*KeyMask ? "\\":"",KeyMask);
  while (1)
  {
    sprintf(FullKeyName,MaskKeyName,Position++);
    sprintf(NextFullKeyName,MaskKeyName,Position);
    if (!CopyKeyTree(NextFullKeyName,FullKeyName))
    {
      DeleteFullKeyTree(FullKeyName);
      break;
    }
  }
}


void InsertKeyRecord(char *KeyMask,int Position,int TotalKeys)
{
  char FullKeyName[512],PrevFullKeyName[512],MaskKeyName[512];
  sprintf(MaskKeyName,"%s%s%s",Opt.RegRoot,*KeyMask ? "\\":"",KeyMask);
  for (int CurPos=TotalKeys;CurPos>Position;CurPos--)
  {
    sprintf(FullKeyName,MaskKeyName,CurPos);
    sprintf(PrevFullKeyName,MaskKeyName,CurPos-1);
    if (!CopyKeyTree(PrevFullKeyName,FullKeyName))
      break;
  }
  sprintf(FullKeyName,MaskKeyName,Position);
  DeleteFullKeyTree(FullKeyName);
}


int CopyKeyTree(char *Src,char *Dest,char *Skip)
{
  HKEY hSrcKey,hDestKey;
  if (RegOpenKeyEx(hRegRootKey,Src,0,KEY_READ,&hSrcKey)!=ERROR_SUCCESS)
    return(FALSE);
  DeleteFullKeyTree(Dest);
  DWORD Disposition;
  if (RegCreateKeyEx(hRegRootKey,Dest,0,NULL,0,KEY_WRITE,NULL,&hDestKey,&Disposition)!=ERROR_SUCCESS)
  {
    CloseRegKey(hSrcKey);
    return(FALSE);
  }

  int I;
  for (I=0;;I++)
  {
    char ValueName[200],ValueData[1000];
    DWORD Type,NameSize=sizeof(ValueName),DataSize=sizeof(ValueData);
    if (RegEnumValue(hSrcKey,I,ValueName,&NameSize,NULL,&Type,(BYTE *)ValueData,&DataSize)!=ERROR_SUCCESS)
      break;
    RegSetValueEx(hDestKey,ValueName,0,Type,(BYTE *)ValueData,DataSize);
  }
  for (I=0;;I++)
  {
    char SubkeyName[200],SrcKeyName[512],DestKeyName[512];
    DWORD NameSize=sizeof(SubkeyName);
    FILETIME LastWrite;
    if (RegEnumKeyEx(hSrcKey,I,SubkeyName,&NameSize,NULL,NULL,NULL,&LastWrite)!=ERROR_SUCCESS)
      break;
    strcpy(SrcKeyName,Src);
    strcat(SrcKeyName,"\\");
    strcat(SrcKeyName,SubkeyName);
    if (Skip!=NULL)
    {
      bool Found=false;
      char *SkipName=Skip;
      while (!Found && *SkipName)
        if (stricmp(SrcKeyName,SkipName)==0)
          Found=true;
        else
          SkipName+=strlen(SkipName)+1;
      if (Found)
        continue;
    }
    strcpy(DestKeyName,Dest);
    strcat(DestKeyName,"\\");
    strcat(DestKeyName,SubkeyName);
    if (RegCreateKeyEx(hRegRootKey,DestKeyName,0,NULL,0,KEY_WRITE,NULL,&hDestKey,&Disposition)!=ERROR_SUCCESS)
      break;
    CopyKeyTree(SrcKeyName,DestKeyName);
  }
  CloseRegKey(hSrcKey);
  CloseRegKey(hDestKey);
  return(TRUE);
}


void DeleteKeyTree(char *KeyName)
{
  char FullKeyName[200];
  sprintf(FullKeyName,"%s%s%s",Opt.RegRoot,*KeyName ? "\\":"",KeyName);
  if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_WINDOWS ||
      RegDeleteKey(hRegRootKey,FullKeyName)!=ERROR_SUCCESS)
    DeleteFullKeyTree(FullKeyName);
}


void DeleteFullKeyTree(char *KeyName)
{
  do
  {
    DeleteCount=0;
    DeleteKeyTreePart(KeyName);
  } while (DeleteCount!=0);
}


void DeleteKeyTreePart(char *KeyName)
{
  HKEY hKey;
  if (RegOpenKeyEx(hRegRootKey,KeyName,0,KEY_READ,&hKey)!=ERROR_SUCCESS)
    return;
  for (int I=0;;I++)
  {
    char SubkeyName[200],FullKeyName[512];
    DWORD NameSize=sizeof(SubkeyName);
    FILETIME LastWrite;
    if (RegEnumKeyEx(hKey,I,SubkeyName,&NameSize,NULL,NULL,NULL,&LastWrite)!=ERROR_SUCCESS)
      break;
    sprintf(FullKeyName,"%s\\%s",KeyName,SubkeyName);
    DeleteKeyTreePart(FullKeyName);
  }
  CloseRegKey(hKey);
  if (RegDeleteKey(hRegRootKey,KeyName)==ERROR_SUCCESS)
    DeleteCount++;
}


/* 07.03.2001 IS
   Удаление пустого ключа в том случае, если он не содержит никаких переменных
   и подключей. Возвращает TRUE при успехе.
*/
int DeleteEmptyKey(HKEY hRoot, char *FullKeyName)
{
  HKEY hKey;
  int Exist=RegOpenKeyEx(hRoot,FullKeyName,0,KEY_ALL_ACCESS,
                         &hKey)==ERROR_SUCCESS;
  if(Exist)
  {
     int RetCode=FALSE;
     if(hKey)
     {
        FILETIME LastWriteTime;
        char SubName[512];
        DWORD SubSize=sizeof(SubName);

        LONG ExitCode=RegEnumKeyEx(hKey,0,SubName,&SubSize,NULL,NULL,NULL,
                                   &LastWriteTime);

        if(ExitCode!=ERROR_SUCCESS)
           ExitCode=RegEnumValue(hKey,0,SubName,&SubSize,NULL,NULL,NULL, NULL);
        CloseRegKey(hKey);

        if(ExitCode!=ERROR_SUCCESS)
          {
            char KeyName[512], *pSubKey;
            strncpy(KeyName, FullKeyName, sizeof(KeyName));
            if(NULL!=(pSubKey=strrchr(KeyName,'\\')))
              {
                 *pSubKey=0;
                 pSubKey++;
                 Exist=RegOpenKeyEx(hRoot,KeyName,0,KEY_ALL_ACCESS,
                                    &hKey)==ERROR_SUCCESS;
                 if(Exist && hKey)
                 {
                   RetCode=RegDeleteKey(hKey, pSubKey)==ERROR_SUCCESS;
                   CloseRegKey(hKey);
                 }
              }
          }
     }
     return RetCode;
  }
  return TRUE;
}
/* IS $ */


int CheckRegKey(char *Key)
{
  HKEY hKey;
  char FullKeyName[512];
  sprintf(FullKeyName,"%s%s%s",Opt.RegRoot,*Key ? "\\":"",Key);
  int Exist=RegOpenKeyEx(hRegRootKey,FullKeyName,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS;
  CloseRegKey(hKey);
  return(Exist);
}

/* 15.09.2000 IS
   Возвращает FALSE, если указанная переменная не содержит данные
   или размер данных равен нулю.
*/
int CheckRegValue(char *Key,char *ValueName)
{
  int ExitCode;
  DWORD DataSize=0;
  HKEY hKey=OpenRegKey(Key);
  if(hKey)
  {
    DWORD Type;
    ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,NULL,&DataSize);
    CloseRegKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS || !DataSize)
    return(FALSE);
  return(TRUE);
}
/* IS $ */


int EnumRegKey(char *Key,DWORD Index,char *DestName,DWORD DestSize)
{
  HKEY hKey=OpenRegKey(Key);
  if(hKey)
  {
    FILETIME LastWriteTime;
    char SubName[512];
    DWORD SubSize=sizeof(SubName);
    int ExitCode=RegEnumKeyEx(hKey,Index,SubName,&SubSize,NULL,NULL,NULL,&LastWriteTime);
    CloseRegKey(hKey);
    if (ExitCode==ERROR_SUCCESS)
    {
      char TempName[512];
      strcpy(TempName,Key);
      if (*TempName)
        AddEndSlash(TempName);
      strcat(TempName,SubName);
      strncpy(DestName,TempName,DestSize);
      return(TRUE);
    }
  }
  return(FALSE);
}


LONG CloseRegKey(HKEY hKey)
{
  if (hRegCurrentKey)
    return(ERROR_SUCCESS);
  return(RegCloseKey(hKey));
}
