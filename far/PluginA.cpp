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

#ifndef NO_WRAPPER

#include "plugins.hpp"
#include "codepage.hpp"
#include "chgprior.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "farexcpt.hpp"
#include "config.hpp"
#include "plclass.hpp"
#include "PluginA.hpp"
#include "keyboard.hpp"
#include "interf.hpp"
#include "pathmix.hpp"
#include "mix.hpp"
#include "colormix.hpp"
#include "FarGuid.hpp"
#include "keys.hpp"
#include "processname.hpp"
#include "language.hpp"
#include "filepanels.hpp"

namespace wrapper
{
typedef void   (WINAPI *iClosePanelPrototype)          (HANDLE hPlugin);
typedef int    (WINAPI *iComparePrototype)             (HANDLE hPlugin,const oldfar::PluginPanelItem *Item1,const oldfar::PluginPanelItem *Item2,unsigned int Mode);
typedef int    (WINAPI *iConfigurePrototype)           (int ItemNumber);
typedef int    (WINAPI *iDeleteFilesPrototype)         (HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef void   (WINAPI *iExitFARPrototype)             ();
typedef void   (WINAPI *iFreeFindDataPrototype)        (HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber);
typedef void   (WINAPI *iFreeVirtualFindDataPrototype) (HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber);
typedef int    (WINAPI *iGetFilesPrototype)            (HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
typedef int    (WINAPI *iGetFindDataPrototype)         (HANDLE hPlugin,oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
typedef int    (WINAPI *iGetMinFarVersionPrototype)    ();
typedef void   (WINAPI *iGetOpenPanelInfoPrototype)    (HANDLE hPlugin,oldfar::OpenPanelInfo *Info);
typedef void   (WINAPI *iGetPluginInfoPrototype)       (oldfar::PluginInfo *Info);
typedef int    (WINAPI *iGetVirtualFindDataPrototype)  (HANDLE hPlugin,oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber,const char *Path);
typedef int    (WINAPI *iMakeDirectoryPrototype)       (HANDLE hPlugin,char *Name,int OpMode);
typedef HANDLE (WINAPI *iOpenFilePluginPrototype)      (char *Name,const unsigned char *Data,int DataSize);
typedef HANDLE (WINAPI *iOpenPrototype)                (int OpenFrom,intptr_t Item);
typedef int    (WINAPI *iProcessEditorEventPrototype)  (int Event,void *Param);
typedef int    (WINAPI *iProcessEditorInputPrototype)  (const INPUT_RECORD *Rec);
typedef int    (WINAPI *iProcessPanelEventPrototype)   (HANDLE hPlugin,int Event,void *Param);
typedef int    (WINAPI *iProcessHostFilePrototype)     (HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef int    (WINAPI *iProcessPanelInputPrototype)   (HANDLE hPlugin,int Key,unsigned int ControlState);
typedef int    (WINAPI *iPutFilesPrototype)            (HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
typedef int    (WINAPI *iSetDirectoryPrototype)        (HANDLE hPlugin,const char *Dir,int OpMode);
typedef int    (WINAPI *iSetFindListPrototype)         (HANDLE hPlugin,const oldfar::PluginPanelItem *PanelItem,int ItemsNumber);
typedef void   (WINAPI *iSetStartupInfoPrototype)      (const oldfar::PluginStartupInfo *Info);
typedef int    (WINAPI *iProcessViewerEventPrototype)  (int Event,void *Param);
typedef int    (WINAPI *iProcessDialogEventPrototype)  (int Event,void *Param);

inline int UnicodeToOEM(const wchar_t* src, char* dst, size_t lendst)
{
	return WideCharToMultiByte(CP_OEMCP, 0, src, -1, dst, (int) lendst, nullptr, nullptr);
}

inline int OEMToUnicode(const char* src, wchar_t* dst, size_t lendst)
{
	return MultiByteToWideChar(CP_OEMCP, 0, src, -1, dst, (int) lendst);
}

inline std::string ansi(const string& str)
{
	std::vector<char> Buffer(str.size() + 1);
	UnicodeToOEM(str.data(), Buffer.data(), Buffer.size());
	return std::string(Buffer.data(), str.size());
}

OEMPluginModel::OEMPluginModel(PluginManager* owner):
	NativePluginModel(owner)
{
	static const GenericPluginModel::export_name ExportsNames[] =
	{
		WA(""), // GetGlobalInfo not used
		WA("SetStartupInfo"),
		WA("OpenPlugin"),
		WA("ClosePlugin"),
		WA("GetPluginInfo"),
		WA("GetOpenPluginInfo"),
		WA("GetFindData"),
		WA("FreeFindData"),
		WA("GetVirtualFindData"),
		WA("FreeVirtualFindData"),
		WA("SetDirectory"),
		WA("GetFiles"),
		WA("PutFiles"),
		WA("DeleteFiles"),
		WA("MakeDirectory"),
		WA("ProcessHostFile"),
		WA("SetFindList"),
		WA("Configure"),
		WA("ExitFAR"),
		WA("ProcessKey"),
		WA("ProcessEvent"),
		WA("ProcessEditorEvent"),
		WA("Compare"),
		WA("ProcessEditorInput"),
		WA("ProcessViewerEvent"),
		WA("ProcessDialogEvent"),
		WA(""), // ProcessSynchroEvent not used
		WA(""), // ProcessConsoleEvent not used
		WA(""), // Analyze not used
		WA(""), // CloseAnalyze not used
		WA(""), // GetCustomData not used
		WA(""), // FreeCustomData not used

		WA("OpenFilePlugin"),
		WA("GetMinFarVersion"),
	};
	static_assert(ARRAYSIZE(ExportsNames) == ExportsCount, "Not all exports names are defined");
	m_ExportsNames = ExportsNames;
}

Plugin* OEMPluginModel::CreatePlugin(const string& filename)
{
	return IsPlugin(filename)? new PluginA(this, filename) : nullptr;
}

const std::string& OEMPluginModel::getUserName()
{
	if (m_userName.empty())
		m_userName = "Software\\Far Manager" + (Global->strRegUser.empty() ? "" : "\\Users\\" + ansi(Global->strRegUser)) + "\\Plugins";
	return m_userName;
}

bool OEMPluginModel::FindExport(const char* ExportName)
{
	// module with ANY known export can be OEM plugin
	auto ExportsBegin = m_ExportsNames, ExportsEnd = ExportsBegin + ExportsCount;
	return std::find_if(ExportsBegin, ExportsEnd, [&](const export_name& i)
	{
		return !strcmp(ExportName, i.AName);
	}) != ExportsEnd;
}


static inline int IsSpaceA(int x) { return x==' '  || x=='\t'; }
static inline int IsEolA(int x)   { return x=='\r' || x=='\n'; }
static inline int IsSlashA(int x) { return x=='\\' || x=='/'; }

static unsigned char LowerToUpper[256];
static unsigned char UpperToLower[256];
static unsigned char IsUpperOrLower[256];

static void LocalUpperInit()
{
	static bool Inited = false;
	if (!Inited)
	{
		for (size_t I=0; I<ARRAYSIZE(LowerToUpper); I++)
		{
			char CvtStr[]={static_cast<char>(I), L'\0'}, ReverseCvtStr[2];
			LowerToUpper[I]=UpperToLower[I]=static_cast<char>(I);
			OemToCharA(CvtStr,CvtStr);
			CharToOemA(CvtStr,ReverseCvtStr);
			IsUpperOrLower[I]=0;

			if (IsCharAlphaA(CvtStr[0]) && ReverseCvtStr[0]==static_cast<char>(I))
			{
				IsUpperOrLower[I]=IsCharLowerA(CvtStr[0])?1:(IsCharUpperA(CvtStr[0])?2:0);
				CharUpperA(CvtStr);
				CharToOemA(CvtStr,CvtStr);
				LowerToUpper[I]=CvtStr[0];
				CvtStr[0]=static_cast<char>(I);
				OemToCharA(CvtStr,CvtStr);
				CharLowerA(CvtStr);
				CharToOemA(CvtStr,CvtStr);
				UpperToLower[I]=CvtStr[0];
			}
		}
		Inited = true;
	}
}

typedef int(*comparer)(const void*, const void*);
typedef int(*comparer_ex)(const void*, const void*, void*);

struct comparer_helper
{
	comparer_ex cmp;
	void* user;
};

static int WINAPI comparer_wrapper(const void *one, const void *two, void *user)
{
	return reinterpret_cast<comparer>(user)(one, two);
}

static int WINAPI comparer_ex_wrapper(const void *one, const void *two, void *user)
{
	auto helper = static_cast<comparer_helper*>(user);
	return helper->cmp(one,two,helper->user);
}

static int LocalStricmp(const char *s1, const char *s2)
{
	while (1)
	{
		if (UpperToLower[(unsigned)*s1] != UpperToLower[(unsigned)*s2])
			return (UpperToLower[(unsigned)*s1] < UpperToLower[(unsigned)*s2]) ? -1 : 1;

		if (!*(s1++))
			break;

		s2++;
	}

	return 0;
}

static int LocalStrnicmp(const char *s1, const char *s2, int n)
{
	while (n-- > 0)
	{
		if (UpperToLower[(unsigned)*s1] != UpperToLower[(unsigned)*s2])
			return (UpperToLower[(unsigned)*s1] < UpperToLower[(unsigned)*s2]) ? -1 : 1;

		if (!*(s1++))
			break;

		s2++;
	}

	return 0;
}

static const char *FirstSlashA(const char *String)
{
	do
	{
		if (IsSlashA(*String))
			return String;
	} while (*String++);

	return nullptr;
}

static void AnsiToUnicodeBin(const char *lpszAnsiString, wchar_t *lpwszUnicodeString, int nLength, uintptr_t CodePage = CP_OEMCP)
{
	if (lpszAnsiString && lpwszUnicodeString && nLength)
	{
		wmemset(lpwszUnicodeString, 0, nLength);
		MultiByteToWideChar(CodePage, 0, lpszAnsiString, nLength, lpwszUnicodeString, nLength);
	}
}

static wchar_t *AnsiToUnicodeBin(const char *lpszAnsiString, int nLength, uintptr_t CodePage = CP_OEMCP)
{
	auto Result = new wchar_t[nLength];
	AnsiToUnicodeBin(lpszAnsiString, Result, nLength, CodePage);
	return Result;
}

static wchar_t *AnsiToUnicode(const char *lpszAnsiString, uintptr_t CodePage = CP_OEMCP)
{
	if (!lpszAnsiString)
		return nullptr;

	return AnsiToUnicodeBin(lpszAnsiString, (int)strlen(lpszAnsiString) + 1, CodePage);
}

static char *UnicodeToAnsiBin(const wchar_t *lpwszUnicodeString, int nLength, uintptr_t CodePage = CP_OEMCP)
{
	/* $ 06.01.2008 TS
	! Увеличил размер выделяемой под строку памяти на 1 байт для нормальной
	работы старых плагинов, которые не знали, что надо смотреть на длину,
	а не на завершающий ноль (например в EditorGetString.StringText).
	*/
	if (!lpwszUnicodeString || (nLength < 0))
		return nullptr;

	auto Result = new char[nLength + 1]();

	if (nLength)
	{
		WideCharToMultiByte(
			CodePage,
			0,
			lpwszUnicodeString,
			nLength,
			Result,
			nLength,
			nullptr,
			nullptr
			);
	}

	return Result;
}

static char *UnicodeToAnsi(const wchar_t *lpwszUnicodeString, uintptr_t CodePage = CP_OEMCP)
{
	if (!lpwszUnicodeString)
		return nullptr;

	return UnicodeToAnsiBin(lpwszUnicodeString, StrLength(lpwszUnicodeString) + 1, CodePage);
}

static wchar_t **ArrayAnsiToUnicode(const char* const * const lpaszAnsiString, size_t iCount)
{
	wchar_t** Result = nullptr;

	if (lpaszAnsiString)
	{
		Result = new wchar_t*[iCount + 1];

		for (size_t i = 0; i<iCount; i++)
		{
			Result[i] = (lpaszAnsiString[i]) ? AnsiToUnicode(lpaszAnsiString[i]) : nullptr;
		}

		Result[iCount] = (wchar_t*)(intptr_t)1; //Array end mark
	}

	return Result;
}

static void FreeArrayUnicode(const wchar_t* const* lpawszUnicodeString)
{
	if (lpawszUnicodeString)
	{
		for (int i = 0; (intptr_t)lpawszUnicodeString[i] != 1; i++) //Until end mark
		{
			delete[] lpawszUnicodeString[i];
		}
		delete[] lpawszUnicodeString;
	}
}

static DWORD OldKeyToKey(DWORD dOldKey)
{
	if (dOldKey & 0x100)
	{
		dOldKey = (dOldKey ^ 0x100) | EXTENDED_KEY_BASE;
	}
	else if (dOldKey & 0x200)
	{
		dOldKey = (dOldKey ^ 0x200) | INTERNAL_KEY_BASE;
	}
	else
	{
		DWORD CleanKey = dOldKey&~KEY_CTRLMASK;

		if (CleanKey>0x80 && CleanKey<0x100)
		{
			char OemChar = static_cast<char>(CleanKey);
			wchar_t WideChar = 0;
			if (MultiByteToWideChar(CP_OEMCP, 0, &OemChar, 1, &WideChar, 1))
				dOldKey = (dOldKey^CleanKey) | WideChar;
		}
	}

	return dOldKey;
}

static DWORD KeyToOldKey(DWORD dKey)
{
	if (dKey&EXTENDED_KEY_BASE)
	{
		dKey = (dKey^EXTENDED_KEY_BASE) | 0x100;
	}
	else if (dKey&INTERNAL_KEY_BASE)
	{
		dKey = (dKey^INTERNAL_KEY_BASE) | 0x200;
	}
	else
	{
		DWORD CleanKey = dKey&~KEY_CTRLMASK;

		if (CleanKey>0x80 && CleanKey<0x10000)
		{
			wchar_t WideChar = static_cast<wchar_t>(CleanKey);
			char OemChar = 0;
			if (WideCharToMultiByte(CP_OEMCP, 0, &WideChar, 1, &OemChar, 1, 0, nullptr))
				dKey = (dKey^CleanKey) | as_unsigned(OemChar);
		}
	}

	return dKey;
}

static void ConvertInfoPanelLinesA(const oldfar::InfoPanelLine *iplA, InfoPanelLine **piplW, size_t iCount)
{
	if (iplA && piplW && (iCount>0))
	{
		auto iplW = new InfoPanelLine[iCount];

		for (size_t i = 0; i<iCount; i++)
		{
			iplW[i].Text = AnsiToUnicodeBin(iplA[i].Text, 80); //BUGBUG
			iplW[i].Data = AnsiToUnicodeBin(iplA[i].Data, 80); //BUGBUG
			iplW[i].Flags = 0;
			if (iplA[i].Separator)
				iplW[i].Flags |= IPLFLAGS_SEPARATOR;
		}

		*piplW = iplW;
	}
}

static void FreeUnicodeInfoPanelLines(const InfoPanelLine *iplW, size_t InfoLinesNumber)
{
	for (size_t i = 0; i<InfoLinesNumber; i++)
	{
		delete[] iplW[i].Text;
		delete[] iplW[i].Data;
	}

	delete[] iplW;
}

static void ConvertPanelModesA(const oldfar::PanelMode *pnmA, PanelMode **ppnmW, size_t iCount)
{
	if (pnmA && ppnmW && (iCount>0))
	{
		PanelMode *pnmW = new PanelMode[iCount]();

		for (size_t i = 0; i<iCount; i++)
		{
			size_t iColumnCount = pnmA[i].ColumnTypes ? split_to_vector::get(wide(pnmA[i].ColumnTypes), 0, L",").size() : 0;

			pnmW[i].ColumnTypes = (pnmA[i].ColumnTypes) ? AnsiToUnicode(pnmA[i].ColumnTypes) : nullptr;
			pnmW[i].ColumnWidths = (pnmA[i].ColumnWidths) ? AnsiToUnicode(pnmA[i].ColumnWidths) : nullptr;
			pnmW[i].ColumnTitles = (pnmA[i].ColumnTitles && (iColumnCount>0)) ? ArrayAnsiToUnicode(pnmA[i].ColumnTitles, iColumnCount) : nullptr;
			pnmW[i].StatusColumnTypes = (pnmA[i].StatusColumnTypes) ? AnsiToUnicode(pnmA[i].StatusColumnTypes) : nullptr;
			pnmW[i].StatusColumnWidths = (pnmA[i].StatusColumnWidths) ? AnsiToUnicode(pnmA[i].StatusColumnWidths) : nullptr;
			pnmW[i].Flags = 0;
			if (pnmA[i].FullScreen) pnmW[i].Flags |= PMFLAGS_FULLSCREEN;
			if (pnmA[i].DetailedStatus) pnmW[i].Flags |= PMFLAGS_DETAILEDSTATUS;
			if (pnmA[i].AlignExtensions) pnmW[i].Flags |= PMFLAGS_ALIGNEXTENSIONS;
			if (pnmA[i].CaseConversion) pnmW[i].Flags |= PMFLAGS_CASECONVERSION;
		}

		*ppnmW = pnmW;
	}
}

static void FreeUnicodePanelModes(const PanelMode *pnmW, size_t iCount)
{
	if (pnmW)
	{
		for (size_t i = 0; i<iCount; i++)
		{
			delete[] pnmW[i].ColumnTypes;
			delete[] pnmW[i].ColumnWidths;
			FreeArrayUnicode(pnmW[i].ColumnTitles);
			delete[] pnmW[i].StatusColumnTypes;
			delete[] pnmW[i].StatusColumnWidths;
		}
		delete[] pnmW;
	}
}

static void ProcLabels(const char *Title, KeyBarLabel *Label, int& j, WORD Key, DWORD Shift)
{
	if (Title)
	{
		Label->Text = AnsiToUnicode(Title);
		Label->LongText = nullptr;
		Label->Key.VirtualKeyCode = Key;
		Label->Key.ControlKeyState = Shift;
		j++;
	}
}

static void ConvertKeyBarTitlesA(const oldfar::KeyBarTitles *kbtA, KeyBarTitles *kbtW, bool FullStruct = true)
{
	if (kbtA && kbtW)
	{
		int i;
		kbtW->CountLabels = 0;
		kbtW->Labels = nullptr;

		for (i = 0; i<12; i++)
		{
			if (kbtA->Titles[i])
				kbtW->CountLabels++;
			if (kbtA->CtrlTitles[i])
				kbtW->CountLabels++;
			if (kbtA->AltTitles[i])
				kbtW->CountLabels++;
			if (kbtA->ShiftTitles[i])
				kbtW->CountLabels++;
			if (FullStruct)
			{
				if (kbtA->CtrlShiftTitles[i])
					kbtW->CountLabels++;
				if (kbtA->AltShiftTitles[i])
					kbtW->CountLabels++;
				if (kbtA->CtrlAltTitles[i])
					kbtW->CountLabels++;
			}
		}

		if (kbtW->CountLabels)
		{
			kbtW->Labels = new KeyBarLabel[kbtW->CountLabels];

			int j;
			for (j = i = 0; i<12; i++)
			{
				if (kbtA->Titles[i])
					ProcLabels(kbtA->Titles[i], kbtW->Labels + j, j, VK_F1 + i, 0);
				if (kbtA->CtrlTitles[i])
					ProcLabels(kbtA->CtrlTitles[i], kbtW->Labels + j, j, VK_F1 + i, LEFT_CTRL_PRESSED);
				if (kbtA->AltTitles[i])
					ProcLabels(kbtA->AltTitles[i], kbtW->Labels + j, j, VK_F1 + i, LEFT_ALT_PRESSED);
				if (kbtA->ShiftTitles[i])
					ProcLabels(kbtA->ShiftTitles[i], kbtW->Labels + j, j, VK_F1 + i, SHIFT_PRESSED);

				if (FullStruct)
				{
					if (kbtA->CtrlShiftTitles[i])
						ProcLabels(kbtA->CtrlShiftTitles[i], kbtW->Labels + j, j, VK_F1 + i, LEFT_CTRL_PRESSED | SHIFT_PRESSED);
					if (kbtA->AltShiftTitles[i])
						ProcLabels(kbtA->AltShiftTitles[i], kbtW->Labels + j, j, VK_F1 + i, LEFT_ALT_PRESSED | SHIFT_PRESSED);
					if (kbtA->CtrlAltTitles[i])
						ProcLabels(kbtA->CtrlAltTitles[i], kbtW->Labels + j, j, VK_F1 + i, LEFT_CTRL_PRESSED | LEFT_ALT_PRESSED);
				}
			}
		}
	}
}

static void FreeUnicodeKeyBarTitles(KeyBarTitles *kbtW)
{
	if (kbtW && kbtW->CountLabels && kbtW->Labels)
	{
		for (size_t i = 0; i < kbtW->CountLabels; i++)
		{
			delete[] kbtW->Labels[i].Text;
		}
		delete[] kbtW->Labels;
		kbtW->Labels = nullptr;
		kbtW->CountLabels = 0;
	}
}

static void WINAPI FreeUserData(void* UserData, const FarPanelItemFreeInfo* Info)
{
	xf_free(UserData);
}

static PluginPanelItem* ConvertAnsiPanelItemsToUnicode(const oldfar::PluginPanelItem *PanelItemA, size_t ItemsNumber)
{
	PluginPanelItem *Result = new PluginPanelItem[ItemsNumber]();

	auto AIter = PanelItemA;
	auto WIter = Result;

	for (; AIter != PanelItemA + ItemsNumber; ++AIter, ++WIter)
	{
		WIter->Flags = 0;
		if (AIter->Flags&oldfar::PPIF_PROCESSDESCR)
			WIter->Flags |= PPIF_PROCESSDESCR;
		if (AIter->Flags&oldfar::PPIF_SELECTED)
			WIter->Flags |= PPIF_SELECTED;

		WIter->NumberOfLinks = AIter->NumberOfLinks;

		if (AIter->Description)
			WIter->Description = AnsiToUnicode(AIter->Description);

		if (AIter->Owner)
			WIter->Owner = AnsiToUnicode(AIter->Owner);

		if (AIter->CustomColumnNumber)
		{
			WIter->CustomColumnNumber = AIter->CustomColumnNumber;
			WIter->CustomColumnData = ArrayAnsiToUnicode(AIter->CustomColumnData, AIter->CustomColumnNumber);
		}

		if (AIter->Flags&oldfar::PPIF_USERDATA)
		{
			void* UserData = (void*)AIter->UserData;
			DWORD Size = *(DWORD *)UserData;
			WIter->UserData.Data = xf_malloc(Size);
			memcpy(WIter->UserData.Data, UserData, Size);
			WIter->UserData.FreeData = FreeUserData;
		}
		else
		{
			WIter->UserData.Data = (void*)AIter->UserData;
			WIter->UserData.FreeData = nullptr;
		}
		WIter->CRC32 = AIter->CRC32;
		WIter->FileAttributes = AIter->FindData.dwFileAttributes;
		WIter->CreationTime = AIter->FindData.ftCreationTime;
		WIter->LastAccessTime = AIter->FindData.ftLastAccessTime;
		WIter->LastWriteTime = AIter->FindData.ftLastWriteTime;
		WIter->FileSize = (unsigned __int64)AIter->FindData.nFileSizeLow + (((unsigned __int64)AIter->FindData.nFileSizeHigh) << 32);
		WIter->AllocationSize = (unsigned __int64)AIter->PackSize + (((unsigned __int64)AIter->PackSizeHigh) << 32);
		WIter->FileName = AnsiToUnicode(AIter->FindData.cFileName);
		WIter->AlternateFileName = AnsiToUnicode(AIter->FindData.cAlternateFileName);
	}
	return Result;
}

static void ConvertPanelItemToAnsi(const PluginPanelItem &PanelItem, oldfar::PluginPanelItem &PanelItemA, size_t PathOffset = 0)
{
	PanelItemA.Flags = 0;

	if (PanelItem.Flags&PPIF_PROCESSDESCR)
		PanelItemA.Flags |= oldfar::PPIF_PROCESSDESCR;

	if (PanelItem.Flags&PPIF_SELECTED)
		PanelItemA.Flags |= oldfar::PPIF_SELECTED;

	if (PanelItem.UserData.FreeData == FreeUserData)
		PanelItemA.Flags |= oldfar::PPIF_USERDATA;

	PanelItemA.NumberOfLinks = PanelItem.NumberOfLinks;

	if (PanelItem.Description)
		PanelItemA.Description = UnicodeToAnsi(PanelItem.Description);

	if (PanelItem.Owner)
		PanelItemA.Owner = UnicodeToAnsi(PanelItem.Owner);

	if (PanelItem.CustomColumnNumber)
	{
		PanelItemA.CustomColumnNumber = static_cast<int>(PanelItem.CustomColumnNumber);
		PanelItemA.CustomColumnData = new char*[PanelItem.CustomColumnNumber];

		for (size_t j = 0; j<PanelItem.CustomColumnNumber; j++)
			PanelItemA.CustomColumnData[j] = UnicodeToAnsi(PanelItem.CustomColumnData[j]);
	}

	if (PanelItem.UserData.Data&&PanelItem.UserData.FreeData == FreeUserData)
	{
		DWORD Size = *(DWORD *)PanelItem.UserData.Data;
		PanelItemA.UserData = (intptr_t)xf_malloc(Size);
		memcpy((void *)PanelItemA.UserData, PanelItem.UserData.Data, Size);
	}
	else
		PanelItemA.UserData = (intptr_t)PanelItem.UserData.Data;

	PanelItemA.CRC32 = PanelItem.CRC32;
	PanelItemA.FindData.dwFileAttributes = PanelItem.FileAttributes;
	PanelItemA.FindData.ftCreationTime = PanelItem.CreationTime;
	PanelItemA.FindData.ftLastAccessTime = PanelItem.LastAccessTime;
	PanelItemA.FindData.ftLastWriteTime = PanelItem.LastWriteTime;
	PanelItemA.FindData.nFileSizeLow = (DWORD)(PanelItem.FileSize & 0xFFFFFFFF);
	PanelItemA.FindData.nFileSizeHigh = (DWORD)(PanelItem.FileSize >> 32);
	PanelItemA.PackSize = (DWORD)(PanelItem.AllocationSize & 0xFFFFFFFF);
	PanelItemA.PackSizeHigh = (DWORD)(PanelItem.AllocationSize >> 32);
	UnicodeToOEM(PanelItem.FileName + PathOffset, PanelItemA.FindData.cFileName, sizeof(PanelItemA.FindData.cFileName));
	UnicodeToOEM(PanelItem.AlternateFileName, PanelItemA.FindData.cAlternateFileName, sizeof(PanelItemA.FindData.cAlternateFileName));
}

static void ConvertPanelItemsArrayToAnsi(const PluginPanelItem *PanelItemW, oldfar::PluginPanelItem *&PanelItemA, size_t ItemsNumber)
{
	PanelItemA = new oldfar::PluginPanelItem[ItemsNumber]();

	for (size_t i = 0; i<ItemsNumber; i++)
	{
		ConvertPanelItemToAnsi(PanelItemW[i], PanelItemA[i]);
	}
}

static void FreeUnicodePanelItem(PluginPanelItem *PanelItem, size_t ItemsNumber)
{
	for (size_t i = 0; i<ItemsNumber; i++)
	{
		delete[] PanelItem[i].Description;
		delete[] PanelItem[i].Owner;

		if (PanelItem[i].CustomColumnNumber)
		{
			for (size_t j = 0; j<PanelItem[i].CustomColumnNumber; j++)
				delete[] PanelItem[i].CustomColumnData[j];

			delete[] PanelItem[i].CustomColumnData;
		}

		FreePluginPanelItem(PanelItem[i]);
	}

	delete[] PanelItem;
}

static void FreePanelItemA(const oldfar::PluginPanelItem *PanelItem, size_t ItemsNumber, bool bFreeArray = true)
{
	for (size_t i = 0; i<ItemsNumber; i++)
	{
		delete[] PanelItem[i].Description;
		delete[] PanelItem[i].Owner;

		if (PanelItem[i].CustomColumnNumber)
		{
			for (int j = 0; j<PanelItem[i].CustomColumnNumber; j++)
				delete[] PanelItem[i].CustomColumnData[j];

			delete[] PanelItem[i].CustomColumnData;
		}

		if (PanelItem[i].UserData&&PanelItem[i].Flags&oldfar::PPIF_USERDATA)
		{
			xf_free((PVOID)PanelItem[i].UserData);
		}
	}

	if (bFreeArray)
		delete[] PanelItem;
}

static char *InsertQuoteA(char *Str)
{
	size_t l = strlen(Str);

	if (*Str != '"')
	{
		memmove(Str + 1, Str, ++l);
		*Str = '"';
	}

	if (Str[l - 1] != '"')
	{
		Str[l++] = '\"';
		Str[l] = 0;
	}

	return Str;
}

static inline const GUID* GetPluginGuid(intptr_t n) { return &reinterpret_cast<Plugin*>(n)->GetGUID(); }

struct DialogData
{
	oldfar::FARWINDOWPROC DlgProc;
	HANDLE hDlg;
	oldfar::FarDialogItem *diA;
	FarDialogItem *di;
	FarList *l;
};

static std::list<DialogData>& DialogList()
{
	static FN_RETURN_TYPE(DialogList) s_DialogList;
	return s_DialogList;
}

oldfar::FarDialogItem* OneDialogItem = nullptr;

static DialogData* FindDialogData(HANDLE hDlg)
{
	auto ItemIterator = std::find_if(RANGE(DialogList(), i)
	{
		return i.hDlg == hDlg;
	});

	DialogData* Result = nullptr;

	if (ItemIterator != DialogList().end())
	{
		Result = &*ItemIterator;
	}
	return Result;
}

// can be nullptr in case of the ansi dialog plugin
static oldfar::FarDialogItem* CurrentDialogItemA(HANDLE hDlg, int ItemNumber)
{
	auto current = FindDialogData(hDlg);
	return current ? &current->diA[ItemNumber] : nullptr;
}

static FarDialogItem& CurrentDialogItem(HANDLE hDlg, int ItemNumber)
{
	return FindDialogData(hDlg)->di[ItemNumber];
}

static FarList& CurrentList(HANDLE hDlg, int ItemNumber)
{
	return FindDialogData(hDlg)->l[ItemNumber];
}

static std::stack<FarDialogEvent>& OriginalEvents()
{
	static FN_RETURN_TYPE(OriginalEvents) sOriginalEvents;
	return sOriginalEvents;
}

static void UnicodeListItemToAnsi(const FarListItem* li, oldfar::FarListItem* liA)
{
	UnicodeToOEM(li->Text, liA->Text, sizeof(liA->Text) - 1);
	liA->Flags = 0;

	if (li->Flags&LIF_SELECTED)       liA->Flags |= oldfar::LIF_SELECTED;

	if (li->Flags&LIF_CHECKED)        liA->Flags |= oldfar::LIF_CHECKED;

	if (li->Flags&LIF_SEPARATOR)      liA->Flags |= oldfar::LIF_SEPARATOR;

	if (li->Flags&LIF_DISABLE)        liA->Flags |= oldfar::LIF_DISABLE;

	if (li->Flags&LIF_GRAYED)         liA->Flags |= oldfar::LIF_GRAYED;

	if (li->Flags&LIF_HIDDEN)         liA->Flags |= oldfar::LIF_HIDDEN;

	if (li->Flags&LIF_DELETEUSERDATA) liA->Flags |= oldfar::LIF_DELETEUSERDATA;
}

static size_t GetAnsiVBufSize(const oldfar::FarDialogItem &diA)
{
	return (diA.X2 - diA.X1 + 1)*(diA.Y2 - diA.Y1 + 1);
}

static PCHAR_INFO GetAnsiVBufPtr(FAR_CHAR_INFO* VBuf, size_t Size)
{
	PCHAR_INFO VBufA = nullptr;
	if (VBuf)
	{
		VBufA = *reinterpret_cast<PCHAR_INFO*>(&VBuf[Size]);
	}
	return VBufA;
}

static void SetAnsiVBufPtr(FAR_CHAR_INFO* VBuf, PCHAR_INFO VBufA, size_t Size)
{
	if (VBuf)
	{
		*reinterpret_cast<PCHAR_INFO*>(&VBuf[Size]) = VBufA;
	}
}

static void AnsiVBufToUnicode(PCHAR_INFO VBufA, FAR_CHAR_INFO* VBuf, size_t Size, bool NoCvt)
{
	if (VBuf && VBufA)
	{
		for (size_t i = 0; i < Size; i++)
		{
			if (NoCvt)
			{
				VBuf[i].Char = VBufA[i].Char.UnicodeChar;
			}
			else
			{
				AnsiToUnicodeBin(&VBufA[i].Char.AsciiChar, &VBuf[i].Char, 1);
			}
			VBuf[i].Attributes = Colors::ConsoleColorToFarColor(VBufA[i].Attributes);
		}
	}
}

static FAR_CHAR_INFO* AnsiVBufToUnicode(oldfar::FarDialogItem &diA)
{
	FAR_CHAR_INFO* VBuf = nullptr;

	if (diA.VBuf)
	{
		size_t Size = GetAnsiVBufSize(diA);
		// + 1 потому что там храним поинтер на анси vbuf.
		VBuf = new FAR_CHAR_INFO[Size + 1];

		AnsiVBufToUnicode(diA.VBuf, VBuf, Size, (diA.Flags&oldfar::DIF_NOTCVTUSERCONTROL) == oldfar::DIF_NOTCVTUSERCONTROL);
		SetAnsiVBufPtr(VBuf, diA.VBuf, Size);
	}

	return VBuf;
}

static void AnsiListItemToUnicode(const oldfar::FarListItem* liA, FarListItem* li)
{
	li->Text = AnsiToUnicode(liA->Text);
	li->Flags = 0;

	if (liA->Flags&oldfar::LIF_SELECTED)       li->Flags |= LIF_SELECTED;

	if (liA->Flags&oldfar::LIF_CHECKED)        li->Flags |= LIF_CHECKED;

	if (liA->Flags&oldfar::LIF_SEPARATOR)      li->Flags |= LIF_SEPARATOR;

	if (liA->Flags&oldfar::LIF_DISABLE)        li->Flags |= LIF_DISABLE;

	if (liA->Flags&oldfar::LIF_GRAYED)         li->Flags |= LIF_GRAYED;

	if (liA->Flags&oldfar::LIF_HIDDEN)         li->Flags |= LIF_HIDDEN;

	if (liA->Flags&oldfar::LIF_DELETEUSERDATA) li->Flags |= LIF_DELETEUSERDATA;
}

static void AnsiDialogItemToUnicodeSafe(const oldfar::FarDialogItem &diA, FarDialogItem &di)
{
	switch (diA.Type)
	{
	case oldfar::DI_TEXT:
		di.Type = DI_TEXT;
		break;
	case oldfar::DI_VTEXT:
		di.Type = DI_VTEXT;
		break;
	case oldfar::DI_SINGLEBOX:
		di.Type = DI_SINGLEBOX;
		break;
	case oldfar::DI_DOUBLEBOX:
		di.Type = DI_DOUBLEBOX;
		break;
	case oldfar::DI_EDIT:
		di.Type = DI_EDIT;
		break;
	case oldfar::DI_PSWEDIT:
		di.Type = DI_PSWEDIT;
		break;
	case oldfar::DI_FIXEDIT:
		di.Type = DI_FIXEDIT;
		break;
	case oldfar::DI_BUTTON:
		di.Type = DI_BUTTON;
		di.Selected = diA.Selected;
		break;
	case oldfar::DI_CHECKBOX:
		di.Type = DI_CHECKBOX;
		di.Selected = diA.Selected;
		break;
	case oldfar::DI_RADIOBUTTON:
		di.Type = DI_RADIOBUTTON;
		di.Selected = diA.Selected;
		break;
	case oldfar::DI_COMBOBOX:
		di.Type = DI_COMBOBOX;
		break;
	case oldfar::DI_LISTBOX:
		di.Type = DI_LISTBOX;
		break;
	case oldfar::DI_MEMOEDIT:
		di.Type = DI_MEMOEDIT;
		break;
	case oldfar::DI_USERCONTROL:
		di.Type = DI_USERCONTROL;
		break;
	}

	di.X1 = diA.X1;
	di.Y1 = diA.Y1;
	di.X2 = diA.X2;
	di.Y2 = diA.Y2;
	di.Flags = 0;

	if (diA.Focus)
	{
		di.Flags |= DIF_FOCUS;
	}

	// emulate old listbox behaviour
	if (diA.Type == oldfar::DI_LISTBOX)
	{
		di.Flags |= DIF_LISTTRACKMOUSE;
	}

	if (diA.Flags)
	{
		if (diA.Flags&oldfar::DIF_BOXCOLOR)
			di.Flags |= DIF_BOXCOLOR;

		if (diA.Flags&oldfar::DIF_GROUP)
			di.Flags |= DIF_GROUP;

		if (diA.Flags&oldfar::DIF_LEFTTEXT)
			di.Flags |= DIF_LEFTTEXT;

		if (diA.Flags&oldfar::DIF_MOVESELECT)
			di.Flags |= DIF_MOVESELECT;

		if (diA.Flags&oldfar::DIF_SHOWAMPERSAND)
			di.Flags |= DIF_SHOWAMPERSAND;

		if (diA.Flags&oldfar::DIF_CENTERGROUP)
			di.Flags |= DIF_CENTERGROUP;

		if (diA.Flags&oldfar::DIF_NOBRACKETS)
			di.Flags |= DIF_NOBRACKETS;

		if (diA.Flags&oldfar::DIF_MANUALADDHISTORY)
			di.Flags |= DIF_MANUALADDHISTORY;

		if (diA.Flags&oldfar::DIF_SEPARATOR)
			di.Flags |= DIF_SEPARATOR;

		if (diA.Flags&oldfar::DIF_SEPARATOR2)
			di.Flags |= DIF_SEPARATOR2;

		if (diA.Flags&oldfar::DIF_EDITOR)
			di.Flags |= DIF_EDITOR;

		if (diA.Flags&oldfar::DIF_LISTNOAMPERSAND)
			di.Flags |= DIF_LISTNOAMPERSAND;

		if (diA.Flags&oldfar::DIF_LISTNOBOX)
			di.Flags |= DIF_LISTNOBOX;

		if (diA.Flags&oldfar::DIF_HISTORY)
			di.Flags |= DIF_HISTORY;

		if (diA.Flags&oldfar::DIF_BTNNOCLOSE)
			di.Flags |= DIF_BTNNOCLOSE;

		if (diA.Flags&oldfar::DIF_CENTERTEXT)
			di.Flags |= DIF_CENTERTEXT;

		if (diA.Flags&oldfar::DIF_SEPARATORUSER)
			di.Flags |= DIF_SEPARATORUSER;

		if (diA.Flags&oldfar::DIF_EDITEXPAND)
			di.Flags |= DIF_EDITEXPAND;

		if (diA.Flags&oldfar::DIF_DROPDOWNLIST)
			di.Flags |= DIF_DROPDOWNLIST;

		if (diA.Flags&oldfar::DIF_USELASTHISTORY)
			di.Flags |= DIF_USELASTHISTORY;

		if (diA.Flags&oldfar::DIF_MASKEDIT)
			di.Flags |= DIF_MASKEDIT;

		if (diA.Flags&oldfar::DIF_SELECTONENTRY)
			di.Flags |= DIF_SELECTONENTRY;

		if (diA.Flags&oldfar::DIF_3STATE)
			di.Flags |= DIF_3STATE;

		if (diA.Flags&oldfar::DIF_EDITPATH)
			di.Flags |= DIF_EDITPATH;

		if (diA.Flags&oldfar::DIF_LISTWRAPMODE)
			di.Flags |= DIF_LISTWRAPMODE;

		if (diA.Flags&oldfar::DIF_LISTAUTOHIGHLIGHT)
			di.Flags |= DIF_LISTAUTOHIGHLIGHT;

		if (diA.Flags&oldfar::DIF_AUTOMATION)
			di.Flags |= DIF_AUTOMATION;

		if (diA.Flags&oldfar::DIF_HIDDEN)
			di.Flags |= DIF_HIDDEN;

		if (diA.Flags&oldfar::DIF_READONLY)
			di.Flags |= DIF_READONLY;

		if (diA.Flags&oldfar::DIF_NOFOCUS)
			di.Flags |= DIF_NOFOCUS;

		if (diA.Flags&oldfar::DIF_DISABLE)
			di.Flags |= DIF_DISABLE;
	}

	if (diA.DefaultButton) di.Flags |= DIF_DEFAULTBUTTON;
}

static void AnsiDialogItemToUnicode(oldfar::FarDialogItem &diA, FarDialogItem &di, FarList &l)
{
	ClearStruct(di);
	AnsiDialogItemToUnicodeSafe(diA, di);

	switch (di.Type)
	{
	case DI_LISTBOX:
	case DI_COMBOBOX:
	{
		if (diA.ListItems && global::IsPtr(diA.ListItems))
		{
			l.Items = new FarListItem[diA.ListItems->ItemsNumber];
			l.ItemsNumber = diA.ListItems->ItemsNumber;
			di.ListItems = &l;

			for (size_t j = 0; j < di.ListItems->ItemsNumber; j++)
				AnsiListItemToUnicode(&diA.ListItems->Items[j], &l.Items[j]);
		}

		break;
	}
	case DI_USERCONTROL:
		di.VBuf = AnsiVBufToUnicode(diA);
		break;
	case DI_EDIT:
	case DI_FIXEDIT:
	{
		if (diA.Flags&oldfar::DIF_HISTORY && diA.History)
			di.History = AnsiToUnicode(diA.History);
		else if (diA.Flags&oldfar::DIF_MASKEDIT && diA.Mask)
			di.Mask = AnsiToUnicode(diA.Mask);

		break;
	}
	default:
		break;
	}

	if (diA.Type == oldfar::DI_USERCONTROL)
	{
		wchar_t* Buffer = new wchar_t[ARRAYSIZE(diA.Data)];
		memcpy(Buffer, diA.Data, sizeof(diA.Data));
		di.Data = Buffer;
		di.MaxLength = 0;
	}
	else if ((diA.Type == oldfar::DI_EDIT || diA.Type == oldfar::DI_COMBOBOX) && diA.Flags&oldfar::DIF_VAREDIT)
		di.Data = AnsiToUnicode(diA.Ptr.PtrData);
	else
		di.Data = AnsiToUnicode(diA.Data);

	//BUGBUG тут надо придумать как сделать лучше: maxlen=513 например и также подумать что делать для DIF_VAREDIT
	//di->MaxLen = 0;
}

static void FreeUnicodeDialogItem(FarDialogItem &di)
{
	switch (di.Type)
	{
	case DI_EDIT:
	case DI_FIXEDIT:

		if ((di.Flags&DIF_HISTORY) && di.History)
			delete[] di.History;
		else if ((di.Flags&DIF_MASKEDIT) && di.Mask)
			delete[] di.Mask;
		break;

	case DI_LISTBOX:
	case DI_COMBOBOX:
		if (di.ListItems)
		{
			if (di.ListItems->Items)
			{
				for (size_t i = 0; i < di.ListItems->ItemsNumber; i++)
				{
					delete[] di.ListItems->Items[i].Text;
				}

				delete[] di.ListItems->Items;
				di.ListItems->Items = nullptr;
			}
		}
		break;

	case DI_USERCONTROL:
		delete[] di.VBuf;
		break;

	default:
		break;
	}

	delete[] di.Data;
}

static void FreeAnsiDialogItem(oldfar::FarDialogItem &diA)
{
	if ((diA.Type == oldfar::DI_EDIT || diA.Type == oldfar::DI_FIXEDIT) && (diA.Flags&oldfar::DIF_HISTORY || diA.Flags&oldfar::DIF_MASKEDIT) && diA.History)
	{
		delete[] diA.History;
		diA.History = nullptr;
	}

	if ((diA.Type == oldfar::DI_EDIT || diA.Type == oldfar::DI_COMBOBOX) && diA.Flags&oldfar::DIF_VAREDIT && diA.Ptr.PtrData)
	{
		delete[] diA.Ptr.PtrData;
		diA.Ptr.PtrData = nullptr;
	}
}

static void UnicodeDialogItemToAnsiSafe(const FarDialogItem &di, oldfar::FarDialogItem &diA)
{
	switch (di.Type)
	{
	case DI_TEXT:
		diA.Type = oldfar::DI_TEXT;
		break;
	case DI_VTEXT:
		diA.Type = oldfar::DI_VTEXT;
		break;
	case DI_SINGLEBOX:
		diA.Type = oldfar::DI_SINGLEBOX;
		break;
	case DI_DOUBLEBOX:
		diA.Type = oldfar::DI_DOUBLEBOX;
		break;
	case DI_EDIT:
		diA.Type = oldfar::DI_EDIT;
		break;
	case DI_PSWEDIT:
		diA.Type = oldfar::DI_PSWEDIT;
		break;
	case DI_FIXEDIT:
		diA.Type = oldfar::DI_FIXEDIT;
		break;
	case DI_BUTTON:
		diA.Type = oldfar::DI_BUTTON;
		diA.Selected = di.Selected;
		break;
	case DI_CHECKBOX:
		diA.Type = oldfar::DI_CHECKBOX;
		diA.Selected = di.Selected;
		break;
	case DI_RADIOBUTTON:
		diA.Type = oldfar::DI_RADIOBUTTON;
		diA.Selected = di.Selected;
		break;
	case DI_COMBOBOX:
		diA.Type = oldfar::DI_COMBOBOX;
		break;
	case DI_LISTBOX:
		diA.Type = oldfar::DI_LISTBOX;
		break;
	case DI_MEMOEDIT:
		diA.Type = oldfar::DI_MEMOEDIT;
		break;
	case DI_USERCONTROL:
		diA.Type = oldfar::DI_USERCONTROL;
		break;
	}

	diA.X1 = di.X1;
	diA.Y1 = di.Y1;
	diA.X2 = di.X2;
	diA.Y2 = di.Y2;
	diA.Focus = (di.Flags&DIF_FOCUS) != 0;

	if (diA.Flags&oldfar::DIF_SETCOLOR)
	{
		diA.Flags = oldfar::DIF_SETCOLOR | (diA.Flags&oldfar::DIF_COLORMASK);
	}
	else
	{
		diA.Flags = 0;
	}

	if (di.Flags)
	{
		if (di.Flags&DIF_BOXCOLOR)
			diA.Flags |= oldfar::DIF_BOXCOLOR;

		if (di.Flags&DIF_GROUP)
			diA.Flags |= oldfar::DIF_GROUP;

		if (di.Flags&DIF_LEFTTEXT)
			diA.Flags |= oldfar::DIF_LEFTTEXT;

		if (di.Flags&DIF_MOVESELECT)
			diA.Flags |= oldfar::DIF_MOVESELECT;

		if (di.Flags&DIF_SHOWAMPERSAND)
			diA.Flags |= oldfar::DIF_SHOWAMPERSAND;

		if (di.Flags&DIF_CENTERGROUP)
			diA.Flags |= oldfar::DIF_CENTERGROUP;

		if (di.Flags&DIF_NOBRACKETS)
			diA.Flags |= oldfar::DIF_NOBRACKETS;

		if (di.Flags&DIF_MANUALADDHISTORY)
			diA.Flags |= oldfar::DIF_MANUALADDHISTORY;

		if (di.Flags&DIF_SEPARATOR)
			diA.Flags |= oldfar::DIF_SEPARATOR;

		if (di.Flags&DIF_SEPARATOR2)
			diA.Flags |= oldfar::DIF_SEPARATOR2;

		if (di.Flags&DIF_EDITOR)
			diA.Flags |= oldfar::DIF_EDITOR;

		if (di.Flags&DIF_LISTNOAMPERSAND)
			diA.Flags |= oldfar::DIF_LISTNOAMPERSAND;

		if (di.Flags&DIF_LISTNOBOX)
			diA.Flags |= oldfar::DIF_LISTNOBOX;

		if (di.Flags&DIF_HISTORY)
			diA.Flags |= oldfar::DIF_HISTORY;

		if (di.Flags&DIF_BTNNOCLOSE)
			diA.Flags |= oldfar::DIF_BTNNOCLOSE;

		if (di.Flags&DIF_CENTERTEXT)
			diA.Flags |= oldfar::DIF_CENTERTEXT;

		if (di.Flags&DIF_SEPARATORUSER)
			diA.Flags |= oldfar::DIF_SEPARATORUSER;

		if (di.Flags&DIF_EDITEXPAND)
			diA.Flags |= oldfar::DIF_EDITEXPAND;

		if (di.Flags&DIF_DROPDOWNLIST)
			diA.Flags |= oldfar::DIF_DROPDOWNLIST;

		if (di.Flags&DIF_USELASTHISTORY)
			diA.Flags |= oldfar::DIF_USELASTHISTORY;

		if (di.Flags&DIF_MASKEDIT)
			diA.Flags |= oldfar::DIF_MASKEDIT;

		if (di.Flags&DIF_SELECTONENTRY)
			diA.Flags |= oldfar::DIF_SELECTONENTRY;

		if (di.Flags&DIF_3STATE)
			diA.Flags |= oldfar::DIF_3STATE;

		if (di.Flags&DIF_EDITPATH)
			diA.Flags |= oldfar::DIF_EDITPATH;

		if (di.Flags&DIF_LISTWRAPMODE)
			diA.Flags |= oldfar::DIF_LISTWRAPMODE;

		if (di.Flags&DIF_LISTAUTOHIGHLIGHT)
			diA.Flags |= oldfar::DIF_LISTAUTOHIGHLIGHT;

		if (di.Flags&DIF_AUTOMATION)
			diA.Flags |= oldfar::DIF_AUTOMATION;

		if (di.Flags&DIF_HIDDEN)
			diA.Flags |= oldfar::DIF_HIDDEN;

		if (di.Flags&DIF_READONLY)
			diA.Flags |= oldfar::DIF_READONLY;

		if (di.Flags&DIF_NOFOCUS)
			diA.Flags |= oldfar::DIF_NOFOCUS;

		if (di.Flags&DIF_DISABLE)
			diA.Flags |= oldfar::DIF_DISABLE;
	}

	diA.DefaultButton = (di.Flags&DIF_DEFAULTBUTTON) != 0;
}

static oldfar::FarDialogItem* UnicodeDialogItemToAnsi(FarDialogItem &di, HANDLE hDlg, int ItemNumber)
{
	oldfar::FarDialogItem *diA = CurrentDialogItemA(hDlg, ItemNumber);

	if (!diA)
	{
		delete OneDialogItem;
		OneDialogItem = new oldfar::FarDialogItem();
		diA = OneDialogItem;
	}

	FreeAnsiDialogItem(*diA);
	UnicodeDialogItemToAnsiSafe(di, *diA);

	switch (diA->Type)
	{
	case oldfar::DI_USERCONTROL:
		diA->VBuf = GetAnsiVBufPtr(di.VBuf, GetAnsiVBufSize(*diA));
		break;
	case oldfar::DI_EDIT:
	case oldfar::DI_FIXEDIT:
	{
		if (di.Flags&DIF_HISTORY)
			diA->History = UnicodeToAnsi(di.History);
		else if (di.Flags&DIF_MASKEDIT)
			diA->Mask = UnicodeToAnsi(di.Mask);
	}
		break;
	case oldfar::DI_COMBOBOX:
	case oldfar::DI_LISTBOX:
		diA->ListPos = static_cast<int>(NativeInfo.SendDlgMessage(hDlg, DM_LISTGETCURPOS, ItemNumber, 0));
		break;
	}

	if (diA->Type == oldfar::DI_USERCONTROL)
	{
		if (di.Data)
			memcpy(diA->Data, di.Data, sizeof(diA->Data));
	}
	else if ((diA->Type == oldfar::DI_EDIT || diA->Type == oldfar::DI_COMBOBOX) && diA->Flags&oldfar::DIF_VAREDIT)
	{
		diA->Ptr.PtrLength = StrLength(di.Data);
		diA->Ptr.PtrData = new char[diA->Ptr.PtrLength + 1];
		UnicodeToOEM(di.Data, diA->Ptr.PtrData, diA->Ptr.PtrLength + 1);
	}
	else
		UnicodeToOEM(di.Data, diA->Data, sizeof(diA->Data));

	return diA;
}

static void ConvertUnicodePanelInfoToAnsi(const PanelInfo* PIW, oldfar::PanelInfo* PIA)
{
	PIA->PanelType = 0;

	switch (PIW->PanelType)
	{
	case PTYPE_FILEPANEL:  PIA->PanelType = oldfar::PTYPE_FILEPANEL;  break;
	case PTYPE_TREEPANEL:  PIA->PanelType = oldfar::PTYPE_TREEPANEL;  break;
	case PTYPE_QVIEWPANEL: PIA->PanelType = oldfar::PTYPE_QVIEWPANEL; break;
	case PTYPE_INFOPANEL:  PIA->PanelType = oldfar::PTYPE_INFOPANEL;  break;
	}

	PIA->Plugin = (PIW->Flags&PFLAGS_PLUGIN) ? 1 : 0;
	PIA->PanelRect.left = PIW->PanelRect.left;
	PIA->PanelRect.top = PIW->PanelRect.top;
	PIA->PanelRect.right = PIW->PanelRect.right;
	PIA->PanelRect.bottom = PIW->PanelRect.bottom;
	PIA->ItemsNumber = static_cast<int>(PIW->ItemsNumber);
	PIA->SelectedItemsNumber = static_cast<int>(PIW->SelectedItemsNumber);
	PIA->PanelItems = nullptr;
	PIA->SelectedItems = nullptr;
	PIA->CurrentItem = static_cast<int>(PIW->CurrentItem);
	PIA->TopPanelItem = static_cast<int>(PIW->TopPanelItem);
	PIA->Visible = (PIW->Flags&PFLAGS_VISIBLE) ? 1 : 0;;
	PIA->Focus = (PIW->Flags&PFLAGS_FOCUS) ? 1 : 0;;
	PIA->ViewMode = PIW->ViewMode;
	PIA->ShortNames = (PIW->Flags&PFLAGS_ALTERNATIVENAMES) ? 1 : 0;;
	PIA->SortMode = 0;

	switch (PIW->SortMode)
	{
	case SM_DEFAULT:        PIA->SortMode = oldfar::SM_DEFAULT;        break;
	case SM_UNSORTED:       PIA->SortMode = oldfar::SM_UNSORTED;       break;
	case SM_NAME:           PIA->SortMode = oldfar::SM_NAME;           break;
	case SM_EXT:            PIA->SortMode = oldfar::SM_EXT;            break;
	case SM_MTIME:          PIA->SortMode = oldfar::SM_MTIME;          break;
	case SM_CTIME:          PIA->SortMode = oldfar::SM_CTIME;          break;
	case SM_ATIME:          PIA->SortMode = oldfar::SM_ATIME;          break;
	case SM_SIZE:           PIA->SortMode = oldfar::SM_SIZE;           break;
	case SM_DESCR:          PIA->SortMode = oldfar::SM_DESCR;          break;
	case SM_OWNER:          PIA->SortMode = oldfar::SM_OWNER;          break;
	case SM_COMPRESSEDSIZE: PIA->SortMode = oldfar::SM_COMPRESSEDSIZE; break;
	case SM_NUMLINKS:       PIA->SortMode = oldfar::SM_NUMLINKS;       break;
	default:
		break;
	}

	PIA->Flags = 0;

	if (PIW->Flags&PFLAGS_SHOWHIDDEN)       PIA->Flags |= oldfar::PFLAGS_SHOWHIDDEN;

	if (PIW->Flags&PFLAGS_HIGHLIGHT)        PIA->Flags |= oldfar::PFLAGS_HIGHLIGHT;

	if (PIW->Flags&PFLAGS_REVERSESORTORDER) PIA->Flags |= oldfar::PFLAGS_REVERSESORTORDER;

	if (PIW->Flags&PFLAGS_USESORTGROUPS)    PIA->Flags |= oldfar::PFLAGS_USESORTGROUPS;

	if (PIW->Flags&PFLAGS_SELECTEDFIRST)    PIA->Flags |= oldfar::PFLAGS_SELECTEDFIRST;

	if (PIW->Flags&PFLAGS_REALNAMES)        PIA->Flags |= oldfar::PFLAGS_REALNAMES;

	if (PIW->Flags&PFLAGS_NUMERICSORT)      PIA->Flags |= oldfar::PFLAGS_NUMERICSORT;

	if (PIW->Flags&PFLAGS_PANELLEFT)        PIA->Flags |= oldfar::PFLAGS_PANELLEFT;

}

static void FreeAnsiPanelInfo(oldfar::PanelInfo* PIA)
{
	if (PIA->PanelItems)
		FreePanelItemA(PIA->PanelItems, PIA->ItemsNumber);

	if (PIA->SelectedItems)
		FreePanelItemA(PIA->SelectedItems, PIA->SelectedItemsNumber);

	ClearStruct(*PIA);
}

struct oldPanelInfoContainer
{
	oldPanelInfoContainer(): Info() {}
	~oldPanelInfoContainer() { FreeAnsiPanelInfo(&Info); }

	oldfar::PanelInfo Info;
};

static __int64 GetSetting(FARSETTINGS_SUBFOLDERS Root, const wchar_t* Name)
{
	__int64 result = 0;
	FarSettingsCreate settings = { sizeof(FarSettingsCreate), FarGuid, INVALID_HANDLE_VALUE };
	HANDLE Settings = NativeInfo.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &settings) ? settings.Handle : 0;
	if (Settings)
	{
		FarSettingsItem item = { sizeof(FarSettingsItem), static_cast<size_t>(Root), Name, FST_UNKNOWN, {} };
		if (NativeInfo.SettingsControl(Settings, SCTL_GET, 0, &item) && FST_QWORD == item.Type)
		{
			result = item.Number;
		}
		NativeInfo.SettingsControl(Settings, SCTL_FREE, 0, 0);
	}
	return result;
}

static uintptr_t GetEditorCodePageA()
{
	EditorInfo info = { sizeof(EditorInfo) };
	NativeInfo.EditorControl(-1, ECTL_GETINFO, 0, &info);
	uintptr_t CodePage = info.CodePage;
	CPINFO cpi;

	if (!GetCPInfo(static_cast<UINT>(CodePage), &cpi) || cpi.MaxCharSize > 1)
		CodePage = GetACP();

	return CodePage;
}

static int GetEditorCodePageFavA()
{
	uintptr_t CodePage = GetEditorCodePageA();
	int result = -((int)CodePage + 2);

	if (GetOEMCP() == CodePage)
	{
		result = 0;
	}
	else if (GetACP() == CodePage)
	{
		result = 1;
	}
	else
	{
		DWORD FavIndex = 2;
		const auto strCP = std::to_wstring(CodePage);
		const auto CpEnum = Codepages().GetFavoritesEnumerator();
		std::any_of(CONST_RANGE(CpEnum, i) -> bool
		{
			if (i.second & CPST_FAVORITE)
			{
				if (i.first == strCP)
				{
					result = FavIndex;
					return true;
				}
				FavIndex++;
			}
			return false;
		});
	}

	return result;
}

static void MultiByteRecode(UINT nCPin, UINT nCPout, char *szBuffer, int nLength)
{
	if (szBuffer && nLength > 0)
	{
		std::vector<wchar_t> TempTable(nLength);
		MultiByteToWideChar(nCPin, 0, szBuffer, nLength, TempTable.data(), nLength);
		WideCharToMultiByte(nCPout, 0, TempTable.data(), nLength, szBuffer, nLength, nullptr, nullptr);
	}
};

static uintptr_t ConvertCharTableToCodePage(int Command)
{
	uintptr_t nCP = CP_DEFAULT;

	if (Command < 0)
	{
		nCP = -(Command + 2);
	}
	else
	{
		switch (Command)
		{
		case 0 /* OEM */: 	nCP = GetOEMCP();	break;
		case 1 /* ANSI */:	nCP = GetACP(); 	break;
		default:
		{
			int FavIndex = 2;
			const auto CpEnum = Codepages().GetFavoritesEnumerator();
			std::any_of(CONST_RANGE(CpEnum, i) -> bool
			{
				if (i.second & CPST_FAVORITE)
				{
					if (FavIndex == Command)
					{
						nCP = std::stoi(i.first);
						return true;
					}
					FavIndex++;
				}
				return false;
			});
		}
		}
	}

	return nCP;
}

struct FAR_SEARCH_A_CALLBACK_PARAM
{
	oldfar::FRSUSERFUNC Func;
	void *Param;
};

namespace pluginapi
{
static void WINAPI qsort(void *base, size_t nelem, size_t width, comparer cmp) noexcept
{
	try
	{
		return NativeFSF.qsort(base, nelem, width, comparer_wrapper, reinterpret_cast<void*>(cmp));
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

static void WINAPI qsortex(void *base, size_t nelem, size_t width, comparer_ex cmp, void *userparam) noexcept
{
	try
	{
		comparer_helper helper = { cmp, userparam };
		return NativeFSF.qsort(base, nelem, width, comparer_ex_wrapper, &helper);
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

static void* WINAPI bsearch(const void *key, const void *base, size_t nelem, size_t width, comparer cmp) noexcept
{
	try
	{
		return NativeFSF.bsearch(key, base, nelem, width, comparer_wrapper, reinterpret_cast<void*>(cmp));
	}
	catch (...)
	{
		// TODO: log
		return nullptr;
	}
}

static int WINAPI LocalIslower(unsigned Ch) noexcept
{
	try
	{
		return Ch < 256 && IsUpperOrLower[Ch] == 1;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI LocalIsupper(unsigned Ch) noexcept
{
	try
	{
		return Ch < 256 && IsUpperOrLower[Ch] == 2;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI LocalIsalpha(unsigned Ch) noexcept
{
	try
	{
		if (Ch >= 256)
			return FALSE;

		char CvtCh = Ch;
		OemToCharBuffA(&CvtCh, &CvtCh, 1);
		return IsCharAlphaA(CvtCh);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI LocalIsalphanum(unsigned Ch) noexcept
{
	try
	{
		if (Ch >= 256)
			return FALSE;

		char CvtCh = Ch;
		OemToCharBuffA(&CvtCh, &CvtCh, 1);
		return IsCharAlphaNumericA(CvtCh);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static unsigned WINAPI LocalUpper(unsigned LowerChar) noexcept
{
	try
	{
		return LowerChar < 256 ? LowerToUpper[LowerChar] : LowerChar;
	}
	catch (...)
	{
		// TODO: log
		return LowerChar;
	}
}

static void WINAPI LocalUpperBuf(char *Buf, int Length) noexcept
{
	try
	{
		auto str = as_string(Buf, Length);
		std::for_each(RANGE(str, i) { i = LowerToUpper[as_unsigned(i)]; });
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

static unsigned WINAPI LocalLower(unsigned UpperChar) noexcept
{
	try
	{
		return UpperChar < 256 ? UpperToLower[UpperChar] : UpperChar;
	}
	catch (...)
	{
		// TODO: log
		return UpperChar;
	}
}

static void WINAPI LocalLowerBuf(char *Buf, int Length) noexcept
{
	try
	{
		auto str = as_string(Buf, Length);
		std::for_each(RANGE(str, i) { i = UpperToLower[as_unsigned(i)]; });
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

static void WINAPI LocalStrupr(char *s1) noexcept
{
	try
	{
		auto str = as_string(s1);
		std::for_each(RANGE(str, i) { i = LowerToUpper[as_unsigned(i)]; });
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

static void WINAPI LocalStrlwr(char *s1) noexcept
{
	try
	{
		auto str = as_string(s1);
		std::for_each(RANGE(str, i) { i = UpperToLower[as_unsigned(i)]; });
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

static int WINAPI LStricmp(const char *s1, const char *s2) noexcept
{
	try
	{
		return LocalStricmp(s1, s2);
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}

}

static int WINAPI LStrnicmp(const char *s1, const char *s2, int n) noexcept
{
	try
	{
		return LocalStrnicmp(s1, s2, n);
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

static char* WINAPI RemoveTrailingSpacesA(char *Str) noexcept
{
	try
	{
		if (*Str == '\0')
			return Str;

		char *ChPtr;
		size_t I;

		for (ChPtr = Str + (I = strlen(Str)) - 1; I > 0; I--, ChPtr--)
		{
			if (IsSpaceA(*ChPtr) || IsEolA(*ChPtr))
				*ChPtr = 0;
			else
				break;
		}

		return Str;
	}
	catch (...)
	{
		// TODO: log
		return Str;
	}
}

static char *WINAPI FarItoaA(int value, char *string, int radix) noexcept
{
	try
	{
		return _itoa(value, string, radix);
	}
	catch (...)
	{
		// TODO: log
		return string;
	}
}

static char *WINAPI FarItoa64A(__int64 value, char *string, int radix) noexcept
{
	try
	{
		return _i64toa(value, string, radix);
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

static int WINAPI FarAtoiA(const char *s) noexcept
{
	try
	{
		return atoi(s);
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

static __int64 WINAPI FarAtoi64A(const char *s) noexcept
{
	try
	{
		return _atoi64(s);
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

static char* WINAPI PointToNameA(char *Path) noexcept
{
	try
	{
		char* PathPtr = Path;
		char *NamePtr = PathPtr;

		while (*PathPtr)
		{
			if (IsSlashA(*PathPtr) || (*PathPtr == ':' && PathPtr == NamePtr + 1))
				NamePtr = PathPtr + 1;

			PathPtr++;
		}

		return NamePtr;
	}
	catch (...)
	{
		// TODO: log
		return Path;
	}
}

static void WINAPI UnquoteA(char *Str) noexcept
{
	try
	{
		char *Dst = Str;

		while (*Str)
		{
			if (*Str != '\"')
				*Dst++ = *Str;

			Str++;
		}

		*Dst = 0;
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

static char* WINAPI RemoveLeadingSpacesA(char *Str) noexcept
{
	try
	{
		char *ChPtr;

		if (!(ChPtr = Str))
			return nullptr;

		for (; IsSpaceA(*ChPtr); ChPtr++)
			;

		if (ChPtr != Str)
			memmove(Str, ChPtr, strlen(ChPtr) + 1);

		return Str;
	}
	catch (...)
	{
		// TODO: log
		return Str;
	}
}

static char* WINAPI RemoveExternalSpacesA(char *Str) noexcept
{
	try
	{
		return RemoveTrailingSpacesA(RemoveLeadingSpacesA(Str));
	}
	catch (...)
	{
		// TODO: log
		return Str;
	}
}

static char* WINAPI TruncStrA(char *Str, int MaxLength) noexcept
{
	try
	{
		int Length;

		if (MaxLength < 0)
			MaxLength = 0;

		if ((Length = (int)strlen(Str)) > MaxLength)
		{
			if (MaxLength > 3)
			{
				char *MovePos = Str + Length - MaxLength + 3;
				memmove(Str + 3, MovePos, strlen(MovePos) + 1);
				memcpy(Str, "...", 3);
			}

			Str[MaxLength] = 0;
		}
		return Str;
	}
	catch (...)
	{
		// TODO: log
		return Str;
	}

}

static char* WINAPI TruncPathStrA(char *Str, int MaxLength) noexcept
{
	try
	{
		int nLength = (int)strlen(Str);

		if (nLength > MaxLength)
		{
			char *lpStart = nullptr;

			if (*Str && (Str[1] == ':') && IsSlash(Str[2]))
				lpStart = Str + 3;
			else
			{
				if ((Str[0] == '\\') && (Str[1] == '\\'))
				{
					if ((lpStart = const_cast<char*>(FirstSlashA(Str + 2))))
						if ((lpStart = const_cast<char*>(FirstSlashA(lpStart + 1))))
							lpStart++;
				}
			}

			if (!lpStart || (lpStart - Str > MaxLength - 5))
				return TruncStrA(Str, MaxLength);

			char *lpInPos = lpStart + 3 + (nLength - MaxLength);
			memmove(lpStart + 3, lpInPos, strlen(lpInPos) + 1);
			memcpy(lpStart, "...", 3);
		}
		return Str;
	}
	catch (...)
	{
		// TODO: log
		return Str;
	}
}

static char* WINAPI QuoteSpaceOnlyA(char *Str) noexcept
{
	try
	{
		if (strchr(Str, ' '))
			InsertQuoteA(Str);
		return Str;
	}
	catch (...)
	{
		// TODO: log
		return Str;
	}
}

static BOOL WINAPI AddEndSlashA(char *Path) noexcept
{
	try
	{
		BOOL Result = FALSE;
		/* $ 06.12.2000 IS
		! Теперь функция работает с обоими видами слешей, также происходит
		изменение уже существующего конечного слеша на такой, который
		встречается чаще.
		*/
		char *end;
		int Slash = 0, BackSlash = 0;

		end = Path;

		while (*end)
		{
			Slash += (*end == '\\');
			BackSlash += (*end == '/');
			end++;
		}

		int Length = (int)(end - Path);
		char c = (Slash < BackSlash) ? '/' : '\\';
		Result = TRUE;

		if (!Length)
		{
			*end = c;
			end[1] = 0;
		}
		else
		{
			end--;

			if (!IsSlashA(*end))
			{
				end[1] = c;
				end[2] = 0;
			}
			else
				*end = c;
		}
		return Result;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}

}

static void WINAPI GetPathRootA(const char *Path, char *Root) noexcept
{
	try
	{
		wchar_t Buffer[MAX_PATH];
		NativeFSF.GetPathRoot(wide(Path).data(), Buffer, ARRAYSIZE(Buffer));
		UnicodeToOEM(Buffer, Root, ARRAYSIZE(Buffer));
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

static int WINAPI CopyToClipboardA(const char *Data) noexcept
{
	try
	{
		return NativeFSF.CopyToClipboard(FCT_STREAM, wide(Data).data());
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}

}

static char* WINAPI PasteFromClipboardA() noexcept
{
	try
	{
		size_t size = NativeFSF.PasteFromClipboard(FCT_ANY, nullptr, 0);
		if (size)
		{
			std::vector<wchar_t> p(size);
			NativeFSF.PasteFromClipboard(FCT_STREAM, p.data(), p.size());
			return UnicodeToAnsi(p.data());
		}
		return nullptr;
	}
	catch (...)
	{
		// TODO: log
		return nullptr;
	}
}

static void WINAPI DeleteBufferA(void* Buffer) noexcept
{
	try
	{
		delete[] static_cast<char*>(Buffer);
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

static int WINAPI ProcessNameA(const char *Param1, char *Param2, DWORD Flags) noexcept
{
	try
	{
		string strP1 = wide(Param1), strP2 = wide(Param2);
		int size = (int)(strP1.size() + strP2.size() + oldfar::NM) + 1; //а хрен ещё как угадать скока там этот Param2 для PN_GENERATENAME
		wchar_t_ptr p(size);
		wcscpy(p.get(), strP2.data());
		int newFlags = 0;

		if (Flags&oldfar::PN_SKIPPATH)
		{
			newFlags |= PN_SKIPPATH;
			Flags &= ~oldfar::PN_SKIPPATH;
		}

		if (Flags == oldfar::PN_CMPNAME)
		{
			newFlags |= PN_CMPNAME;
		}
		else if (Flags == oldfar::PN_CMPNAMELIST)
		{
			newFlags |= PN_CMPNAMELIST;
		}
		else if (Flags&oldfar::PN_GENERATENAME)
		{
			newFlags |= PN_GENERATENAME | (Flags & 0xFF);
		}

		int ret = static_cast<int>(NativeFSF.ProcessName(strP1.data(), p.get(), size, newFlags));

		if (newFlags&PN_GENERATENAME)
			UnicodeToOEM(p.get(), Param2, size);

		return ret;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI KeyNameToKeyA(const char *Name) noexcept
{
	try
	{
		return KeyToOldKey(KeyNameToKey(wide(Name)));
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

static BOOL WINAPI FarKeyToNameA(int Key, char *KeyText, int Size) noexcept
{
	try
	{
		string strKT;
		if (KeyToText(OldKeyToKey(Key), strKT))
		{
			UnicodeToOEM(strKT.data(), KeyText, Size > 0 ? Size + 1 : 32);
			return TRUE;
		}
		return FALSE;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI InputRecordToKeyA(const INPUT_RECORD *r) noexcept
{
	try
	{
		return KeyToOldKey(InputRecordToKey(r));
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

static char* WINAPI FarMkTempA(char *Dest, const char *Prefix) noexcept
{
	try
	{
		wchar_t D[oldfar::NM] = {};
		NativeFSF.MkTemp(D, ARRAYSIZE(D), wide(Prefix).data());
		UnicodeToOEM(D, Dest, sizeof(D));
		return Dest;
	}
	catch (...)
	{
		// TODO: log
		return Dest;
	}
}

static int WINAPI FarMkLinkA(const char *Src, const char *Dest, DWORD OldFlags) noexcept
{
	try
	{
		LINK_TYPE Type = LINK_HARDLINK;

		switch (OldFlags & 0xf)
		{
		case oldfar::FLINK_HARDLINK:    Type = LINK_HARDLINK; break;
		case oldfar::FLINK_JUNCTION:    Type = LINK_JUNCTION; break;
		case oldfar::FLINK_VOLMOUNT:    Type = LINK_VOLMOUNT; break;
		case oldfar::FLINK_SYMLINKFILE: Type = LINK_SYMLINKFILE; break;
		case oldfar::FLINK_SYMLINKDIR:  Type = LINK_SYMLINKDIR; break;
		}

		MKLINK_FLAGS Flags = MLF_NONE;
		if (OldFlags&oldfar::FLINK_SHOWERRMSG)
			Flags |= MLF_SHOWERRMSG;

		if (OldFlags&oldfar::FLINK_DONOTUPDATEPANEL)
			Flags |= MLF_DONOTUPDATEPANEL;

		return NativeFSF.MkLink(wide(Src).data(), wide(Dest).data(), Type, Flags);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI GetNumberOfLinksA(const char *Name) noexcept
{
	try
	{
		return static_cast<int>(NativeFSF.GetNumberOfLinks(wide(Name).data()));
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

static int WINAPI ConvertNameToRealA(const char *Src, char *Dest, int DestSize) noexcept
{
	try
	{
		string strDest;
		ConvertNameToReal(wide(Src), strDest);

		if (!Dest)
			return (int)strDest.size();
		else
			UnicodeToOEM(strDest.data(), Dest, DestSize);

		return std::min((int)strDest.size(), DestSize);
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

static int WINAPI FarGetReparsePointInfoA(const char *Src, char *Dest, int DestSize) noexcept
{
	try
	{
		int Result = 0;
		string strSrc = wide(Src);
		wchar_t Buffer[MAX_PATH];
		Result = static_cast<int>(NativeFSF.GetReparsePointInfo(strSrc.data(), Buffer, ARRAYSIZE(Buffer)));
		if (DestSize && Dest)
		{
			if (Result > MAX_PATH)
			{
				std::vector<wchar_t> Tmp(DestSize);
				NativeFSF.GetReparsePointInfo(strSrc.data(), Tmp.data(), Tmp.size());
				UnicodeToOEM(Tmp.data(), Dest, DestSize);
			}
			else
			{
				UnicodeToOEM(Buffer, Dest, DestSize);
			}
		}
		return Result;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI FarRecursiveSearchA_Callback(const PluginPanelItem *FData, const wchar_t *FullName, void *param) noexcept
{
	try
	{
		auto pCallbackParam = static_cast<FAR_SEARCH_A_CALLBACK_PARAM*>(param);
		WIN32_FIND_DATAA FindData = {};
		FindData.dwFileAttributes = FData->FileAttributes;
		FindData.ftCreationTime = FData->CreationTime;
		FindData.ftLastAccessTime = FData->LastAccessTime;
		FindData.ftLastWriteTime = FData->LastWriteTime;
		FindData.nFileSizeLow = (DWORD)FData->FileSize;
		FindData.nFileSizeHigh = (DWORD)(FData->FileSize >> 32);
		UnicodeToOEM(FData->FileName, FindData.cFileName, sizeof(FindData.cFileName));
		UnicodeToOEM(FData->AlternateFileName, FindData.cAlternateFileName, sizeof(FindData.cAlternateFileName));
		char FullNameA[oldfar::NM];
		UnicodeToOEM(FullName, FullNameA, sizeof(FullNameA));
		return pCallbackParam->Func(&FindData, FullNameA, pCallbackParam->Param);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static void WINAPI FarRecursiveSearchA(const char *InitDir, const char *Mask, oldfar::FRSUSERFUNC Func, DWORD Flags, void *Param) noexcept
{
	try
	{
		FAR_SEARCH_A_CALLBACK_PARAM CallbackParam;
		CallbackParam.Func = Func;
		CallbackParam.Param = Param;
		int newFlags = 0;

		if (Flags&oldfar::FRS_RETUPDIR)
			newFlags |= FRS_RETUPDIR;

		if (Flags&oldfar::FRS_RECUR)
			newFlags |= FRS_RECUR;

		if (Flags&oldfar::FRS_SCANSYMLINK)
			newFlags |= FRS_SCANSYMLINK;

		NativeFSF.FarRecursiveSearch(wide(InitDir).data(), wide(Mask).data(), FarRecursiveSearchA_Callback, newFlags, static_cast<void *>(&CallbackParam));
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

static DWORD WINAPI ExpandEnvironmentStrA(const char *src, char *dest, size_t size) noexcept
{
	try
	{
		string strD = api::env::expand_strings(wide(src));
		DWORD len = (DWORD)std::min(strD.size(), size - 1);
		UnicodeToOEM(strD.data(), dest, len + 1);
		return len;
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

static int WINAPI FarViewerA(const char *FileName, const char *Title, int X1, int Y1, int X2, int Y2, DWORD Flags) noexcept
{
	try
	{
		return NativeInfo.Viewer(wide(FileName).data(), wide(Title).data(), X1, Y1, X2, Y2, Flags, CP_DEFAULT);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI FarEditorA(const char *FileName, const char *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, int StartLine, int StartChar) noexcept
{
	try
	{
		return NativeInfo.Editor(wide(FileName).data(), wide(Title).data(), X1, Y1, X2, Y2, Flags, StartLine, StartChar, CP_DEFAULT);
	}
	catch (...)
	{
		// TODO: log
		return EEC_OPEN_ERROR;
	}
}

static int WINAPI FarCmpNameA(const char *pattern, const char *str, int skippath) noexcept
{
	try
	{
		return ProcessNameA(pattern, const_cast<char*>(str), oldfar::PN_CMPNAME | (skippath ? oldfar::PN_SKIPPATH : 0));
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static void WINAPI FarTextA(int X, int Y, int ConColor, const char *Str) noexcept
{
	try
	{
		FarColor Color = Colors::ConsoleColorToFarColor(ConColor);
		return NativeInfo.Text(X, Y, &Color, Str ? wide(Str).data() : nullptr);
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

static BOOL WINAPI FarShowHelpA(const char *ModuleName, const char *HelpTopic, DWORD Flags) noexcept
{
	try
	{
		return NativeInfo.ShowHelp(wide(ModuleName).data(), (HelpTopic ? wide(HelpTopic).data() : nullptr), Flags);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI FarInputBoxA(const char *Title, const char *Prompt, const char *HistoryName, const char *SrcText, char *DestText, int DestLength, const char *HelpTopic, DWORD Flags) noexcept
{
	try
	{
		DWORD NewFlags = 0;

		if (Flags&oldfar::FIB_ENABLEEMPTY)
			NewFlags |= FIB_ENABLEEMPTY;
		if (Flags&oldfar::FIB_PASSWORD)
			NewFlags |= FIB_PASSWORD;
		if (Flags&oldfar::FIB_EXPANDENV)
			NewFlags |= FIB_EXPANDENV;
		if (Flags&oldfar::FIB_NOUSELASTHISTORY)
			NewFlags |= FIB_NOUSELASTHISTORY;
		if (Flags&oldfar::FIB_BUTTONS)
			NewFlags |= FIB_BUTTONS;
		if (Flags&oldfar::FIB_NOAMPERSAND)
			NewFlags |= FIB_NOAMPERSAND;

		wchar_t_ptr Buffer(DestLength);

		int ret = NativeInfo.InputBox(&FarGuid, &FarGuid,
			Title ? wide(Title).data() : nullptr,
			Prompt ? wide(Prompt).data() : nullptr,
			HistoryName ? wide(HistoryName).data() : nullptr,
			SrcText ? wide(SrcText).data() : nullptr,
			Buffer.get(), Buffer.size(),
			HelpTopic ? wide(HelpTopic).data() : nullptr,
			NewFlags);

		if (ret && DestText)
			UnicodeToOEM(Buffer.get(), DestText, DestLength + 1);

		return ret;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI FarMessageFnA(intptr_t PluginNumber, DWORD Flags, const char *HelpTopic, const char * const *Items, int ItemsNumber, int ButtonsNumber) noexcept
{
	try
	{
		Flags &= ~oldfar::FMSG_DOWN;

		std::unique_ptr<wchar_t[]> AllInOneAnsiItem;
		std::vector<std::unique_ptr<wchar_t[]>> AnsiItems;

		if (Flags&oldfar::FMSG_ALLINONE)
		{
			AllInOneAnsiItem.reset(AnsiToUnicode((const char *)Items));
		}
		else
		{
			AnsiItems.reserve(ItemsNumber);

			for (int i = 0; i < ItemsNumber; i++)
				AnsiItems.emplace_back(AnsiToUnicode(Items[i]));
		}

		FARMESSAGEFLAGS NewFlags = FMSG_NONE;
		if (Flags&oldfar::FMSG_WARNING)
			NewFlags |= FMSG_WARNING;
		if (Flags&oldfar::FMSG_ERRORTYPE)
			NewFlags |= FMSG_ERRORTYPE;
		if (Flags&oldfar::FMSG_KEEPBACKGROUND)
			NewFlags |= FMSG_KEEPBACKGROUND;
		if (Flags&oldfar::FMSG_LEFTALIGN)
			NewFlags |= FMSG_LEFTALIGN;
		if (Flags&oldfar::FMSG_ALLINONE)
			NewFlags |= FMSG_ALLINONE;

		switch (Flags & 0x000f0000)
		{
		case oldfar::FMSG_MB_OK:
			NewFlags |= FMSG_MB_OK;
			break;
		case oldfar::FMSG_MB_OKCANCEL:
			NewFlags |= FMSG_MB_OKCANCEL;
			break;
		case oldfar::FMSG_MB_ABORTRETRYIGNORE:
			NewFlags |= FMSG_MB_ABORTRETRYIGNORE;
			break;
		case oldfar::FMSG_MB_YESNO:
			NewFlags |= FMSG_MB_YESNO;
			break;
		case oldfar::FMSG_MB_YESNOCANCEL:
			NewFlags |= FMSG_MB_YESNOCANCEL;
			break;
		case oldfar::FMSG_MB_RETRYCANCEL:
			NewFlags |= FMSG_MB_RETRYCANCEL;
			break;
		}

		return NativeInfo.Message(GetPluginGuid(PluginNumber), &FarGuid, NewFlags,
			HelpTopic ? wide(HelpTopic).data() : nullptr,
			AnsiItems.empty() ? reinterpret_cast<const wchar_t* const *>(AllInOneAnsiItem.get()) : reinterpret_cast<const wchar_t* const *>(AnsiItems.data()),
			ItemsNumber, ButtonsNumber);
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

static const char * WINAPI FarGetMsgFnA(intptr_t PluginHandle, int MsgId) noexcept
{
	try
	{
		//BUGBUG, надо проверять, что PluginHandle - плагин
		auto pPlugin = reinterpret_cast<PluginA*>(PluginHandle);
		string strPath = pPlugin->GetModuleName();
		CutToSlash(strPath);

		if (pPlugin->InitLang(strPath))
			return pPlugin->GetMsgA(static_cast<LNGID>(MsgId));

		return "";
	}
	catch (...)
	{
		// TODO: log
		return "";
	}

}

static int WINAPI FarMenuFnA(intptr_t PluginNumber, int X, int Y, int MaxHeight, DWORD Flags, const char *Title, const char *Bottom, const char *HelpTopic, const int *BreakKeys, int *BreakCode, const oldfar::FarMenuItem *Item, int ItemsNumber) noexcept
{
	try
	{
		DWORD NewFlags = 0;

		if (Flags&oldfar::FMENU_SHOWAMPERSAND)
			NewFlags |= FMENU_SHOWAMPERSAND;
		if (Flags&oldfar::FMENU_WRAPMODE)
			NewFlags |= FMENU_WRAPMODE;
		if (Flags&oldfar::FMENU_AUTOHIGHLIGHT)
			NewFlags |= FMENU_AUTOHIGHLIGHT;
		if (Flags&oldfar::FMENU_REVERSEAUTOHIGHLIGHT)
			NewFlags |= FMENU_REVERSEAUTOHIGHLIGHT;
		if (Flags&oldfar::FMENU_CHANGECONSOLETITLE)
			NewFlags |= FMENU_CHANGECONSOLETITLE;

		if (!Item) ItemsNumber = 0;

		std::vector<FarMenuItem> mi(ItemsNumber);

		if (Flags&oldfar::FMENU_USEEXT)
		{
			const oldfar::FarMenuItemEx *p = (const oldfar::FarMenuItemEx *)Item;

			for (int i = 0; i < ItemsNumber; i++)
			{
				mi[i].Flags = 0;
				if (p[i].Flags&oldfar::MIF_SELECTED)
					mi[i].Flags |= MIF_SELECTED;
				if (p[i].Flags&oldfar::MIF_CHECKED)
					mi[i].Flags |= MIF_CHECKED;
				if (p[i].Flags&oldfar::MIF_SEPARATOR)
					mi[i].Flags |= MIF_SEPARATOR;
				if (p[i].Flags&oldfar::MIF_DISABLE)
					mi[i].Flags |= MIF_DISABLE;
				if (p[i].Flags&oldfar::MIF_GRAYED)
					mi[i].Flags |= MIF_GRAYED;
				if (p[i].Flags&oldfar::MIF_HIDDEN)
					mi[i].Flags |= MIF_HIDDEN;
				mi[i].Text = AnsiToUnicode(p[i].Flags&oldfar::MIF_USETEXTPTR ? p[i].TextPtr : p[i].Text);
				INPUT_RECORD input = {};
				KeyToInputRecord(OldKeyToKey(p[i].AccelKey), &input);
				mi[i].AccelKey.VirtualKeyCode = input.Event.KeyEvent.dwControlKeyState;
				mi[i].AccelKey.ControlKeyState = input.Event.KeyEvent.dwControlKeyState;
				mi[i].Reserved[0] = p[i].Reserved;
				mi[i].Reserved[1] = 0;
				mi[i].UserData = p[i].UserData;
			}
		}
		else
		{
			for (int i = 0; i < ItemsNumber; i++)
			{
				mi[i].Flags = 0;

				if (Item[i].Selected)
					mi[i].Flags |= MIF_SELECTED;

				if (Item[i].Checked)
				{
					mi[i].Flags |= MIF_CHECKED;

					if (Item[i].Checked>1)
						AnsiToUnicodeBin((const char*)&Item[i].Checked, (wchar_t*)&mi[i].Flags, 1);
				}

				if (Item[i].Separator)
				{
					mi[i].Flags |= MIF_SEPARATOR;
					mi[i].Text = nullptr;
				}
				else
				{
					mi[i].Text = AnsiToUnicode(Item[i].Text);
				}

				mi[i].AccelKey.VirtualKeyCode = 0;
				mi[i].AccelKey.ControlKeyState = 0;
				mi[i].Reserved[0] = mi[i].Reserved[1] = 0;
				mi[i].UserData = 0;
			}
		}

		std::vector<FarKey> NewBreakKeys;
		if (BreakKeys)
		{
			int BreakKeysCount = 0;
			while (BreakKeys[BreakKeysCount++]);
			if (BreakKeysCount)
			{
				NewBreakKeys.resize(BreakKeysCount);
				std::transform(BreakKeys, BreakKeys + BreakKeysCount, NewBreakKeys.begin(), [&](int i)->VALUE_TYPE(NewBreakKeys)
				{
					VALUE_TYPE(NewBreakKeys) NewItem;
					NewItem.VirtualKeyCode = i & 0xffff;
					NewItem.ControlKeyState = 0;
					DWORD ItemFlags = i >> 16;
					if (ItemFlags & oldfar::PKF_CONTROL) NewItem.ControlKeyState |= LEFT_CTRL_PRESSED;
					if (ItemFlags & oldfar::PKF_ALT) NewItem.ControlKeyState |= LEFT_ALT_PRESSED;
					if (ItemFlags & oldfar::PKF_SHIFT) NewItem.ControlKeyState |= SHIFT_PRESSED;
					return NewItem;
				});
			}
		}

		intptr_t NewBreakCode;

		int ret = NativeInfo.Menu(GetPluginGuid(PluginNumber), &FarGuid, X, Y, MaxHeight, NewFlags,
			Title ? wide(Title).data() : nullptr,
			Bottom ? wide(Bottom).data() : nullptr,
			HelpTopic ? wide(HelpTopic).data() : nullptr,
			BreakKeys ? NewBreakKeys.data() : nullptr,
			&NewBreakCode, mi.data(), ItemsNumber);

		if (BreakCode)
			*BreakCode = NewBreakCode;

		std::for_each(CONST_RANGE(mi, i) { delete[] i.Text; });

		return ret;
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

static intptr_t WINAPI FarDefDlgProcA(HANDLE hDlg, int Msg, int Param1, void* Param2) noexcept
{
	try
	{
		FarDialogEvent& TopEvent = OriginalEvents().top();
		intptr_t Result = NativeInfo.DefDlgProc(TopEvent.hDlg, TopEvent.Msg, TopEvent.Param1, TopEvent.Param2);
		switch (Msg)
		{
		case DN_CTLCOLORDIALOG:
		case DN_CTLCOLORDLGITEM:
			Result = reinterpret_cast<intptr_t>(Param2);
			break;
		}
		return Result;
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

static intptr_t WINAPI CurrentDlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) noexcept
{
	try
	{
		auto Data = FindDialogData(hDlg);
		return (Data->DlgProc ? Data->DlgProc : FarDefDlgProcA)(hDlg, Msg, Param1, Param2);
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

static intptr_t WINAPI DlgProcA(HANDLE hDlg, intptr_t NewMsg, intptr_t Param1, void* Param2) noexcept
{
	try
	{
		FarDialogEvent e = {sizeof(FarDialogEvent), hDlg, NewMsg, Param1, Param2};

		OriginalEvents().push(e);
		SCOPE_EXIT{ OriginalEvents().pop(); };

		int Msg = oldfar::DM_FIRST;
		if(NewMsg>DM_USER)
		{
			Msg = NewMsg;
		}
		else switch (NewMsg)
		{
			case DN_CLOSE:           Msg=oldfar::DN_CLOSE; break;
			case DN_LISTHOTKEY:      Msg=oldfar::DN_LISTHOTKEY; break;
			case DN_BTNCLICK:        Msg=oldfar::DN_BTNCLICK; break;

			case DN_CTLCOLORDIALOG:
				{
					auto Color = static_cast<FarColor*>(Param2);
					*Color = Colors::ConsoleColorToFarColor(static_cast<int>(CurrentDlgProc(hDlg, oldfar::DN_CTLCOLORDIALOG, Param1,
					ToPtr(Colors::FarColorToConsoleColor(*Color)))));
				}
				break;

			case DN_DRAWDIALOG:
				Msg=oldfar::DN_DRAWDIALOG;
				break;

			case DN_CTLCOLORDLGITEM:
				{
					auto lc = static_cast<FarDialogItemColors*>(Param2);

					auto diA = CurrentDialogItemA(hDlg, Param1);

					// first, emulate DIF_SETCOLOR
					if(diA->Flags&oldfar::DIF_SETCOLOR)
					{
						lc->Colors[0] = Colors::ConsoleColorToFarColor(diA->Flags&oldfar::DIF_COLORMASK);
					}

					DWORD Result = static_cast<DWORD>(CurrentDlgProc(hDlg, oldfar::DN_CTLCOLORDLGITEM, Param1, ToPtr(MAKELONG(
						MAKEWORD(Colors::FarColorToConsoleColor(lc->Colors[0]), Colors::FarColorToConsoleColor(lc->Colors[1])),
						MAKEWORD(Colors::FarColorToConsoleColor(lc->Colors[2]), Colors::FarColorToConsoleColor(lc->Colors[3]))))));
					if(lc->ColorsCount > 0)
						lc->Colors[0] = Colors::ConsoleColorToFarColor(LOBYTE(LOWORD(Result)));
					if(lc->ColorsCount > 1)
						lc->Colors[1] = Colors::ConsoleColorToFarColor(HIBYTE(LOWORD(Result)));
					if(lc->ColorsCount > 2)
						lc->Colors[2] = Colors::ConsoleColorToFarColor(LOBYTE(HIWORD(Result)));
					if(lc->ColorsCount > 3)
						lc->Colors[3] = Colors::ConsoleColorToFarColor(HIBYTE(HIWORD(Result)));
				}
				break;

			case DN_CTLCOLORDLGLIST:
				{
					auto lc = static_cast<FarDialogItemColors*>(Param2);
					std::vector<BYTE> AnsiColors(lc->ColorsCount);
					std::transform(lc->Colors, lc->Colors + lc->ColorsCount, AnsiColors.begin(), [](const FarColor& i)
					{
						return static_cast<BYTE>(Colors::FarColorToConsoleColor(i));
					});
					oldfar::FarListColors lcA={0, 0, static_cast<int>(AnsiColors.size()), AnsiColors.data()};
					intptr_t Result = CurrentDlgProc(hDlg, oldfar::DN_CTLCOLORDLGLIST, Param1, &lcA);
					if(Result)
					{
						lc->ColorsCount = lcA.ColorCount;
						std::transform(lcA.Colors, lcA.Colors + lcA.ColorCount, lc->Colors, [](BYTE i)
						{
							return Colors::ConsoleColorToFarColor(i);
						});
					}
					return Result != 0;
				}
				break;

			case DN_DRAWDLGITEM:
			{
				Msg=oldfar::DN_DRAWDLGITEM;
				auto di = static_cast<FarDialogItem*>(Param2);
				auto FarDiA=UnicodeDialogItemToAnsi(*di,hDlg,Param1);
				intptr_t ret = CurrentDlgProc(hDlg, Msg, Param1, FarDiA);
				if (ret && (di->Type==DI_USERCONTROL) && (di->VBuf))
				{
					AnsiVBufToUnicode(FarDiA->VBuf, di->VBuf, GetAnsiVBufSize(*FarDiA),(FarDiA->Flags&oldfar::DIF_NOTCVTUSERCONTROL)==oldfar::DIF_NOTCVTUSERCONTROL);
				}

				return ret;
			}
			case DN_EDITCHANGE:
				return CurrentDlgProc(hDlg, oldfar::DN_EDITCHANGE, Param1, UnicodeDialogItemToAnsi(*static_cast<FarDialogItem*>(Param2), hDlg, Param1));

			case DN_ENTERIDLE: Msg=oldfar::DN_ENTERIDLE; break;
			case DN_GOTFOCUS:  Msg=oldfar::DN_GOTFOCUS; break;
			case DN_HELP:
			{
				char* HelpTopicA = UnicodeToAnsi((const wchar_t *)Param2);
				intptr_t ret = CurrentDlgProc(hDlg, oldfar::DN_HELP, Param1, HelpTopicA);
				if (ret && ret != reinterpret_cast<intptr_t>(Param2)) // changed
				{
					static wchar_t* HelpTopic = nullptr;
					delete[] HelpTopic;

					HelpTopic = AnsiToUnicode((const char *)ret);
					ret = (intptr_t)HelpTopic;
				}
				delete[] HelpTopicA;
				return ret;
			}
			case DN_HOTKEY:
				Msg=oldfar::DN_HOTKEY;
				Param2=ToPtr(KeyToOldKey((DWORD)InputRecordToKey((const INPUT_RECORD *)Param2)));
				break;
			case DN_INITDIALOG:
				Msg=oldfar::DN_INITDIALOG;
				break;
			case DN_KILLFOCUS:      Msg=oldfar::DN_KILLFOCUS; break;
			case DN_LISTCHANGE:     Msg=oldfar::DN_LISTCHANGE; break;
			case DN_DRAGGED:        Msg=oldfar::DN_DRAGGED; break;
			case DN_RESIZECONSOLE:  Msg=oldfar::DN_RESIZECONSOLE; break;
			case DN_DRAWDIALOGDONE: Msg=oldfar::DN_DRAWDIALOGDONE; break;
			case DN_DRAWDLGITEMDONE:Msg=oldfar::DN_DRAWDIALOGDONE; break;
			case DM_KILLSAVESCREEN: Msg=oldfar::DM_KILLSAVESCREEN; break;
			case DM_ALLKEYMODE:     Msg=oldfar::DM_ALLKEYMODE; break;
			case DN_INPUT:
				{
					INPUT_RECORD* record=(INPUT_RECORD *)Param2;
					if (record->EventType==MOUSE_EVENT)
					{
						Msg=oldfar::DN_MOUSEEVENT;
						Param1=0;
						Param2=&record->Event.MouseEvent;
						break;
					}
					else if (record->EventType==FOCUS_EVENT)
					{
						Msg=oldfar::DN_ACTIVATEAPP;
						Param1=record->Event.FocusEvent.bSetFocus?1:0;
						Param2=0;
						break;
					}
				}
				return NativeInfo.DefDlgProc(hDlg, NewMsg, Param1, Param2);
			case DN_CONTROLINPUT:
				{
					INPUT_RECORD* record=(INPUT_RECORD *)Param2;
					if (record->EventType==MOUSE_EVENT)
					{
						Msg=oldfar::DN_MOUSECLICK;
						Param2=&record->Event.MouseEvent;
						break;
					}
					else if (record->EventType==KEY_EVENT)
					{
						Msg=oldfar::DN_KEY;
						Param2=ToPtr(KeyToOldKey((DWORD)InputRecordToKey((const INPUT_RECORD *)Param2)));
						break;
					}
				}
				return NativeInfo.DefDlgProc(hDlg, NewMsg, Param1, Param2);
			default:
				break;
		}
		return CurrentDlgProc(hDlg, Msg, Param1, Param2);
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

static intptr_t WINAPI FarSendDlgMessageA(HANDLE hDlg, int OldMsg, int Param1, void* Param2) noexcept
{
	try
	{
		int Msg = DM_FIRST;
		if(OldMsg>oldfar::DM_USER)
		{
			Msg = OldMsg;
		}
		else switch (OldMsg)
		{
			case oldfar::DN_BTNCLICK:     Msg = DN_BTNCLICK; break;

			case oldfar::DM_CLOSE:        Msg = DM_CLOSE; break;
			case oldfar::DM_ENABLE:       Msg = DM_ENABLE; break;
			case oldfar::DM_ENABLEREDRAW: Msg = DM_ENABLEREDRAW; break;
			case oldfar::DM_GETDLGDATA:   Msg = DM_GETDLGDATA; break;
			case oldfar::DM_GETDLGITEM:
			{
				size_t item_size=NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, Param1, 0);

				if (item_size)
				{
					block_ptr<FarDialogItem> Buffer(item_size);
					FarGetDialogItem gdi = {sizeof(FarGetDialogItem), item_size, Buffer.get()};

					if (gdi.Item)
					{
						NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, Param1, &gdi);
						auto FarDiA=UnicodeDialogItemToAnsi(*gdi.Item,hDlg,Param1);
						*static_cast<oldfar::FarDialogItem*>(Param2) = *FarDiA;
						return TRUE;
					}
				}

				return FALSE;
			}
			case oldfar::DM_GETDLGRECT: Msg = DM_GETDLGRECT; break;
			case oldfar::DM_GETTEXT:
			{
				if (!Param2)
					return NativeInfo.SendDlgMessage(hDlg, DM_GETTEXT, Param1, 0);

				oldfar::FarDialogItemData* didA = (oldfar::FarDialogItemData*)Param2;
				if (!didA->PtrLength) //вот такой хреновый API!!!
					didA->PtrLength = static_cast<int>(NativeInfo.SendDlgMessage(hDlg, DM_GETTEXT, Param1, 0));
				std::vector<wchar_t> text(didA->PtrLength + 1);
				//BUGBUG: если didA->PtrLength=0, то вернётся с учётом '\0', в Энц написано, что без, хз как правильно.
				FarDialogItemData did = {sizeof(FarDialogItemData), (size_t)didA->PtrLength, text.data()};
				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_GETTEXT, Param1, &did);
				didA->PtrLength = (unsigned)did.PtrLength;
				UnicodeToOEM(text.data(), didA->PtrData, didA->PtrLength+1);
				return ret;
			}
			case oldfar::DM_GETTEXTLENGTH: Msg = DM_GETTEXT; break;

			case oldfar::DM_KEY:
			{
				if (!Param1 || !Param2) return FALSE;

				int Count = Param1;
				DWORD* KeysA = (DWORD*)Param2;
				std::vector<INPUT_RECORD> KeysW(Count);

				for (int i=0; i<Count; i++)
				{
					KeyToInputRecord(OldKeyToKey(KeysA[i]), &KeysW[i]);
				}
				return NativeInfo.SendDlgMessage(hDlg, DM_KEY, Param1, KeysW.data());
			}
			case oldfar::DM_MOVEDIALOG: Msg = DM_MOVEDIALOG; break;
			case oldfar::DM_SETDLGDATA: Msg = DM_SETDLGDATA; break;
			case oldfar::DM_SETDLGITEM:
			{
				if (!Param2)
					return FALSE;

				FarDialogItem& di=CurrentDialogItem(hDlg,Param1);

				if (di.Type==DI_LISTBOX || di.Type==DI_COMBOBOX)
					di.ListItems = &CurrentList(hDlg,Param1);

				FreeUnicodeDialogItem(di);
				oldfar::FarDialogItem *diA = (oldfar::FarDialogItem *)Param2;
				AnsiDialogItemToUnicode(*diA, di, *di.ListItems);

				// save color info
				if(diA->Flags&oldfar::DIF_SETCOLOR)
				{
					oldfar::FarDialogItem *diA_Copy=CurrentDialogItemA(hDlg,Param1);
					diA_Copy->Flags = diA->Flags;
				}

				return NativeInfo.SendDlgMessage(hDlg, DM_SETDLGITEM, Param1, &di);
			}
			case oldfar::DM_SETFOCUS: Msg = DM_SETFOCUS; break;
			case oldfar::DM_REDRAW:   Msg = DM_REDRAW; break;
			case oldfar::DM_SETTEXT:
			{
				if (!Param2)return 0;

				oldfar::FarDialogItemData* didA = (oldfar::FarDialogItemData*)Param2;

				if (!didA->PtrData) return 0;

				//BUGBUG - PtrLength ни на что не влияет.
				string text(wide(didA->PtrData));
				FarDialogItemData di = {sizeof(FarDialogItemData),(size_t)didA->PtrLength, UNSAFE_CSTR(text)};
				return NativeInfo.SendDlgMessage(hDlg, DM_SETTEXT, Param1, &di);
			}
			case oldfar::DM_SETMAXTEXTLENGTH: Msg = DM_SETMAXTEXTLENGTH; break;
			case oldfar::DM_SHOWDIALOG:       Msg = DM_SHOWDIALOG; break;
			case oldfar::DM_GETFOCUS:         Msg = DM_GETFOCUS; break;
			case oldfar::DM_GETCURSORPOS:     Msg = DM_GETCURSORPOS; break;
			case oldfar::DM_SETCURSORPOS:     Msg = DM_SETCURSORPOS; break;
			case oldfar::DM_GETTEXTPTR:
			{
				intptr_t length = NativeInfo.SendDlgMessage(hDlg, DM_GETTEXT, Param1, 0);

				if (!Param2) return length;

				std::vector<wchar_t> text(length + 1);
				FarDialogItemData item = {sizeof(FarDialogItemData), static_cast<size_t>(length), text.data()};
				length = NativeInfo.SendDlgMessage(hDlg, DM_GETTEXT, Param1, &item);
				UnicodeToOEM(text.data(), (char *)Param2, length+1);
				return length;
			}
			case oldfar::DM_SETTEXTPTR:
			{
				return Param2? NativeInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, Param1, UNSAFE_CSTR(wide(static_cast<const char*>(Param2)))) : FALSE;
			}
			case oldfar::DM_SHOWITEM: Msg = DM_SHOWITEM; break;
			case oldfar::DM_ADDHISTORY:
			{
				return Param2? NativeInfo.SendDlgMessage(hDlg, DM_ADDHISTORY, Param1, UNSAFE_CSTR(wide(static_cast<const char*>(Param2)))) : FALSE;
			}
			case oldfar::DM_GETCHECK:
			{
				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_GETCHECK, Param1, 0);
				intptr_t state = 0;

				if (ret == oldfar::BSTATE_UNCHECKED) state=BSTATE_UNCHECKED;
				else if (ret == oldfar::BSTATE_CHECKED)   state=BSTATE_CHECKED;
				else if (ret == oldfar::BSTATE_3STATE)    state=BSTATE_3STATE;

				return state;
			}
			case oldfar::DM_SETCHECK:
			{
				FARCHECKEDSTATE State = BSTATE_UNCHECKED;
				switch(static_cast<oldfar::FARCHECKEDSTATE>(reinterpret_cast<intptr_t>(Param2)))
				{
				case oldfar::BSTATE_UNCHECKED:
					State=BSTATE_UNCHECKED;
					break;
				case oldfar::BSTATE_CHECKED:
					State=BSTATE_CHECKED;
					break;
				case oldfar::BSTATE_3STATE:
					State=BSTATE_3STATE;
					break;
				case oldfar::BSTATE_TOGGLE:
					State=BSTATE_TOGGLE;
					break;
				default:
					break;
				}
				return NativeInfo.SendDlgMessage(hDlg, DM_SETCHECK, Param1, ToPtr(State));
			}
			case oldfar::DM_SET3STATE: Msg = DM_SET3STATE; break;
			case oldfar::DM_LISTSORT:  Msg = DM_LISTSORT; break;
			case oldfar::DM_LISTGETITEM: //BUGBUG, недоделано в фаре
			{
				if (!Param2) return FALSE;

				oldfar::FarListGetItem* lgiA = (oldfar::FarListGetItem*)Param2;
				FarListGetItem lgi = {sizeof(FarListGetItem),lgiA->ItemIndex};
				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTGETITEM, Param1, &lgi);
				UnicodeListItemToAnsi(&lgi.Item, &lgiA->Item);
				return ret;
			}
			case oldfar::DM_LISTGETCURPOS:

				if (Param2)
				{
					FarListPos lp={sizeof(FarListPos)};
					intptr_t ret=NativeInfo.SendDlgMessage(hDlg, DM_LISTGETCURPOS, Param1, &lp);
					oldfar::FarListPos *lpA = (oldfar::FarListPos *)Param2;
					lpA->SelectPos=lp.SelectPos;
					lpA->TopPos=lp.TopPos;
					return ret;
				}
				else return NativeInfo.SendDlgMessage(hDlg, DM_LISTGETCURPOS, Param1, 0);

			case oldfar::DM_LISTSETCURPOS:
			{
				if (!Param2) return FALSE;

				oldfar::FarListPos *lpA = (oldfar::FarListPos *)Param2;
				FarListPos lp = {sizeof(FarListPos),lpA->SelectPos,lpA->TopPos};
				return NativeInfo.SendDlgMessage(hDlg, DM_LISTSETCURPOS, Param1, &lp);
			}
			case oldfar::DM_LISTDELETE:
			{
				oldfar::FarListDelete *ldA = (oldfar::FarListDelete *)Param2;
				FarListDelete ld={sizeof(FarListDelete)};

				if (Param2)
				{
					ld.Count = ldA->Count;
					ld.StartIndex = ldA->StartIndex;
				}

				return NativeInfo.SendDlgMessage(hDlg, DM_LISTDELETE, Param1, Param2?&ld:0);
			}
			case oldfar::DM_LISTADD:
			{
				FarList newlist = {sizeof(FarList)};
				std::vector<FarListItem> Items;

				if (Param2)
				{
					oldfar::FarList *oldlist = (oldfar::FarList*) Param2;
					newlist.ItemsNumber = oldlist->ItemsNumber;

					if (newlist.ItemsNumber)
					{
						Items.resize(newlist.ItemsNumber);
						for (size_t i=0; i<newlist.ItemsNumber; i++)
							AnsiListItemToUnicode(&oldlist->Items[i], &Items[i]);

						newlist.Items = Items.data();
					}
				}

				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTADD, Param1, Param2?&newlist:0);

				std::for_each(RANGE(Items, i)
				{
					delete[] i.Text;
				});

				return ret;
			}
			case oldfar::DM_LISTADDSTR:
			{
				wchar_t* newstr = nullptr;

				if (Param2)
				{
					newstr = AnsiToUnicode((char*)Param2);
				}

				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTADDSTR, Param1, newstr);

				delete[] newstr;

				return ret;
			}
			case oldfar::DM_LISTUPDATE:
			{
				FarListUpdate newui = {sizeof(FarListUpdate)};

				if (Param2)
				{
					oldfar::FarListUpdate *oldui = (oldfar::FarListUpdate*) Param2;
					newui.Index=oldui->Index;
					AnsiListItemToUnicode(&oldui->Item, &newui.Item);
				}

				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTUPDATE, Param1, Param2?&newui:0);

				delete[] newui.Item.Text;

				return ret;
			}
			case oldfar::DM_LISTINSERT:
			{
				FarListInsert newli = {sizeof(FarListInsert)};

				if (Param2)
				{
					oldfar::FarListInsert *oldli = (oldfar::FarListInsert*) Param2;
					newli.Index=oldli->Index;
					AnsiListItemToUnicode(&oldli->Item, &newli.Item);
				}

				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTINSERT, Param1, Param2?&newli:0);

				delete[] newli.Item.Text;

				return ret;
			}
			case oldfar::DM_LISTFINDSTRING:
			{
				FarListFind newlf = {sizeof(FarListFind)};

				if (Param2)
				{
					oldfar::FarListFind *oldlf = (oldfar::FarListFind*) Param2;
					newlf.StartIndex=oldlf->StartIndex;
					newlf.Pattern = (oldlf->Pattern)?AnsiToUnicode(oldlf->Pattern):nullptr;

					if (oldlf->Flags&oldfar::LIFIND_EXACTMATCH) newlf.Flags|=LIFIND_EXACTMATCH;
				}

				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTFINDSTRING, Param1, Param2?&newlf:0);

				delete[] newlf.Pattern;

				return ret;
			}
			case oldfar::DM_LISTINFO:
			{
				if (!Param2) return FALSE;

				FarListInfo li={sizeof(FarListInfo)};
				intptr_t Result=NativeInfo.SendDlgMessage(hDlg, DM_LISTINFO, Param1, &li);
				if (Result)
				{
					oldfar::FarListInfo *liA = (oldfar::FarListInfo *)Param2;
					liA->Flags=0;
					if (li.Flags&LINFO_SHOWNOBOX) liA->Flags|=LINFO_SHOWNOBOX;
					if (li.Flags&LINFO_AUTOHIGHLIGHT) liA->Flags|=LINFO_AUTOHIGHLIGHT;
					if (li.Flags&LINFO_REVERSEHIGHLIGHT) liA->Flags|=LINFO_REVERSEHIGHLIGHT;
					if (li.Flags&LINFO_WRAPMODE) liA->Flags|=LINFO_WRAPMODE;
					if (li.Flags&LINFO_SHOWAMPERSAND) liA->Flags|=LINFO_SHOWAMPERSAND;
					liA->ItemsNumber=static_cast<int>(li.ItemsNumber);
					liA->SelectPos=li.SelectPos;
					liA->TopPos=li.TopPos;
					liA->MaxHeight=li.MaxHeight;
					liA->MaxLength=li.MaxLength;
				}
				return Result;
			}
			case oldfar::DM_LISTGETDATA:
			{
				intptr_t Size = NativeInfo.SendDlgMessage(hDlg, DM_LISTGETDATASIZE, Param1, Param2);
				intptr_t Data = NativeInfo.SendDlgMessage(hDlg, DM_LISTGETDATA, Param1, Param2);
				if(Size<=4) Data=Data?*reinterpret_cast<UINT*>(Data):0;
				return Data;
			}
			case oldfar::DM_LISTSETDATA:
			{
				FarListItemData newlid = {sizeof(FarListItemData)};

				if (Param2)
				{
					oldfar::FarListItemData *oldlid = (oldfar::FarListItemData*) Param2;
					newlid.Index=oldlid->Index;
					newlid.DataSize=oldlid->DataSize;
					newlid.Data=oldlid->Data;
					if(0==newlid.DataSize)
					{
						newlid.DataSize=(wcslen((wchar_t*)oldlid->Data)+1)*sizeof(wchar_t);
					}
					else if(newlid.DataSize<=4)
					{
						newlid.Data=&oldlid->Data;
					}
				}

				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTSETDATA, Param1, Param2?&newlid:0);
				return ret;
			}
			case oldfar::DM_LISTSETTITLES:
			{
				if (!Param2) return FALSE;

				oldfar::FarListTitles *ltA = (oldfar::FarListTitles *)Param2;
				FarListTitles lt = {sizeof(FarListTitles),0,ltA->Title?AnsiToUnicode(ltA->Title):nullptr,0,ltA->Bottom?AnsiToUnicode(ltA->Bottom):nullptr};
				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTSETTITLES, Param1, &lt);

				delete[] lt.Bottom;
				delete[] lt.Title;

				return ret;
			}
			case oldfar::DM_LISTGETTITLES:
			{
				if (Param2)
				{
					oldfar::FarListTitles *OldListTitle=(oldfar::FarListTitles *)Param2;
					FarListTitles ListTitle={sizeof(FarListTitles)};

					if (OldListTitle->Title)
					{
						ListTitle.TitleSize=OldListTitle->TitleLen+1;
						ListTitle.Title = new wchar_t[ListTitle.TitleSize];
					}

					if (OldListTitle->Bottom)
					{
						ListTitle.BottomSize=OldListTitle->BottomLen+1;
						ListTitle.Bottom = new wchar_t[ListTitle.BottomSize];
					}

					intptr_t Ret=NativeInfo.SendDlgMessage(hDlg, DM_LISTGETTITLES,Param1,&ListTitle);

					if (Ret)
					{
						UnicodeToOEM(ListTitle.Title,OldListTitle->Title,OldListTitle->TitleLen);
						UnicodeToOEM(ListTitle.Bottom,OldListTitle->Bottom,OldListTitle->BottomLen);
					}

					delete[] ListTitle.Title;
					delete[] ListTitle.Bottom;

					return Ret;
				}

				return FALSE;
			}
			case oldfar::DM_RESIZEDIALOG:      Msg = DM_RESIZEDIALOG; break;
			case oldfar::DM_SETITEMPOSITION:   Msg = DM_SETITEMPOSITION; break;
			case oldfar::DM_GETDROPDOWNOPENED: Msg = DM_GETDROPDOWNOPENED; break;
			case oldfar::DM_SETDROPDOWNOPENED: Msg = DM_SETDROPDOWNOPENED; break;
			case oldfar::DM_SETHISTORY:

				if (!Param2)
					return NativeInfo.SendDlgMessage(hDlg, DM_SETHISTORY, Param1, 0);
				else
				{
					FarDialogItem& di = CurrentDialogItem(hDlg,Param1);
					delete[] di.History;
					di.History = AnsiToUnicode((const char *)Param2);
					return NativeInfo.SendDlgMessage(hDlg, DM_SETHISTORY, Param1, const_cast<wchar_t*>(di.History));
				}

			case oldfar::DM_GETITEMPOSITION:     Msg = DM_GETITEMPOSITION; break;
			case oldfar::DM_SETMOUSEEVENTNOTIFY: Msg = DM_SETMOUSEEVENTNOTIFY; break;
			case oldfar::DM_EDITUNCHANGEDFLAG:   Msg = DM_EDITUNCHANGEDFLAG; break;
			case oldfar::DM_GETITEMDATA:         Msg = DM_GETITEMDATA; break;
			case oldfar::DM_SETITEMDATA:         Msg = DM_SETITEMDATA; break;
			case oldfar::DM_LISTSET:
			{
				FarList newlist = {sizeof(FarList)};

				if (Param2)
				{
					oldfar::FarList *oldlist = (oldfar::FarList*) Param2;
					newlist.ItemsNumber = oldlist->ItemsNumber;

					if (newlist.ItemsNumber)
					{
						newlist.Items = new FarListItem[newlist.ItemsNumber];
						for (size_t i=0; i<newlist.ItemsNumber; i++)
							AnsiListItemToUnicode(&oldlist->Items[i], &newlist.Items[i]);
					}
				}

				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTSET, Param1, Param2?&newlist:0);

				if (newlist.Items)
				{
					for (size_t i=0; i<newlist.ItemsNumber; i++)
						delete[] newlist.Items[i].Text;
					delete[] newlist.Items;
				}

				return ret;
			}
			case oldfar::DM_LISTSETMOUSEREACTION:
			{
				oldfar::FARLISTMOUSEREACTIONTYPE OldType = static_cast<oldfar::FARLISTMOUSEREACTIONTYPE>(reinterpret_cast<intptr_t>(Param2));
				FarDialogItem DlgItem = {};
				NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEMSHORT, Param1, &DlgItem);
				FARDIALOGITEMFLAGS OldFlags = DlgItem.Flags;
				DlgItem.Flags&=~(DIF_LISTTRACKMOUSE|DIF_LISTTRACKMOUSEINFOCUS);
				switch (OldType)
				{
				case oldfar::LMRT_ONLYFOCUS:
					DlgItem.Flags|=DIF_LISTTRACKMOUSEINFOCUS;
					break;
				case oldfar::LMRT_ALWAYS:
					DlgItem.Flags|=DIF_LISTTRACKMOUSE;
					break;
				case oldfar::LMRT_NEVER:
					break;
				default:
					DlgItem.Flags = OldFlags;
					break;
				}
				NativeInfo.SendDlgMessage(hDlg, DM_SETDLGITEMSHORT, Param1, &DlgItem);
				DWORD OldValue = oldfar::LMRT_NEVER;
				if (OldFlags&DIF_LISTTRACKMOUSE)
					OldValue = oldfar::LMRT_ALWAYS;
				else if (OldFlags&DIF_LISTTRACKMOUSEINFOCUS)
					OldValue = oldfar::LMRT_ONLYFOCUS;
				return OldValue;
			}
			case oldfar::DM_GETCURSORSIZE:   Msg = DM_GETCURSORSIZE; break;
			case oldfar::DM_SETCURSORSIZE:   Msg = DM_SETCURSORSIZE; break;
			case oldfar::DM_LISTGETDATASIZE: Msg = DM_LISTGETDATASIZE; break;
			case oldfar::DM_GETSELECTION:
			{
				if (!Param2) return FALSE;

				EditorSelect es={sizeof(EditorSelect)};
				intptr_t ret=NativeInfo.SendDlgMessage(hDlg, DM_GETSELECTION, Param1, &es);
				oldfar::EditorSelect *esA = (oldfar::EditorSelect *)Param2;
				esA->BlockType      = es.BlockType;
				esA->BlockStartLine = es.BlockStartLine;
				esA->BlockStartPos  = es.BlockStartPos;
				esA->BlockWidth     = es.BlockWidth;
				esA->BlockHeight    = es.BlockHeight;
				return ret;
			}
			case oldfar::DM_SETSELECTION:
			{
				if (!Param2) return FALSE;

				oldfar::EditorSelect *esA = (oldfar::EditorSelect *)Param2;
				EditorSelect es={sizeof(EditorSelect)};
				es.BlockType      = esA->BlockType;
				es.BlockStartLine = esA->BlockStartLine;
				es.BlockStartPos  = esA->BlockStartPos;
				es.BlockWidth     = esA->BlockWidth;
				es.BlockHeight    = esA->BlockHeight;
				return NativeInfo.SendDlgMessage(hDlg, DM_SETSELECTION, Param1, &es);
			}
			case oldfar::DM_GETEDITPOSITION:
				Msg=DM_GETEDITPOSITION;
				break;
			case oldfar::DM_SETEDITPOSITION:
				Msg=DM_SETEDITPOSITION;
				break;
			case oldfar::DM_SETCOMBOBOXEVENT:
				Msg=DM_SETCOMBOBOXEVENT;
				break;
			case oldfar::DM_GETCOMBOBOXEVENT:
				Msg=DM_GETCOMBOBOXEVENT;
				break;
			case oldfar::DM_KILLSAVESCREEN:
			case oldfar::DM_ALLKEYMODE:
			case oldfar::DN_ACTIVATEAPP:
				break;
			default:
				break;
		}
		return NativeInfo.SendDlgMessage(hDlg, Msg, Param1, Param2);
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

static int WINAPI FarDialogExA(intptr_t PluginNumber, int X1, int Y1, int X2, int Y2, const char *HelpTopic, oldfar::FarDialogItem *Item, int ItemsNumber, DWORD Reserved, DWORD Flags, oldfar::FARWINDOWPROC DlgProc, void* Param) noexcept
{
	try
	{
		std::vector<oldfar::FarDialogItem> diA(ItemsNumber);

		// to save DIF_SETCOLOR state
		for_each_2(ALL_RANGE(diA), Item, [](oldfar::FarDialogItem& diA_i, const oldfar::FarDialogItem& Item_i)
		{
			diA_i.Flags = Item_i.Flags;
		});

		std::vector<FarDialogItem> di(ItemsNumber);
		std::vector<FarList> l(ItemsNumber);

		for (int i=0; i<ItemsNumber; i++)
		{
			AnsiDialogItemToUnicode(Item[i],di[i],l[i]);
		}

		DWORD DlgFlags = 0;

		if (Flags&oldfar::FDLG_WARNING)
			DlgFlags|=FDLG_WARNING;

		if (Flags&oldfar::FDLG_SMALLDIALOG)
			DlgFlags|=FDLG_SMALLDIALOG;

		if (Flags&oldfar::FDLG_NODRAWSHADOW)
			DlgFlags|=FDLG_NODRAWSHADOW;

		if (Flags&oldfar::FDLG_NODRAWPANEL)
			DlgFlags|=FDLG_NODRAWPANEL;

		if (Flags&oldfar::FDLG_NONMODAL)
			DlgFlags|=FDLG_NONMODAL;

		int ret = -1;
		HANDLE hDlg = NativeInfo.DialogInit(GetPluginGuid(PluginNumber), &FarGuid, X1, Y1, X2, Y2, (HelpTopic? wide(HelpTopic).data() : nullptr), di.data(), ItemsNumber, 0, DlgFlags, DlgProcA, Param);
		DialogData NewDialogData;
		NewDialogData.DlgProc=DlgProc;
		NewDialogData.hDlg=hDlg;
		NewDialogData.diA=diA.data();
		NewDialogData.di=di.data();
		NewDialogData.l=l.data();

		DialogList().emplace_back(NewDialogData);

		if (hDlg != INVALID_HANDLE_VALUE)
		{
			ret = NativeInfo.DialogRun(hDlg);

			for (int i=0; i<ItemsNumber; i++)
			{
				size_t Size = NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, i, 0);
				block_ptr<FarDialogItem> Buffer(Size);
				FarGetDialogItem gdi = {sizeof(FarGetDialogItem), Size, Buffer.get()};

				if (gdi.Item)
				{
					NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, i, &gdi);
					UnicodeDialogItemToAnsiSafe(*gdi.Item,Item[i]);
					const wchar_t *res = NullToEmpty(gdi.Item->Data);

					if ((di[i].Type==DI_EDIT || di[i].Type==DI_COMBOBOX) && Item[i].Flags&oldfar::DIF_VAREDIT)
						UnicodeToOEM(res, Item[i].Ptr.PtrData, Item[i].Ptr.PtrLength+1);
					else
						UnicodeToOEM(res, Item[i].Data, sizeof(Item[i].Data));

					if (gdi.Item->Type==DI_USERCONTROL)
					{
						di[i].VBuf=gdi.Item->VBuf;
						Item[i].VBuf=GetAnsiVBufPtr(gdi.Item->VBuf,GetAnsiVBufSize(Item[i]));
					}

					if (gdi.Item->Type==DI_COMBOBOX || gdi.Item->Type==DI_LISTBOX)
					{
						Item[i].ListPos = static_cast<int>(NativeInfo.SendDlgMessage(hDlg, DM_LISTGETCURPOS,i,0));
					}
				}

				FreeAnsiDialogItem(diA[i]);
			}

			NativeInfo.DialogFree(hDlg);

			for (int i=0; i<ItemsNumber; i++)
			{
				if (di[i].Type==DI_LISTBOX || di[i].Type==DI_COMBOBOX)
					di[i].ListItems = &CurrentList(hDlg,i);

				FreeUnicodeDialogItem(di[i]);
			}
		}

		DialogList().pop_back();

		return ret;
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

static int WINAPI FarDialogFnA(intptr_t PluginNumber, int X1, int Y1, int X2, int Y2, const char *HelpTopic, oldfar::FarDialogItem *Item, int ItemsNumber) noexcept
{
	try
	{
		return FarDialogExA(PluginNumber, X1, Y1, X2, Y2, HelpTopic, Item, ItemsNumber, 0, 0, 0, 0);
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

static int WINAPI FarPanelControlA(HANDLE hPlugin, int Command, void *Param) noexcept
{
	try
	{
		static oldPanelInfoContainer PanelInfoA, AnotherPanelInfoA;

		if (!hPlugin || hPlugin==INVALID_HANDLE_VALUE)
			hPlugin=PANEL_ACTIVE;

		switch (Command)
		{
			case oldfar::FCTL_CHECKPANELSEXIST:
				return static_cast<int>(NativeInfo.PanelControl(hPlugin,FCTL_CHECKPANELSEXIST,0,Param));
			case oldfar::FCTL_CLOSEPLUGIN:
			{
				wchar_t *ParamW = nullptr;

				if (Param)
					ParamW = AnsiToUnicode((const char *)Param);

				int ret = static_cast<int>(NativeInfo.PanelControl(hPlugin,FCTL_CLOSEPANEL,0,ParamW));

				delete[] ParamW;

				return ret;
			}
			case oldfar::FCTL_GETANOTHERPANELINFO:
			case oldfar::FCTL_GETPANELINFO:
			{
				if (!Param)
					return FALSE;

				bool Passive=Command==oldfar::FCTL_GETANOTHERPANELINFO;

				static int Reenter=0;
				if (Reenter)
				{
					//Попытка борьбы с рекурсией (вызов GET*PANELINFO из GetOpenPanelInfo).
					//Так как у нас всё статик то должно сработать нормально в 99% случаев
					*(oldfar::PanelInfo*)Param = Passive? AnotherPanelInfoA.Info : PanelInfoA.Info;
					return TRUE;
				}

				Reenter++;

				if (Passive)
					hPlugin=PANEL_PASSIVE;

				oldfar::PanelInfo* OldPI = Passive? &AnotherPanelInfoA.Info : &PanelInfoA.Info;
				PanelInfo PI = {sizeof(PanelInfo)};
				int ret = static_cast<int>(NativeInfo.PanelControl(hPlugin,FCTL_GETPANELINFO,0,&PI));
				FreeAnsiPanelInfo(OldPI);

				if (ret)
				{
					ConvertUnicodePanelInfoToAnsi(&PI,OldPI);

					if (PI.ItemsNumber)
					{
						OldPI->PanelItems = new oldfar::PluginPanelItem[PI.ItemsNumber]();

							block_ptr<PluginPanelItem> PPI; size_t PPISize=0;

							for (int i=0; i<static_cast<int>(PI.ItemsNumber); i++)
							{
								size_t NewPPISize=static_cast<size_t>(NativeInfo.PanelControl(hPlugin,FCTL_GETPANELITEM,i,0));

								if (NewPPISize>PPISize)
								{
									PPI.reset(NewPPISize);
									PPISize=NewPPISize;
								}
								FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), PPISize, PPI.get()};
								NativeInfo.PanelControl(hPlugin,FCTL_GETPANELITEM, i, &gpi);
								if(PPI)
								{
									ConvertPanelItemToAnsi(*PPI,OldPI->PanelItems[i]);
								}
							}
					}

					if (PI.SelectedItemsNumber)
					{
						OldPI->SelectedItems = new oldfar::PluginPanelItem[PI.SelectedItemsNumber]();

							block_ptr<PluginPanelItem> PPI; size_t PPISize=0;

							for (int i=0; i<static_cast<int>(PI.SelectedItemsNumber); i++)
							{
								size_t NewPPISize=static_cast<size_t>(NativeInfo.PanelControl(hPlugin,FCTL_GETSELECTEDPANELITEM,i,0));

								if (NewPPISize>PPISize)
								{
									PPI.reset(NewPPISize);
									PPISize=NewPPISize;
								}
								FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), PPISize, PPI.get()};
								NativeInfo.PanelControl(hPlugin,FCTL_GETSELECTEDPANELITEM, i, &gpi);
								if(PPI)
								{
									ConvertPanelItemToAnsi(*PPI,OldPI->SelectedItems[i]);
								}
							}
					}

					size_t dirSize=NativeInfo.PanelControl(hPlugin,FCTL_GETPANELDIRECTORY,0,0);
					if(dirSize)
					{
						block_ptr<FarPanelDirectory> dirInfo(dirSize);
						dirInfo->StructSize=sizeof(FarPanelDirectory);
						NativeInfo.PanelControl(hPlugin, FCTL_GETPANELDIRECTORY, dirSize, dirInfo.get());
						UnicodeToOEM(dirInfo->Name,OldPI->CurDir,sizeof(OldPI->CurDir));
					}
					wchar_t ColumnTypes[sizeof(OldPI->ColumnTypes)];
					NativeInfo.PanelControl(hPlugin,FCTL_GETCOLUMNTYPES,sizeof(OldPI->ColumnTypes),ColumnTypes);
					UnicodeToOEM(ColumnTypes,OldPI->ColumnTypes,sizeof(OldPI->ColumnTypes));
					wchar_t ColumnWidths[sizeof(OldPI->ColumnWidths)];
					NativeInfo.PanelControl(hPlugin,FCTL_GETCOLUMNWIDTHS,sizeof(OldPI->ColumnWidths),ColumnWidths);
					UnicodeToOEM(ColumnWidths,OldPI->ColumnWidths,sizeof(OldPI->ColumnWidths));
					*(oldfar::PanelInfo*)Param=*OldPI;
				}
				else
				{
					ClearStruct(*(oldfar::PanelInfo*)Param);
				}

				Reenter--;
				return ret;
			}
			case oldfar::FCTL_GETANOTHERPANELSHORTINFO:
			case oldfar::FCTL_GETPANELSHORTINFO:
			{
				if (!Param)
					return FALSE;

				oldfar::PanelInfo *OldPI=(oldfar::PanelInfo*)Param;
				ClearStruct(*OldPI);

				if (Command==oldfar::FCTL_GETANOTHERPANELSHORTINFO)
					hPlugin=PANEL_PASSIVE;

				PanelInfo PI = {sizeof(PanelInfo)};
				int ret = static_cast<int>(NativeInfo.PanelControl(hPlugin,FCTL_GETPANELINFO,0,&PI));

				if (ret)
				{
					ConvertUnicodePanelInfoToAnsi(&PI,OldPI);
					size_t dirSize=NativeInfo.PanelControl(hPlugin,FCTL_GETPANELDIRECTORY,0,0);
					if(dirSize)
					{
						block_ptr<FarPanelDirectory> dirInfo(dirSize);
						dirInfo->StructSize=sizeof(FarPanelDirectory);
						NativeInfo.PanelControl(hPlugin, FCTL_GETPANELDIRECTORY, dirSize, dirInfo.get());
						UnicodeToOEM(dirInfo->Name, OldPI->CurDir, sizeof(OldPI->CurDir));
					}
					wchar_t ColumnTypes[sizeof(OldPI->ColumnTypes)];
					NativeInfo.PanelControl(hPlugin,FCTL_GETCOLUMNTYPES,sizeof(OldPI->ColumnTypes),ColumnTypes);
					UnicodeToOEM(ColumnTypes,OldPI->ColumnTypes,sizeof(OldPI->ColumnTypes));
					wchar_t ColumnWidths[sizeof(OldPI->ColumnWidths)];
					NativeInfo.PanelControl(hPlugin,FCTL_GETCOLUMNWIDTHS,sizeof(OldPI->ColumnWidths),ColumnWidths);
					UnicodeToOEM(ColumnWidths,OldPI->ColumnWidths,sizeof(OldPI->ColumnWidths));
				}

				return ret;
			}
			case oldfar::FCTL_SETANOTHERSELECTION:
				hPlugin=PANEL_PASSIVE;
			case oldfar::FCTL_SETSELECTION:
			{
				if (!Param)
					return FALSE;

				oldfar::PanelInfo *OldPI=(oldfar::PanelInfo*)Param;
				NativeInfo.PanelControl(hPlugin,FCTL_BEGINSELECTION,0,0);

				for (int i=0; i<OldPI->ItemsNumber; i++)
				{
					NativeInfo.PanelControl(hPlugin,FCTL_SETSELECTION,i,ToPtr(OldPI->PanelItems[i].Flags & oldfar::PPIF_SELECTED));
				}

				NativeInfo.PanelControl(hPlugin,FCTL_ENDSELECTION,0,0);
				return TRUE;
			}
			case oldfar::FCTL_REDRAWANOTHERPANEL:
				hPlugin = PANEL_PASSIVE;
			case oldfar::FCTL_REDRAWPANEL:
			{
				if (!Param)
					return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_REDRAWPANEL,0,0));

				oldfar::PanelRedrawInfo* priA = (oldfar::PanelRedrawInfo*)Param;
				PanelRedrawInfo pri = {sizeof(PanelRedrawInfo), (size_t)priA->CurrentItem,(size_t)priA->TopPanelItem};
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_REDRAWPANEL,0,&pri));
			}
			case oldfar::FCTL_SETANOTHERNUMERICSORT:
				hPlugin = PANEL_PASSIVE;
			case oldfar::FCTL_SETNUMERICSORT:
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETNUMERICSORT,(Param&&(*(int*)Param))?1:0,0));
			case oldfar::FCTL_SETANOTHERPANELDIR:
				hPlugin = PANEL_PASSIVE;
			case oldfar::FCTL_SETPANELDIR:
			{
				if (!Param)
					return FALSE;

				string Dir(wide(static_cast<const char*>(Param)));
				FarPanelDirectory dirInfo={sizeof(FarPanelDirectory),Dir.data(),nullptr,FarGuid,nullptr};
				int ret = static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETPANELDIRECTORY,0,&dirInfo));
				return ret;
			}
			case oldfar::FCTL_SETANOTHERSORTMODE:
				hPlugin = PANEL_PASSIVE;
			case oldfar::FCTL_SETSORTMODE:

				if (!Param)
					return FALSE;

				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETSORTMODE,*(int*)Param,0));
			case oldfar::FCTL_SETANOTHERSORTORDER:
				hPlugin = PANEL_PASSIVE;
			case oldfar::FCTL_SETSORTORDER:
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETSORTORDER,Param&&(*(int*)Param),0));
			case oldfar::FCTL_SETANOTHERVIEWMODE:
				hPlugin = PANEL_PASSIVE;
			case oldfar::FCTL_SETVIEWMODE:
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETVIEWMODE,Param?*(int *)Param:0,0));
			case oldfar::FCTL_UPDATEANOTHERPANEL:
				hPlugin = PANEL_PASSIVE;
			case oldfar::FCTL_UPDATEPANEL:
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_UPDATEPANEL,Param?1:0,0));
			case oldfar::FCTL_GETCMDLINE:
			case oldfar::FCTL_GETCMDLINESELECTEDTEXT:
			{
				if (Param)
				{
					const size_t Size = 1024;
					wchar_t _s[Size], *s=_s;
					NativeInfo.PanelControl(hPlugin, FCTL_GETCMDLINE, Size, s);
					if(Command==oldfar::FCTL_GETCMDLINESELECTEDTEXT)
					{
						CmdLineSelect cls={sizeof(CmdLineSelect)};
						NativeInfo.PanelControl(hPlugin,FCTL_GETCMDLINESELECTION, 0, &cls);
						if(cls.SelStart >=0 && cls.SelEnd > cls.SelStart)
						{
							s[cls.SelEnd] = 0;
							s += cls.SelStart;
						}
					}
					UnicodeToOEM(s, static_cast<char*>(Param), Size);
					return TRUE;
				}

				return FALSE;
			}
			case oldfar::FCTL_GETCMDLINEPOS:

				if (!Param)
					return FALSE;

				return static_cast<int>(NativeInfo.PanelControl(hPlugin,FCTL_GETCMDLINEPOS,0,Param));
			case oldfar::FCTL_GETCMDLINESELECTION:
			{
				if (!Param)
					return FALSE;

				CmdLineSelect cls={sizeof(CmdLineSelect)};
				int ret = static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_GETCMDLINESELECTION,0,&cls));

				if (ret)
				{
					oldfar::CmdLineSelect* clsA = (oldfar::CmdLineSelect*)Param;
					clsA->SelStart = cls.SelStart;
					clsA->SelEnd = cls.SelEnd;
				}

				return ret;
			}

			case oldfar::FCTL_INSERTCMDLINE:
				return Param ? static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_INSERTCMDLINE, 0, UNSAFE_CSTR(wide(static_cast<const char*>(Param))))) : FALSE;

			case oldfar::FCTL_SETCMDLINE:
				return Param ? static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETCMDLINE, 0, UNSAFE_CSTR(wide(static_cast<const char*>(Param))))) : FALSE;

			case oldfar::FCTL_SETCMDLINEPOS:
				return Param ? static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETCMDLINEPOS, *static_cast<int*>(Param), 0)) : FALSE;

			case oldfar::FCTL_SETCMDLINESELECTION:
			{
				if (!Param)
					return FALSE;

				oldfar::CmdLineSelect* clsA = (oldfar::CmdLineSelect*)Param;
				CmdLineSelect cls = {sizeof(CmdLineSelect),clsA->SelStart,clsA->SelEnd};
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETCMDLINESELECTION,0,&cls));
			}
			case oldfar::FCTL_GETUSERSCREEN:
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_GETUSERSCREEN,0,0));
			case oldfar::FCTL_SETUSERSCREEN:
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETUSERSCREEN,0,0));
		}
		return FALSE;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static HANDLE WINAPI FarSaveScreenA(int X1, int Y1, int X2, int Y2) noexcept
{
	try
	{
		return NativeInfo.SaveScreen(X1, Y1, X2, Y2);
	}
	catch (...)
	{
		// TODO: log
		return nullptr;
	}
}

static int WINAPI FarGetDirListA(const char *Dir, oldfar::PluginPanelItem **pPanelItem, int *pItemsNumber) noexcept
{
	try
	{
		*pPanelItem = nullptr;
		*pItemsNumber = 0;
		string strDir = wide(Dir);
		DeleteEndSlash(strDir);

		PluginPanelItem *pItems;
		size_t ItemsNumber;
		int ret = NativeInfo.GetDirList(strDir.data(), &pItems, &ItemsNumber);

		size_t PathOffset = ExtractFilePath(strDir).size() + 1;

		if (ret && ItemsNumber)
		{
			// + 1 чтоб хранить ItemsNumber ибо в FarFreeDirListA как то надо знать
			*pPanelItem = new oldfar::PluginPanelItem[ItemsNumber + 1]();
			*pItemsNumber = static_cast<int>(ItemsNumber);
			(*pPanelItem)[0].Reserved[0] = *pItemsNumber;
			++*pPanelItem;

			for (size_t i = 0; i < ItemsNumber; i++)
			{
				ConvertPanelItemToAnsi(pItems[i], (*pPanelItem)[i], PathOffset);
			}

			NativeInfo.FreeDirList(pItems, ItemsNumber);
		}

		return ret;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI FarGetPluginDirListA(intptr_t PluginNumber, HANDLE hPlugin, const char *Dir, oldfar::PluginPanelItem **pPanelItem, int *pItemsNumber) noexcept
{
	try
	{
		*pPanelItem = nullptr;
		*pItemsNumber = 0;

		PluginPanelItem *pPanelItemW;
		size_t ItemsNumber;
		int ret = NativeInfo.GetPluginDirList(GetPluginGuid(PluginNumber), hPlugin, wide(Dir).data(), &pPanelItemW, &ItemsNumber);

		if (ret && ItemsNumber)
		{
			// + 1 чтоб хранить ItemsNumber ибо в FarFreeDirListA как то надо знать
			*pPanelItem = new oldfar::PluginPanelItem[ItemsNumber + 1]();

			*pItemsNumber = static_cast<int>(ItemsNumber);
			(*pPanelItem)[0].Reserved[0] = *pItemsNumber;
			++*pPanelItem;

			for (size_t i = 0; i < ItemsNumber; i++)
			{
				ConvertPanelItemToAnsi(pPanelItemW[i], (*pPanelItem)[i]);
			}

			NativeInfo.FreePluginDirList(hPlugin, pPanelItemW, ItemsNumber);
		}

		return ret;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static void WINAPI FarFreeDirListA(const oldfar::PluginPanelItem *PanelItem) noexcept
{
	try
	{
		//Тут хранится ItemsNumber полученный в FarGetDirListA или FarGetPluginDirListA
		--PanelItem;
		size_t count = PanelItem->Reserved[0];
		FreePanelItemA(PanelItem, count, false);
		delete[] PanelItem;
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

static intptr_t WINAPI FarAdvControlA(intptr_t ModuleNumber, oldfar::ADVANCED_CONTROL_COMMANDS Command, void *Param) noexcept
{
	try
	{
		switch (Command)
		{
			case oldfar::ACTL_GETFARVERSION:
			{
				VersionInfo Info;
				NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETFARMANAGERVERSION, 0, &Info);
				DWORD FarVer = Info.Major<<8|Info.Minor|Info.Build<<16;

				if (Param)
					*(DWORD*)Param=FarVer;

				return FarVer;
			}
			case oldfar::ACTL_CONSOLEMODE:
				return IsConsoleFullscreen();

			case oldfar::ACTL_GETSYSWORDDIV:
			{
				intptr_t Length = 0;
				FarSettingsCreate settings={sizeof(FarSettingsCreate),FarGuid,INVALID_HANDLE_VALUE};
				HANDLE Settings=NativeInfo.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings)?settings.Handle:0;
				if(Settings)
				{
					FarSettingsItem item={sizeof(FarSettingsItem),FSSF_EDITOR,L"WordDiv",FST_UNKNOWN,{}};
					if(NativeInfo.SettingsControl(Settings,SCTL_GET,0,&item)&&FST_STRING==item.Type)
					{
						Length=std::min(oldfar::NM,StrLength(item.String)+1);
						if(Param) UnicodeToOEM(item.String,(char*)Param,oldfar::NM);
					}
					NativeInfo.SettingsControl(Settings,SCTL_FREE,0,0);
				}
				return Length;
			}
			case oldfar::ACTL_WAITKEY:
				{
					INPUT_RECORD input = {};
					KeyToInputRecord(OldKeyToKey(static_cast<int>(reinterpret_cast<intptr_t>(Param))),&input);
					return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_WAITKEY, 0, &input);
				}
				break;

			case oldfar::ACTL_GETCOLOR:
				{
					FarColor Color;
					int ColorIndex = static_cast<int>(reinterpret_cast<intptr_t>(Param));

					// there was a reserved position after COL_VIEWERARROWS in Far 1.x.
					if(ColorIndex > COL_VIEWERARROWS-COL_FIRSTPALETTECOLOR)
					{
						ColorIndex--;
					}
					return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETCOLOR, ColorIndex, &Color)? Colors::FarColorToConsoleColor(Color) :-1;
				}
				break;

			case oldfar::ACTL_GETARRAYCOLOR:
				{
					size_t PaletteSize = NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETARRAYCOLOR, 0, nullptr);
					if(Param)
					{
						std::vector<FarColor> Color(PaletteSize);
						NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETARRAYCOLOR, 0, Color.data());
						auto OldColors = static_cast<LPBYTE>(Param);
						std::transform(ALL_CONST_RANGE(Color), OldColors, [](const FarColor& i)
						{
							return static_cast<BYTE>(Colors::FarColorToConsoleColor(i));
						});
					}
					return PaletteSize;
				}
				break;
			case oldfar::ACTL_EJECTMEDIA:
			{

				ActlEjectMedia eject={sizeof(ActlEjectMedia)};
				if(Param)
				{
					oldfar::ActlEjectMedia* ejectA=(oldfar::ActlEjectMedia*)Param;
					eject.Letter=ejectA->Letter;
					eject.Flags=ejectA->Flags;
				}
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_EJECTMEDIA, 0, &eject);
			}
			case oldfar::ACTL_KEYMACRO:
			{
				if (!Param) return FALSE;

				oldfar::ActlKeyMacro *kmA=(oldfar::ActlKeyMacro *)Param;
				FAR_MACRO_CONTROL_COMMANDS MacroCommand = MCTL_LOADALL;
				int Param1=0;
				bool Process=true;

				MacroSendMacroText mtW = {};
				mtW.StructSize = sizeof(MacroSendMacroText);

				switch (kmA->Command)
				{
					case oldfar::MCMD_LOADALL:
						MacroCommand = MCTL_LOADALL;
						break;
					case oldfar::MCMD_SAVEALL:
						MacroCommand = MCTL_SAVEALL;
						break;
					case oldfar::MCMD_GETSTATE:
						MacroCommand = MCTL_GETSTATE;
						break;
					case oldfar::MCMD_POSTMACROSTRING:
						MacroCommand = MCTL_SENDSTRING;
						Param1=MSSC_POST;
						mtW.SequenceText=AnsiToUnicode(kmA->PlainText.SequenceText);

						if (!(kmA->PlainText.Flags&oldfar::KSFLAGS_DISABLEOUTPUT)) mtW.Flags|=KMFLAGS_ENABLEOUTPUT;

						if (kmA->PlainText.Flags&oldfar::KSFLAGS_NOSENDKEYSTOPLUGINS) mtW.Flags|=KMFLAGS_NOSENDKEYSTOPLUGINS;

						break;

					case oldfar::MCMD_CHECKMACRO:
						MacroCommand = MCTL_SENDSTRING;
						Param1=MSSC_CHECK;
						mtW.SequenceText=AnsiToUnicode(kmA->PlainText.SequenceText);
						break;

					default:
						Process=false;
						break;
				}

				intptr_t res=0;

				if (Process)
				{
					res = NativeInfo.MacroControl(0, MacroCommand, Param1, &mtW);

					if (MacroCommand == MCTL_SENDSTRING)
					{
						switch (Param1)
						{
							case MSSC_CHECK:
							{
								kmA->MacroResult.ErrMsg1 = "";
								kmA->MacroResult.ErrMsg2 = "";
								kmA->MacroResult.ErrMsg3 = "";
							}

							case MSSC_POST:
								delete[] mtW.SequenceText;
								break;
						}
					}
				}

				return res;
			}
			case oldfar::ACTL_POSTKEYSEQUENCE:
			{
				if (!Param)
					return FALSE;

				oldfar::KeySequence *ksA = (oldfar::KeySequence*)Param;

				if (!ksA->Count || !ksA->Sequence)
					return FALSE;

				FARKEYMACROFLAGS Flags=KMFLAGS_LUA;

				if (!(ksA->Flags&oldfar::KSFLAGS_DISABLEOUTPUT))
					Flags|=KMFLAGS_ENABLEOUTPUT;

				if (ksA->Flags&oldfar::KSFLAGS_NOSENDKEYSTOPLUGINS)
					Flags|=KMFLAGS_NOSENDKEYSTOPLUGINS;

				string strSequence=L"Keys(\"";
				string strKeyText;
				for (int i=0; i<ksA->Count; i++)
				{
					if (KeyToText(OldKeyToKey(ksA->Sequence[i]), strKeyText))
					{
						strSequence += L" " + strKeyText;
					}
				}
				strSequence += L"\")";

				intptr_t ret = Global->CtrlObject->Macro.PostNewMacro(strSequence.data(), Flags);

				return ret;
			}
			case oldfar::ACTL_GETSHORTWINDOWINFO:
			case oldfar::ACTL_GETWINDOWINFO:
			{
				if (!Param)
					return FALSE;

				oldfar::WindowInfo *wiA = (oldfar::WindowInfo *)Param;
				WindowInfo wi={sizeof(wi)};
				wi.Pos = wiA->Pos;
				intptr_t ret = NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETWINDOWINFO, 0, &wi);

				if (ret)
				{
					switch (wi.Type)
					{
						case WTYPE_PANELS: wiA->Type = oldfar::WTYPE_PANELS; break;
						case WTYPE_VIEWER: wiA->Type = oldfar::WTYPE_VIEWER; break;
						case WTYPE_EDITOR: wiA->Type = oldfar::WTYPE_EDITOR; break;
						case WTYPE_DIALOG: wiA->Type = oldfar::WTYPE_DIALOG; break;
						case WTYPE_VMENU:  wiA->Type = oldfar::WTYPE_VMENU;  break;
						case WTYPE_HELP:   wiA->Type = oldfar::WTYPE_HELP;   break;
						default: break;
					}

					wiA->Modified = (wi.Flags&WIF_MODIFIED) != 0;
					wiA->Current = (wi.Flags&WIF_CURRENT) != 0;

					if (Command==oldfar::ACTL_GETWINDOWINFO)
					{
						if (wi.TypeNameSize)
						{
							wi.TypeName=new wchar_t[wi.TypeNameSize];
						}

						if (wi.NameSize)
						{
							wi.Name=new wchar_t[wi.NameSize];
						}

						if (wi.TypeName && wi.Name)
						{
							NativeInfo.AdvControl(GetPluginGuid(ModuleNumber),ACTL_GETWINDOWINFO, 0, &wi);
							UnicodeToOEM(wi.TypeName,wiA->TypeName,sizeof(wiA->TypeName));
							UnicodeToOEM(wi.Name,wiA->Name,sizeof(wiA->Name));
						}

						delete[] wi.TypeName;
						delete[] wi.Name;
					}
					else
					{
						*wiA->TypeName=0;
						*wiA->Name=0;
					}
				}

				return ret;
			}
			case oldfar::ACTL_GETWINDOWCOUNT:
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETWINDOWCOUNT, 0, 0);
			case oldfar::ACTL_SETCURRENTWINDOW:
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_SETCURRENTWINDOW, reinterpret_cast<intptr_t>(Param), nullptr);
			case oldfar::ACTL_COMMIT:
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_COMMIT, 0, 0);
			case oldfar::ACTL_GETFARHWND:
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETFARHWND, 0, 0);
			case oldfar::ACTL_GETSYSTEMSETTINGS:
			{
				intptr_t ret = oldfar::FSS_CLEARROATTRIBUTE;

				if (GetSetting(FSSF_SYSTEM,L"DeleteToRecycleBin")) ret|=oldfar::FSS_DELETETORECYCLEBIN;
				if (GetSetting(FSSF_SYSTEM,L"CopyOpened")) ret|=oldfar::FSS_COPYFILESOPENEDFORWRITING;
				if (GetSetting(FSSF_SYSTEM,L"ScanJunction")) ret|=oldfar::FSS_SCANSYMLINK;

				return ret;
			}
			case oldfar::ACTL_GETPANELSETTINGS:
			{
				intptr_t ret = 0;

				if (GetSetting(FSSF_PANEL,L"ShowHidden")) ret|=oldfar::FPS_SHOWHIDDENANDSYSTEMFILES;
				if (GetSetting(FSSF_PANELLAYOUT,L"ColumnTitles")) ret|=oldfar::FPS_SHOWCOLUMNTITLES;
				if (GetSetting(FSSF_PANELLAYOUT,L"StatusLine")) ret|=oldfar::FPS_SHOWSTATUSLINE;
				if (GetSetting(FSSF_PANELLAYOUT,L"SortMode")) ret|=oldfar::FPS_SHOWSORTMODELETTER;

				return ret;
			}
			case oldfar::ACTL_GETINTERFACESETTINGS:
			{
				intptr_t ret = 0;
				if (GetSetting(FSSF_SCREEN,L"KeyBar")) ret|=oldfar::FIS_SHOWKEYBAR;
				if (GetSetting(FSSF_INTERFACE,L"ShowMenuBar")) ret|=oldfar::FIS_ALWAYSSHOWMENUBAR;
				return ret;
			}
			case oldfar::ACTL_GETCONFIRMATIONS:
			{
				intptr_t ret = 0;

				if (GetSetting(FSSF_CONFIRMATIONS,L"Copy")) ret|=oldfar::FCS_COPYOVERWRITE;
				if (GetSetting(FSSF_CONFIRMATIONS,L"Move")) ret|=oldfar::FCS_MOVEOVERWRITE;
				if (GetSetting(FSSF_CONFIRMATIONS,L"Drag")) ret|=oldfar::FCS_DRAGANDDROP;
				if (GetSetting(FSSF_CONFIRMATIONS,L"Delete")) ret|=oldfar::FCS_DELETE;
				if (GetSetting(FSSF_CONFIRMATIONS,L"DeleteFolder")) ret|=oldfar::FCS_DELETENONEMPTYFOLDERS;
				if (GetSetting(FSSF_CONFIRMATIONS,L"Esc")) ret|=oldfar::FCS_INTERRUPTOPERATION;
				if (GetSetting(FSSF_CONFIRMATIONS,L"RemoveConnection")) ret|=oldfar::FCS_DISCONNECTNETWORKDRIVE;
				if (GetSetting(FSSF_CONFIRMATIONS,L"HistoryClear")) ret|=oldfar::FCS_CLEARHISTORYLIST;
				if (GetSetting(FSSF_CONFIRMATIONS,L"Exit")) ret|=oldfar::FCS_EXIT;

				return ret;
			}
			case oldfar::ACTL_GETDESCSETTINGS:
			{
				intptr_t ret = 0;
				return ret;
			}
			case oldfar::ACTL_SETARRAYCOLOR:
			{
				if (!Param)
					return FALSE;

				auto scA = static_cast<const oldfar::FarSetColors*>(Param);
				std::vector<FarColor> Colors(scA->ColorCount);
				std::transform(scA->Colors, scA->Colors + scA->ColorCount, Colors.begin(), [](BYTE i)
				{
					return Colors::ConsoleColorToFarColor(i);
				});
				FarSetColors sc = {sizeof(FarSetColors), 0, (size_t)scA->StartIndex, Colors.size(), Colors.data()};
				if (scA->Flags&oldfar::FCLR_REDRAW)
					sc.Flags|=FSETCLR_REDRAW;
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_SETARRAYCOLOR, 0, &sc);
			}
			case oldfar::ACTL_GETWCHARMODE:
				return TRUE;
			case oldfar::ACTL_GETPLUGINMAXREADDATA:
				return GetSetting(FSSF_SYSTEM,L"PluginMaxReadData");
			case oldfar::ACTL_GETDIALOGSETTINGS:
			{
				intptr_t ret = 0;

				if (GetSetting(FSSF_DIALOG,L"EditBlock")) ret|=oldfar::FDIS_PERSISTENTBLOCKSINEDITCONTROLS;
				if (GetSetting(FSSF_DIALOG,L"EULBsClear")) ret|=oldfar::FDIS_BSDELETEUNCHANGEDTEXT;

				return ret;
			}
			case oldfar::ACTL_REMOVEMEDIA:
			case oldfar::ACTL_GETMEDIATYPE:
				return FALSE;
			case oldfar::ACTL_REDRAWALL:
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_REDRAWALL, 0, 0);
		}
		return FALSE;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI FarEditorControlA(oldfar::EDITOR_CONTROL_COMMANDS OldCommand, void* Param) noexcept
{
	try
	{
		static char *gt=nullptr;
		static char *geol=nullptr;
		static char *fn=nullptr;
		intptr_t et;
		EDITOR_CONTROL_COMMANDS Command=ECTL_GETSTRING;
		switch (OldCommand)
		{
			case oldfar::ECTL_ADDCOLOR:
				if(Param)
				{
					auto ecA = static_cast<oldfar::EditorColor*>(Param);
					EditorColor ec={sizeof(ec)};
					ec.StringNumber = ecA->StringNumber;
					ec.StartPos = ecA->StartPos;
					ec.EndPos = ecA->EndPos;
					ec.Color = Colors::ConsoleColorToFarColor(ecA->Color);
					if(ecA->Color&oldfar::ECF_TAB1) ec.Color.Flags|=ECF_TABMARKFIRST;
					ec.Priority=EDITOR_COLOR_NORMAL_PRIORITY;
					ec.Owner=FarGuid;
					EditorDeleteColor edc={sizeof(edc)};
					edc.Owner=FarGuid;
					edc.StringNumber = ecA->StringNumber;
					edc.StartPos = ecA->StartPos;
					return static_cast<int>(ecA->Color?NativeInfo.EditorControl(-1, ECTL_ADDCOLOR, 0, &ec):NativeInfo.EditorControl(-1, ECTL_DELCOLOR, 0, &edc));
				}
				return FALSE;
			case oldfar::ECTL_GETCOLOR:
				if(Param)
				{
					oldfar::EditorColor* ecA = static_cast<oldfar::EditorColor*>(Param);
					EditorColor ec={sizeof(ec)};
					ec.StringNumber = ecA->StringNumber;
					ec.ColorItem = ecA->ColorItem;
					int Result = static_cast<int>(NativeInfo.EditorControl(-1, ECTL_GETCOLOR, 0, &ec));
					if(Result)
					{
						ecA->StartPos = ec.StartPos;
						ecA->EndPos = ec.EndPos;
						ecA->Color = Colors::FarColorToConsoleColor(ec.Color);
						if(ec.Color.Flags&ECF_TABMARKFIRST) ecA->Color|=oldfar::ECF_TAB1;
					}
					return Result;
				}
				return FALSE;

			case oldfar::ECTL_GETSTRING:
			{
				EditorGetString egs={sizeof(EditorGetString)};
				oldfar::EditorGetString *oegs=(oldfar::EditorGetString *)Param;

				if (!oegs) return FALSE;

				egs.StringNumber=oegs->StringNumber;
				int ret=static_cast<int>(NativeInfo.EditorControl(-1,ECTL_GETSTRING,0,&egs));

				if (ret)
				{
					oegs->StringNumber=egs.StringNumber;
					oegs->StringLength=egs.StringLength;
					oegs->SelStart=egs.SelStart;
					oegs->SelEnd=egs.SelEnd;

					delete[] gt;
					delete[] geol;

					uintptr_t CodePage=GetEditorCodePageA();
					gt = UnicodeToAnsiBin(egs.StringText,egs.StringLength,CodePage);
					geol = UnicodeToAnsi(egs.StringEOL,CodePage);
					oegs->StringText=gt;
					oegs->StringEOL=geol;
					return TRUE;
				}

				return FALSE;
			}
			case oldfar::ECTL_INSERTTEXT:
			{
				return Param ? static_cast<int>(NativeInfo.EditorControl(-1, ECTL_INSERTTEXT, 0, UNSAFE_CSTR(wide(static_cast<const char*>(Param)).data()))) : FALSE;
			}
			case oldfar::ECTL_GETINFO:
			{
				EditorInfo ei={sizeof(EditorInfo)};
				oldfar::EditorInfo *oei=(oldfar::EditorInfo *)Param;

				if (!oei)
					return FALSE;

				int ret=static_cast<int>(NativeInfo.EditorControl(-1,ECTL_GETINFO,0,&ei));

				if (ret)
				{
					delete[] fn;
					ClearStruct(*oei);
					size_t FileNameSize=NativeInfo.EditorControl(-1,ECTL_GETFILENAME,0,0);

					if (FileNameSize)
					{
						wchar_t_ptr FileName(FileNameSize);
						NativeInfo.EditorControl(-1,ECTL_GETFILENAME,FileNameSize,FileName.get());
						fn = UnicodeToAnsi(FileName.get());
						oei->FileName=fn;
					}

					oei->EditorID=ei.EditorID;
					oei->WindowSizeX=ei.WindowSizeX;
					oei->WindowSizeY=ei.WindowSizeY;
					oei->TotalLines=ei.TotalLines;
					oei->CurLine=ei.CurLine;
					oei->CurPos=ei.CurPos;
					oei->CurTabPos=ei.CurTabPos;
					oei->TopScreenLine=ei.TopScreenLine;
					oei->LeftPos=ei.LeftPos;
					oei->Overtype=ei.Overtype;
					oei->BlockType=ei.BlockType;
					oei->BlockStartLine=ei.BlockStartLine;
					oei->AnsiMode=0;
					oei->TableNum=GetEditorCodePageFavA();
					oei->Options=ei.Options;
					oei->TabSize=ei.TabSize;
					oei->BookMarkCount=(int)ei.BookmarkCount;
					oei->CurState=ei.CurState;
					return TRUE;
				}

				return FALSE;
			}
			case oldfar::ECTL_EDITORTOOEM:
			case oldfar::ECTL_OEMTOEDITOR:
			{
				if (!Param)
					return FALSE;

				oldfar::EditorConvertText *ect=(oldfar::EditorConvertText*) Param;
				uintptr_t CodePage=GetEditorCodePageA();
				MultiByteRecode(OldCommand==oldfar::ECTL_OEMTOEDITOR ? CP_OEMCP : CodePage, OldCommand==oldfar::ECTL_OEMTOEDITOR ?  CodePage : CP_OEMCP, ect->Text, ect->TextLength);
				return TRUE;
			}
			case oldfar::ECTL_SAVEFILE:
			{
				EditorSaveFile newsf = {sizeof(EditorSaveFile)};
				string FileName, EOL;
				if (Param)
				{
					oldfar::EditorSaveFile *oldsf = (oldfar::EditorSaveFile*) Param;
					if (*oldsf->FileName)
					{
						FileName = wide(oldsf->FileName);
						newsf.FileName = FileName.data();
					}
					if (oldsf->FileEOL)
					{
						EOL = wide(oldsf->FileName);
						newsf.FileEOL = EOL.data();
					}
					newsf.CodePage = CP_DEFAULT;
				}

				return static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SAVEFILE, 0, Param?&newsf:0));
			}
			case oldfar::ECTL_PROCESSINPUT:	//BUGBUG?
			{
				if (Param)
				{
					INPUT_RECORD *pIR = (INPUT_RECORD*) Param;

					switch (pIR->EventType)
					{
						case KEY_EVENT:
						case FARMACRO_KEY_EVENT:
						{
							wchar_t res;
							if (MultiByteToWideChar(
								CP_OEMCP,
								0,
								&pIR->Event.KeyEvent.uChar.AsciiChar,
								1,
								&res,
								1
							))
								pIR->Event.KeyEvent.uChar.UnicodeChar=res;
						}
						default:
							break;
					}
				}

				return static_cast<int>(NativeInfo.EditorControl(-1,ECTL_PROCESSINPUT, 0, Param));
			}
			case oldfar::ECTL_PROCESSKEY:
			{
				INPUT_RECORD r={};
				KeyToInputRecord(OldKeyToKey(static_cast<int>(reinterpret_cast<intptr_t>(Param))),&r);
				return static_cast<int>(NativeInfo.EditorControl(-1,ECTL_PROCESSINPUT, 0, &r));
			}
			case oldfar::ECTL_READINPUT:	//BUGBUG?
			{
				int ret = static_cast<int>(NativeInfo.EditorControl(-1,ECTL_READINPUT, 0, Param));

				if (Param)
				{
					INPUT_RECORD *pIR = (INPUT_RECORD*) Param;

					switch (pIR->EventType)
					{
						case KEY_EVENT:
						case FARMACRO_KEY_EVENT:
						{
							char res;
							if (WideCharToMultiByte(
								CP_OEMCP,
								0,
								&pIR->Event.KeyEvent.uChar.UnicodeChar,
								1,
								&res,
								1,
								nullptr,
								nullptr
							))
								pIR->Event.KeyEvent.uChar.UnicodeChar=res;
						}
					}
				}

				return ret;
			}
			case oldfar::ECTL_SETKEYBAR:
			{
				switch ((intptr_t)Param)
				{
					case 0:
					case -1:
						return static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SETKEYBAR, 0, Param));
					default:
						oldfar::KeyBarTitles* oldkbt = (oldfar::KeyBarTitles*)Param;
						KeyBarTitles newkbt;
						FarSetKeyBarTitles newfskbt={sizeof(FarSetKeyBarTitles),&newkbt};
						ConvertKeyBarTitlesA(oldkbt, &newkbt);
						int ret = static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SETKEYBAR, 0, &newfskbt));
						FreeUnicodeKeyBarTitles(&newkbt);
						return ret;
				}
			}
			case oldfar::ECTL_SETPARAM:
			{
				EditorSetParameter newsp = {sizeof(EditorSetParameter)};

				if (Param)
				{
					oldfar::EditorSetParameter *oldsp= (oldfar::EditorSetParameter*) Param;
					newsp.iParam = oldsp->iParam;

					switch (oldsp->Type)
					{
						case oldfar::ESPT_AUTOINDENT:
							newsp.Type = ESPT_AUTOINDENT;
							break;
						case oldfar::ESPT_CHARCODEBASE:
							newsp.Type = ESPT_CHARCODEBASE;
							break;
						case oldfar::ESPT_CURSORBEYONDEOL:
							newsp.Type = ESPT_CURSORBEYONDEOL;
							break;
						case oldfar::ESPT_LOCKMODE:
							newsp.Type = ESPT_LOCKMODE;
							break;
						case oldfar::ESPT_SAVEFILEPOSITION:
							newsp.Type = ESPT_SAVEFILEPOSITION;
							break;
						case oldfar::ESPT_TABSIZE:
							newsp.Type = ESPT_TABSIZE;
							break;
						case oldfar::ESPT_CHARTABLE: //BUGBUG, недоделано в фаре
						{
							if (!oldsp->iParam) return FALSE;

							newsp.Type = ESPT_CODEPAGE;

							switch (oldsp->iParam)
							{
								case 1:
									newsp.iParam=GetOEMCP();
									break;
								case 2:
									newsp.iParam=GetACP();
									break;
								default:
									newsp.iParam=oldsp->iParam;

									if (newsp.iParam>0) newsp.iParam-=3;

									newsp.iParam=ConvertCharTableToCodePage(newsp.iParam);

									if (static_cast<uintptr_t>(newsp.iParam) == CP_DEFAULT)
										return FALSE;

									break;
							}

							break;
						}
						case oldfar::ESPT_EXPANDTABS:
						{
							newsp.Type = ESPT_EXPANDTABS;

							switch (oldsp->iParam)
							{
								case oldfar::EXPAND_NOTABS:		newsp.iParam = EXPAND_NOTABS; break;
								case oldfar::EXPAND_ALLTABS:	newsp.iParam = EXPAND_ALLTABS; break;
								case oldfar::EXPAND_NEWTABS: 	newsp.iParam = EXPAND_NEWTABS; break;
								default: return FALSE;
							}

							break;
						}
						case oldfar::ESPT_SETWORDDIV:
						{
							newsp.Type = ESPT_SETWORDDIV;
							string cParam;
							if (oldsp->cParam)
							{
								cParam = wide(oldsp->cParam);
								newsp.wszParam = UNSAFE_CSTR(cParam);
							}
							return static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SETPARAM, 0, &newsp));
						}
						case oldfar::ESPT_GETWORDDIV:
						{
							if (!oldsp->cParam) return FALSE;

							*oldsp->cParam=0;
							newsp.Type = ESPT_GETWORDDIV;
							newsp.Size = NativeInfo.EditorControl(-1,ECTL_SETPARAM, 0, &newsp);
							std::vector<wchar_t> Buffer(newsp.Size);
							newsp.wszParam = Buffer.data();
							int ret = static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SETPARAM, 0, &newsp));
							char *olddiv = UnicodeToAnsi(newsp.wszParam);

							if (olddiv)
							{
								xstrncpy(oldsp->cParam, olddiv, 0x100);
								delete[] olddiv;
							}

							return ret;
						}
						default:
							return FALSE;
					}
				}

				return static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SETPARAM, 0, Param?&newsp:0));
			}
			case oldfar::ECTL_SETSTRING:
			{
				EditorSetString newss = {sizeof(EditorSetString)};

				if (Param)
				{
					oldfar::EditorSetString *oldss = (oldfar::EditorSetString*) Param;
					newss.StringNumber=oldss->StringNumber;
					uintptr_t CodePage=GetEditorCodePageA();
					newss.StringText=(oldss->StringText)?AnsiToUnicodeBin(oldss->StringText, oldss->StringLength,CodePage):nullptr;
					newss.StringEOL=(oldss->StringEOL && *oldss->StringEOL)?AnsiToUnicode(oldss->StringEOL,CodePage):nullptr;
					newss.StringLength=oldss->StringLength;
				}

				int ret = static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SETSTRING, 0, Param?&newss:0));

				delete[] newss.StringText;
				delete[] newss.StringEOL;

				return ret;
			}
			case oldfar::ECTL_SETTITLE:
			{
				string newtit;

				if (Param)
				{
					newtit = wide(static_cast<const char*>(Param));
				}
				return static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SETTITLE, 0, Param? UNSAFE_CSTR(newtit) : nullptr));
			}
			// BUGBUG, convert params
			case oldfar::ECTL_DELETEBLOCK:	Command = ECTL_DELETEBLOCK; break;
			case oldfar::ECTL_DELETECHAR:		Command = ECTL_DELETECHAR; break;
			case oldfar::ECTL_DELETESTRING:	Command = ECTL_DELETESTRING; break;
			case oldfar::ECTL_EXPANDTABS:
			{
				Command = ECTL_EXPANDTABS;
				if (Param)
				{
					et = *((int *)Param);
					Param = &et;
				}
				break;
			}
			case oldfar::ECTL_GETBOOKMARKS:
			case oldfar::ECTL_GETSTACKBOOKMARKS:
			{
				bool bStack = OldCommand == oldfar::ECTL_GETSTACKBOOKMARKS;
				if (!Param)
				{
					if (!bStack) return FALSE;
					EditorInfo ei={sizeof(EditorInfo)};
					if (!NativeInfo.EditorControl(-1,ECTL_GETINFO,0,&ei)) return FALSE;
					return (int)ei.SessionBookmarkCount;
				}
				Command = bStack ? ECTL_GETSESSIONBOOKMARKS : ECTL_GETBOOKMARKS;
				intptr_t size = NativeInfo.EditorControl(-1,Command,0,nullptr);
				if (!size) return FALSE;
				block_ptr<EditorBookmarks> newbm(size);
				newbm->StructSize = sizeof(*newbm);
				newbm->Size = size;
				if (!NativeInfo.EditorControl(-1,Command,0,newbm.get()))
				{
					return FALSE;
				}
				oldfar::EditorBookMarks *oldbm = (oldfar::EditorBookMarks *)Param;
				for (size_t i=0; i<newbm->Count; i++)
				{
					if (oldbm->Line)
						oldbm->Line[i] = newbm->Line[i];
					if (oldbm->Cursor)
						oldbm->Cursor[i] = newbm->Cursor[i];
					if (oldbm->ScreenLine)
						oldbm->ScreenLine[i] = newbm->ScreenLine[i];
					if (oldbm->LeftPos)
						oldbm->LeftPos[i] = newbm->LeftPos[i];
				}
				int count = (int)newbm->Count;
				if (bStack) return count;
				else return TRUE;
			}
			case oldfar::ECTL_INSERTSTRING:	Command = ECTL_INSERTSTRING; break;
			case oldfar::ECTL_QUIT:					Command = ECTL_QUIT; break;

			case oldfar::ECTL_REALTOTAB:
			case oldfar::ECTL_TABTOREAL:
			{
				if(!Param)
					return FALSE;
				oldfar::EditorConvertPos *oldecp = (oldfar::EditorConvertPos*) Param;
				EditorConvertPos newecp={sizeof(EditorConvertPos),oldecp->StringNumber,oldecp->SrcPos,oldecp->DestPos};
				int ret=static_cast<int>(NativeInfo.EditorControl(-1, OldCommand == oldfar::ECTL_REALTOTAB ? ECTL_REALTOTAB : ECTL_TABTOREAL, 0, &newecp));
				oldecp->DestPos=newecp.DestPos;
				return ret;
			}
			case oldfar::ECTL_SELECT:
			{
				oldfar::EditorSelect *oldes = (oldfar::EditorSelect*) Param;
				EditorSelect newes={sizeof(EditorSelect),oldes->BlockType,oldes->BlockStartLine,oldes->BlockStartPos,oldes->BlockWidth,oldes->BlockHeight};
				return static_cast<int>(NativeInfo.EditorControl(-1, ECTL_SELECT, 0, &newes));
			}
			case oldfar::ECTL_REDRAW:				Command = ECTL_REDRAW; break;
			case oldfar::ECTL_SETPOSITION:
			{
				oldfar::EditorSetPosition *oldsp = (oldfar::EditorSetPosition*) Param;
				EditorSetPosition newsp={sizeof(EditorSetPosition),oldsp->CurLine,oldsp->CurPos,oldsp->CurTabPos,oldsp->TopScreenLine,oldsp->LeftPos,oldsp->Overtype};
				return static_cast<int>(NativeInfo.EditorControl(-1, ECTL_SETPOSITION, 0, &newsp));
			}
			case oldfar::ECTL_ADDSTACKBOOKMARK:			Command = ECTL_ADDSESSIONBOOKMARK; break;
			case oldfar::ECTL_PREVSTACKBOOKMARK:		Command = ECTL_PREVSESSIONBOOKMARK; break;
			case oldfar::ECTL_NEXTSTACKBOOKMARK:		Command = ECTL_NEXTSESSIONBOOKMARK; break;
			case oldfar::ECTL_CLEARSTACKBOOKMARKS:	Command = ECTL_CLEARSESSIONBOOKMARKS; break;
			case oldfar::ECTL_DELETESTACKBOOKMARK:	Command = ECTL_DELETESESSIONBOOKMARK; break;
			default:
				return FALSE;
		}
		return static_cast<int>(NativeInfo.EditorControl(-1, Command, 0, Param));
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI FarViewerControlA(int Command, void* Param) noexcept
{
	try
	{
		static char* filename=nullptr;

		switch (Command)
		{
			case oldfar::VCTL_GETINFO:
			{
				if (!Param) return FALSE;

				oldfar::ViewerInfo* viA = (oldfar::ViewerInfo*)Param;

				if (!viA->StructSize) return FALSE;

				ViewerInfo viW = {sizeof(viW)};

				if (NativeInfo.ViewerControl(-1,VCTL_GETINFO, 0, &viW) == FALSE) return FALSE;

				viA->ViewerID = viW.ViewerID;

				delete[] filename;

				size_t FileNameSize=NativeInfo.ViewerControl(-1,VCTL_GETFILENAME,0,0);

				if (FileNameSize)
				{
					wchar_t_ptr FileName(FileNameSize);
					NativeInfo.ViewerControl(-1,VCTL_GETFILENAME,FileNameSize,FileName.get());
					filename = UnicodeToAnsi(FileName.get());
					viA->FileName = filename;
				}
				viA->FileSize = viW.FileSize;
				viA->FilePos = viW.FilePos;
				viA->WindowSizeX = viW.WindowSizeX;
				viA->WindowSizeY = viW.WindowSizeY;
				viA->Options = 0;

				if (viW.Options&VOPT_SAVEFILEPOSITION) viA->Options |= oldfar::VOPT_SAVEFILEPOSITION;

				if (viW.Options&VOPT_AUTODETECTCODEPAGE)  viA->Options |= oldfar::VOPT_AUTODETECTTABLE;

				viA->TabSize = viW.TabSize;
				viA->CurMode.UseDecodeTable = 0;
				viA->CurMode.TableNum       = 0;
				viA->CurMode.AnsiMode       = viW.CurMode.CodePage == GetACP();
				viA->CurMode.Unicode        = IsUnicodeCodePage(viW.CurMode.CodePage);
				viA->CurMode.Wrap           = (viW.CurMode.Flags&VMF_WRAP)?1:0;
				viA->CurMode.WordWrap       = (viW.CurMode.Flags&VMF_WORDWRAP)?1:0;
				viA->CurMode.Hex            = viW.CurMode.ViewMode;
				viA->LeftPos = (int)viW.LeftPos;
				viA->Reserved3 = 0;
				break;
			}
			case oldfar::VCTL_QUIT:
				return static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_QUIT,0, 0));
			case oldfar::VCTL_REDRAW:
				return static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_REDRAW,0, 0));
			case oldfar::VCTL_SETKEYBAR:
			{
				switch ((intptr_t)Param)
				{
					case 0:
					case -1:
						return static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_SETKEYBAR,0, Param));
					default:
						oldfar::KeyBarTitles* kbtA = (oldfar::KeyBarTitles*)Param;
						KeyBarTitles kbt;
						FarSetKeyBarTitles newfskbt={sizeof(FarSetKeyBarTitles),&kbt};
						ConvertKeyBarTitlesA(kbtA, &kbt);
						int ret=static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_SETKEYBAR,0, &newfskbt));
						FreeUnicodeKeyBarTitles(&kbt);
						return ret;
				}
			}
			case oldfar::VCTL_SETPOSITION:
			{
				if (!Param) return FALSE;

				oldfar::ViewerSetPosition* vspA = (oldfar::ViewerSetPosition*)Param;
				ViewerSetPosition vsp={sizeof(ViewerSetPosition)};
				vsp.Flags = 0;

				if (vspA->Flags&oldfar::VSP_NOREDRAW)    vsp.Flags|=VSP_NOREDRAW;

				if (vspA->Flags&oldfar::VSP_PERCENT)     vsp.Flags|=VSP_PERCENT;

				if (vspA->Flags&oldfar::VSP_RELATIVE)    vsp.Flags|=VSP_RELATIVE;

				if (vspA->Flags&oldfar::VSP_NORETNEWPOS) vsp.Flags|=VSP_NORETNEWPOS;

				vsp.StartPos = vspA->StartPos;
				vsp.LeftPos = vspA->LeftPos;
				int ret = static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_SETPOSITION,0, &vsp));
				vspA->StartPos = vsp.StartPos;
				return ret;
			}
			case oldfar::VCTL_SELECT:
			{
				if (!Param) return static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_SELECT,0, 0));

				oldfar::ViewerSelect* vsA = (oldfar::ViewerSelect*)Param;
				ViewerSelect vs = {sizeof(ViewerSelect),vsA->BlockStartPos,vsA->BlockLen};
				return static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_SELECT,0, &vs));
			}
			case oldfar::VCTL_SETMODE:
			{
				if (!Param) return FALSE;

				oldfar::ViewerSetMode* vsmA = (oldfar::ViewerSetMode*)Param;
				ViewerSetMode vsm={sizeof(ViewerSetMode)};

				switch (vsmA->Type)
				{
					case oldfar::VSMT_HEX:      vsm.Type = VSMT_VIEWMODE; break;
					case oldfar::VSMT_WRAP:     vsm.Type = VSMT_WRAP;     break;
					case oldfar::VSMT_WORDWRAP: vsm.Type = VSMT_WORDWRAP; break;
				}

				vsm.iParam = vsmA->iParam;

				if (vsmA->Flags&oldfar::VSMFL_REDRAW) vsm.Flags|=VSMFL_REDRAW;

				return static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_SETMODE,0, &vsm));
			}
		}
		return TRUE;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static int WINAPI FarCharTableA(int Command, char *Buffer, int BufferSize) noexcept
{
	try
	{
		if (Command != oldfar::FCT_DETECT)
		{
			if (BufferSize != (int) sizeof(oldfar::CharTableSet))
				return -1;

			oldfar::CharTableSet *TableSet=(oldfar::CharTableSet*)Buffer;
			//Preset. Also if Command != FCT_DETECT and failed, buffer must be filled by OEM data.
			strcpy(TableSet->TableName,"<failed>");

			for (unsigned int i = 0; i < 256; ++i)
			{
				TableSet->EncodeTable[i] = TableSet->DecodeTable[i] = i;
				TableSet->UpperTable[i] = LocalUpper(i);
				TableSet->LowerTable[i] = LocalLower(i);
			}

			auto nCP = ConvertCharTableToCodePage(Command);

			if (nCP==CP_DEFAULT) return -1;

			CPINFOEX cpiex;

			if (!GetCPInfoEx(static_cast<UINT>(nCP), 0, &cpiex))
			{
				CPINFO cpi;

				if (!GetCPInfo(static_cast<UINT>(nCP), &cpi))
					return -1;

				cpiex.MaxCharSize = cpi.MaxCharSize;
				cpiex.CodePageName[0] = L'\0';
			}

			if (cpiex.MaxCharSize != 1)
				return -1;

			string CodepageName(cpiex.CodePageName);
			Codepages().FormatCodePageName(nCP, CodepageName);
			string sTableName = std::to_wstring(nCP);
			sTableName.resize(std::max(sTableName.size(), size_t(5)), L' ');
			sTableName.append(1, BoxSymbols[BS_V1]).append(1, L' ').append(CodepageName);
			UnicodeToOEM(sTableName.data(), TableSet->TableName, sizeof(TableSet->TableName) - 1);
			std::unique_ptr<wchar_t[]> us(AnsiToUnicodeBin((char*)TableSet->DecodeTable, sizeof(TableSet->DecodeTable), nCP));
			CharLowerBuff(us.get(), sizeof(TableSet->DecodeTable));
			WideCharToMultiByte(static_cast<UINT>(nCP), 0, us.get(), sizeof(TableSet->DecodeTable), (char*)TableSet->LowerTable, sizeof(TableSet->DecodeTable), nullptr, nullptr);
			CharUpperBuff(us.get(), sizeof(TableSet->DecodeTable));
			WideCharToMultiByte(static_cast<UINT>(nCP), 0, us.get(), sizeof(TableSet->DecodeTable), (char*)TableSet->UpperTable, sizeof(TableSet->DecodeTable), nullptr, nullptr);
			MultiByteRecode(static_cast<UINT>(nCP), CP_OEMCP, (char *)TableSet->DecodeTable, sizeof(TableSet->DecodeTable));
			MultiByteRecode(CP_OEMCP, static_cast<UINT>(nCP), (char *)TableSet->EncodeTable, sizeof(TableSet->EncodeTable));
			return Command;
		}
		return -1;
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

char* WINAPI XlatA(
	char *Line,                    // исходная строка
	int StartPos,                  // начало переконвертирования
	int EndPos,                    // конец переконвертирования
	const oldfar::CharTableSet *TableSet, // перекодировочная таблица (может быть nullptr)
	DWORD Flags)                   // флаги (см. enum XLATMODE)
{
	try
	{
		DWORD NewFlags = 0;

		if (Flags&oldfar::XLAT_SWITCHKEYBLAYOUT)
			NewFlags |= XLAT_SWITCHKEYBLAYOUT;

		if (Flags&oldfar::XLAT_SWITCHKEYBBEEP)
			NewFlags |= XLAT_SWITCHKEYBBEEP;

		if (Flags&oldfar::XLAT_USEKEYBLAYOUTNAME)
			NewFlags |= XLAT_USEKEYBLAYOUTNAME;

		if (Flags&oldfar::XLAT_CONVERTALLCMDLINE)
			NewFlags |= XLAT_CONVERTALLCMDLINE;

		string strLine = wide(Line);
		// XLat expects null-terminated string
		std::vector<wchar_t> Buffer(strLine.data(), strLine.data() + strLine.size() + 1);
		NativeFSF.XLat(Buffer.data(), StartPos, EndPos, NewFlags);
		UnicodeToOEM(Buffer.data(), Line, Buffer.size());
		return Line;
	}
	catch (...)
	{
		// TODO: log
		return Line;
	}
}

static int WINAPI GetFileOwnerA(const char *Computer, const char *Name, char *Owner) noexcept
{
	try
	{
		wchar_t wOwner[MAX_PATH];
		int Ret = static_cast<int>(NativeFSF.GetFileOwner(wide(Computer).data(), wide(Name).data(), wOwner, ARRAYSIZE(wOwner)));
		if (Ret)
		{
			UnicodeToOEM(wOwner, Owner, oldfar::NM);
		}
		return Ret;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

}

static void CheckScreenLock()
{
	if (Global->ScrBuf->GetLockCount() > 0 && !Global->CtrlObject->Macro.PeekKey())
	{
//		Global->ScrBuf->SetLockCount(0);
		Global->ScrBuf->Flush();
	}
}

class file_version
{
public:
	file_version(const string& file):file(file){}
	~file_version(){};

	bool Read()
	{
		bool Result = false;
		DWORD dummy = 0, dwlen = GetFileVersionInfoSize(file.data(), &dummy);
		if (dwlen)
		{
			buffer.reset(dwlen);
			if (GetFileVersionInfo(file.data(), dummy, dwlen, buffer.get()))
			{
				DWORD *Translation;
				UINT len;
				if (VerQueryValue(buffer.get(), L"\\VarFileInfo\\Translation", (void **)&Translation, &len) && len)
				{
					std::wostringstream tmp;
					tmp << std::hex << std::setw(4) << std::setfill(L'0') << LOWORD(*Translation)
					    << std::hex << std::setw(4) << std::setfill(L'0') << HIWORD(*Translation);
					path = L"\\StringFileInfo\\" + tmp.str() + L"\\";
					Result = true;
				}
			}
		}
		return Result;
	}

	const wchar_t* GetStringValue(const string& value) const
	{
		wchar_t* Value;
		UINT Length;
		if (VerQueryValue(buffer.get(), (path + value).data(), reinterpret_cast<void**>(&Value), &Length) && Length > 1)
			return Value;
		return nullptr;
	}

	const VS_FIXEDFILEINFO* GetFixedInfo() const
	{
		VS_FIXEDFILEINFO* Info;
		UINT Length;
		if (VerQueryValue(buffer.get(), L"\\", reinterpret_cast<void**>(&Info), &Length) && Length)
			return Info;
		return nullptr;
	}

private:
	string file;
	string path;
	wchar_t_ptr buffer;
};


static int SendKeyToPluginHook(const Manager::Key& key)
{
	DWORD KeyM = (key.FarKey&(~KEY_CTRLMASK));

	if (!((KeyM >= KEY_MACRO_BASE && KeyM <= KEY_MACRO_ENDBASE) || (KeyM >= KEY_OP_BASE && KeyM <= KEY_OP_ENDBASE))) // пропустим макро-коды
	{
		if (Global->WindowManager->IsPanelsActive())
		{
			if (Global->CtrlObject->Cp()->ActivePanel()->GetMode() == PLUGIN_PANEL)
			{
				auto ph = Global->CtrlObject->Cp()->ActivePanel()->GetPluginHandle();
				if (ph && ph->pPlugin->IsOemPlugin() && Global->CtrlObject->Cp()->ActivePanel()->SendKeyToPlugin(key.FarKey, true))
					return TRUE;
			}
		}
	}
	return FALSE;
}

static void RegisterSendKeyToPluginHook()
{
	static bool registered = false;
	if (!registered)
	{
		Global->WindowManager->AddGlobalKeyHandler(SendKeyToPluginHook);
		registered = true;
	}
}


PluginA::PluginA(OEMPluginModel* model, const string& lpwszModuleName) :
	Plugin(model,lpwszModuleName),
	PI(),
	OPI(),
	pFDPanelItemA(nullptr),
	pVFDPanelItemA(nullptr),
	OEMApiCnt(0),
	opif_shortcut(false)
{
	LocalUpperInit();
	RegisterSendKeyToPluginHook();
}

PluginA::~PluginA()
{
	FreePluginInfo();
	FreeOpenPanelInfo();
}

static oldfar::FarStandardFunctions StandardFunctions =
{
	sizeof(StandardFunctions),
	wrapper::pluginapi::FarAtoiA,
	wrapper::pluginapi::FarAtoi64A,
	wrapper::pluginapi::FarItoaA,
	wrapper::pluginapi::FarItoa64A,
	sprintf,
	sscanf,
	wrapper::pluginapi::qsort,
	wrapper::pluginapi::bsearch,
	wrapper::pluginapi::qsortex,
	_snprintf,
	{},
	wrapper::pluginapi::LocalIslower,
	wrapper::pluginapi::LocalIsupper,
	wrapper::pluginapi::LocalIsalpha,
	wrapper::pluginapi::LocalIsalphanum,
	wrapper::pluginapi::LocalUpper,
	wrapper::pluginapi::LocalLower,
	wrapper::pluginapi::LocalUpperBuf,
	wrapper::pluginapi::LocalLowerBuf,
	wrapper::pluginapi::LocalStrupr,
	wrapper::pluginapi::LocalStrlwr,
	wrapper::pluginapi::LStricmp,
	wrapper::pluginapi::LStrnicmp,
	wrapper::pluginapi::UnquoteA,
	wrapper::pluginapi::ExpandEnvironmentStrA,
	wrapper::pluginapi::RemoveLeadingSpacesA,
	wrapper::pluginapi::RemoveTrailingSpacesA,
	wrapper::pluginapi::RemoveExternalSpacesA,
	wrapper::pluginapi::TruncStrA,
	wrapper::pluginapi::TruncPathStrA,
	wrapper::pluginapi::QuoteSpaceOnlyA,
	wrapper::pluginapi::PointToNameA,
	wrapper::pluginapi::GetPathRootA,
	wrapper::pluginapi::AddEndSlashA,
	wrapper::pluginapi::CopyToClipboardA,
	wrapper::pluginapi::PasteFromClipboardA,
	wrapper::pluginapi::FarKeyToNameA,
	wrapper::pluginapi::KeyNameToKeyA,
	wrapper::pluginapi::InputRecordToKeyA,
	wrapper::pluginapi::XlatA,
	wrapper::pluginapi::GetFileOwnerA,
	wrapper::pluginapi::GetNumberOfLinksA,
	wrapper::pluginapi::FarRecursiveSearchA,
	wrapper::pluginapi::FarMkTempA,
	wrapper::pluginapi::DeleteBufferA,
	wrapper::pluginapi::ProcessNameA,
	wrapper::pluginapi::FarMkLinkA,
	wrapper::pluginapi::ConvertNameToRealA,
	wrapper::pluginapi::FarGetReparsePointInfoA,
};

static oldfar::PluginStartupInfo StartupInfo =
{
	sizeof(StartupInfo),
	"", // ModuleName, dynamic
	0, // ModuleNumber, dynamic
	nullptr, // RootKey, dynamic
	wrapper::pluginapi::FarMenuFnA,
	wrapper::pluginapi::FarDialogFnA,
	wrapper::pluginapi::FarMessageFnA,
	wrapper::pluginapi::FarGetMsgFnA,
	wrapper::pluginapi::FarPanelControlA,
	wrapper::pluginapi::FarSaveScreenA,
	nullptr, // copy from NativeInfo
	wrapper::pluginapi::FarGetDirListA,
	wrapper::pluginapi::FarGetPluginDirListA,
	wrapper::pluginapi::FarFreeDirListA,
	wrapper::pluginapi::FarViewerA,
	wrapper::pluginapi::FarEditorA,
	wrapper::pluginapi::FarCmpNameA,
	wrapper::pluginapi::FarCharTableA,
	wrapper::pluginapi::FarTextA,
	wrapper::pluginapi::FarEditorControlA,
	nullptr, // FSF, dynamic
	wrapper::pluginapi::FarShowHelpA,
	wrapper::pluginapi::FarAdvControlA,
	wrapper::pluginapi::FarInputBoxA,
	wrapper::pluginapi::FarDialogExA,
	wrapper::pluginapi::FarSendDlgMessageA,
	wrapper::pluginapi::FarDefDlgProcA,
	0,
	wrapper::pluginapi::FarViewerControlA,
};

static void CreatePluginStartupInfoA(PluginA *pPlugin, oldfar::PluginStartupInfo *PSI, oldfar::FarStandardFunctions *FSF)
{
	StartupInfo.RestoreScreen = NativeInfo.RestoreScreen;

	*PSI=StartupInfo;
	*FSF=StandardFunctions;
	PSI->ModuleNumber=(intptr_t)pPlugin;
	PSI->FSF=FSF;
	UnicodeToOEM(pPlugin->GetModuleName().data(), PSI->ModuleName, sizeof(PSI->ModuleName));
}


bool PluginA::GetGlobalInfo(GlobalInfo* Info)
{
	Info->StructSize = sizeof(GlobalInfo);
	Info->Description = L"Far 1.x plugin";
	Info->Author = L"unknown";

	const string& module = GetModuleName();
	Info->Title = PointToName(module);

	FileVersion = std::make_unique<file_version>(module);
	bool GuidFound = false;
	GUID PluginGuid = {};

	if (FileVersion->Read())
	{
		const wchar_t* Value;
		if ((Value = FileVersion->GetStringValue(L"InternalName")) || (Value = FileVersion->GetStringValue(L"OriginalName")))
		{
			Info->Title = Value;
		}

		if ((Value = FileVersion->GetStringValue(L"CompanyName")) || (Value = FileVersion->GetStringValue(L"LegalCopyright")) )
		{
			Info->Author = Value;
		}

		if ((Value = FileVersion->GetStringValue(L"FileDescription")))
		{
			Info->Description = Value;
		}

		if ((Value = FileVersion->GetStringValue(L"PluginGUID")))
		{
			if (UuidFromString(reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(Value)), &PluginGuid) == RPC_S_OK)
				GuidFound = true;
		}

		auto FileInfo = FileVersion->GetFixedInfo();
		if (FileInfo)
		{
			Info->Version.Major = HIWORD(FileInfo->dwFileVersionMS);
			Info->Version.Minor = LOWORD(FileInfo->dwFileVersionMS);
			Info->Version.Build = HIWORD(FileInfo->dwFileVersionLS);
			Info->Version.Revision = LOWORD(FileInfo->dwFileVersionLS);
		}
	}

	if (StrCmpI(Info->Title, L"FarFtp") == 0 || StrCmpI(Info->Title, L"MultiArc") == 0)
		opif_shortcut = true;

	if (GuidFound)
	{
		Info->Guid = PluginGuid;
	}
	else
	{
		int nb = std::min(StrLength(Info->Title), 8);
		while (nb > 0) {
			--nb;
			((char *)&Info->Guid)[8+nb] = (char)Info->Title[nb];
		}
	}

	return true;
}

bool PluginA::SetStartupInfo(PluginStartupInfo* Info)
{
	ExecuteStruct es = {iSetStartupInfo};
	if (Exports[es.id] && !Global->ProcessException)
	{
		oldfar::PluginStartupInfo _info;
		oldfar::FarStandardFunctions _fsf;
		CreatePluginStartupInfoA(this, &_info, &_fsf);
		// скорректируем адреса и плагино-зависимые поля

		_info.RootKey = static_cast<OEMPluginModel*>(m_model)->getUserName().data();

		if (Global->strRegUser.empty())
			api::env::delete_variable(L"FARUSER");
		else
			api::env::set_variable(L"FARUSER", Global->strRegUser);

		EXECUTE_FUNCTION(FUNCTION(iSetStartupInfo)(&_info));

		if (bPendingRemove)
		{
			return false;
		}
	}
	return true;
}

bool PluginA::CheckMinFarVersion()
{
	ExecuteStruct es = {iGetMinFarVersion};
	if (Exports[es.id] && !Global->ProcessException)
	{
		EXECUTE_FUNCTION(es = FUNCTION(iGetMinFarVersion)());
		if (bPendingRemove)
		{
			return false;
		}
	}
	return true;
}

static void* TranslateResult(void* hResult)
{
	if(INVALID_HANDLE_VALUE==hResult)
		return nullptr;
	if (hResult == reinterpret_cast<void*>(-2))
		return static_cast<void*>(PANEL_STOP);
	return hResult;
}

void* PluginA::Open(OpenInfo* Info)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);

	CheckScreenLock();

	ExecuteStruct es = {iOpen};

	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		std::unique_ptr<char[]> Buffer;
		intptr_t Ptr;

		int OpenFromA = oldfar::OPEN_PLUGINSMENU;

		oldfar::OpenDlgPluginData DlgData;

		switch (Info->OpenFrom)
		{
		case OPEN_COMMANDLINE:
			OpenFromA = oldfar::OPEN_COMMANDLINE;
			if (Info->Data)
			{
				Buffer.reset(UnicodeToAnsi(reinterpret_cast<OpenCommandLineInfo*>(Info->Data)->CommandLine));
				Ptr = reinterpret_cast<intptr_t>(Buffer.get());
			}
			break;

		case OPEN_SHORTCUT:
			OpenFromA = oldfar::OPEN_SHORTCUT;
			if (Info->Data)
			{
				auto shortcutdata = reinterpret_cast<OpenShortcutInfo*>(Info->Data)->ShortcutData;
				if (!shortcutdata)
					shortcutdata = reinterpret_cast<OpenShortcutInfo*>(Info->Data)->HostFile;

				Buffer.reset(UnicodeToAnsi(shortcutdata));
				Ptr = reinterpret_cast<intptr_t>(Buffer.get());
			}
			break;

		case OPEN_LEFTDISKMENU:
		case OPEN_RIGHTDISKMENU:
		case OPEN_PLUGINSMENU:
		case OPEN_FINDLIST:
		case OPEN_EDITOR:
		case OPEN_VIEWER:
			switch (Info->OpenFrom)
			{
			case OPEN_LEFTDISKMENU:
			case OPEN_RIGHTDISKMENU:
				OpenFromA = oldfar::OPEN_DISKMENU;
				break;
			case OPEN_PLUGINSMENU:
				OpenFromA = oldfar::OPEN_PLUGINSMENU;
				break;
			case OPEN_FINDLIST:
				OpenFromA = oldfar::OPEN_FINDLIST;
				break;
			case OPEN_EDITOR:
				OpenFromA = oldfar::OPEN_EDITOR;
				break;
			case OPEN_VIEWER:
				OpenFromA = oldfar::OPEN_VIEWER;
				break;
			default:
				break;
			}
			Ptr = Info->Guid->Data1;
			break;

		case OPEN_FROMMACRO:
			OpenFromA = oldfar::OPEN_FROMMACRO|Global->CtrlObject->Macro.GetMode();
			Buffer.reset(UnicodeToAnsi(reinterpret_cast<OpenMacroInfo*>(Info->Data)->Count ? reinterpret_cast<OpenMacroInfo*>(Info->Data)->Values[0].String : L""));
			Ptr = reinterpret_cast<intptr_t>(Buffer.get());
			break;

		case OPEN_DIALOG:
			OpenFromA = oldfar::OPEN_DIALOG;
			DlgData.ItemNumber=Info->Guid->Data1;
			DlgData.hDlg=reinterpret_cast<OpenDlgPluginData*>(Info->Data)->hDlg;
			Ptr = reinterpret_cast<intptr_t>(&DlgData);
			break;

		default:
			break;
		}

		EXECUTE_FUNCTION(es = FUNCTION(iOpen)(OpenFromA, Ptr));
	}

	return TranslateResult(es);
}

void* PluginA::OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, size_t DataSize, int OpMode)
{
	ExecuteStruct es = {iOpenFilePlugin};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		char *NameA = Name? UnicodeToAnsi(Name) : nullptr;
		EXECUTE_FUNCTION(es = FUNCTION(iOpenFilePlugin)(NameA, Data, static_cast<int>(DataSize)));
		delete[] NameA;
	}
	return TranslateResult(es);
}

int PluginA::SetFindList(SetFindListInfo* Info)
{
	ExecuteStruct es = {iSetFindList};
	if (Exports[es.id] && !Global->ProcessException)
	{
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(Info->PanelItem, PanelItemA, Info->ItemsNumber);
		EXECUTE_FUNCTION(es = FUNCTION(iSetFindList)(Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber)));
		FreePanelItemA(PanelItemA, Info->ItemsNumber);
	}
	return es;
}

