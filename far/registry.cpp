/*
registry.cpp

Работа с registry

*/

/* Revision: 1.33 25.05.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "global.hpp"
#include "array.hpp"

static LONG CloseRegKey(HKEY hKey);

int CopyKeyTreeW(const wchar_t *Src,const wchar_t *Dest,const wchar_t *Skip=NULL);
void DeleteFullKeyTreeW(const wchar_t *KeyName);
static void DeleteKeyTreePartW(const wchar_t *KeyName);

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



LONG SetRegKeyW(const wchar_t *Key,const wchar_t *ValueName,const wchar_t * const ValueData)
{
  HKEY hKey;
  LONG Ret=ERROR_SUCCESS;

  if((hKey=CreateRegKeyW(Key)) != NULL)
    Ret=RegSetValueExW(hKey,ValueName,0,REG_SZ,(unsigned char *)ValueData,(wcslen(ValueData)+1)*sizeof(wchar_t));
  CloseRegKey(hKey);
  return Ret;
}

LONG SetRegKeyW(const wchar_t *Key,const wchar_t *ValueName,DWORD ValueData)
{
  HKEY hKey;
  LONG Ret=ERROR_SUCCESS;
  if((hKey=CreateRegKeyW(Key)) != NULL)
    Ret=RegSetValueExW(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
  CloseRegKey(hKey);
  return Ret;
}


LONG SetRegKey64W(const wchar_t *Key,const wchar_t *ValueName,unsigned __int64 ValueData)
{
  HKEY hKey;
  LONG Ret=ERROR_SUCCESS;
  if((hKey=CreateRegKeyW(Key)) != NULL)
    Ret=RegSetValueExW(hKey,ValueName,0,REG_QWORD,(BYTE *)&ValueData,sizeof(ValueData));
  CloseRegKey(hKey);
  return Ret;
}

LONG SetRegKeyW(const wchar_t *Key,const wchar_t *ValueName,const BYTE *ValueData,DWORD ValueSize)
{
  HKEY hKey;
  LONG Ret=ERROR_SUCCESS;
  if((hKey=CreateRegKeyW(Key)) != NULL)
    Ret=RegSetValueExW(hKey,ValueName,0,REG_BINARY,ValueData,ValueSize);
  CloseRegKey(hKey);
  return Ret;
}



int GetRegKeySizeW(const wchar_t *Key,const wchar_t *ValueName)
{
  HKEY hKey=OpenRegKeyW(Key);
  DWORD QueryDataSize=GetRegKeySizeW(hKey,ValueName);
  CloseRegKey(hKey);
  return QueryDataSize;
}


int GetRegKeySizeW(HKEY hKey,const wchar_t *ValueName)
{
  if(hKey)
  {
    BYTE Buffer;
    DWORD Type,QueryDataSize=sizeof(Buffer);
    int ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,(unsigned char *)&Buffer,&QueryDataSize);
    if(ExitCode==ERROR_SUCCESS || ExitCode == ERROR_MORE_DATA)
      return QueryDataSize;
  }
  return 0;
}


/* $ 22.02.2001 SVS
  Для получения строки (GetRegKey) отработаем ситуацию с ERROR_MORE_DATA
  Если такая ситуация встретилась - получим сколько надо в любом случае
*/

int GetRegKeyW(const wchar_t *Key,const wchar_t *ValueName,string &strValueData,const wchar_t *Default)
{
  int ExitCode;
  HKEY hKey=OpenRegKeyW(Key);
  if(hKey) // надобно проверить!
  {
    DWORD Type,QueryDataSize=0;

    if ( (ExitCode = RegQueryValueExW (
            hKey,
            ValueName,
            0,
            &Type,
            NULL,
            &QueryDataSize
            )) == ERROR_SUCCESS )
    {
      wchar_t *TempBuffer = strValueData.GetBuffer (QueryDataSize+1); // ...то выделим сколько надо

      ExitCode = RegQueryValueExW(hKey,ValueName,0,&Type,(unsigned char *)TempBuffer,&QueryDataSize);

      strValueData.ReleaseBuffer();
    }
    CloseRegKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    strValueData = Default;
    return(FALSE);
  }
  return(TRUE);
}

