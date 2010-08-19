#pragma once

/*
registry.cpp

Работа с registry
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



void SetRegRootKey(HKEY hRootKey);

LONG SetRegKey(const wchar_t *Key,const wchar_t *ValueName,const wchar_t * const ValueData, int SizeData, DWORD Type);

LONG SetRegKey(const wchar_t *Key,const wchar_t *ValueName,const wchar_t * const ValueData);

LONG SetRegKey(const wchar_t *Key,const wchar_t *ValueName,DWORD ValueData);

LONG SetRegKey(const wchar_t *Key,const wchar_t *ValueName,const BYTE *ValueData,DWORD ValueSize);

int GetRegKey(const wchar_t *Key,const wchar_t *ValueName, string &strValueData,const wchar_t *Default,DWORD *pType=nullptr);

int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,BYTE *ValueData,const BYTE *Default,DWORD DataSize,DWORD *pType=nullptr);

int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,int &ValueData,DWORD Default);

int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,DWORD Default);

HKEY CreateRegKey(const wchar_t *Key);

HKEY OpenRegKey(const wchar_t *Key);

int GetRegKeySize(const wchar_t *Key,const wchar_t *ValueName);

int GetRegKeySize(HKEY hKey,const wchar_t *ValueName);

int EnumRegValue(const wchar_t *Key,DWORD Index, string &strDestName, LPBYTE SData,DWORD SDataSize,LPDWORD IData=nullptr,__int64* IData64=nullptr);

int EnumRegValueEx(const wchar_t *Key,DWORD Index, string &strDestName, string &strData, LPDWORD IData=nullptr,__int64* IData64=nullptr, DWORD *Type=nullptr);

LONG SetRegKey64(const wchar_t *Key,const wchar_t *ValueName,unsigned __int64 ValueData);

int GetRegKey64(const wchar_t *Key,const wchar_t *ValueName,__int64 &ValueData,unsigned __int64 Default);

__int64 GetRegKey64(const wchar_t *Key,const wchar_t *ValueName,unsigned __int64 Default);

void DeleteRegKey(const wchar_t *Key);

void DeleteRegValue(const wchar_t *Key,const wchar_t *Value);

void DeleteKeyRecord(const wchar_t *KeyMask,int Position);

void InsertKeyRecord(const wchar_t *KeyMask,int Position,int TotalKeys);

void RenumKeyRecord(const wchar_t *KeyRoot,const wchar_t *KeyMask,const wchar_t *KeyMask0);

void DeleteKeyTree(const wchar_t *KeyName);

int CheckRegKey(const wchar_t *Key);

int CheckRegValue(const wchar_t *Key,const wchar_t *ValueName,DWORD *pType=nullptr,DWORD *pDataSize=nullptr);

int DeleteEmptyKey(HKEY hRoot, const wchar_t *FullKeyName);

int EnumRegKey(const wchar_t *Key,DWORD Index,string &strDestName);

int CopyKeyTree(const wchar_t *Src,const wchar_t *Dest,const wchar_t *Skip=nullptr);

int CopyLocalKeyTree(const wchar_t *Src,const wchar_t *Dest);

void UseSameRegKey();

void CloseSameRegKey();

int RegQueryStringValueEx(HKEY hKey, const wchar_t *lpwszValueName, string &strData, const wchar_t *lpwszDefault = L"");

int RegQueryStringValue(HKEY hKey, const wchar_t *lpwszSubKey, string &strData, const wchar_t *lpwszDefault = L"");