int PluginA::ProcessEditorInput(ProcessEditorInputInfo* Info)
{
	ExecuteStruct es = {iProcessEditorInput};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		const INPUT_RECORD *Ptr = &Info->Rec;
		INPUT_RECORD OemRecord;
		if (Ptr->EventType==KEY_EVENT)
		{
			OemRecord = Info->Rec;
			CharToOemBuff(&Info->Rec.Event.KeyEvent.uChar.UnicodeChar,&OemRecord.Event.KeyEvent.uChar.AsciiChar,1);
			Ptr=&OemRecord;
		}
		EXECUTE_FUNCTION(es = FUNCTION(iProcessEditorInput)(Ptr));
	}
	return es;
}

int PluginA::ProcessEditorEvent(ProcessEditorEventInfo* Info)
{
	ExecuteStruct es = {iProcessEditorEvent};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		switch(Info->Event)
		{
			case EE_CLOSE:
			case EE_GOTFOCUS:
			case EE_KILLFOCUS:
				Info->Param = &Info->EditorID;
			case EE_READ:
			case EE_SAVE:
			case EE_REDRAW:
				EXECUTE_FUNCTION(es = FUNCTION(iProcessEditorEvent)(Info->Event, Info->Param));
				break;
		}
	}
	return es;
}

int PluginA::ProcessViewerEvent(ProcessViewerEventInfo* Info)
{
	ExecuteStruct es = {iProcessViewerEvent};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		switch(Info->Event)
		{
			case VE_CLOSE:
			case VE_GOTFOCUS:
			case VE_KILLFOCUS:
				Info->Param = &Info->ViewerID;
			case VE_READ:
				EXECUTE_FUNCTION(es = FUNCTION(iProcessViewerEvent)(Info->Event, Info->Param));
				break;
		}
	}
	return es;
}

