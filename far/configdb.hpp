#pragma once

/*
configdb.hpp

хранение настроек в базе sqlite.
*/
/*
Copyright © 2011 Far Group
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

class GeneralConfig {

public:

	enum {
		TYPE_INTEGER,
		TYPE_TEXT,
		TYPE_BLOB
	};

	virtual ~GeneralConfig() {}
	virtual void Begin() = 0;
	virtual void End() = 0;
	virtual bool SetValue(const wchar_t *Key, const wchar_t *Name, const wchar_t *Value) = 0;
	virtual bool SetValue(const wchar_t *Key, const wchar_t *Name, unsigned __int64 Value) = 0;
	virtual bool SetValue(const wchar_t *Key, const wchar_t *Name, const void *Value, int Size) = 0;
	virtual void GetValue(const wchar_t *Key, const wchar_t *Name, DWORD *Value, DWORD Default) = 0;
	virtual void GetValue(const wchar_t *Key, const wchar_t *Name, string &strValue, const wchar_t *Default) = 0;
	virtual int GetValue(const wchar_t *Key, const wchar_t *Name, char *Value, int Size, const char *Default) = 0;
};

class PluginsConfig {

public:

	enum {
		TYPE_INTEGER,
		TYPE_TEXT,
		TYPE_BLOB
	};

	virtual ~PluginsConfig() {}
	virtual unsigned __int64 CreateKey(unsigned __int64 Root, const wchar_t *Name, const wchar_t *Description=nullptr) = 0;
	virtual unsigned __int64 GetKeyID(unsigned __int64 Root, const wchar_t *Name) = 0;
	virtual bool SetKeyDescription(unsigned __int64 Root, const wchar_t *Description) = 0;
	virtual bool SetValue(unsigned __int64 Root, const wchar_t *Name, const wchar_t *Value) = 0;
	virtual bool SetValue(unsigned __int64 Root, const wchar_t *Name, unsigned __int64 Value) = 0;
	virtual bool SetValue(unsigned __int64 Root, const wchar_t *Name, const void *Value, int Size) = 0;
	virtual bool GetValue(unsigned __int64 Root, const wchar_t *Name, unsigned __int64 *Value) = 0;
	virtual bool GetValue(unsigned __int64 Root, const wchar_t *Name, string &strValue) = 0;
	virtual int GetValue(unsigned __int64 Root, const wchar_t *Name, char *Value, int Size) = 0;
	virtual bool DeleteKeyTree(unsigned __int64 KeyID) = 0;
	virtual bool DeleteValue(unsigned __int64 Root, const wchar_t *Name) = 0;
	virtual bool EnumKeys(unsigned __int64 Root, DWORD Index, string &strName) = 0;
	virtual bool EnumValues(unsigned __int64 Root, DWORD Index, string &strName, DWORD *Type) = 0;
};

GeneralConfig *GetGeneralConfig();

PluginsConfig *GetPluginsConfig();
