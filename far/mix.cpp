/*
mix.cpp

Куча разных вспомогательных функций
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

#include "headers.hpp"
#pragma hdrstop

#include "mix.hpp"
#include "CFileMask.hpp"
#include "scantree.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "frame.hpp"

#include "cmdline.hpp"
#include "dlgedit.hpp"

int ToPercent(unsigned long N1,unsigned long N2)
{
	if (N1 > 10000)
	{
		N1/=100;
		N2/=100;
	}

	if (!N2)
		return 0;

	return((int)(N1*100/N2));
}

int ToPercent64(unsigned __int64 N1, unsigned __int64 N2)
{
	if (N1 > 10000)
	{
		N1/=100;
		N2/=100;
	}

	if (!N2)
		return 0;

	return static_cast<int>(N1*100/N2);
}


/*
             v - точка
   prefXXX X X XXX
       \ / ^   ^^^\ PID + TID
        |  \------/
        |
        +---------- [0A-Z]
*/
string& FarMkTempEx(string &strDest, const wchar_t *Prefix, BOOL WithTempPath, const wchar_t *UserTempPath)
{
	static UINT s_shift = 0;
	if (!(Prefix && *Prefix))
		Prefix=L"F3T";

	string strPath = L".";

	if (WithTempPath)
	{
		apiGetTempPath(strPath);
	}
	else if(UserTempPath)
	{
		strPath=UserTempPath;
	}

	AddEndSlash(strPath);

	wchar_t *lpwszDest = strDest.GetBuffer(StrLength(Prefix)+strPath.GetLength()+13);

	UINT uniq = 23*GetCurrentProcessId() + s_shift, uniq0 = uniq ? uniq : 1;
	s_shift = (s_shift + 1) % 23;

	for (;;)
	{
		if (!uniq) ++uniq;

		if (GetTempFileName(strPath, Prefix, uniq, lpwszDest))
		{
			string tname(lpwszDest);
			FindFile f(tname,false);
			FAR_FIND_DATA_EX ffdata;
			if (!f.Get(ffdata))
				break;
		}

		if ((++uniq & 0xffff) == (uniq0 & 0xffff))
		{
			*lpwszDest = 0;
			break;
		}
	}

	strDest.ReleaseBuffer();
	return strDest;
}

void PluginPanelItemToFindDataEx(const PluginPanelItem *pSrc, FAR_FIND_DATA_EX *pDest)
{
	pDest->dwFileAttributes = pSrc->FileAttributes;
	pDest->ftCreationTime = pSrc->CreationTime;
	pDest->ftLastAccessTime = pSrc->LastAccessTime;
	pDest->ftLastWriteTime = pSrc->LastWriteTime;
	pDest->ftChangeTime = pSrc->ChangeTime;
	pDest->nFileSize = pSrc->FileSize;
	pDest->nAllocationSize = pSrc->AllocationSize;
	pDest->strFileName = pSrc->FileName;
	pDest->strAlternateFileName = pSrc->AlternateFileName;
}

void FindDataExToPluginPanelItem(const FAR_FIND_DATA_EX *pSrc, PluginPanelItem *pDest)
{
	pDest->FileAttributes = pSrc->dwFileAttributes;
	pDest->CreationTime = pSrc->ftCreationTime;
	pDest->LastAccessTime = pSrc->ftLastAccessTime;
	pDest->LastWriteTime = pSrc->ftLastWriteTime;
	pDest->ChangeTime = pSrc->ftChangeTime;
	pDest->FileSize = pSrc->nFileSize;
	pDest->AllocationSize = pSrc->nAllocationSize;
	pDest->FileName = xf_wcsdup(pSrc->strFileName);
	pDest->AlternateFileName = xf_wcsdup(pSrc->strAlternateFileName);
}

void FreePluginPanelItem(PluginPanelItem *pData)
{
	xf_free(pData->FileName);
	xf_free(pData->AlternateFileName);
}

WINDOWINFO_TYPE ModalType2WType(const int fType)
{
	static int WiTypes[]={
		MODALTYPE_VIRTUAL,    WTYPE_VIRTUAL,
		MODALTYPE_PANELS,     WTYPE_PANELS,
		MODALTYPE_VIEWER,     WTYPE_VIEWER,
		MODALTYPE_EDITOR,     WTYPE_EDITOR,
		MODALTYPE_DIALOG,     WTYPE_DIALOG,
		MODALTYPE_VMENU,      WTYPE_VMENU,
		MODALTYPE_HELP,       WTYPE_HELP,
		MODALTYPE_COMBOBOX,   WTYPE_COMBOBOX,
		MODALTYPE_FINDFOLDER, WTYPE_FINDFOLDER,
		MODALTYPE_USER,       WTYPE_USER,
	};

	for (size_t I=0; I < ARRAYSIZE(WiTypes); I+=2)
		if (fType == WiTypes[I])
			return static_cast<WINDOWINFO_TYPE>(WiTypes[I+1]);

	return static_cast<WINDOWINFO_TYPE>(-1);
}

SetAutocomplete::SetAutocomplete(EditControl* edit, bool NewState):
	edit(edit),
	OldState(edit->GetAutocomplete())
{
	edit->SetAutocomplete(NewState);
}

SetAutocomplete::SetAutocomplete(DlgEdit* dedit, bool NewState):
	edit(dedit->lineEdit),
	OldState(edit->GetAutocomplete())
{
	edit->SetAutocomplete(NewState);
}

SetAutocomplete::SetAutocomplete(CommandLine* cedit, bool NewState):
	edit(&cedit->CmdStr),
	OldState(edit->GetAutocomplete())
{
	edit->SetAutocomplete(NewState);
}

SetAutocomplete::~SetAutocomplete()
{
	edit->SetAutocomplete(OldState);
};