int PluginA::ProcessDialogEvent(ProcessDialogEventInfo* Info)
{
	ExecuteStruct es = {iProcessDialogEvent};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		EXECUTE_FUNCTION(es = FUNCTION(iProcessDialogEvent)(Info->Event, Info->Param));
	}
	return es;
}

int PluginA::GetVirtualFindData(GetVirtualFindDataInfo* Info)
{
	ExecuteStruct es = {iGetVirtualFindData};
	if (Exports[es.id] && !Global->ProcessException)
	{
		pVFDPanelItemA = nullptr;
		size_t Size = wcslen(Info->Path) + 1;
		char_ptr PathA(Size);
		UnicodeToOEM(Info->Path, PathA.get(), Size);
		int ItemsNumber = 0;
		EXECUTE_FUNCTION(es = FUNCTION(iGetVirtualFindData)(Info->hPanel, &pVFDPanelItemA, &ItemsNumber, PathA.get()));
		Info->ItemsNumber = ItemsNumber;

		if (es && ItemsNumber)
		{
			Info->PanelItem = ConvertAnsiPanelItemsToUnicode(pVFDPanelItemA, ItemsNumber);
		}
	}

	return es;
}

void PluginA::FreeVirtualFindData(FreeFindDataInfo* Info)
{
	FreeUnicodePanelItem(Info->PanelItem, Info->ItemsNumber);

	ExecuteStruct es = {iFreeVirtualFindData};
	if (Exports[es.id] && !Global->ProcessException && pVFDPanelItemA)
	{
		EXECUTE_FUNCTION(FUNCTION(iFreeVirtualFindData)(Info->hPanel, pVFDPanelItemA, static_cast<int>(Info->ItemsNumber)));
		pVFDPanelItemA = nullptr;
	}
}

