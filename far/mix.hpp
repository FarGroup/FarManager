#pragma once

/*
mix.hpp

Mix
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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
template<class T>
inline unsigned int ToPercent(T N1, T N2)
{
	if (N1 > 10000)
	{
		N1 /= 100;
		N2 /= 100;
	}

	return N2? static_cast<unsigned int>(N1 * 100 / N2) : 0;
}

bool FarMkTempEx(string &strDest, const wchar_t *Prefix=nullptr, BOOL WithTempPath=TRUE, const wchar_t *UserTempPath=nullptr);

void PluginPanelItemToFindDataEx(
    const PluginPanelItem *pSrc,
    api::FAR_FIND_DATA *pDest
);

void FindDataExToPluginPanelItem(
    const api::FAR_FIND_DATA *pSrc,
    PluginPanelItem *pDest
);

void FreePluginPanelItem(PluginPanelItem& Data);

void FreePluginPanelItemsUserData(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber);

WINDOWINFO_TYPE ModalType2WType(const int fType);

class SetAutocomplete: NonCopyable
{
public:
	SetAutocomplete(class EditControl* edit, bool NewState = false);
	SetAutocomplete(class DlgEdit* dedit, bool NewState = false);
	SetAutocomplete(class CommandLine* cedit, bool NewState = false);
	~SetAutocomplete();

private:
	class EditControl* edit;
	bool OldState;
};

struct uuid_hash
{
	size_t operator ()(const GUID& Key) const
	{
		RPC_STATUS Status;
		return UuidHash(const_cast<UUID*>(&Key), &Status);
	}
};

struct uuid_equal
{
	bool operator ()(const GUID& a, const GUID& b) const
	{
		// In WinSDK's older than 8.0 operator== for GUIDs declared as int (sic!), This will suppress the warning:
		return (a == b) != 0;
	}
};

struct color_hash
{
	size_t operator ()(const FarColor& Key) const
	{
		return std::hash<long long>()(Key.Flags)
			^ std::hash<int>()(Key.BackgroundColor)
			^ std::hash<int>()(Key.ForegroundColor)
			^ std::hash<void*>()(Key.Reserved);
	}
};

void ReloadEnvironment();

unsigned int CRC32(unsigned int crc, const void* buffer, size_t size);