/* SVS $ */

int GetRegKeyW(const wchar_t *Key,const wchar_t *ValueName,int &ValueData,DWORD Default)
{
  int ExitCode;
  HKEY hKey=OpenRegKeyW(Key);
  if(hKey)
  {
    DWORD Type,Size=sizeof(ValueData);
    ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,(BYTE *)&ValueData,&Size);
    CloseRegKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    ValueData=Default;
    return(FALSE);
  }
  return(TRUE);
}

int GetRegKeyW(const wchar_t *Key,const wchar_t *ValueName,DWORD Default)
{
  int ValueData;
  GetRegKeyW(Key,ValueName,ValueData,Default);
  return(ValueData);
}

int GetRegKey64W(const wchar_t *Key,const wchar_t *ValueName,__int64 &ValueData,unsigned __int64 Default)
{
  int ExitCode;
  HKEY hKey=OpenRegKeyW(Key);
  if(hKey)
  {
    DWORD Type,Size=sizeof(ValueData);
    ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,(BYTE *)&ValueData,&Size);
    CloseRegKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    ValueData=Default;
    return(FALSE);
  }
  return(TRUE);
}

__int64 GetRegKey64W(const wchar_t *Key,const wchar_t *ValueName,unsigned __int64 Default)
{
  __int64 ValueData;
  GetRegKey64W(Key,ValueName,ValueData,Default);
  return(ValueData);
}

int GetRegKeyW(const wchar_t *Key,const wchar_t *ValueName,BYTE *ValueData,const BYTE *Default,DWORD DataSize)
{
  int ExitCode;
  HKEY hKey=OpenRegKeyW(Key);
  DWORD Required=DataSize;
  if(hKey)
  {
    DWORD Type;
    ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,ValueData,&Required);
    if(ExitCode == ERROR_MORE_DATA) // если размер не подходящие...
    {
      char *TempBuffer=new char[Required+1]; // ...то выделим сколько надо
      if(TempBuffer) // Если с памятью все нормально...
      {
        if((ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,(unsigned char *)TempBuffer,&Required)) == ERROR_SUCCESS)
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
    return(0);
  }
  return(Required);
}

static string &MkKeyNameW(const wchar_t *Key, string &strDest)
{
  strDest = Opt.strRegRoot;

  if(*Key)
  {
    strDest += L"\\";
    strDest += Key;
  }

  return strDest;
}


HKEY CreateRegKeyW(const wchar_t *Key)
{
  if (hRegCurrentKey)
    return(hRegCurrentKey);
  HKEY hKey;
  DWORD Disposition;

  string strFullKeyName;
  MkKeyNameW(Key,strFullKeyName);
  if(RegCreateKeyExW(hRegRootKey,strFullKeyName,0,NULL,0,KEY_WRITE,NULL,
                 &hKey,&Disposition) != ERROR_SUCCESS)
    hKey=NULL;
  if (RequestSameKey)
  {
    RequestSameKey=FALSE;
    hRegCurrentKey=hKey;
  }
  return(hKey);
}


