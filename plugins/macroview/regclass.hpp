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
    char CurrentPath[MAX_PATH_LEN];
    char S[MAX_PATH_LEN];
    BOOL CloseRootKey;
    void __fastcall ChangeKey(HKEY Value, char *Path);
    HKEY __fastcall GetBaseKey(BOOL Relative);
    HKEY __fastcall GetKey(char *Key);
    void __fastcall SetCurrentKey(HKEY Value);
    void __fastcall MoveValue(HKEY SrcKey,HKEY DestKey,char *Name);
    void __fastcall CopyValues(HKEY SrcKey,HKEY DestKey);
    void __fastcall CopyKeys(HKEY SrcKey,HKEY DestKey);

  public:
    TReg(HKEY Key=HKEY_CURRENT_USER);
    ~TReg();
    void CloseKey();
    void __fastcall SetRootKey(HKEY Value);
    BOOL __fastcall CreateKey(char *Key);
    BOOL __fastcall OpenKey(char *Key, BOOL CanCreate=FALSE);
    BOOL __fastcall DeleteKey(char *Key);
    BOOL __fastcall DeleteValue(char *Name);
    BOOL GetKeyInfo(TRegKeyInfo &Value);
    BOOL GetKeyNames(TStrList *&List);
    BOOL GetValueNames(TStrList *&List);
    BOOL GetDataInfo(/*const */char *ValueName,TRegDataInfo &Value);
    int __fastcall GetDataSize(/*const */char *ValueName);
    TRegDataType GetDataType(char *ValueName);
    BOOL __fastcall ValueExists(char *Name);
    void __fastcall RenameValue(char *OldName,char *NewName);
    BOOL __fastcall WriteString(/*const */char *Name,char *Value);
    char *__fastcall ReadString(/*const */char *Name,char *Str,int size);
    BOOL __fastcall WriteInteger(char *Name,DWORD Value);
    int __fastcall ReadInteger(char *Name);
    BOOL __fastcall WriteBinaryData(char *Name,void *Buffer,int BufSize);
    int __fastcall ReadBinaryData(char *Name,void *Buffer,int BufSize);
    BOOL HasSubKeys();
    int GetData(/*const */char *Name,void *Buffer,DWORD BufSize,TRegDataType &RegData);
    BOOL PutData(/*const */char *Name,void *Buffer,DWORD BufSize,TRegDataType RegData);    
#ifdef CREATE_REG_FILE
    BOOL __fastcall SaveKey(char *Key, char *FileName);
#endif
    BOOL __fastcall KeyExists(char *Key);
    void __fastcall MoveKey(char *OldName,char *NewName,BOOL Delete=TRUE);
};

#endif //REG_CLASS_INCLUDED