int PluginA::GetFiles(GetFilesInfo* Info)
{
	ExecuteStruct es = {iGetFiles, -1};
	if (Exports[es.id] && !Global->ProcessException)
	{
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(Info->PanelItem, PanelItemA, Info->ItemsNumber);
		char DestA[oldfar::NM];
		UnicodeToOEM(Info->DestPath, DestA, sizeof(DestA));
		EXECUTE_FUNCTION(es = FUNCTION(iGetFiles)(Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber), Info->Move, DestA, Info->OpMode));
		static wchar_t DestW[oldfar::NM];
		OEMToUnicode(DestA,DestW,ARRAYSIZE(DestW));
		Info->DestPath=DestW;
		FreePanelItemA(PanelItemA, Info->ItemsNumber);
	}
	return es;
}

int PluginA::PutFiles(PutFilesInfo* Info)
{
	ExecuteStruct es = {iPutFiles, -1};
	if (Exports[es.id] && !Global->ProcessException)
	{
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(Info->PanelItem, PanelItemA, Info->ItemsNumber);
		EXECUTE_FUNCTION(es = FUNCTION(iPutFiles)(Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber), Info->Move, Info->OpMode));
		FreePanelItemA(PanelItemA, Info->ItemsNumber);
	}
	return es;
}

int PluginA::DeleteFiles(DeleteFilesInfo* Info)
{
	ExecuteStruct es = {iDeleteFiles};
	if (Exports[es.id] && !Global->ProcessException)
	{
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(Info->PanelItem, PanelItemA, Info->ItemsNumber);
		EXECUTE_FUNCTION(es = FUNCTION(iDeleteFiles)(Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber), Info->OpMode));
		FreePanelItemA(PanelItemA, Info->ItemsNumber);
	}
	return es;
}