HKEY OpenRegKeyW(const wchar_t *Key)
{
  if (hRegCurrentKey)
    return(hRegCurrentKey);
  HKEY hKey;
  string strFullKeyName;
  MkKeyNameW(Key,strFullKeyName);
  if (RegOpenKeyExW(hRegRootKey,strFullKeyName,0,KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS,&hKey)!=ERROR_SUCCESS)
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


void DeleteRegKeyW(const wchar_t *Key)
{
  string strFullKeyName;
  MkKeyNameW(Key,strFullKeyName);
  RegDeleteKeyW(hRegRootKey,strFullKeyName);
}


void DeleteRegValueW(const wchar_t *Key,const wchar_t *Value)
{
  HKEY hKey;
  string strFullKeyName;
  MkKeyNameW(Key,strFullKeyName);
  if (RegOpenKeyExW(hRegRootKey,strFullKeyName,0,KEY_WRITE,&hKey)==ERROR_SUCCESS)
  {
    RegDeleteValueW(hKey,Value);
    CloseRegKey(hKey);
  }
}

void DeleteKeyRecordW(const wchar_t *KeyMask,int Position)
{
  string strFullKeyName, strNextFullKeyName;
  string strMaskKeyName;

  MkKeyNameW(KeyMask, strMaskKeyName);

  while (1)
  {
    strFullKeyName.Format ((const wchar_t*)strMaskKeyName,Position++);
    strNextFullKeyName.Format ((const wchar_t*)strMaskKeyName,Position);
    if (!CopyKeyTreeW(strNextFullKeyName,strFullKeyName))
    {
      DeleteFullKeyTreeW(strFullKeyName);
      break;
    }
  }
}

void InsertKeyRecordW(const wchar_t *KeyMask,int Position,int TotalKeys)
{
  string strFullKeyName, strPrevFullKeyName;
  string strMaskKeyName;

  MkKeyNameW(KeyMask,strMaskKeyName);
  for (int CurPos=TotalKeys;CurPos>Position;CurPos--)
  {
    strFullKeyName.Format ((const wchar_t*)strMaskKeyName,CurPos);
    strPrevFullKeyName.Format ((const wchar_t*)strMaskKeyName,CurPos-1);
    if (!CopyKeyTreeW(strPrevFullKeyName,strFullKeyName))
      break;
  }
  strFullKeyName.Format ((const wchar_t*)strMaskKeyName,Position);
  DeleteFullKeyTreeW(strFullKeyName);
}


class KeyRecordItem
{
  public:
   int ItemIdx;
   KeyRecordItem() { ItemIdx=0; }
   bool operator==(const KeyRecordItem &rhs) const{
     return ItemIdx == rhs.ItemIdx;
   };
   int operator<(const KeyRecordItem &rhs) const{
     return ItemIdx < rhs.ItemIdx;
   };
   const KeyRecordItem& operator=(const KeyRecordItem &rhs)
   {
     ItemIdx = rhs.ItemIdx;
     return *this;
   };

   ~KeyRecordItem()
   {
   }
};

void RenumKeyRecordW(const wchar_t *KeyRoot,const wchar_t *KeyMask,const wchar_t *KeyMask0)
{
  TArray<KeyRecordItem> KAItems;
  KeyRecordItem KItem;
  int CurPos;
  string strRegKey;
  string strFullKeyName, strPrevFullKeyName;
  string strMaskKeyName;
  BOOL Processed=FALSE;

  // сбор данных
  for (CurPos=0;;CurPos++)
  {
    if(!EnumRegKeyW(KeyRoot,CurPos,strRegKey))
      break;
    KItem.ItemIdx=_wtoi((const wchar_t*)strRegKey+wcslen(KeyMask0));
    if(KItem.ItemIdx != CurPos)
      Processed=TRUE;
    KAItems.addItem(KItem);
  }

  if(Processed)
  {
    KAItems.Sort();

    MkKeyNameW(KeyMask,strMaskKeyName);
    for(int CurPos=0;;++CurPos)
    {
      KeyRecordItem *Item=KAItems.getItem(CurPos);
      if(!Item)
        break;

      // проверям существование CurPos
      strFullKeyName.Format (KeyMask,CurPos);
      if(!CheckRegKeyW(strFullKeyName))
      {
        strFullKeyName.Format ((const wchar_t*)strMaskKeyName,CurPos);
        strPrevFullKeyName.Format ((const wchar_t*)strMaskKeyName,Item->ItemIdx);
        if (!CopyKeyTreeW(strPrevFullKeyName,strFullKeyName))
          break;
        DeleteFullKeyTreeW(strPrevFullKeyName);
      }
    }
  }
}


int CopyKeyTreeW(const wchar_t *Src,const wchar_t *Dest,const wchar_t *Skip)
{
  HKEY hSrcKey,hDestKey;
  if (RegOpenKeyExW(hRegRootKey,Src,0,KEY_READ,&hSrcKey)!=ERROR_SUCCESS)
    return(FALSE);
  DeleteFullKeyTreeW(Dest);
  DWORD Disposition;
  if (RegCreateKeyExW(hRegRootKey,Dest,0,NULL,0,KEY_WRITE,NULL,&hDestKey,&Disposition)!=ERROR_SUCCESS)
  {
    CloseRegKey(hSrcKey);
    return(FALSE);
  }

  int I;
  for (I=0;;I++)
  {
    wchar_t ValueName[200],ValueData[1000]; //BUGBUG, dynamic
    DWORD Type,NameSize=sizeof(ValueName),DataSize=sizeof(ValueData);
    if (RegEnumValueW(hSrcKey,I,ValueName,&NameSize,NULL,&Type,(BYTE *)ValueData,&DataSize)!=ERROR_SUCCESS)
      break;
    RegSetValueExW(hDestKey,ValueName,0,Type,(BYTE *)ValueData,DataSize);
  }
  for (I=0;;I++)
  {
    wchar_t SubkeyName[200]; //BUGBUG, dynamic
    string strSrcKeyName, strDestKeyName;

    DWORD NameSize=sizeof(SubkeyName);

    FILETIME LastWrite;
    if (RegEnumKeyExW(hSrcKey,I,SubkeyName,&NameSize,NULL,NULL,NULL,&LastWrite)!=ERROR_SUCCESS)
      break;

    strSrcKeyName = Src;
    strSrcKeyName += L"\\";
    strSrcKeyName += SubkeyName;
    if (Skip!=NULL)
    {
      bool Found=false;
      const wchar_t *SkipName=Skip;
      while (!Found && *SkipName)
        if (LocalStricmpW(strSrcKeyName,SkipName)==0)
          Found=true;
        else
          SkipName+=wcslen(SkipName)+1;
      if (Found)
        continue;
    }

    strDestKeyName = Dest;
    strDestKeyName += L"\\";
    strDestKeyName = SubkeyName;
    if (RegCreateKeyExW(hRegRootKey,strDestKeyName,0,NULL,0,KEY_WRITE,NULL,&hDestKey,&Disposition)!=ERROR_SUCCESS)
      break;
    CopyKeyTreeW(strSrcKeyName,strDestKeyName);
  }
  CloseRegKey(hSrcKey);
  CloseRegKey(hDestKey);
  return(TRUE);
}

void DeleteKeyTreeW(const wchar_t *KeyName)
{
  string strFullKeyName;
  MkKeyNameW(KeyName,strFullKeyName);
  if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_WINDOWS ||
      RegDeleteKeyW(hRegRootKey,strFullKeyName)!=ERROR_SUCCESS)
    DeleteFullKeyTreeW(strFullKeyName);
}

