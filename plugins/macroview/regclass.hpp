#ifndef REG_CLASS_INCLUDED
#define REG_CLASS_INCLUDED
#define MAX_KEY_LEN 128
#define MAX_PATH_LEN 512

struct TRegKeyInfo
{
  DWORD NumSubKeys;
  DWORD MaxSubKeyLen;
  DWORD NumValues;
  DWORD MaxValueLen;
  DWORD MaxDataLen;
};

enum TRegDataType
{
	rdUnknown,
	rdString,
	rdExpandString,
	rdInteger,
	rdBinary,
	rdMultiString,
};

struct TRegDataInfo
{
  TRegDataType RegData;
  DWORD DataSize;
};

class TReg
{
  private:
    HKEY RootKey;
    HKEY CurrentKey;
    TCHAR CurrentPath[MAX_PATH_LEN];
    TCHAR S[MAX_PATH_LEN];
    BOOL CloseRootKey;
    void __fastcall ChangeKey(HKEY Value, TCHAR *Path);
    HKEY __fastcall GetBaseKey(BOOL Relative);
    HKEY __fastcall GetKey(TCHAR *Key);
    void __fastcall SetCurrentKey(HKEY Value);
    void __fastcall MoveValue(HKEY SrcKey,HKEY DestKey,TCHAR *Name);
    void __fastcall CopyValues(HKEY SrcKey,HKEY DestKey);
    void __fastcall CopyKeys(HKEY SrcKey,HKEY DestKey);

  public:
    TReg(HKEY Key=HKEY_CURRENT_USER);
    ~TReg();
    void CloseKey();
    void __fastcall SetRootKey(HKEY Value);
    BOOL __fastcall CreateKey(TCHAR *Key);
    BOOL __fastcall OpenKey(TCHAR *Key, BOOL CanCreate=FALSE);
    BOOL __fastcall DeleteKey(TCHAR *Key);
    BOOL __fastcall DeleteValue(TCHAR *Name);
    BOOL GetKeyInfo(TRegKeyInfo &Value);
    BOOL GetKeyNames(TStrList *&List);
    BOOL GetValueNames(TStrList *&List);
    BOOL GetDataInfo(/*const */TCHAR *ValueName,TRegDataInfo &Value);
    int __fastcall GetDataSize(/*const */TCHAR *ValueName);
    TRegDataType GetDataType(TCHAR *ValueName);
    BOOL __fastcall ValueExists(TCHAR *Name);
    void __fastcall RenameValue(TCHAR *OldName,TCHAR *NewName);
    BOOL __fastcall WriteString(/*const */TCHAR *Name,TCHAR *Value);
    TCHAR *__fastcall ReadString(/*const */TCHAR *Name,TCHAR *Str,int size);
    BOOL __fastcall WriteInteger(TCHAR *Name,DWORD Value);
    int __fastcall ReadInteger(TCHAR *Name);
    BOOL __fastcall WriteBinaryData(TCHAR *Name,void *Buffer,int BufSize);
    int __fastcall ReadBinaryData(TCHAR *Name,void *Buffer,int BufSize);
    BOOL HasSubKeys();
    int GetData(/*const */TCHAR *Name,void *Buffer,DWORD BufSize,TRegDataType &RegData);
    BOOL PutData(/*const */TCHAR *Name,void *Buffer,DWORD BufSize,TRegDataType RegData);
#ifdef CREATE_REG_FILE
    BOOL __fastcall SaveKey(TCHAR *Key, TCHAR *FileName);
#endif
    BOOL __fastcall KeyExists(TCHAR *Key);
    void __fastcall MoveKey(TCHAR *OldName,TCHAR *NewName,BOOL Delete=TRUE);
};

#endif //REG_CLASS_INCLUDED