int PluginA::MakeDirectory(MakeDirectoryInfo* Info)
{
	ExecuteStruct es = {iMakeDirectory, -1};
	if (Exports[es.id] && !Global->ProcessException)
	{
		char NameA[oldfar::NM];
		UnicodeToOEM(Info->Name, NameA, sizeof(NameA));
		EXECUTE_FUNCTION(es = FUNCTION(iMakeDirectory)(Info->hPanel, NameA, Info->OpMode));
		static wchar_t NameW[oldfar::NM];
		OEMToUnicode(NameA,NameW,ARRAYSIZE(NameW));
		Info->Name=NameW;
	}
	return es;
}

int PluginA::ProcessHostFile(ProcessHostFileInfo* Info)
{
	ExecuteStruct es = {iProcessHostFile};
	if (Exports[es.id] && !Global->ProcessException)
	{
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(Info->PanelItem, PanelItemA, Info->ItemsNumber);
		EXECUTE_FUNCTION(es = FUNCTION(iProcessHostFile)(Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber), Info->OpMode));
		FreePanelItemA(PanelItemA, Info->ItemsNumber);
	}
	return es;
}

int PluginA::ProcessPanelEvent(ProcessPanelEventInfo* Info)
{
	ExecuteStruct es = {iProcessPanelEvent};
	if (Exports[es.id] && !Global->ProcessException)
	{
		PVOID ParamA = Info->Param;

		if (Info->Param && (Info->Event == FE_COMMAND || Info->Event == FE_CHANGEVIEWMODE))
			ParamA = UnicodeToAnsi((const wchar_t *)Info->Param);

		EXECUTE_FUNCTION(es = FUNCTION(iProcessPanelEvent)(Info->hPanel, Info->Event, ParamA));

		if (ParamA && (Info->Event == FE_COMMAND || Info->Event == FE_CHANGEVIEWMODE))
			delete[] static_cast<char*>(ParamA);
	}
	return es;
}