void DeleteFullKeyTreeW(const wchar_t *KeyName)
{
  do
  {
    DeleteCount=0;
    DeleteKeyTreePartW(KeyName);
  } while (DeleteCount!=0);
}

void DeleteKeyTreePartW(const wchar_t *KeyName)
{
  HKEY hKey;
  if (RegOpenKeyExW(hRegRootKey,KeyName,0,KEY_READ,&hKey)!=ERROR_SUCCESS)
    return;
  for (int I=0;;I++)
  {
    wchar_t SubkeyName[200]; //BUGBUG, dynamic
    string strFullKeyName;
    DWORD NameSize=sizeof(SubkeyName);
    FILETIME LastWrite;
    if (RegEnumKeyExW(hKey,I,SubkeyName,&NameSize,NULL,NULL,NULL,&LastWrite)!=ERROR_SUCCESS)
      break;

    strFullKeyName = KeyName;
    strFullKeyName += L"\\";
    strFullKeyName += SubkeyName;
    DeleteKeyTreePartW(strFullKeyName);
  }
  CloseRegKey(hKey);
  if (RegDeleteKeyW(hRegRootKey,KeyName)==ERROR_SUCCESS)
    DeleteCount++;
}


/* 07.03.2001 IS
   Удаление пустого ключа в том случае, если он не содержит никаких переменных
   и подключей. Возвращает TRUE при успехе.
*/

