#include "regclass.hpp"

BOOL IsRelative(TCHAR *Value)
{
	return (*Value != _T('\\'));
}

DWORD RegDataToDataType(TRegDataType Value)
{
	switch (Value)
	{
		case rdString: return REG_SZ;
		case rdExpandString: return REG_EXPAND_SZ;
		case rdMultiString: return REG_MULTI_SZ;
		case rdInteger: return REG_DWORD;
		case rdBinary: return REG_BINARY;
		default: return REG_NONE;
	}
}

TRegDataType DataTypeToRegData(DWORD Value)
{
	switch (Value)
	{
		case REG_SZ: return rdString;
		case REG_EXPAND_SZ: return rdExpandString;
		case REG_MULTI_SZ: return rdMultiString;
		case REG_DWORD: return rdInteger;
		case REG_BINARY: return rdBinary;
		default: return rdUnknown;
	}
}

TReg::TReg(HKEY Key)
{
	RootKey=Key;
	CurrentKey=Key;
}

TReg::~TReg(void)
{
	CloseKey();
}

void TReg::CloseKey(void)
{
	if (CurrentKey!=0)
	{
		RegCloseKey(CurrentKey);
		CurrentKey=0;
		CurrentPath[0]=0;
	}
}

void __fastcall TReg::SetRootKey(HKEY Value)
{
	if (RootKey!=Value)
		if (CloseRootKey)
		{
			RegCloseKey(RootKey);
			CloseRootKey=FALSE;
		}

	RootKey=Value;
	CloseKey();
}

void __fastcall TReg::ChangeKey(HKEY Value, const TCHAR *Path)
{
	CloseKey();
	CurrentKey=Value;

	if (Path)
		lstrcpy(CurrentPath,Path);
	else
		CurrentPath[0]=0;
}

HKEY __fastcall TReg::GetBaseKey(BOOL Relative)
{
	if ((CurrentKey==0) || (!Relative))
		return RootKey;
	else
		return CurrentKey;
}

void __fastcall TReg::SetCurrentKey(HKEY Value)
{
	CurrentKey=Value;
}

BOOL __fastcall TReg::CreateKey(const TCHAR *Key)
{
	DWORD Disposition;
	HKEY TempKey=0;

	if (Key)
		lstrcpy(S,Key);
	else
		S[0]=0;

	BOOL Relative=IsRelative(S);

	if (!Relative)
	{
		if (Key)
			lstrcpy(S,&Key[1]);
		else
			S[0]=0;
	}

	BOOL Result=RegCreateKeyEx(GetBaseKey(Relative),S,0,NULL,
	                           REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&TempKey,
	                           &Disposition)==ERROR_SUCCESS;

	if (Disposition==REG_OPENED_EXISTING_KEY)
		RegCloseKey(TempKey);

	return Result;
}

BOOL __fastcall TReg::OpenKey(const TCHAR *Key, BOOL CanCreate)
{
	DWORD Disposition;
	BOOL Result;

	if (Key)
		lstrcpy(S,Key);
	else
		S[0]=0;

	BOOL Relative=IsRelative(S);

	if (!Relative)
	{
		if (Key)
			lstrcpy(S,&Key[1]);
		else
			S[0]=0;
	}

	HKEY TempKey=0;

	if ((!CanCreate) || (S[0]==0))
		Result=RegOpenKeyEx(GetBaseKey(Relative),S,0,KEY_ALL_ACCESS,&TempKey)==ERROR_SUCCESS;
	else
		Result=RegCreateKeyEx(GetBaseKey(Relative),S,0,NULL,
		                      REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&TempKey,
		                      &Disposition)==ERROR_SUCCESS;

	if (Result)
	{
		if ((CurrentKey!=0) && Relative)
		{
			TCHAR Str[MAX_PATH_LEN];
			wsprintf(Str,_T("%s\\%s"),CurrentPath,S);
			ChangeKey(TempKey, Str);
		}
		else
			ChangeKey(TempKey, S);
	}

	return Result;
}