int PluginA::Compare(CompareInfo* Info)
{
	ExecuteStruct es = {iCompare, -2};
	if (Exports[es.id] && !Global->ProcessException)
	{
		oldfar::PluginPanelItem *Item1A = nullptr;
		oldfar::PluginPanelItem *Item2A = nullptr;
		ConvertPanelItemsArrayToAnsi(Info->Item1, Item1A, 1);
		ConvertPanelItemsArrayToAnsi(Info->Item2, Item2A, 1);
		EXECUTE_FUNCTION(es = FUNCTION(iCompare)(Info->hPanel, Item1A, Item2A, Info->Mode));
		FreePanelItemA(Item1A,1);
		FreePanelItemA(Item2A,1);
	}
	return es;
}

int PluginA::GetFindData(GetFindDataInfo* Info)
{
	ExecuteStruct es = {iGetFindData};
	if (Exports[es.id] && !Global->ProcessException)
	{
		pFDPanelItemA = nullptr;
		int ItemsNumber = 0;
		//BUGBUG, translate OpMode
		EXECUTE_FUNCTION(es = FUNCTION(iGetFindData)(Info->hPanel, &pFDPanelItemA, &ItemsNumber, Info->OpMode));

		Info->ItemsNumber = ItemsNumber;
		if (es && ItemsNumber)
		{
			Info->PanelItem = ConvertAnsiPanelItemsToUnicode(pFDPanelItemA, ItemsNumber);
		}
	}
	return es;
}