int DeleteEmptyKeyW(HKEY hRoot, const wchar_t *FullKeyName)
{
  HKEY hKey;
  int Exist=RegOpenKeyExW(hRoot,FullKeyName,0,KEY_ALL_ACCESS,&hKey)==ERROR_SUCCESS;
  if(Exist)
  {
     int RetCode=FALSE;
     if(hKey)
     {
        FILETIME LastWriteTime;
        wchar_t SubName[512]; //BUGBUG, dynamic
        DWORD SubSize=sizeof(SubName);

        LONG ExitCode=RegEnumKeyExW(hKey,0,SubName,&SubSize,NULL,NULL,NULL,
                                   &LastWriteTime);

        if(ExitCode!=ERROR_SUCCESS)
           ExitCode=RegEnumValueW(hKey,0,SubName,&SubSize,NULL,NULL,NULL, NULL);
        CloseRegKey(hKey);

        if(ExitCode!=ERROR_SUCCESS)
          {
            string strKeyName = FullKeyName;
            wchar_t *pSubKey = strKeyName.GetBuffer ();

            if(NULL!=(pSubKey=wcsrchr(pSubKey,L'\\')))
              {
                 *pSubKey=0;
                 pSubKey++;
                 Exist=RegOpenKeyExW(hRoot,strKeyName,0,KEY_ALL_ACCESS,&hKey)==ERROR_SUCCESS; //BUGBUG strKeyName
                 if(Exist && hKey)
                 {
                   RetCode=RegDeleteKeyW(hKey, pSubKey)==ERROR_SUCCESS;
                   CloseRegKey(hKey);
                 }
              }

            strKeyName.ReleaseBuffer ();
          }
     }
     return RetCode;
  }
  return TRUE;
}
/* IS $ */

int CheckRegKeyW(const wchar_t *Key)
{
  HKEY hKey;
  string strFullKeyName;
  MkKeyNameW(Key,strFullKeyName);
  int Exist=RegOpenKeyExW(hRegRootKey,strFullKeyName,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS;
  CloseRegKey(hKey);
  return(Exist);
}

/* 15.09.2000 IS
   Возвращает FALSE, если указанная переменная не содержит данные
   или размер данных равен нулю.
*/
int CheckRegValueW(const wchar_t *Key,const wchar_t *ValueName)
{
  int ExitCode;
  DWORD DataSize=0;
  HKEY hKey=OpenRegKeyW(Key);
  if(hKey)
  {
    DWORD Type;
    ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,NULL,&DataSize);
    CloseRegKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS || !DataSize)
    return(FALSE);
  return(TRUE);
}

int EnumRegKeyW(const wchar_t *Key,DWORD Index,string &strDestName)
{
  HKEY hKey=OpenRegKeyW(Key);
  if(hKey)
  {
    FILETIME LastWriteTime;
    wchar_t SubName[512]; //BUGBUG, dynamic
    DWORD SubSize=sizeof(SubName);
    int ExitCode=RegEnumKeyExW(hKey,Index,SubName,&SubSize,NULL,NULL,NULL,&LastWriteTime);
    CloseRegKey(hKey);
    if (ExitCode==ERROR_SUCCESS)
    {
      string strTempName;
      strTempName = Key;
      if ( !strTempName.IsEmpty() )
        AddEndSlashW(strTempName);

      strTempName += SubName;

      strDestName = strTempName; //???
      return(TRUE);
    }
  }
  return(FALSE);
}