BOOL __fastcall TReg::DeleteKey(const TCHAR *Key)
{
	TCHAR KeyName[MAX_PATH_LEN];
	DWORD i,len;
	TRegKeyInfo KeyInfo;

	if (Key)
		lstrcpy(S,Key);
	else
		S[0]=0;

	BOOL Relative=IsRelative(S);

	if (!Relative)
	{
		if (Key)
			lstrcpy(S,&Key[1]);
		else
			S[0]=0;
	}

	HKEY OldKey=CurrentKey;
	HKEY DeleteKey=GetKey(Key);

	if (DeleteKey!=0)
	{
		SetCurrentKey(DeleteKey);

		if (GetKeyInfo(KeyInfo))
		{
			for (i=0; i<KeyInfo.NumSubKeys; i++)
			{
				len=KeyInfo.MaxSubKeyLen+1;

				if (RegEnumKeyEx(DeleteKey,i,KeyName,&len,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
					this->DeleteKey(KeyName);
			}
		}
	}
	else
	{
		SetCurrentKey(OldKey);
		return FALSE;
	}

	SetCurrentKey(OldKey);
	BOOL Result=RegDeleteKey(GetBaseKey(Relative),S)==ERROR_SUCCESS;
	return Result;
}

BOOL __fastcall TReg::DeleteValue(const TCHAR *Name)
{
	return(RegDeleteValue(CurrentKey,Name)==ERROR_SUCCESS);
}

BOOL TReg::GetKeyInfo(TRegKeyInfo &Value)
{
	ZeroMemory(&Value,sizeof(TRegKeyInfo));
	return(RegQueryInfoKey(CurrentKey,NULL,NULL,NULL,
	                       &Value.NumSubKeys,&Value.MaxSubKeyLen,NULL,&Value.NumValues,
	                       &Value.MaxValueLen,&Value.MaxDataLen,NULL,NULL)==ERROR_SUCCESS);
}

BOOL TReg::GetKeyNames(TStrList *List)
{
	DWORD i,len;
	TRegKeyInfo KeyInfo;

	if (GetKeyInfo(KeyInfo))
	{
		if (KeyInfo.NumSubKeys!=0)
		{
			for (i=0; i<KeyInfo.NumSubKeys; i++)
			{
				len=KeyInfo.MaxSubKeyLen+1;
				RegEnumKeyEx(CurrentKey,i,S,&len,NULL,NULL,NULL,NULL);
				List->Add(S);
			}

			return TRUE;
		}
	}

	return FALSE;
}

BOOL TReg::GetValueNames(TStrList *List)
{
	DWORD i,len;
	TRegKeyInfo Info;

	if (GetKeyInfo(Info))
	{
		if (Info.NumValues!=0)
		{
			for (i=0; i<Info.NumValues; i++)
			{
				len=Info.MaxValueLen+1;
				RegEnumValue(CurrentKey,i,S,&len,NULL,NULL,NULL,NULL);
				List->Add(S);
			}

			return TRUE;
		}
	}

	return FALSE;
}

BOOL TReg::GetDataInfo(const TCHAR *ValueName,TRegDataInfo &Value)
{
	DWORD DataType;
	ZeroMemory(&Value,sizeof(TRegDataInfo));
	BOOL Result=RegQueryValueEx(CurrentKey,ValueName,NULL,&DataType,NULL,
	                            &Value.DataSize)==ERROR_SUCCESS;
	Value.RegData=DataTypeToRegData(DataType);
	return Result;
}

int __fastcall TReg::GetDataSize(const TCHAR *ValueName)
{
	TRegDataInfo Info;

	if (GetDataInfo(ValueName,Info))
		return (int)Info.DataSize;
	else
		return -1;
}

TRegDataType TReg::GetDataType(const TCHAR *ValueName)
{
	TRegDataInfo Info;

	if (GetDataInfo(ValueName, Info))
		return Info.RegData;
	else
		return rdUnknown;
}

BOOL __fastcall TReg::WriteString(const TCHAR *Name,const TCHAR *Value)
{
	return(PutData(Name,(const BYTE *)Value,(lstrlen(Value)+1)*sizeof(TCHAR),rdString));
}

TCHAR *__fastcall TReg::ReadString(const TCHAR *Name,TCHAR *Str,int size)
{
	int Len;
	TRegDataType RegData;
	Len=GetDataSize(Name);

	if ((Len>0) && (Len<size))
	{
		if ((GetData(Name,Str,Len,RegData))==-1)
			Str[0]=0;
		else if ((RegData!=rdString) && (RegData!=rdExpandString))
			Str[0]=0;
	}
	else
		Str[0]=0;

	return Str;
}

BOOL __fastcall TReg::WriteInteger(const TCHAR *Name,DWORD Value)
{
	return(PutData(Name,(const BYTE *)&Value,sizeof(Value),rdInteger));
}

int __fastcall TReg::ReadInteger(const TCHAR *Name)
{
	TRegDataType RegData;
	int Result;

	if ((GetData(Name,&Result,sizeof(Result),RegData)==-1) ||
	        (RegData!=rdInteger))
		return -1;

	return Result;
}

BOOL __fastcall TReg::WriteBinaryData(const TCHAR *Name,void *Buffer,int BufSize)
{
	return(PutData(Name,(const BYTE *)Buffer,BufSize,rdBinary));
}

int __fastcall TReg::ReadBinaryData(const TCHAR *Name,void *Buffer,int BufSize)
{
	TRegDataType RegData;
	TRegDataInfo Info;
	int Result;

	if (GetDataInfo(Name,Info))
	{
		Result=Info.DataSize;
		RegData=Info.RegData;

		if ((RegData==rdBinary) && (Result<=BufSize))
			GetData(Name,Buffer,Result,RegData);
		else
			Result=-1;
	}
	else
		Result=0;

	return Result;
}

BOOL TReg::PutData(const TCHAR *Name,const BYTE *Buffer,DWORD BufSize,TRegDataType RegData)
{
	DWORD DataType;
	DataType=RegDataToDataType(RegData);
	return((RegSetValueEx(CurrentKey,Name,0,DataType,Buffer,BufSize))==ERROR_SUCCESS);
}

int TReg::GetData(const TCHAR *Name,void *Buffer,DWORD BufSize,TRegDataType &RegData)
{
	DWORD DataType;
	int Result;
	DataType=REG_NONE;

	if ((RegQueryValueEx(CurrentKey,Name,NULL,&DataType,(BYTE *)Buffer,
	                     &BufSize))!=ERROR_SUCCESS)
		Result=-1;
	else
		Result=(int)BufSize;

	RegData=DataTypeToRegData(DataType);
	return Result;
}

BOOL TReg::HasSubKeys()
{
	TRegKeyInfo Info;

	if (GetKeyInfo(Info))
		return (Info.NumSubKeys>0);
	else
		return FALSE;
}

BOOL __fastcall TReg::ValueExists(const TCHAR *Name)
{
	TRegDataInfo Info;
	return(GetDataInfo(Name, Info));
}

HKEY __fastcall TReg::GetKey(const TCHAR *Key)
{
	HKEY Rkey;
	BOOL Relative;

	if (Key)
		lstrcpy(S,Key);
	else
		S[0]=0;

	Relative=IsRelative(S);

	if (!Relative)
	{
		if (Key)
			lstrcpy(S,&Key[1]);
		else
			S[0]=0;
	}

	Rkey=0;
	RegOpenKeyEx(GetBaseKey(Relative),S,0,KEY_ALL_ACCESS,&Rkey);
	return Rkey;
}

#ifdef CREATE_REG_FILE
BOOL __fastcall TReg::SaveKey(const TCHAR *Key, TCHAR *FileName)
{
	HKEY SaveKey;
	BOOL Result=FALSE;
	SaveKey=GetKey(Key);

	if (SaveKey!=0)
	{
		Result=(RegSaveKey(SaveKey,FileName,NULL)==ERROR_SUCCESS);
		RegCloseKey(SaveKey);
	}

	return Result;
}
#endif

BOOL __fastcall TReg::KeyExists(const TCHAR *Key)
{
	HKEY TempKey;
	TempKey=GetKey(Key);

	if (TempKey!=0)
		RegCloseKey(TempKey);

	return (TempKey!=0);
}

void __fastcall TReg::RenameValue(const TCHAR *OldName,const TCHAR *NewName)
{
	int Len;
	TRegDataType RegData;

	if ((ValueExists(OldName)) && (!ValueExists(NewName)))
	{
		Len=GetDataSize(OldName);

		if (Len>0)
		{
			TCHAR *Buffer=new TCHAR[Len+1];
			Len=GetData(OldName,Buffer,Len,RegData);
			DeleteValue(OldName);
			PutData(NewName,(const BYTE *)Buffer,Len,RegData);
			delete[] Buffer;
		}
	}
}

void __fastcall TReg::MoveValue(HKEY SrcKey,HKEY DestKey,const TCHAR *Name)
{
	int Len;
	HKEY OldKey,PrevKey;
	TRegDataType RegData;
	OldKey=CurrentKey;
	SetCurrentKey(SrcKey);
	Len=GetDataSize(Name);

	if (Len>0)
	{
		TCHAR *Buffer=new TCHAR[Len+1];
		Len=GetData(Name,Buffer,Len,RegData);
		PrevKey=CurrentKey;
		SetCurrentKey(DestKey);
		PutData(Name,(const BYTE *)Buffer,Len,RegData);
		SetCurrentKey(PrevKey);
		delete[] Buffer;
	}
	else
	{
		SetCurrentKey(OldKey);
		return;
	}

	SetCurrentKey(OldKey);
}

void __fastcall TReg::CopyValues(HKEY SrcKey,HKEY DestKey)
{
	DWORD I,Len;
	TRegKeyInfo KeyInfo;
	HKEY OldKey;
	OldKey=CurrentKey;
	SetCurrentKey(SrcKey);

	if (GetKeyInfo(KeyInfo))
	{
		MoveValue(SrcKey,DestKey,_T(""));

		for (I=0; I<KeyInfo.NumValues; I++)
		{
			Len=KeyInfo.MaxValueLen+1;

			if (RegEnumValue(SrcKey,I,S,&Len,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
				MoveValue(SrcKey,DestKey,S);
		}
	}

	SetCurrentKey(OldKey);
}

void __fastcall TReg::CopyKeys(HKEY SrcKey,HKEY DestKey)
{
	DWORD I,Len;
	TRegKeyInfo Info;
	HKEY OldKey,PrevKey,NewSrc,NewDest;
	OldKey=CurrentKey;
	SetCurrentKey(SrcKey);

	if (GetKeyInfo(Info))
	{
		for (I=0; I<Info.NumSubKeys; I++)
		{
			Len=Info.MaxSubKeyLen+1;

			if (RegEnumKeyEx(SrcKey,I,S,&Len,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
			{
				NewSrc=GetKey(S);

				if (NewSrc!=0)
				{
					PrevKey=CurrentKey;
					SetCurrentKey(DestKey);
					CreateKey(S);
					NewDest=GetKey(S);
					CopyValues(NewSrc,NewDest);
					CopyKeys(NewSrc,NewDest);
					RegCloseKey(NewDest);
					SetCurrentKey(PrevKey);
					RegCloseKey(NewSrc);
				}
			}
		}
	}
	else
	{
		SetCurrentKey(OldKey);
		return;
	}

	SetCurrentKey(OldKey);
}

void __fastcall TReg::MoveKey(const TCHAR *OldName,const TCHAR *NewName,BOOL Delete)
{
	HKEY SrcKey,DestKey;

	if (KeyExists(OldName)) //&& (!KeyExists(NewName)))
	{
		SrcKey=GetKey(OldName);

		if (SrcKey!=0)
		{
			CreateKey(NewName);
			DestKey=GetKey(NewName);

			if (DestKey!=0)
			{
				CopyValues(SrcKey,DestKey);
				CopyKeys(SrcKey,DestKey);

				if ((Delete) && (CmpStr(OldName,NewName)!=0))
					DeleteKey(OldName);

				RegCloseKey(DestKey);
			}

			RegCloseKey(SrcKey);
		}
	}
}