void PluginA::FreeFindData(FreeFindDataInfo* Info)
{
	FreeUnicodePanelItem(Info->PanelItem, Info->ItemsNumber);

	ExecuteStruct es = {iFreeFindData};
	if (Exports[es.id] && !Global->ProcessException && pFDPanelItemA)
	{
		EXECUTE_FUNCTION(FUNCTION(iFreeFindData)(Info->hPanel, pFDPanelItemA, static_cast<int>(Info->ItemsNumber)));
		pFDPanelItemA = nullptr;
	}
}

int PluginA::ProcessPanelInput(ProcessPanelInputInfo* Info)
{
	ExecuteStruct es = {iProcessPanelInput};
	if (Exports[es.id] && !Global->ProcessException)
	{
		int VirtKey;
		int dwControlState;

		bool Prepocess = (Info->Rec.EventType & 0x4000) != 0;
		Info->Rec.EventType &= ~0x4000;

		//BUGBUG: здесь можно проще.
		TranslateKeyToVK(InputRecordToKey(&Info->Rec),VirtKey,dwControlState);
		if (dwControlState&PKF_RALT)
			dwControlState = (dwControlState & (~PKF_RALT)) | PKF_ALT;
		if (dwControlState&PKF_RCONTROL)
			dwControlState = (dwControlState & (~PKF_RCONTROL)) | PKF_CONTROL;

		EXECUTE_FUNCTION(es = FUNCTION(iProcessPanelInput)(Info->hPanel, VirtKey|(Prepocess?PKF_PREPROCESS:0), dwControlState));
	}
	return es;
}