int EnumRegValueW(const wchar_t *Key,DWORD Index, string &strDestName,LPBYTE SData,DWORD SDataSize,LPDWORD IData,__int64* IData64)
{
  HKEY hKey=OpenRegKeyW(Key);
  int RetCode=REG_NONE;

  if(hKey)
  {
    wchar_t ValueName[512]; //BUGBUG, dynamic

    while( TRUE )
    {
      DWORD ValSize=sizeof(ValueName);
      DWORD Type=-1;

      if (RegEnumValueW(hKey,Index,ValueName,&ValSize,NULL,&Type,SData,&SDataSize) != ERROR_SUCCESS)
        break;

      RetCode=Type;

      strDestName = ValueName;

      if(Type == REG_SZ)
        break;
      else if(Type == REG_DWORD)
      {
        if(IData)
          *IData=*(DWORD*)SData;
        break;
      }
      else if(Type == REG_QWORD)
      {
        if(IData64)
          *IData64=*(__int64*)SData;
        break;
      }
    }

    CloseRegKey(hKey);
  }
  return RetCode;
}

int EnumRegValueExW(const wchar_t *Key,DWORD Index, string &strDestName, string strSData, LPDWORD IData,__int64* IData64)
{
  HKEY hKey=OpenRegKeyW(Key);
  int RetCode=REG_NONE;

  if(hKey)
  {
    wchar_t ValueName[512]; //BUGBUG, dynamic

    while( TRUE )
    {
      DWORD ValSize=sizeof(ValueName);
      DWORD Type=-1;
      DWORD Size = 0;

      if (RegEnumValueW(hKey,Index,ValueName,&ValSize, NULL, &Type, NULL, &Size) != ERROR_SUCCESS)
        break;

      wchar_t *Data = strSData.GetBuffer (Size/sizeof (wchar_t)+1);

      if (RegEnumValueW(hKey,Index,ValueName,&ValSize,NULL,&Type,(LPBYTE)Data,&Size) != ERROR_SUCCESS)
      {
        strSData.ReleaseBuffer ();
        break;
      }
      else
        strSData.ReleaseBuffer ();

      RetCode=Type;

      strDestName = ValueName;

      if(Type == REG_SZ)
        break;
      else if(Type == REG_DWORD)
      {
        if(IData)
          *IData=*(DWORD*)(const wchar_t*)strSData;
        break;
      }
      else if(Type == REG_QWORD)
      {
        if(IData64)
          *IData64=*(__int64*)(const wchar_t*)strSData;
        break;
      }
    }

    CloseRegKey(hKey);
  }
  return RetCode;
}



LONG CloseRegKey(HKEY hKey)
{
  if (hRegCurrentKey || !hKey)
    return ERROR_SUCCESS;
  return(RegCloseKey(hKey));
}


int RegQueryStringValueEx (
        HKEY hKey,
        const wchar_t *lpwszValueName,
        string &strData,
        const wchar_t *lpwszDefault
        )
{
    DWORD cbSize = 0;

    int nResult = RegQueryValueExW (
            hKey,
            lpwszValueName,
            NULL,
            NULL,
            NULL,
            &cbSize
            );

    if ( nResult == ERROR_SUCCESS )
    {
        wchar_t *lpwszData = strData.GetBuffer (cbSize+1);

        nResult = RegQueryValueExW (
            hKey,
            lpwszValueName,
            NULL,
            NULL,
            (LPBYTE)lpwszData,
            &cbSize
            );

        strData.ReleaseBuffer ();
    }

    if ( nResult != ERROR_SUCCESS )
        strData = lpwszDefault;

    return nResult;
}

int RegQueryStringValue (
        HKEY hKey,
        const wchar_t *lpwszSubKey,
        string &strData,
        const wchar_t *lpwszDefault
        )
{
    LONG cbSize = 0;

    int nResult = RegQueryValueW (
            hKey,
            lpwszSubKey,
            NULL,
            &cbSize
            );

    if ( nResult == ERROR_SUCCESS )
    {
        wchar_t *lpwszData = strData.GetBuffer (cbSize+1);

        nResult = RegQueryValueW (
            hKey,
            lpwszSubKey,
            (LPWSTR)lpwszData,
            &cbSize
            );

        strData.ReleaseBuffer ();
    }

    if ( nResult != ERROR_SUCCESS )
        strData = lpwszDefault;

    return nResult;
}