void PluginA::ClosePanel(ClosePanelInfo* Info)
{
	ExecuteStruct es = {iClosePanel};
	if (Exports[es.id] && !Global->ProcessException)
	{
		EXECUTE_FUNCTION(FUNCTION(iClosePanel)(Info->hPanel));
	}
	FreeOpenPanelInfo();
}

int PluginA::SetDirectory(SetDirectoryInfo* Info)
{
	ExecuteStruct es = {iSetDirectory};
	if (Exports[es.id] && !Global->ProcessException)
	{
		char *DirA = UnicodeToAnsi(Info->Dir);
		EXECUTE_FUNCTION(es = FUNCTION(iSetDirectory)(Info->hPanel, DirA, Info->OpMode));
		delete[] DirA;
	}
	return es;
}

void PluginA::FreeOpenPanelInfo()
{
	delete[] OPI.CurDir;
	delete[] OPI.HostFile;
	delete[] OPI.Format;
	delete[] OPI.PanelTitle;

	if (OPI.InfoLines && OPI.InfoLinesNumber)
	{
		FreeUnicodeInfoPanelLines(OPI.InfoLines,OPI.InfoLinesNumber);
	}

	if (OPI.DescrFiles)
	{
		FreeArrayUnicode(OPI.DescrFiles);
	}

	if (OPI.PanelModesArray)
	{
		FreeUnicodePanelModes(OPI.PanelModesArray, OPI.PanelModesNumber);
	}

	if (OPI.KeyBar)
	{
		FreeUnicodeKeyBarTitles(const_cast<KeyBarTitles*>(OPI.KeyBar));
		delete OPI.KeyBar;
	}

	delete[] OPI.ShortcutData;

	ClearStruct(OPI);
}

void PluginA::ConvertOpenPanelInfo(const oldfar::OpenPanelInfo &Src, OpenPanelInfo *Dest)
{
	FreeOpenPanelInfo();
	OPI.StructSize = sizeof(OPI);
	OPI.Flags = 0;
	if (!(Src.Flags&oldfar::OPIF_USEFILTER)) OPI.Flags|=OPIF_DISABLEFILTER;
	if (!(Src.Flags&oldfar::OPIF_USESORTGROUPS)) OPI.Flags|=OPIF_DISABLESORTGROUPS;
	if (!(Src.Flags&oldfar::OPIF_USEHIGHLIGHTING)) OPI.Flags|=OPIF_DISABLEHIGHLIGHTING;
	if (Src.Flags&oldfar::OPIF_ADDDOTS) OPI.Flags|=OPIF_ADDDOTS;
	if (Src.Flags&oldfar::OPIF_RAWSELECTION) OPI.Flags|=OPIF_RAWSELECTION;
	if (Src.Flags&oldfar::OPIF_REALNAMES) OPI.Flags|=OPIF_REALNAMES;
	if (Src.Flags&oldfar::OPIF_SHOWNAMESONLY) OPI.Flags|=OPIF_SHOWNAMESONLY;
	if (Src.Flags&oldfar::OPIF_SHOWRIGHTALIGNNAMES) OPI.Flags|=OPIF_SHOWRIGHTALIGNNAMES;
	if (Src.Flags&oldfar::OPIF_SHOWPRESERVECASE) OPI.Flags|=OPIF_SHOWPRESERVECASE;
	if (Src.Flags&oldfar::OPIF_COMPAREFATTIME) OPI.Flags|=OPIF_COMPAREFATTIME;
	if (Src.Flags&oldfar::OPIF_EXTERNALGET) OPI.Flags|=OPIF_EXTERNALGET;
	if (Src.Flags&oldfar::OPIF_EXTERNALPUT) OPI.Flags|=OPIF_EXTERNALPUT;
	if (Src.Flags&oldfar::OPIF_EXTERNALDELETE) OPI.Flags|=OPIF_EXTERNALDELETE;
	if (Src.Flags&oldfar::OPIF_EXTERNALMKDIR) OPI.Flags|=OPIF_EXTERNALMKDIR;
	if (Src.Flags&oldfar::OPIF_USEATTRHIGHLIGHTING) OPI.Flags|=OPIF_USEATTRHIGHLIGHTING;
	if (opif_shortcut) OPI.Flags|=OPIF_SHORTCUT;

	if (Src.CurDir)
		OPI.CurDir = AnsiToUnicode(Src.CurDir);

	if (Src.HostFile)
		OPI.HostFile = AnsiToUnicode(Src.HostFile);

	if (Src.Format)
		OPI.Format = AnsiToUnicode(Src.Format);

	if (Src.PanelTitle)
		OPI.PanelTitle = AnsiToUnicode(Src.PanelTitle);

	if (Src.InfoLines && Src.InfoLinesNumber)
	{
		ConvertInfoPanelLinesA(Src.InfoLines, const_cast<InfoPanelLine**>(&OPI.InfoLines), Src.InfoLinesNumber);
		OPI.InfoLinesNumber = Src.InfoLinesNumber;
	}

	if (Src.DescrFiles && Src.DescrFilesNumber)
	{
		OPI.DescrFiles = ArrayAnsiToUnicode(Src.DescrFiles, Src.DescrFilesNumber);
		OPI.DescrFilesNumber = Src.DescrFilesNumber;
	}

	if (Src.PanelModesArray && Src.PanelModesNumber)
	{
		ConvertPanelModesA(Src.PanelModesArray, const_cast<PanelMode**>(&OPI.PanelModesArray), Src.PanelModesNumber);
		OPI.PanelModesNumber	= Src.PanelModesNumber;
		OPI.StartPanelMode		= Src.StartPanelMode;

		switch(Src.StartSortMode)
		{
		case oldfar::SM_DEFAULT:
			OPI.StartSortMode = SM_DEFAULT;
			break;
		case oldfar::SM_UNSORTED:
			OPI.StartSortMode = SM_UNSORTED;
			break;
		case oldfar::SM_NAME:
			OPI.StartSortMode = SM_NAME;
			break;
		case oldfar::SM_EXT:
			OPI.StartSortMode = SM_EXT;
			break;
		case oldfar::SM_MTIME:
			OPI.StartSortMode = SM_MTIME;
			break;
		case oldfar::SM_CTIME:
			OPI.StartSortMode = SM_CTIME;
			break;
		case oldfar::SM_ATIME:
			OPI.StartSortMode = SM_ATIME;
			break;
		case oldfar::SM_SIZE:
			OPI.StartSortMode = SM_SIZE;
			break;
		case oldfar::SM_DESCR:
			OPI.StartSortMode = SM_DESCR;
			break;
		case oldfar::SM_OWNER:
			OPI.StartSortMode = SM_OWNER;
			break;
		case oldfar::SM_COMPRESSEDSIZE:
			OPI.StartSortMode = SM_COMPRESSEDSIZE;
			break;
		case oldfar::SM_NUMLINKS:
			OPI.StartSortMode = SM_NUMLINKS;
			break;
		}

		OPI.StartSortOrder		= Src.StartSortOrder;
	}

	if (Src.KeyBar)
	{
		OPI.KeyBar = new KeyBarTitles;
		ConvertKeyBarTitlesA(Src.KeyBar, const_cast<KeyBarTitles*>(OPI.KeyBar), Src.StructSize>=(int)sizeof(oldfar::OpenPanelInfo));
	}

	if (Src.ShortcutData)
		OPI.ShortcutData = AnsiToUnicode(Src.ShortcutData);

	*Dest=OPI;
}

void PluginA::GetOpenPanelInfo(OpenPanelInfo *pInfo)
{
	pInfo->StructSize = sizeof(OpenPanelInfo);

	ExecuteStruct es = {iGetOpenPanelInfo};
	if (Exports[es.id] && !Global->ProcessException)
	{
		oldfar::OpenPanelInfo InfoA={};
		EXECUTE_FUNCTION(FUNCTION(iGetOpenPanelInfo)(pInfo->hPanel, &InfoA));
		ConvertOpenPanelInfo(InfoA,pInfo);
	}
}

int PluginA::Configure(ConfigureInfo* Info)
{
	ExecuteStruct es = {iConfigure};
	if (Load() && Exports[es.id] && !Global->ProcessException)
	{
		EXECUTE_FUNCTION(es = FUNCTION(iConfigure)(Info->Guid->Data1));
	}
	return es;
}

void PluginA::FreePluginInfo()
{
	if (PI.DiskMenu.Count)
	{
		for (size_t i=0; i<PI.DiskMenu.Count; i++)
			delete[] PI.DiskMenu.Strings[i];

		delete[] PI.DiskMenu.Guids;
		delete[] PI.DiskMenu.Strings;
	}

	if (PI.PluginMenu.Count)
	{
		for (size_t i=0; i<PI.PluginMenu.Count; i++)
			delete[] PI.PluginMenu.Strings[i];

		delete[] PI.PluginMenu.Guids;
		delete[] PI.PluginMenu.Strings;
	}

	if (PI.PluginConfig.Count)
	{
		for (size_t i=0; i<PI.PluginConfig.Count; i++)
			delete[] PI.PluginConfig.Strings[i];

		delete[] PI.PluginConfig.Guids;
		delete[] PI.PluginConfig.Strings;
	}

	delete[] PI.CommandPrefix;

	ClearStruct(PI);
}

void PluginA::ConvertPluginInfo(const oldfar::PluginInfo &Src, PluginInfo *Dest)
{
	FreePluginInfo();
	PI.StructSize = sizeof(PI);
	PI.Flags=PF_NONE;
	if(Src.Flags&oldfar::PF_PRELOAD)
		PI.Flags|=PF_PRELOAD;
	if(Src.Flags&oldfar::PF_DISABLEPANELS)
		PI.Flags|=PF_DISABLEPANELS;
	if(Src.Flags&oldfar::PF_EDITOR)
		PI.Flags|=PF_EDITOR;
	if(Src.Flags&oldfar::PF_VIEWER)
		PI.Flags|=PF_VIEWER;
	if(Src.Flags&oldfar::PF_DIALOG)
		PI.Flags|=PF_DIALOG;
	if(Src.Flags&oldfar::PF_FULLCMDLINE)
		PI.Flags|=PF_FULLCMDLINE;

	if (Src.DiskMenuStringsNumber)
	{
		auto p = new wchar_t*[Src.DiskMenuStringsNumber];
		auto guid = new GUID[Src.DiskMenuStringsNumber]();

		for (int i=0; i<Src.DiskMenuStringsNumber; i++)
		{
			p[i] = AnsiToUnicode(Src.DiskMenuStrings[i]);
			guid[i].Data1=i;
		}

		PI.DiskMenu.Guids = guid;
		PI.DiskMenu.Strings = p;
		PI.DiskMenu.Count = Src.DiskMenuStringsNumber;
	}

	if (Src.PluginMenuStringsNumber)
	{
		auto p = new wchar_t*[Src.PluginMenuStringsNumber];
		auto guid = new GUID[Src.PluginMenuStringsNumber]();

		for (int i=0; i<Src.PluginMenuStringsNumber; i++)
		{
			p[i] = AnsiToUnicode(Src.PluginMenuStrings[i]);
			guid[i].Data1=i;
		}

		PI.PluginMenu.Guids = guid;
		PI.PluginMenu.Strings = p;
		PI.PluginMenu.Count = Src.PluginMenuStringsNumber;
	}

	if (Src.PluginConfigStringsNumber)
	{
		auto p = new wchar_t*[Src.PluginConfigStringsNumber];
		auto guid = new GUID[Src.PluginConfigStringsNumber]();

		for (int i=0; i<Src.PluginConfigStringsNumber; i++)
		{
			p[i] = AnsiToUnicode(Src.PluginConfigStrings[i]);
			guid[i].Data1=i;
		}

		PI.PluginConfig.Guids = guid;
		PI.PluginConfig.Strings = p;
		PI.PluginConfig.Count = Src.PluginConfigStringsNumber;
	}

	if (Src.CommandPrefix)
		PI.CommandPrefix = AnsiToUnicode(Src.CommandPrefix);

	*Dest=PI;
}

bool PluginA::GetPluginInfo(PluginInfo *pi)
{
	ClearStruct(*pi);

	ExecuteStruct es = {iGetPluginInfo};
	if (Exports[es.id] && !Global->ProcessException)
	{
		oldfar::PluginInfo InfoA={sizeof(InfoA)};
		EXECUTE_FUNCTION(FUNCTION(iGetPluginInfo)(&InfoA));

		if (!bPendingRemove)
		{
			ConvertPluginInfo(InfoA, pi);
			return true;
		}
	}

	return false;
}

void PluginA::ExitFAR(ExitInfo *Info)
{
	ExecuteStruct es = {iExitFAR};
	if (Exports[es.id] && !Global->ProcessException)
	{
		// ExitInfo ignored for ansi plugins
		EXECUTE_FUNCTION(FUNCTION(iExitFAR)());
	}
}

class AnsiLanguage: public Language
{
public:
	AnsiLanguage(const string& Path) { init(Path); }
	const char* GetMsgA(LNGID nID) const { return CheckMsgId(nID)? m_AnsiMessages[nID].data() : ""; }

private:
	virtual size_t size() const override { return m_AnsiMessages.size(); }
	virtual void reserve(size_t size) override { m_AnsiMessages.reserve(size); }
	virtual void add(string&& str) override { m_AnsiMessages.emplace_back(ansi(str)); }

	std::vector<std::string> m_AnsiMessages;
};

bool PluginA::InitLang(const string& Path)
{
	bool Result = true;
	if (!PluginLang)
	{
		try
		{
			PluginLang = std::make_unique<AnsiLanguage>(Path);
		}
		catch (const std::exception&)
		{
			Result = false;
		}
	}
	return Result;
}

const char* PluginA::GetMsgA(LNGID nID) const
{
	return static_cast<AnsiLanguage*>(PluginLang.get())->GetMsgA(nID);
}

};

#endif // NO_WRAPPER
