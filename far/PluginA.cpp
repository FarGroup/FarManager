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
#include "language.hpp"
#include "filepanels.hpp"

#define OLDFAR_TO_FAR_MAP(x) { oldfar::x, x }
#include "strmix.hpp"

DECLARE_PLUGIN_FUNCTION(iClosePanel,          void   (WINAPI*)(HANDLE hPlugin))
DECLARE_PLUGIN_FUNCTION(iCompare,             int    (WINAPI*)(HANDLE hPlugin, const wrapper::oldfar::PluginPanelItem *Item1, const wrapper::oldfar::PluginPanelItem *Item2, unsigned int Mode))
DECLARE_PLUGIN_FUNCTION(iConfigure,           int    (WINAPI*)(int ItemNumber))
DECLARE_PLUGIN_FUNCTION(iDeleteFiles,         int    (WINAPI*)(HANDLE hPlugin, wrapper::oldfar::PluginPanelItem *PanelItem, int ItemsNumber, int OpMode))
DECLARE_PLUGIN_FUNCTION(iExitFAR,             void   (WINAPI*)())
DECLARE_PLUGIN_FUNCTION(iFreeFindData,        void   (WINAPI*)(HANDLE hPlugin, wrapper::oldfar::PluginPanelItem *PanelItem, int ItemsNumber))
DECLARE_PLUGIN_FUNCTION(iFreeVirtualFindData, void   (WINAPI*)(HANDLE hPlugin, wrapper::oldfar::PluginPanelItem *PanelItem, int ItemsNumber))
DECLARE_PLUGIN_FUNCTION(iGetFiles,            int    (WINAPI*)(HANDLE hPlugin, wrapper::oldfar::PluginPanelItem *PanelItem, int ItemsNumber, int Move, char *DestPath, int OpMode))
DECLARE_PLUGIN_FUNCTION(iGetFindData,         int    (WINAPI*)(HANDLE hPlugin, wrapper::oldfar::PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode))
DECLARE_PLUGIN_FUNCTION(iGetMinFarVersion,    int    (WINAPI*)())
DECLARE_PLUGIN_FUNCTION(iGetOpenPanelInfo,    void   (WINAPI*)(HANDLE hPlugin, wrapper::oldfar::OpenPanelInfo *Info))
DECLARE_PLUGIN_FUNCTION(iGetPluginInfo,       void   (WINAPI*)(wrapper::oldfar::PluginInfo *Info))
DECLARE_PLUGIN_FUNCTION(iGetVirtualFindData,  int    (WINAPI*)(HANDLE hPlugin, wrapper::oldfar::PluginPanelItem **pPanelItem, int *pItemsNumber, const char *Path))
DECLARE_PLUGIN_FUNCTION(iMakeDirectory,       int    (WINAPI*)(HANDLE hPlugin, char *Name, int OpMode))
DECLARE_PLUGIN_FUNCTION(iOpenFilePlugin,      HANDLE (WINAPI*)(char *Name, const unsigned char *Data, int DataSize))
DECLARE_PLUGIN_FUNCTION(iOpen,                HANDLE (WINAPI*)(int OpenFrom, intptr_t Item))
DECLARE_PLUGIN_FUNCTION(iProcessEditorEvent,  int    (WINAPI*)(int Event, void *Param))
DECLARE_PLUGIN_FUNCTION(iProcessEditorInput,  int    (WINAPI*)(const INPUT_RECORD *Rec))
DECLARE_PLUGIN_FUNCTION(iProcessPanelEvent,   int    (WINAPI*)(HANDLE hPlugin, int Event, void *Param))
DECLARE_PLUGIN_FUNCTION(iProcessHostFile,     int    (WINAPI*)(HANDLE hPlugin, wrapper::oldfar::PluginPanelItem *PanelItem, int ItemsNumber, int OpMode))
DECLARE_PLUGIN_FUNCTION(iProcessPanelInput,   int    (WINAPI*)(HANDLE hPlugin, int Key, unsigned int ControlState))
DECLARE_PLUGIN_FUNCTION(iPutFiles,            int    (WINAPI*)(HANDLE hPlugin, wrapper::oldfar::PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode))
DECLARE_PLUGIN_FUNCTION(iSetDirectory,        int    (WINAPI*)(HANDLE hPlugin, const char *Dir, int OpMode))
DECLARE_PLUGIN_FUNCTION(iSetFindList,         int    (WINAPI*)(HANDLE hPlugin, const wrapper::oldfar::PluginPanelItem *PanelItem, int ItemsNumber))
DECLARE_PLUGIN_FUNCTION(iSetStartupInfo,      void   (WINAPI*)(const wrapper::oldfar::PluginStartupInfo *Info))
DECLARE_PLUGIN_FUNCTION(iProcessViewerEvent,  int    (WINAPI*)(int Event, void *Param))
DECLARE_PLUGIN_FUNCTION(iProcessDialogEvent,  int    (WINAPI*)(int Event, void *Param))

namespace wrapper
{

static inline auto UnicodeToOEM(const wchar_t* src, char* dst, size_t lendst)
{
	return static_cast<int>(unicode::to(CP_OEMCP, src, wcslen(src) + 1, dst, lendst));
}

template<size_t N>
static inline auto UnicodeToOEM(const wchar_t* src, char(&dst)[N])
{
	return UnicodeToOEM(src, dst, N);
}

static inline auto OEMToUnicode(const char* src, wchar_t* dst, size_t lendst)
{
	return static_cast<int>(unicode::from(CP_OEMCP, src, strlen(src) + 1, dst, lendst));
}

template<size_t N>
static inline auto OEMToUnicode(const char* src, wchar_t(&dst)[N])
{
	return OEMToUnicode(src, dst, N);
}

class oem_plugin_factory: public native_plugin_factory
{
public:
	oem_plugin_factory(PluginManager* Owner):
		native_plugin_factory(Owner)
	{
		static const plugin_factory::export_name ExportsNames[] =
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
			WA(""), // GetContentFields not used
			WA(""), // GetContentData not used
			WA(""), // FreeContentData not used

			WA("OpenFilePlugin"),
			WA("GetMinFarVersion"),
		};
		static_assert(std::size(ExportsNames) == ExportsCount, "Not all exports names are defined");
		m_ExportsNames = make_range(ALL_CONST_RANGE(ExportsNames));
	}

	virtual std::unique_ptr<Plugin> CreatePlugin(const string& filename) override
	{
		return IsPlugin(filename)? std::make_unique<PluginA>(this, filename) : nullptr;
	}

	const std::string& getUserName()
	{
		if (m_userName.empty())
		{
			m_userName = "Software\\Far Manager" + (Global->strRegUser.empty()? "" : "\\Users\\" + narrow(Global->strRegUser)) + "\\Plugins";
		}
		return m_userName;
	}

private:
	virtual bool FindExport(const char* ExportName) const override
	{
		// module with ANY known export can be OEM plugin
		return std::find_if(ALL_RANGE(m_ExportsNames), [&](const export_name& i)
		{
			return !strcmp(ExportName, i.AName);
		}) != m_ExportsNames.end();
	}

	std::string m_userName;
};

plugin_factory_ptr CreateOemPluginFactory(PluginManager* Owner)
{
	return std::make_unique<oem_plugin_factory>(Owner);
}

static inline int IsSpaceA(int x) { return x==' '  || x=='\t'; }
static inline int IsEolA(int x)   { return x=='\r' || x=='\n'; }
static inline int IsSlashA(int x) { return x=='\\' || x=='/'; }

static unsigned char LowerToUpper[256];
static unsigned char UpperToLower[256];
static unsigned char IsUpperOrLower[256];

static bool LocalUpperInit()
{
	const auto Init = []
	{
		for (size_t I=0; I<std::size(LowerToUpper); I++)
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
		return true;
	};
	static const auto InitOnce = Init();
	return InitOnce;
}

using comparer = int(*)(const void*, const void*);
using comparer_ex = int(*)(const void*, const void*, void*);

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
	const auto helper = static_cast<const comparer_helper*>(user);
	return helper->cmp(one,two,helper->user);
}

static int LocalStricmp(const char *s1, const char *s2)
{
	for (;;)
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

static void AnsiToUnicodeBin(const char* AnsiString, wchar_t* UnicodeString, size_t Length, uintptr_t CodePage = CP_OEMCP)
{
	if (AnsiString && UnicodeString && Length)
	{
		// BUGBUG, error checking
		*UnicodeString = 0;
		unicode::from(CodePage, AnsiString, Length, UnicodeString, Length);
	}
}

static wchar_t *AnsiToUnicodeBin(const char* AnsiString, size_t Length, uintptr_t CodePage = CP_OEMCP)
{
	const auto Result = new wchar_t[Length + 1];
	AnsiToUnicodeBin(AnsiString, Result, Length, CodePage);
	Result[Length] = 0;
	return Result;
}

static wchar_t *AnsiToUnicode(const char* AnsiString)
{
	return AnsiString? AnsiToUnicodeBin(AnsiString, strlen(AnsiString) + 1, CP_OEMCP) : nullptr;
}

static char *UnicodeToAnsiBin(const wchar_t* UnicodeString, size_t nLength, uintptr_t CodePage = CP_OEMCP)
{
	/* $ 06.01.2008 TS
	! Увеличил размер выделяемой под строку памяти на 1 байт для нормальной
	работы старых плагинов, которые не знали, что надо смотреть на длину,
	а не на завершающий ноль (например в EditorGetString.StringText).
	*/
	if (!UnicodeString)
		return nullptr;

	const auto Result = new char[nLength + 1];
	Result[0] = 0;
	Result[nLength] = 0;

	if (nLength)
	{
		// BUGBUG, error checking
		unicode::to(CodePage, UnicodeString, nLength, Result, nLength);
	}

	return Result;
}

static char *UnicodeToAnsi(const wchar_t* UnicodeString)
{
	if (!UnicodeString)
		return nullptr;

	return UnicodeToAnsiBin(UnicodeString, wcslen(UnicodeString), CP_OEMCP);
}

static wchar_t **AnsiArrayToUnicode(const char* const* lpaszAnsiString, size_t iCount)
{
	wchar_t** Result = nullptr;

	if (lpaszAnsiString && iCount)
	{
		Result = new wchar_t*[iCount];
		std::transform(lpaszAnsiString, lpaszAnsiString + iCount, Result, AnsiToUnicode);
	}

	return Result;
}

static wchar_t **AnsiArrayToUnicodeMagic(const char* const* lpaszAnsiString, size_t iCount)
{
	wchar_t** Result = nullptr;

	if (lpaszAnsiString && iCount)
	{
		Result = new wchar_t*[iCount + 1];
		Result[0] = static_cast<wchar_t*>(ToPtr(iCount));
		++Result;
		std::transform(lpaszAnsiString, lpaszAnsiString + iCount, Result, AnsiToUnicode);
	}

	return Result;
}

static void FreeUnicodeArrayMagic(const wchar_t* const* Array)
{
	if (Array)
	{
		const auto RealPtr = Array - 1;
		const auto Size = reinterpret_cast<size_t>(RealPtr[0]);
		std::for_each(Array, Array + Size, std::default_delete<const wchar_t[]>());
		delete[] RealPtr;
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
			if (unicode::from(CP_OEMCP, &OemChar, 1, &WideChar, 1))
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
			if (unicode::to(CP_OEMCP, &WideChar, 1, &OemChar, 1))
				dKey = (dKey^CleanKey) | OemChar;
		}
	}

	return dKey;
}

template<class F1, class F2, class M>
static void FirstFlagsToSecond(const F1& FirstFlags, F2& SecondFlags, M& Map)
{
	for (const auto& i: Map)
	{
		if (FirstFlags & i.first)
		{
			SecondFlags |= i.second;
		}
	}
}

template<class F1, class F2, class M>
static void SecondFlagsToFirst(const F2& SecondFlags, F1& FirstFlags, M& Map)
{
	for (const auto& i: Map)
	{
		if (SecondFlags & i.second)
		{
			FirstFlags |= i.first;
		}
	}
}

static void ConvertInfoPanelLinesA(const oldfar::InfoPanelLine *iplA, InfoPanelLine **piplW, size_t iCount)
{
	if (iplA && piplW && (iCount>0))
	{
		std::unique_ptr<InfoPanelLine[]> iplW(new InfoPanelLine[iCount]());

		std::transform(iplA, iplA + iCount, iplW.get(), [](const auto& Item)
		{
			return InfoPanelLine{ AnsiToUnicode(Item.Text), AnsiToUnicode(Item.Data), Item.Separator? IPLFLAGS_SEPARATOR : 0 };
		});

		*piplW = iplW.release();
	}
}

static void FreeUnicodeInfoPanelLines(const InfoPanelLine *iplW, size_t InfoLinesNumber)
{
	std::for_each(iplW, iplW + InfoLinesNumber, [](const InfoPanelLine& Item)
	{
		delete[] Item.Text;
		delete[] Item.Data;
	});

	delete[] iplW;
}

static void ConvertPanelModesA(const oldfar::PanelMode *pnmA, PanelMode **ppnmW, size_t iCount)
{
	if (pnmA && ppnmW && iCount)
	{
		std::unique_ptr<PanelMode[]> pnmW(new PanelMode[iCount]());
		std::vector<string> Strings;
		const auto Handler = [&Strings](const oldfar::PanelMode& Src, PanelMode& Dest)
		{
			size_t iColumnCount = 0;
			if (Src.ColumnTypes)
			{
				split(Strings, wide(Src.ColumnTypes), 0, L",");
				iColumnCount = Strings.size();
			}

			Dest.ColumnTypes = AnsiToUnicode(Src.ColumnTypes);
			Dest.ColumnWidths = AnsiToUnicode(Src.ColumnWidths);
			Dest.ColumnTitles = AnsiArrayToUnicodeMagic(Src.ColumnTitles, iColumnCount);
			Dest.StatusColumnTypes = AnsiToUnicode(Src.StatusColumnTypes);
			Dest.StatusColumnWidths = AnsiToUnicode(Src.StatusColumnWidths);
			Dest.Flags = 0;
			if (Src.FullScreen) Dest.Flags |= PMFLAGS_FULLSCREEN;
			if (Src.DetailedStatus) Dest.Flags |= PMFLAGS_DETAILEDSTATUS;
			if (Src.AlignExtensions) Dest.Flags |= PMFLAGS_ALIGNEXTENSIONS;
			if (Src.CaseConversion) Dest.Flags |= PMFLAGS_CASECONVERSION;
		};
		for_each_zip(Handler, pnmA, pnmA + iCount, pnmW.get());

		*ppnmW = pnmW.release();
	}
}

static void FreeUnicodePanelModes(const PanelMode *pnmW, size_t iCount)
{
	std::for_each(pnmW, pnmW + iCount, [](const PanelMode& Item)
	{
		delete[] Item.ColumnTypes;
		delete[] Item.ColumnWidths;
		FreeUnicodeArrayMagic(Item.ColumnTitles);
		delete[] Item.StatusColumnTypes;
		delete[] Item.StatusColumnWidths;
	});
	delete[] pnmW;
}

static void ConvertKeyBarTitlesA(const oldfar::KeyBarTitles *kbtA, KeyBarTitles *kbtW, bool FullStruct = true)
{
	if (kbtA && kbtW)
	{
		static const std::pair<decltype(&oldfar::KeyBarTitles::Titles), int> LabelsMap[] =
		{
			{ &oldfar::KeyBarTitles::Titles, 0 },
			{ &oldfar::KeyBarTitles::CtrlTitles, LEFT_CTRL_PRESSED },
			{ &oldfar::KeyBarTitles::AltTitles, LEFT_ALT_PRESSED },
			{ &oldfar::KeyBarTitles::ShiftTitles, SHIFT_PRESSED },
		},
		LabelsMapEx[] =
		{
			{ &oldfar::KeyBarTitles::CtrlShiftTitles, LEFT_CTRL_PRESSED | SHIFT_PRESSED },
			{ &oldfar::KeyBarTitles::AltShiftTitles, LEFT_ALT_PRESSED | SHIFT_PRESSED },
			{ &oldfar::KeyBarTitles::CtrlAltTitles, LEFT_CTRL_PRESSED | LEFT_ALT_PRESSED },
		};

		kbtW->CountLabels = 0;
		kbtW->Labels = nullptr;

		for (size_t i = 0; i != 12; ++i)
		{
			const auto CheckLabel = [&](const auto& Item) { return (kbtA->*Item.first)[i] != nullptr; };

			kbtW->CountLabels += std::count_if(ALL_CONST_RANGE(LabelsMap), CheckLabel);

			if (FullStruct)
			{
				kbtW->CountLabels += std::count_if(ALL_CONST_RANGE(LabelsMapEx), CheckLabel);
			}
		}

		if (kbtW->CountLabels)
		{
			std::unique_ptr<KeyBarLabel[]> WideLabels(new KeyBarLabel[kbtW->CountLabels]);

			for (size_t i = 0, j = 0; i != 12; ++i)
			{
				const auto ProcessLabel = [&](const auto& Item)
				{
					if ((kbtA->*Item.first)[i])
					{
						WideLabels[j].Text = AnsiToUnicode((kbtA->*Item.first)[i]);
						WideLabels[j].LongText = nullptr;
						WideLabels[j].Key.VirtualKeyCode = static_cast<WORD>(VK_F1 + i);
						WideLabels[j].Key.ControlKeyState = Item.second;
						++j;
					}
				};

				std::for_each(ALL_CONST_RANGE(LabelsMap), ProcessLabel);

				if (FullStruct)
				{
					std::for_each(ALL_CONST_RANGE(LabelsMapEx), ProcessLabel);
				}
			}

			kbtW->Labels = WideLabels.release();
		}
	}
}

static void FreeUnicodeKeyBarTitles(KeyBarTitles *kbtW)
{
	if (kbtW)
	{
		std::for_each(kbtW->Labels, kbtW->Labels + kbtW->CountLabels, [](const KeyBarLabel& Item)
		{
			delete[] Item.Text;
		});
		delete[] kbtW->Labels;
		kbtW->Labels = nullptr;
		kbtW->CountLabels = 0;
	}
}

static void WINAPI FreeUserData(void* UserData, const FarPanelItemFreeInfo* Info)
{
	delete[] static_cast<char*>(UserData);
}

static PluginPanelItem* ConvertAnsiPanelItemsToUnicode(const oldfar::PluginPanelItem *PanelItemA, size_t ItemsNumber)
{
	std::unique_ptr<PluginPanelItem[]> Result(new PluginPanelItem[ItemsNumber]());
	const auto Handler = [](const auto& Src, auto& Dst)
	{
		Dst.Flags = 0;
		if (Src.Flags&oldfar::PPIF_PROCESSDESCR)
			Dst.Flags |= PPIF_PROCESSDESCR;
		if (Src.Flags&oldfar::PPIF_SELECTED)
			Dst.Flags |= PPIF_SELECTED;

		Dst.NumberOfLinks = Src.NumberOfLinks;

		if (Src.Description)
			Dst.Description = AnsiToUnicode(Src.Description);

		if (Src.Owner)
			Dst.Owner = AnsiToUnicode(Src.Owner);

		if (Src.CustomColumnNumber)
		{
			Dst.CustomColumnNumber = Src.CustomColumnNumber;
			Dst.CustomColumnData = AnsiArrayToUnicode(Src.CustomColumnData, Src.CustomColumnNumber);
		}

		if (Src.Flags&oldfar::PPIF_USERDATA)
		{
			void* UserData = (void*)Src.UserData;
			DWORD Size = *(DWORD *)UserData;
			Dst.UserData.Data = new char[Size];
			memcpy(Dst.UserData.Data, UserData, Size);
			Dst.UserData.FreeData = FreeUserData;
		}
		else
		{
			Dst.UserData.Data = (void*)Src.UserData;
			Dst.UserData.FreeData = nullptr;
		}
		Dst.CRC32 = Src.CRC32;
		Dst.FileAttributes = Src.FindData.dwFileAttributes;
		Dst.CreationTime = Src.FindData.ftCreationTime;
		Dst.LastAccessTime = Src.FindData.ftLastAccessTime;
		Dst.LastWriteTime = Src.FindData.ftLastWriteTime;
		Dst.FileSize = (unsigned __int64)Src.FindData.nFileSizeLow + (((unsigned __int64)Src.FindData.nFileSizeHigh) << 32);
		Dst.AllocationSize = (unsigned __int64)Src.PackSize + (((unsigned __int64)Src.PackSizeHigh) << 32);
		Dst.FileName = AnsiToUnicode(Src.FindData.cFileName);
		Dst.AlternateFileName = AnsiToUnicode(Src.FindData.cAlternateFileName);
	};
	for_each_zip(Handler, PanelItemA, PanelItemA + ItemsNumber, Result.get());
	return Result.release();
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
		std::transform(PanelItem.CustomColumnData, PanelItem.CustomColumnData + PanelItem.CustomColumnNumber, PanelItemA.CustomColumnData, UnicodeToAnsi);
	}

	if (PanelItem.UserData.Data&&PanelItem.UserData.FreeData == FreeUserData)
	{
		DWORD Size = *(DWORD *)PanelItem.UserData.Data;
		PanelItemA.UserData = reinterpret_cast<intptr_t>(new char[Size]);
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
	UnicodeToOEM(PanelItem.FileName + PathOffset, PanelItemA.FindData.cFileName);
	UnicodeToOEM(PanelItem.AlternateFileName, PanelItemA.FindData.cAlternateFileName);
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
	std::for_each(PanelItem, PanelItem + ItemsNumber, [](const PluginPanelItem& i)
	{
		delete[] i.Description;
		delete[] i.Owner;
		DeleteRawArray(i.CustomColumnData, i.CustomColumnNumber);
		FreePluginPanelItem(i);
	});

	delete[] PanelItem;
}

static void FreePanelItemA(const oldfar::PluginPanelItem *PanelItem, size_t ItemsNumber, bool bFreeArray = true)
{
	std::for_each(PanelItem, PanelItem + ItemsNumber, [](const oldfar::PluginPanelItem& Item)
	{
		delete[] Item.Description;
		delete[] Item.Owner;

		DeleteRawArray(Item.CustomColumnData, Item.CustomColumnNumber);

		if (Item.Flags & oldfar::PPIF_USERDATA)
		{
			delete[] reinterpret_cast<char*>(Item.UserData);
		}
	});

	if (bFreeArray)
		delete[] PanelItem;
}

static char *InsertQuoteA(char *Str)
{
	size_t l = strlen(Str);

	if (*Str != '"')
	{
		std::copy_n(Str, ++l, Str + 1);
		*Str = '"';
	}

	if (Str[l - 1] != '"')
	{
		Str[l++] = '\"';
		Str[l] = 0;
	}

	return Str;
}

static inline auto GetPluginGuid(intptr_t n) { return &reinterpret_cast<Plugin*>(n)->GetGUID(); }

struct DialogData
{
	oldfar::FARWINDOWPROC DlgProc;
	HANDLE hDlg;
	oldfar::FarDialogItem *diA;
	FarDialogItem *di;
	FarList *l;
};

static auto& DialogList()
{
	static std::list<DialogData> s_DialogList;
	return s_DialogList;
}

oldfar::FarDialogItem* OneDialogItem = nullptr;

static auto FindDialogData(HANDLE hDlg)
{
	const auto ItemIterator = std::find_if(RANGE(DialogList(), i)
	{
		return i.hDlg == hDlg;
	});

	return ItemIterator == DialogList().end()? nullptr : &*ItemIterator;
}

// can be nullptr in case of the ansi dialog plugin
static auto CurrentDialogItemA(HANDLE hDlg, int ItemNumber)
{
	const auto current = FindDialogData(hDlg);
	return current ? &current->diA[ItemNumber] : nullptr;
}

static auto& CurrentDialogItem(HANDLE hDlg, int ItemNumber)
{
	return FindDialogData(hDlg)->di[ItemNumber];
}

static auto& CurrentList(HANDLE hDlg, int ItemNumber)
{
	return FindDialogData(hDlg)->l[ItemNumber];
}

static auto& OriginalEvents()
{
	static std::stack<FarDialogEvent> sOriginalEvents;
	return sOriginalEvents;
}

static size_t GetAnsiVBufSize(const oldfar::FarDialogItem &diA)
{
	return (diA.X2 - diA.X1 + 1)*(diA.Y2 - diA.Y1 + 1);
}

static auto GetAnsiVBufPtr(FAR_CHAR_INFO* VBuf, size_t Size)
{
	return VBuf? *reinterpret_cast<PCHAR_INFO*>(&VBuf[Size]) : nullptr;
}

static void SetAnsiVBufPtr(FAR_CHAR_INFO* VBuf, CHAR_INFO* VBufA, size_t Size)
{
	*reinterpret_cast<PCHAR_INFO*>(&VBuf[Size]) = VBufA;
}

static void AnsiVBufToUnicode(CHAR_INFO* VBufA, FAR_CHAR_INFO* VBuf, size_t Size, bool NoCvt)
{
	if (!VBuf || !VBufA)
		return;

	const auto Handler = [NoCvt](const auto& Src, auto& Dst)
	{
		if (NoCvt)
		{
			Dst.Char = Src.Char.UnicodeChar;
		}
		else
		{
			AnsiToUnicodeBin(&Src.Char.AsciiChar, &Dst.Char, 1);
		}
		Dst.Attributes = colors::ConsoleColorToFarColor(Src.Attributes);
	};
	for_each_zip(Handler, VBufA, VBufA + Size, VBuf);
}

static auto AnsiVBufToUnicode(const oldfar::FarDialogItem &diA)
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

static const std::pair<oldfar::LISTITEMFLAGS, LISTITEMFLAGS> ListFlagsMap[] =
{
	OLDFAR_TO_FAR_MAP(LIF_SELECTED),
	OLDFAR_TO_FAR_MAP(LIF_CHECKED),
	OLDFAR_TO_FAR_MAP(LIF_SEPARATOR),
	OLDFAR_TO_FAR_MAP(LIF_DISABLE),
	OLDFAR_TO_FAR_MAP(LIF_GRAYED),
	OLDFAR_TO_FAR_MAP(LIF_HIDDEN),
	OLDFAR_TO_FAR_MAP(LIF_DELETEUSERDATA),
};

static void UnicodeListItemToAnsi(const FarListItem* li, oldfar::FarListItem* liA)
{
	UnicodeToOEM(li->Text, liA->Text, std::size(liA->Text) - 1);
	liA->Flags = 0;
	if (li->Flags)
	{
		SecondFlagsToFirst(li->Flags, liA->Flags, ListFlagsMap);
	}
}

static void AnsiListItemToUnicode(const oldfar::FarListItem* liA, FarListItem* li)
{
	li->Text = AnsiToUnicode(liA->Text);
	li->Flags = LIF_NONE;
	if (liA->Flags)
	{
		FirstFlagsToSecond(liA->Flags, li->Flags, ListFlagsMap);
	}
}

static const std::pair<oldfar::FarDialogItemFlags, FARDIALOGFLAGS> DialogItemFlagsMap[] =
{
	OLDFAR_TO_FAR_MAP(DIF_BOXCOLOR),
	OLDFAR_TO_FAR_MAP(DIF_GROUP),
	OLDFAR_TO_FAR_MAP(DIF_LEFTTEXT),
	OLDFAR_TO_FAR_MAP(DIF_MOVESELECT),
	OLDFAR_TO_FAR_MAP(DIF_SHOWAMPERSAND),
	OLDFAR_TO_FAR_MAP(DIF_CENTERGROUP),
	OLDFAR_TO_FAR_MAP(DIF_NOBRACKETS),
	OLDFAR_TO_FAR_MAP(DIF_MANUALADDHISTORY),
	OLDFAR_TO_FAR_MAP(DIF_SEPARATOR),
	OLDFAR_TO_FAR_MAP(DIF_SEPARATOR2),
	OLDFAR_TO_FAR_MAP(DIF_EDITOR),
	OLDFAR_TO_FAR_MAP(DIF_LISTNOAMPERSAND),
	OLDFAR_TO_FAR_MAP(DIF_LISTNOBOX),
	OLDFAR_TO_FAR_MAP(DIF_HISTORY),
	OLDFAR_TO_FAR_MAP(DIF_BTNNOCLOSE),
	OLDFAR_TO_FAR_MAP(DIF_CENTERTEXT),
	OLDFAR_TO_FAR_MAP(DIF_SEPARATORUSER),
	OLDFAR_TO_FAR_MAP(DIF_EDITEXPAND),
	OLDFAR_TO_FAR_MAP(DIF_DROPDOWNLIST),
	OLDFAR_TO_FAR_MAP(DIF_USELASTHISTORY),
	OLDFAR_TO_FAR_MAP(DIF_MASKEDIT),
	OLDFAR_TO_FAR_MAP(DIF_SELECTONENTRY),
	OLDFAR_TO_FAR_MAP(DIF_3STATE),
	OLDFAR_TO_FAR_MAP(DIF_EDITPATH),
	OLDFAR_TO_FAR_MAP(DIF_LISTWRAPMODE),
	OLDFAR_TO_FAR_MAP(DIF_LISTAUTOHIGHLIGHT),
	OLDFAR_TO_FAR_MAP(DIF_AUTOMATION),
	OLDFAR_TO_FAR_MAP(DIF_HIDDEN),
	OLDFAR_TO_FAR_MAP(DIF_READONLY),
	OLDFAR_TO_FAR_MAP(DIF_NOFOCUS),
	OLDFAR_TO_FAR_MAP(DIF_DISABLE),
};

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
	di.Flags = DIF_NONE;

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
		FirstFlagsToSecond(diA.Flags, di.Flags, DialogItemFlagsMap);
	}

	if (diA.DefaultButton) di.Flags |= DIF_DEFAULTBUTTON;
}

static void AnsiDialogItemToUnicode(const oldfar::FarDialogItem &diA, FarDialogItem &di, FarList &l)
{
	ClearStruct(di);
	AnsiDialogItemToUnicodeSafe(diA, di);

	switch (di.Type)
	{
	case DI_LISTBOX:
	case DI_COMBOBOX:
	{
		if (diA.ListItems && os::memory::is_pointer(diA.ListItems))
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
		const auto Buffer = new wchar_t[std::size(diA.Data)];
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

		if (di.Flags&DIF_HISTORY)
			delete[] di.History;
		else if (di.Flags&DIF_MASKEDIT)
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
	if ((diA.Type == oldfar::DI_EDIT || diA.Type == oldfar::DI_FIXEDIT) && (diA.Flags&oldfar::DIF_HISTORY || diA.Flags&oldfar::DIF_MASKEDIT))
	{
		delete[] diA.History;
		diA.History = nullptr;
	}

	if ((diA.Type == oldfar::DI_EDIT || diA.Type == oldfar::DI_COMBOBOX) && diA.Flags&oldfar::DIF_VAREDIT)
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
	diA.Flags = 0;

	if (diA.Flags&oldfar::DIF_SETCOLOR)
	{
		diA.Flags |= oldfar::DIF_SETCOLOR | (diA.Flags&oldfar::DIF_COLORMASK);
	}

	if (di.Flags)
	{
		SecondFlagsToFirst(di.Flags, diA.Flags, DialogItemFlagsMap);
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
		diA->ListPos = static_cast<int>(NativeInfo.SendDlgMessage(hDlg, DM_LISTGETCURPOS, ItemNumber, nullptr));
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
		UnicodeToOEM(di.Data, diA->Data);

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

	static const std::pair<oldfar::PANELINFOFLAGS, PANELINFOFLAGS> FlagsMap[] =
	{
		OLDFAR_TO_FAR_MAP(PFLAGS_SHOWHIDDEN),
		OLDFAR_TO_FAR_MAP(PFLAGS_HIGHLIGHT),
		OLDFAR_TO_FAR_MAP(PFLAGS_REVERSESORTORDER),
		OLDFAR_TO_FAR_MAP(PFLAGS_USESORTGROUPS),
		OLDFAR_TO_FAR_MAP(PFLAGS_SELECTEDFIRST),
		OLDFAR_TO_FAR_MAP(PFLAGS_REALNAMES),
		OLDFAR_TO_FAR_MAP(PFLAGS_NUMERICSORT),
		OLDFAR_TO_FAR_MAP(PFLAGS_PANELLEFT),
	};

	SecondFlagsToFirst(PIW->Flags, PIA->Flags, FlagsMap);
}

static void FreeAnsiPanelInfo(oldfar::PanelInfo* PIA)
{
	if (PIA->PanelItems)
		FreePanelItemA(PIA->PanelItems, PIA->ItemsNumber);

	if (PIA->SelectedItems)
		FreePanelItemA(PIA->SelectedItems, PIA->SelectedItemsNumber);

	ClearStruct(*PIA);
}

struct oldPanelInfoContainer: noncopyable
{
	oldPanelInfoContainer(): Info() {}
	~oldPanelInfoContainer() { FreeAnsiPanelInfo(&Info); }

	oldfar::PanelInfo Info;
};

static __int64 GetSetting(FARSETTINGS_SUBFOLDERS Root, const wchar_t* Name)
{
	__int64 result = 0;
	FarSettingsCreate settings = { sizeof(FarSettingsCreate), FarGuid, INVALID_HANDLE_VALUE };
	HANDLE Settings = NativeInfo.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &settings)? settings.Handle : nullptr;
	if (Settings)
	{
		FarSettingsItem item = { sizeof(FarSettingsItem), static_cast<size_t>(Root), Name, FST_UNKNOWN, {} };
		if (NativeInfo.SettingsControl(Settings, SCTL_GET, 0, &item) && FST_QWORD == item.Type)
		{
			result = item.Number;
		}
		NativeInfo.SettingsControl(Settings, SCTL_FREE, 0, nullptr);
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
		std::any_of(CONST_RANGE(CpEnum, i)
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
		const auto TempTable(unicode::from(nCPin, szBuffer, nLength));
		unicode::to(nCPout, TempTable.data(), nLength, szBuffer, nLength);
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
			std::any_of(CONST_RANGE(CpEnum, i)
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
		std::for_each(Buf, Buf + Length, [](char& i){ i = LowerToUpper[i]; });
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
		std::for_each(Buf, Buf + Length, [](char& i){ i = UpperToLower[i]; });
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
		const auto Iterator = null_iterator(s1);
		std::for_each(Iterator, Iterator.end(), [](char& i){ i = LowerToUpper[i]; });
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
		const auto Iterator = null_iterator(s1);
		std::for_each(Iterator, Iterator.end(), [](char& i){ i = UpperToLower[i]; });
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
		return nullptr;
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
		return std::strtoll(s, nullptr, 10);
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
		auto ChPtr = Str;

		if (!ChPtr)
			return nullptr;

		for (; IsSpaceA(*ChPtr); ChPtr++)
			;

		if (ChPtr != Str)
			std::copy_n(ChPtr, strlen(ChPtr) + 1, Str);

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
				const auto MovePos = Str + Length - MaxLength + 3;
				std::copy_n(MovePos, strlen(MovePos) + 1, Str + 3);
				std::copy_n("...", 3, Str);
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
			char *Start = nullptr;

			if (*Str && (Str[1] == ':') && IsSlash(Str[2]))
				Start = Str + 3;
			else
			{
				if ((Str[0] == '\\') && (Str[1] == '\\'))
				{
					if ((Start = const_cast<char*>(FirstSlashA(Str + 2))) != nullptr)
						if ((Start = const_cast<char*>(FirstSlashA(Start + 1))) != nullptr)
							Start++;
				}
			}

			if (!Start || (Start - Str > MaxLength - 5))
				return TruncStrA(Str, MaxLength);

			const auto lpInPos = Start + 3 + (nLength - MaxLength);
			std::copy_n(lpInPos, strlen(lpInPos) + 1, Start + 3);
			std::copy_n("...", 3, Start);
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
		NativeFSF.GetPathRoot(wide(Path).data(), Buffer, std::size(Buffer));
		UnicodeToOEM(Buffer, Root, std::size(Buffer));
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
		const auto strP1 = wide(Param1), strP2 = wide(Param2);
		const int size = (int)(strP1.size() + strP2.size() + oldfar::NM) + 1; //а хрен ещё как угадать скока там этот Param2 для PN_GENERATENAME
		wchar_t_ptr p(size);
		*std::copy(ALL_CONST_RANGE(strP2), p.get()) = L'\0';
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
		NativeFSF.MkTemp(D, std::size(D), wide(Prefix).data());
		UnicodeToOEM(D, Dest, std::size(D));
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
		Result = static_cast<int>(NativeFSF.GetReparsePointInfo(strSrc.data(), Buffer, std::size(Buffer)));
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
		const auto pCallbackParam = static_cast<const FAR_SEARCH_A_CALLBACK_PARAM*>(param);
		WIN32_FIND_DATAA FindData = {};
		FindData.dwFileAttributes = FData->FileAttributes;
		FindData.ftCreationTime = FData->CreationTime;
		FindData.ftLastAccessTime = FData->LastAccessTime;
		FindData.ftLastWriteTime = FData->LastWriteTime;
		FindData.nFileSizeLow = (DWORD)FData->FileSize;
		FindData.nFileSizeHigh = (DWORD)(FData->FileSize >> 32);
		UnicodeToOEM(FData->FileName, FindData.cFileName);
		UnicodeToOEM(FData->AlternateFileName, FindData.cAlternateFileName);
		char FullNameA[oldfar::NM];
		UnicodeToOEM(FullName, FullNameA);
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
		string strD = os::env::expand_strings(wide(src));
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
		FarColor Color = colors::ConsoleColorToFarColor(ConColor);
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
		static const std::pair<oldfar::INPUTBOXFLAGS, INPUTBOXFLAGS> FlagsMap[] =
		{
			OLDFAR_TO_FAR_MAP(FIB_ENABLEEMPTY),
			OLDFAR_TO_FAR_MAP(FIB_PASSWORD),
			OLDFAR_TO_FAR_MAP(FIB_EXPANDENV),
			OLDFAR_TO_FAR_MAP(FIB_NOUSELASTHISTORY),
			OLDFAR_TO_FAR_MAP(FIB_BUTTONS),
			OLDFAR_TO_FAR_MAP(FIB_NOAMPERSAND),
		};

		auto NewFlags = FIB_NONE;
		FirstFlagsToSecond(Flags, NewFlags, FlagsMap);

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
			std::transform(Items, Items + ItemsNumber, std::back_inserter(AnsiItems), [](const char* Item){ return std::unique_ptr<wchar_t[]>(AnsiToUnicode(Item)); });
		}

		static const std::pair<oldfar::FARMESSAGEFLAGS, FARMESSAGEFLAGS> FlagsMap[] =
		{
			OLDFAR_TO_FAR_MAP(FMSG_WARNING),
			OLDFAR_TO_FAR_MAP(FMSG_ERRORTYPE),
			OLDFAR_TO_FAR_MAP(FMSG_KEEPBACKGROUND),
			OLDFAR_TO_FAR_MAP(FMSG_LEFTALIGN),
			OLDFAR_TO_FAR_MAP(FMSG_ALLINONE),
		};

		auto NewFlags = FMSG_NONE;
		FirstFlagsToSecond(Flags, NewFlags, FlagsMap);

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
		const auto pPlugin = reinterpret_cast<PluginA*>(PluginHandle);
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
		static const std::pair<oldfar::FARMENUFLAGS, FARMENUFLAGS> FlagsMap[] =
		{
			OLDFAR_TO_FAR_MAP(FMENU_SHOWAMPERSAND),
			OLDFAR_TO_FAR_MAP(FMENU_WRAPMODE),
			OLDFAR_TO_FAR_MAP(FMENU_AUTOHIGHLIGHT),
			OLDFAR_TO_FAR_MAP(FMENU_REVERSEAUTOHIGHLIGHT),
			OLDFAR_TO_FAR_MAP(FMENU_CHANGECONSOLETITLE),
		};

		auto NewFlags = FMENU_NONE;
		FirstFlagsToSecond(Flags, NewFlags, FlagsMap);

		if (!Item) ItemsNumber = 0;

		std::vector<FarMenuItem> mi(ItemsNumber);

		if (Flags&oldfar::FMENU_USEEXT)
		{
			const auto p = reinterpret_cast<const oldfar::FarMenuItemEx*>(Item);

			static const std::pair<oldfar::MENUITEMFLAGS, MENUITEMFLAGS> ItemFlagsMap[] =
			{
				OLDFAR_TO_FAR_MAP(MIF_SELECTED),
				OLDFAR_TO_FAR_MAP(MIF_CHECKED),
				OLDFAR_TO_FAR_MAP(MIF_SEPARATOR),
				OLDFAR_TO_FAR_MAP(MIF_DISABLE),
				OLDFAR_TO_FAR_MAP(MIF_GRAYED),
				OLDFAR_TO_FAR_MAP(MIF_HIDDEN),
			};

			for (int i = 0; i < ItemsNumber; i++)
			{
				mi[i].Flags = MIF_NONE;
				FirstFlagsToSecond(p[i].Flags, mi[i].Flags, ItemFlagsMap);
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
			while (BreakKeys[BreakKeysCount++])
				;
			if (BreakKeysCount)
			{
				NewBreakKeys.resize(BreakKeysCount);
				std::transform(BreakKeys, BreakKeys + BreakKeysCount, NewBreakKeys.begin(), [&](int i)
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
		const auto Data = FindDialogData(hDlg);
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
					auto& Color = *static_cast<FarColor*>(Param2);
					Color = colors::ConsoleColorToFarColor(static_cast<int>(CurrentDlgProc(hDlg, oldfar::DN_CTLCOLORDIALOG, Param1,
					ToPtr(colors::FarColorToConsoleColor(Color)))));
				}
				break;

			case DN_DRAWDIALOG:
				Msg=oldfar::DN_DRAWDIALOG;
				break;

			case DN_CTLCOLORDLGITEM:
				{
					const auto lc = static_cast<const FarDialogItemColors*>(Param2);
					const auto diA = CurrentDialogItemA(hDlg, Param1);

					// first, emulate DIF_SETCOLOR
					if(diA->Flags&oldfar::DIF_SETCOLOR)
					{
						lc->Colors[0] = colors::ConsoleColorToFarColor(diA->Flags&oldfar::DIF_COLORMASK);
					}

					DWORD Result = static_cast<DWORD>(CurrentDlgProc(hDlg, oldfar::DN_CTLCOLORDLGITEM, Param1, ToPtr(MAKELONG(
						MAKEWORD(colors::FarColorToConsoleColor(lc->Colors[0]), colors::FarColorToConsoleColor(lc->Colors[1])),
						MAKEWORD(colors::FarColorToConsoleColor(lc->Colors[2]), colors::FarColorToConsoleColor(lc->Colors[3]))))));
					if(lc->ColorsCount > 0)
						lc->Colors[0] = colors::ConsoleColorToFarColor(LOBYTE(LOWORD(Result)));
					if(lc->ColorsCount > 1)
						lc->Colors[1] = colors::ConsoleColorToFarColor(HIBYTE(LOWORD(Result)));
					if(lc->ColorsCount > 2)
						lc->Colors[2] = colors::ConsoleColorToFarColor(LOBYTE(HIWORD(Result)));
					if(lc->ColorsCount > 3)
						lc->Colors[3] = colors::ConsoleColorToFarColor(HIBYTE(HIWORD(Result)));
				}
				break;

			case DN_CTLCOLORDLGLIST:
				{
					const auto lc = static_cast<FarDialogItemColors*>(Param2);
					std::vector<BYTE> AnsiColors(lc->ColorsCount);
					std::transform(lc->Colors, lc->Colors + lc->ColorsCount, AnsiColors.begin(), colors::FarColorToConsoleColor);
					oldfar::FarListColors lcA={0, 0, static_cast<int>(AnsiColors.size()), AnsiColors.data()};
					intptr_t Result = CurrentDlgProc(hDlg, oldfar::DN_CTLCOLORDLGLIST, Param1, &lcA);
					if(Result)
					{
						lc->ColorsCount = lcA.ColorCount;
						std::transform(lcA.Colors, lcA.Colors + lcA.ColorCount, lc->Colors, colors::ConsoleColorToFarColor);
					}
					return Result != 0;
				}
				break;

			case DN_DRAWDLGITEM:
			{
				Msg=oldfar::DN_DRAWDLGITEM;
				const auto di = static_cast<FarDialogItem*>(Param2);
				const auto FarDiA = UnicodeDialogItemToAnsi(*di, hDlg, Param1);
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
					const auto record = static_cast<INPUT_RECORD*>(Param2);
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
						Param2 = nullptr;
						break;
					}
				}
				return NativeInfo.DefDlgProc(hDlg, NewMsg, Param1, Param2);
			case DN_CONTROLINPUT:
				{
					const auto record = static_cast<INPUT_RECORD*>(Param2);
					if (record->EventType==MOUSE_EVENT)
					{
						Msg=oldfar::DN_MOUSECLICK;
						Param2=&record->Event.MouseEvent;
						break;
					}
					else if (record->EventType==KEY_EVENT)
					{
						Msg=oldfar::DN_KEY;
						Param2=ToPtr(KeyToOldKey((DWORD)InputRecordToKey(record)));
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
				size_t item_size = NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, Param1, nullptr);

				if (item_size)
				{
					block_ptr<FarDialogItem> Buffer(item_size);
					FarGetDialogItem gdi = {sizeof(FarGetDialogItem), item_size, Buffer.get()};

					if (gdi.Item)
					{
						NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, Param1, &gdi);
						*static_cast<oldfar::FarDialogItem*>(Param2) = *UnicodeDialogItemToAnsi(*gdi.Item, hDlg, Param1);
						return TRUE;
					}
				}

				return FALSE;
			}
			case oldfar::DM_GETDLGRECT: Msg = DM_GETDLGRECT; break;
			case oldfar::DM_GETTEXT:
			{
				if (!Param2)
					return NativeInfo.SendDlgMessage(hDlg, DM_GETTEXT, Param1, nullptr);

				const auto didA = static_cast<oldfar::FarDialogItemData*>(Param2);
				if (!didA->PtrLength) //вот такой хреновый API!!!
					didA->PtrLength = static_cast<int>(NativeInfo.SendDlgMessage(hDlg, DM_GETTEXT, Param1, nullptr));
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
				const auto KeysA = static_cast<const DWORD*>(Param2);
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
				const auto diA = static_cast<const oldfar::FarDialogItem*>(Param2);
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

				const auto didA = static_cast<const oldfar::FarDialogItemData*>(Param2);

				if (!didA->PtrData) return 0;

				//BUGBUG - PtrLength ни на что не влияет.
				string text(wide(didA->PtrData));
				FarDialogItemData di = {sizeof(FarDialogItemData), text.size(), UNSAFE_CSTR(text)};
				return NativeInfo.SendDlgMessage(hDlg, DM_SETTEXT, Param1, &di);
			}
			case oldfar::DM_SETMAXTEXTLENGTH: Msg = DM_SETMAXTEXTLENGTH; break;
			case oldfar::DM_SHOWDIALOG:       Msg = DM_SHOWDIALOG; break;
			case oldfar::DM_GETFOCUS:         Msg = DM_GETFOCUS; break;
			case oldfar::DM_GETCURSORPOS:     Msg = DM_GETCURSORPOS; break;
			case oldfar::DM_SETCURSORPOS:     Msg = DM_SETCURSORPOS; break;
			case oldfar::DM_GETTEXTPTR:
			{
				intptr_t length = NativeInfo.SendDlgMessage(hDlg, DM_GETTEXT, Param1, nullptr);

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
				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_GETCHECK, Param1, nullptr);
				intptr_t state = 0;

				if (ret == oldfar::BSTATE_UNCHECKED) state=BSTATE_UNCHECKED;
				else if (ret == oldfar::BSTATE_CHECKED)   state=BSTATE_CHECKED;
				else if (ret == oldfar::BSTATE_3STATE)    state=BSTATE_3STATE;

				return state;
			}
			case oldfar::DM_SETCHECK:
			{
				FARCHECKEDSTATE State = BSTATE_UNCHECKED;
				switch (static_cast<oldfar::FARCHECKEDSTATE>(reinterpret_cast<intptr_t>(Param2)))
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

				const auto lgiA = static_cast<oldfar::FarListGetItem*>(Param2);
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
					const auto lpA = static_cast<oldfar::FarListPos*>(Param2);
					lpA->SelectPos=lp.SelectPos;
					lpA->TopPos=lp.TopPos;
					return ret;
				}
				else return NativeInfo.SendDlgMessage(hDlg, DM_LISTGETCURPOS, Param1, nullptr);

			case oldfar::DM_LISTSETCURPOS:
			{
				if (!Param2) return FALSE;

				const auto lpA = static_cast<const oldfar::FarListPos*>(Param2);
				FarListPos lp = {sizeof(FarListPos),lpA->SelectPos,lpA->TopPos};
				return NativeInfo.SendDlgMessage(hDlg, DM_LISTSETCURPOS, Param1, &lp);
			}
			case oldfar::DM_LISTDELETE:
			{
				const auto ldA = static_cast<const oldfar::FarListDelete*>(Param2);
				FarListDelete ld={sizeof(FarListDelete)};

				if (Param2)
				{
					ld.Count = ldA->Count;
					ld.StartIndex = ldA->StartIndex;
				}

				return NativeInfo.SendDlgMessage(hDlg, DM_LISTDELETE, Param1, Param2? &ld : nullptr);
			}
			case oldfar::DM_LISTADD:
			{
				FarList newlist = {sizeof(FarList)};
				std::vector<FarListItem> Items;

				if (Param2)
				{
					const auto oldlist = static_cast<const oldfar::FarList*>(Param2);
					newlist.ItemsNumber = oldlist->ItemsNumber;

					if (newlist.ItemsNumber)
					{
						Items.resize(newlist.ItemsNumber);
						for (size_t i=0; i<newlist.ItemsNumber; i++)
							AnsiListItemToUnicode(&oldlist->Items[i], &Items[i]);

						newlist.Items = Items.data();
					}
				}

				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTADD, Param1, Param2? &newlist : nullptr);

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
					const auto oldui = static_cast<const oldfar::FarListUpdate*>(Param2);
					newui.Index=oldui->Index;
					AnsiListItemToUnicode(&oldui->Item, &newui.Item);
				}

				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTUPDATE, Param1, Param2? &newui : nullptr);

				delete[] newui.Item.Text;

				return ret;
			}
			case oldfar::DM_LISTINSERT:
			{
				FarListInsert newli = {sizeof(FarListInsert)};

				if (Param2)
				{
					const auto oldli = static_cast<const oldfar::FarListInsert*>(Param2);
					newli.Index=oldli->Index;
					AnsiListItemToUnicode(&oldli->Item, &newli.Item);
				}

				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTINSERT, Param1, Param2? &newli : nullptr);

				delete[] newli.Item.Text;

				return ret;
			}
			case oldfar::DM_LISTFINDSTRING:
			{
				FarListFind newlf = {sizeof(FarListFind)};

				if (Param2)
				{
					const auto oldlf = static_cast<const oldfar::FarListFind*>(Param2);
					newlf.StartIndex=oldlf->StartIndex;
					newlf.Pattern = AnsiToUnicode(oldlf->Pattern);

					if (oldlf->Flags&oldfar::LIFIND_EXACTMATCH) newlf.Flags|=LIFIND_EXACTMATCH;
				}

				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTFINDSTRING, Param1, Param2? &newlf : nullptr);

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
					const auto liA = static_cast<oldfar::FarListInfo*>(Param2);
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
					const auto oldlid = static_cast<oldfar::FarListItemData*>(Param2);
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

				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTSETDATA, Param1, Param2? &newlid : nullptr);
				return ret;
			}
			case oldfar::DM_LISTSETTITLES:
			{
				if (!Param2) return FALSE;

				const auto ltA = static_cast<const oldfar::FarListTitles*>(Param2);
				FarListTitles lt = {sizeof(FarListTitles), 0, AnsiToUnicode(ltA->Title), 0 , AnsiToUnicode(ltA->Bottom)};
				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTSETTITLES, Param1, &lt);

				delete[] lt.Bottom;
				delete[] lt.Title;

				return ret;
			}
			case oldfar::DM_LISTGETTITLES:
			{
				if (Param2)
				{
					const auto OldListTitle = static_cast<const oldfar::FarListTitles*>(Param2);
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
					return NativeInfo.SendDlgMessage(hDlg, DM_SETHISTORY, Param1, nullptr);
				else
				{
					FarDialogItem& di = CurrentDialogItem(hDlg,Param1);
					delete[] di.History;
					di.History = AnsiToUnicode((const char *)Param2);
					return NativeInfo.SendDlgMessage(hDlg, DM_SETHISTORY, Param1, const_cast<wchar_t*>(di.History));
				}

			case oldfar::DM_GETITEMPOSITION:     Msg = DM_GETITEMPOSITION; break;
			case oldfar::DM_SETMOUSEEVENTNOTIFY: Msg = DM_SETINPUTNOTIFY; break;
			case oldfar::DM_EDITUNCHANGEDFLAG:   Msg = DM_EDITUNCHANGEDFLAG; break;
			case oldfar::DM_GETITEMDATA:         Msg = DM_GETITEMDATA; break;
			case oldfar::DM_SETITEMDATA:         Msg = DM_SETITEMDATA; break;
			case oldfar::DM_LISTSET:
			{
				FarList newlist = {sizeof(FarList)};

				if (Param2)
				{
					const auto oldlist = static_cast<const oldfar::FarList*>(Param2);
					newlist.ItemsNumber = oldlist->ItemsNumber;

					if (newlist.ItemsNumber)
					{
						newlist.Items = new FarListItem[newlist.ItemsNumber];
						for (size_t i=0; i<newlist.ItemsNumber; i++)
							AnsiListItemToUnicode(&oldlist->Items[i], &newlist.Items[i]);
					}
				}

				intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTSET, Param1, Param2? &newlist : nullptr);

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
				FarDialogItem DlgItem = {};
				NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEMSHORT, Param1, &DlgItem);
				FARDIALOGITEMFLAGS OldFlags = DlgItem.Flags;
				DlgItem.Flags&=~(DIF_LISTTRACKMOUSE|DIF_LISTTRACKMOUSEINFOCUS);
				switch (static_cast<oldfar::FARLISTMOUSEREACTIONTYPE>(reinterpret_cast<intptr_t>(Param2)))
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
				const auto esA = static_cast<oldfar::EditorSelect*>(Param2);
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

				const auto esA = static_cast<const oldfar::EditorSelect*>(Param2);
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
		const auto Handler = [](auto& diA_i, const auto& Item_i)
		{
			diA_i.Flags = Item_i.Flags;
		};
		for_each_zip(Handler, ALL_RANGE(diA), Item);

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
				size_t Size = NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, i, nullptr);
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
						UnicodeToOEM(res, Item[i].Data);

					if (gdi.Item->Type==DI_USERCONTROL)
					{
						di[i].VBuf=gdi.Item->VBuf;
						Item[i].VBuf=GetAnsiVBufPtr(gdi.Item->VBuf,GetAnsiVBufSize(Item[i]));
					}

					if (gdi.Item->Type==DI_COMBOBOX || gdi.Item->Type==DI_LISTBOX)
					{
						Item[i].ListPos = static_cast<int>(NativeInfo.SendDlgMessage(hDlg, DM_LISTGETCURPOS, i, nullptr));
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
		return FarDialogExA(PluginNumber, X1, Y1, X2, Y2, HelpTopic, Item, ItemsNumber, 0, 0, nullptr, nullptr);
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
					*static_cast<oldfar::PanelInfo*>(Param) = Passive? AnotherPanelInfoA.Info : PanelInfoA.Info;
					return TRUE;
				}

				Reenter++;

				if (Passive)
					hPlugin=PANEL_PASSIVE;

				const auto OldPI = Passive ? &AnotherPanelInfoA.Info : &PanelInfoA.Info;
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
								size_t NewPPISize = static_cast<size_t>(NativeInfo.PanelControl(hPlugin, FCTL_GETPANELITEM, i, nullptr));

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
								size_t NewPPISize = static_cast<size_t>(NativeInfo.PanelControl(hPlugin, FCTL_GETSELECTEDPANELITEM, i, nullptr));

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

					size_t dirSize = NativeInfo.PanelControl(hPlugin, FCTL_GETPANELDIRECTORY, 0, nullptr);
					if(dirSize)
					{
						block_ptr<FarPanelDirectory> dirInfo(dirSize);
						dirInfo->StructSize=sizeof(FarPanelDirectory);
						NativeInfo.PanelControl(hPlugin, FCTL_GETPANELDIRECTORY, dirSize, dirInfo.get());
						UnicodeToOEM(dirInfo->Name,OldPI->CurDir);
					}
					wchar_t ColumnTypes[sizeof(OldPI->ColumnTypes)];
					NativeInfo.PanelControl(hPlugin,FCTL_GETCOLUMNTYPES,sizeof(OldPI->ColumnTypes),ColumnTypes);
					UnicodeToOEM(ColumnTypes,OldPI->ColumnTypes);
					wchar_t ColumnWidths[sizeof(OldPI->ColumnWidths)];
					NativeInfo.PanelControl(hPlugin,FCTL_GETCOLUMNWIDTHS,sizeof(OldPI->ColumnWidths),ColumnWidths);
					UnicodeToOEM(ColumnWidths,OldPI->ColumnWidths);
					*static_cast<oldfar::PanelInfo*>(Param) = *OldPI;
				}
				else
				{
					ClearStruct(*static_cast<oldfar::PanelInfo*>(Param));
				}

				Reenter--;
				return ret;
			}
			case oldfar::FCTL_GETANOTHERPANELSHORTINFO:
			case oldfar::FCTL_GETPANELSHORTINFO:
			{
				if (!Param)
					return FALSE;

				const auto OldPI = static_cast<oldfar::PanelInfo*>(Param);
				ClearStruct(*OldPI);

				if (Command==oldfar::FCTL_GETANOTHERPANELSHORTINFO)
					hPlugin=PANEL_PASSIVE;

				PanelInfo PI = {sizeof(PanelInfo)};
				int ret = static_cast<int>(NativeInfo.PanelControl(hPlugin,FCTL_GETPANELINFO,0,&PI));

				if (ret)
				{
					ConvertUnicodePanelInfoToAnsi(&PI,OldPI);
					size_t dirSize = NativeInfo.PanelControl(hPlugin, FCTL_GETPANELDIRECTORY, 0, nullptr);
					if(dirSize)
					{
						block_ptr<FarPanelDirectory> dirInfo(dirSize);
						dirInfo->StructSize=sizeof(FarPanelDirectory);
						NativeInfo.PanelControl(hPlugin, FCTL_GETPANELDIRECTORY, dirSize, dirInfo.get());
						UnicodeToOEM(dirInfo->Name, OldPI->CurDir);
					}
					wchar_t ColumnTypes[sizeof(OldPI->ColumnTypes)];
					NativeInfo.PanelControl(hPlugin,FCTL_GETCOLUMNTYPES, std::size(OldPI->ColumnTypes),ColumnTypes);
					UnicodeToOEM(ColumnTypes, OldPI->ColumnTypes);
					wchar_t ColumnWidths[sizeof(OldPI->ColumnWidths)];
					NativeInfo.PanelControl(hPlugin,FCTL_GETCOLUMNWIDTHS,sizeof(OldPI->ColumnWidths),ColumnWidths);
					UnicodeToOEM(ColumnWidths, OldPI->ColumnWidths);
				}

				return ret;
			}
			case oldfar::FCTL_SETANOTHERSELECTION:
				hPlugin=PANEL_PASSIVE;
			case oldfar::FCTL_SETSELECTION:
			{
				if (!Param)
					return FALSE;

				const auto OldPI = static_cast<const oldfar::PanelInfo*>(Param);
				NativeInfo.PanelControl(hPlugin, FCTL_BEGINSELECTION, 0, nullptr);

				for (int i=0; i<OldPI->ItemsNumber; i++)
				{
					NativeInfo.PanelControl(hPlugin,FCTL_SETSELECTION,i,ToPtr(OldPI->PanelItems[i].Flags & oldfar::PPIF_SELECTED));
				}

				NativeInfo.PanelControl(hPlugin, FCTL_ENDSELECTION, 0, nullptr);
				return TRUE;
			}
			case oldfar::FCTL_REDRAWANOTHERPANEL:
				hPlugin = PANEL_PASSIVE;
			case oldfar::FCTL_REDRAWPANEL:
			{
				if (!Param)
					return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_REDRAWPANEL, 0, nullptr));

				const auto priA = static_cast<const oldfar::PanelRedrawInfo*>(Param);
				PanelRedrawInfo pri = {sizeof(PanelRedrawInfo), (size_t)priA->CurrentItem,(size_t)priA->TopPanelItem};
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_REDRAWPANEL,0,&pri));
			}
			case oldfar::FCTL_SETANOTHERNUMERICSORT:
				hPlugin = PANEL_PASSIVE;
			case oldfar::FCTL_SETNUMERICSORT:
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETNUMERICSORT, (Param && (*(int*)Param))? 1 : 0, nullptr));
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

				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETSORTMODE, *(int*)Param, nullptr));
			case oldfar::FCTL_SETANOTHERSORTORDER:
				hPlugin = PANEL_PASSIVE;
			case oldfar::FCTL_SETSORTORDER:
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETSORTORDER, Param && (*(int*)Param), nullptr));
			case oldfar::FCTL_SETANOTHERVIEWMODE:
				hPlugin = PANEL_PASSIVE;
			case oldfar::FCTL_SETVIEWMODE:
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETVIEWMODE, Param? *(int *)Param : 0, nullptr));
			case oldfar::FCTL_UPDATEANOTHERPANEL:
				hPlugin = PANEL_PASSIVE;
			case oldfar::FCTL_UPDATEPANEL:
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_UPDATEPANEL, Param? 1 : 0, nullptr));
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
					const auto clsA = static_cast<oldfar::CmdLineSelect*>(Param);
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
				return Param ? static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETCMDLINEPOS, *static_cast<int*>(Param), nullptr)) : FALSE;

			case oldfar::FCTL_SETCMDLINESELECTION:
			{
				if (!Param)
					return FALSE;

				const auto clsA = static_cast<const oldfar::CmdLineSelect*>(Param);
				CmdLineSelect cls = {sizeof(CmdLineSelect),clsA->SelStart,clsA->SelEnd};
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETCMDLINESELECTION,0,&cls));
			}
			case oldfar::FCTL_GETUSERSCREEN:
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_GETUSERSCREEN, 0, nullptr));
			case oldfar::FCTL_SETUSERSCREEN:
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETUSERSCREEN, 0, nullptr));
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

static void WINAPI FarRestoreScreenA(HANDLE Screen) noexcept
{
	try
	{
		return NativeInfo.RestoreScreen(Screen);
	}
	catch (...)
	{
		// TODO: log
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
				HANDLE Settings = NativeInfo.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &settings)? settings.Handle : nullptr;
				if(Settings)
				{
					FarSettingsItem item={sizeof(FarSettingsItem),FSSF_EDITOR,L"WordDiv",FST_UNKNOWN,{}};
					if(NativeInfo.SettingsControl(Settings,SCTL_GET,0,&item)&&FST_STRING==item.Type)
					{
						Length=std::min(oldfar::NM,StrLength(item.String)+1);
						if(Param) UnicodeToOEM(item.String,(char*)Param,oldfar::NM);
					}
					NativeInfo.SettingsControl(Settings, SCTL_FREE, 0, nullptr);
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
					if(ColorIndex > COL_VIEWERARROWS)
					{
						ColorIndex--;
					}
					return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETCOLOR, ColorIndex, &Color)? colors::FarColorToConsoleColor(Color) :-1;
				}
				break;

			case oldfar::ACTL_GETARRAYCOLOR:
				{
					size_t PaletteSize = NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETARRAYCOLOR, 0, nullptr);
					if(Param)
					{
						std::vector<FarColor> Color(PaletteSize);
						NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETARRAYCOLOR, 0, Color.data());
						const auto OldColors = static_cast<const LPBYTE>(Param);
						std::transform(ALL_CONST_RANGE(Color), OldColors, colors::FarColorToConsoleColor);
					}
					return PaletteSize;
				}
				break;
			case oldfar::ACTL_EJECTMEDIA:
			{

				ActlEjectMedia eject={sizeof(ActlEjectMedia)};
				if(Param)
				{
					const auto ejectA = static_cast<const oldfar::ActlEjectMedia*>(Param);
					eject.Letter=ejectA->Letter;
					eject.Flags=ejectA->Flags;
				}
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_EJECTMEDIA, 0, &eject);
			}
			case oldfar::ACTL_KEYMACRO:
			{
				if (!Param) return FALSE;

				const auto kmA = static_cast<oldfar::ActlKeyMacro*>(Param);
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
					res = NativeInfo.MacroControl(nullptr, MacroCommand, Param1, &mtW);

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

				const auto ksA = static_cast<const oldfar::KeySequence*>(Param);

				if (!ksA->Count || !ksA->Sequence)
					return FALSE;

				FARKEYMACROFLAGS Flags=KMFLAGS_LUA;

				if (!(ksA->Flags&oldfar::KSFLAGS_DISABLEOUTPUT))
					Flags|=KMFLAGS_ENABLEOUTPUT;

				if (ksA->Flags&oldfar::KSFLAGS_NOSENDKEYSTOPLUGINS)
					Flags|=KMFLAGS_NOSENDKEYSTOPLUGINS;

				string strSequence=L"Keys(\"";
				string strKeyText;
				for (const auto& Key: make_range(ksA->Sequence, ksA->Sequence + ksA->Count))
				{
					if (KeyToText(OldKeyToKey(Key), strKeyText))
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

				const auto wiA = static_cast<oldfar::WindowInfo*>(Param);
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
						std::vector<wchar_t> TypeName, Name;

						if (wi.TypeNameSize)
						{
							TypeName.resize(wi.TypeNameSize);
							wi.TypeName = TypeName.data();
						}

						if (wi.NameSize)
						{
							Name.resize(wi.NameSize);
							wi.Name = Name.data();
						}

						if (wi.TypeName && wi.Name)
						{
							NativeInfo.AdvControl(GetPluginGuid(ModuleNumber),ACTL_GETWINDOWINFO, 0, &wi);
							UnicodeToOEM(wi.TypeName, wiA->TypeName);
							UnicodeToOEM(wi.Name, wiA->Name);
						}
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
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETWINDOWCOUNT, 0, nullptr);
			case oldfar::ACTL_SETCURRENTWINDOW:
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_SETCURRENTWINDOW, reinterpret_cast<intptr_t>(Param), nullptr);
			case oldfar::ACTL_COMMIT:
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_COMMIT, 0, nullptr);
			case oldfar::ACTL_GETFARHWND:
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETFARHWND, 0, nullptr);
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

				const auto scA = static_cast<const oldfar::FarSetColors*>(Param);
				std::vector<FarColor> Colors(scA->ColorCount);
				std::transform(scA->Colors, scA->Colors + scA->ColorCount, Colors.begin(), colors::ConsoleColorToFarColor);
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
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_REDRAWALL, 0, nullptr);
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
					const auto ecA = static_cast<const oldfar::EditorColor*>(Param);
					EditorColor ec={sizeof(ec)};
					ec.StringNumber = ecA->StringNumber;
					ec.StartPos = ecA->StartPos;
					ec.EndPos = ecA->EndPos;
					ec.Color = colors::ConsoleColorToFarColor(ecA->Color);
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
						ecA->Color = colors::FarColorToConsoleColor(ec.Color);
						if(ec.Color.Flags&ECF_TABMARKFIRST) ecA->Color|=oldfar::ECF_TAB1;
					}
					return Result;
				}
				return FALSE;

			case oldfar::ECTL_GETSTRING:
			{
				EditorGetString egs={sizeof(EditorGetString)};
				const auto oegs = static_cast<oldfar::EditorGetString*>(Param);

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
					geol = UnicodeToAnsiBin(egs.StringEOL, wcslen(egs.StringEOL), CodePage);
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
				const auto oei = static_cast<oldfar::EditorInfo*>(Param);

				if (!oei)
					return FALSE;

				int ret=static_cast<int>(NativeInfo.EditorControl(-1,ECTL_GETINFO,0,&ei));

				if (ret)
				{
					delete[] fn;
					ClearStruct(*oei);
					size_t FileNameSize = NativeInfo.EditorControl(-1, ECTL_GETFILENAME, 0, nullptr);

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

				const auto ect = static_cast<const oldfar::EditorConvertText*>(Param);
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
					const auto oldsf = static_cast<const oldfar::EditorSaveFile*>(Param);
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

				return static_cast<int>(NativeInfo.EditorControl(-1, ECTL_SAVEFILE, 0, Param? &newsf : nullptr));
			}
			case oldfar::ECTL_PROCESSINPUT:	//BUGBUG?
			{
				if (Param)
				{
					const auto pIR = static_cast<INPUT_RECORD*>(Param);

					switch (pIR->EventType)
					{
						case KEY_EVENT:
						{
							wchar_t res;
							if (unicode::from(CP_OEMCP, &pIR->Event.KeyEvent.uChar.AsciiChar, 1, &res, 1))
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
					const auto pIR = static_cast<INPUT_RECORD*>(Param);

					switch (pIR->EventType)
					{
						case KEY_EVENT:
						{
							char res;
							if (unicode::to(CP_OEMCP, &pIR->Event.KeyEvent.uChar.UnicodeChar, 1, &res, 1))
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
						const auto oldkbt = static_cast<const oldfar::KeyBarTitles*>(Param);
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
					const auto oldsp = static_cast<const oldfar::EditorSetParameter*>(Param);
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

				return static_cast<int>(NativeInfo.EditorControl(-1, ECTL_SETPARAM, 0, Param? &newsp : nullptr));
			}
			case oldfar::ECTL_SETSTRING:
			{
				EditorSetString newss = {sizeof(EditorSetString)};

				if (Param)
				{
					const auto oldss = static_cast<const oldfar::EditorSetString*>(Param);
					newss.StringNumber=oldss->StringNumber;
					uintptr_t CodePage=GetEditorCodePageA();
					newss.StringText=(oldss->StringText)?AnsiToUnicodeBin(oldss->StringText, oldss->StringLength,CodePage):nullptr;
					newss.StringEOL=(oldss->StringEOL && *oldss->StringEOL)?AnsiToUnicodeBin(oldss->StringEOL,strlen(oldss->StringEOL),CodePage):nullptr;
					newss.StringLength=oldss->StringLength;
				}

				int ret = static_cast<int>(NativeInfo.EditorControl(-1, ECTL_SETSTRING, 0, Param? &newss : nullptr));

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
					et = *static_cast<int*>(Param);
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
				const auto oldbm = static_cast<const oldfar::EditorBookMarks*>(Param);
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
				const auto oldecp = static_cast<oldfar::EditorConvertPos*>(Param);
				EditorConvertPos newecp={sizeof(EditorConvertPos),oldecp->StringNumber,oldecp->SrcPos,oldecp->DestPos};
				int ret=static_cast<int>(NativeInfo.EditorControl(-1, OldCommand == oldfar::ECTL_REALTOTAB ? ECTL_REALTOTAB : ECTL_TABTOREAL, 0, &newecp));
				oldecp->DestPos=newecp.DestPos;
				return ret;
			}
			case oldfar::ECTL_SELECT:
			{
				const auto oldes = static_cast<const oldfar::EditorSelect*>(Param);
				EditorSelect newes={sizeof(EditorSelect),oldes->BlockType,oldes->BlockStartLine,oldes->BlockStartPos,oldes->BlockWidth,oldes->BlockHeight};
				return static_cast<int>(NativeInfo.EditorControl(-1, ECTL_SELECT, 0, &newes));
			}
			case oldfar::ECTL_REDRAW:				Command = ECTL_REDRAW; break;
			case oldfar::ECTL_SETPOSITION:
			{
				const auto oldsp = static_cast<const oldfar::EditorSetPosition*>(Param);
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

				const auto viA = static_cast<oldfar::ViewerInfo*>(Param);

				if (!viA->StructSize) return FALSE;

				ViewerInfo viW = {sizeof(viW)};

				if (NativeInfo.ViewerControl(-1,VCTL_GETINFO, 0, &viW) == FALSE) return FALSE;

				viA->ViewerID = viW.ViewerID;

				delete[] filename;

				size_t FileNameSize = NativeInfo.ViewerControl(-1, VCTL_GETFILENAME, 0, nullptr);

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
				return static_cast<int>(NativeInfo.ViewerControl(-1, VCTL_QUIT, 0, nullptr));
			case oldfar::VCTL_REDRAW:
				return static_cast<int>(NativeInfo.ViewerControl(-1, VCTL_REDRAW, 0, nullptr));
			case oldfar::VCTL_SETKEYBAR:
			{
				switch ((intptr_t)Param)
				{
					case 0:
					case -1:
						return static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_SETKEYBAR,0, Param));
					default:
						const auto kbtA = static_cast<const oldfar::KeyBarTitles*>(Param);
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

				const auto vspA = static_cast<oldfar::ViewerSetPosition*>(Param);
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
				if (!Param) return static_cast<int>(NativeInfo.ViewerControl(-1, VCTL_SELECT, 0, nullptr));

				const auto vsA = static_cast<const oldfar::ViewerSelect*>(Param);
				ViewerSelect vs = {sizeof(ViewerSelect),vsA->BlockStartPos,vsA->BlockLen};
				return static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_SELECT,0, &vs));
			}
			case oldfar::VCTL_SETMODE:
			{
				if (!Param) return FALSE;

				const auto vsmA = static_cast<const oldfar::ViewerSetMode*>(Param);
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

			const auto TableSet = reinterpret_cast<oldfar::CharTableSet*>(Buffer);
			//Preset. Also if Command != FCT_DETECT and failed, buffer must be filled by OEM data.
			strcpy(TableSet->TableName,"<failed>");

			for (unsigned int i = 0; i < 256; ++i)
			{
				TableSet->EncodeTable[i] = TableSet->DecodeTable[i] = i;
				TableSet->UpperTable[i] = LocalUpper(i);
				TableSet->LowerTable[i] = LocalLower(i);
			}

			const auto nCP = ConvertCharTableToCodePage(Command);

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
			UnicodeToOEM(sTableName.data(), TableSet->TableName, std::size(TableSet->TableName) - 1);
			std::unique_ptr<wchar_t[]> us(AnsiToUnicodeBin((char*)TableSet->DecodeTable, sizeof(TableSet->DecodeTable), nCP));
			CharLowerBuff(us.get(), sizeof(TableSet->DecodeTable));
			unicode::to(nCP, us.get(), sizeof(TableSet->DecodeTable), reinterpret_cast<char*>(TableSet->LowerTable), sizeof(TableSet->DecodeTable));
			CharUpperBuff(us.get(), sizeof(TableSet->DecodeTable));
			unicode::to(nCP, us.get(), sizeof(TableSet->DecodeTable), reinterpret_cast<char*>(TableSet->UpperTable), sizeof(TableSet->DecodeTable));
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
		int Ret = static_cast<int>(NativeFSF.GetFileOwner(wide(Computer).data(), wide(Name).data(), wOwner, std::size(wOwner)));
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

class file_version: noncopyable
{
public:
	file_version(const string& File): m_File(File) {}

	bool Read()
	{
		if (const auto Size = GetFileVersionInfoSize(m_File.data(), nullptr))
		{
			m_Buffer.reset(Size);
			if (GetFileVersionInfo(m_File.data(), 0, Size, m_Buffer.get()))
			{
				if (const auto Translation = GetValue<DWORD>(L"\\VarFileInfo\\Translation"))
				{
					std::wostringstream tmp;
					tmp << std::hex << std::setw(4) << std::setfill(L'0') << LOWORD(*Translation)
					    << std::hex << std::setw(4) << std::setfill(L'0') << HIWORD(*Translation);
					m_BlockPath = L"\\StringFileInfo\\" + tmp.str() + L"\\";
					return true;
				}
			}
		}
		return false;
	}

	const wchar_t* GetStringValue(const string& value) const
	{
		return GetValue<wchar_t>((m_BlockPath + value).data());
	}

	const VS_FIXEDFILEINFO* GetFixedInfo() const
	{
		return GetValue<VS_FIXEDFILEINFO>(L"\\");
	}

private:
	template<class T>
	const T* GetValue(const wchar_t* SubBlock) const
	{
		UINT Length;
		T* Result;
		return VerQueryValue(m_Buffer.get(), SubBlock, reinterpret_cast<void**>(&Result), &Length) && Length? Result : nullptr;
	}

	string m_File;
	string m_BlockPath;
	wchar_t_ptr m_Buffer;
};


static int SendKeyToPluginHook(const Manager::Key& key)
{
	DWORD KeyM = (key()&(~KEY_CTRLMASK));

	if (!((KeyM >= KEY_MACRO_BASE && KeyM <= KEY_MACRO_ENDBASE) || (KeyM >= KEY_OP_BASE && KeyM <= KEY_OP_ENDBASE))) // пропустим макро-коды
	{
		if (Global->WindowManager->IsPanelsActive())
		{
			if (Global->CtrlObject->Cp()->ActivePanel()->GetMode() == panel_mode::PLUGIN_PANEL)
			{
				const auto ph = Global->CtrlObject->Cp()->ActivePanel()->GetPluginHandle();
				if (ph && ph->pPlugin->IsOemPlugin() && Global->CtrlObject->Cp()->ActivePanel()->SendKeyToPlugin(key(), true))
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


PluginA::PluginA(plugin_factory* Factory, const string& ModuleName):
	Plugin(Factory, ModuleName),
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
	wrapper::pluginapi::FarRestoreScreenA,
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
		if ((Value = FileVersion->GetStringValue(L"InternalName")) != nullptr || (Value = FileVersion->GetStringValue(L"OriginalName")) != nullptr)
		{
			Info->Title = Value;
		}

		if ((Value = FileVersion->GetStringValue(L"CompanyName")) != nullptr || (Value = FileVersion->GetStringValue(L"LegalCopyright")) != nullptr)
		{
			Info->Author = Value;
		}

		if (const auto Description = FileVersion->GetStringValue(L"FileDescription"))
		{
			Info->Description = Description;
		}

		if (const auto Uuid = FileVersion->GetStringValue(L"PluginGUID"))
		{
			if (UuidFromString(reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(Uuid)), &PluginGuid) == RPC_S_OK)
				GuidFound = true;
		}

		if (const auto FileInfo = FileVersion->GetFixedInfo())
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
	if (has(es.id) && !Global->ProcessException)
	{
		auto InfoCopy = StartupInfo;
		auto FsfCopy = StandardFunctions;
		// скорректируем адреса и плагино-зависимые поля
		InfoCopy.ModuleNumber = reinterpret_cast<intptr_t>(this);
		InfoCopy.FSF = &FsfCopy;
		UnicodeToOEM(GetModuleName().data(), InfoCopy.ModuleName);
		InfoCopy.RootKey = static_cast<oem_plugin_factory*>(m_Factory)->getUserName().data();

		if (Global->strRegUser.empty())
			os::env::delete_variable(L"FARUSER");
		else
			os::env::set_variable(L"FARUSER", Global->strRegUser);

		ExecuteFunction<iSetStartupInfo>(es, &InfoCopy);

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
	if (has(es.id) && !Global->ProcessException)
	{
		ExecuteFunction<iGetMinFarVersion>(es);
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
	if (hResult == ToPtr(-2))
		return static_cast<void*>(PANEL_STOP);
	return hResult;
}

void* PluginA::Open(OpenInfo* Info)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);

	CheckScreenLock();

	ExecuteStruct es = {iOpen};

	if (Load() && has(es.id) && !Global->ProcessException)
	{
		std::unique_ptr<char[]> Buffer;
		intptr_t Ptr = 0;

		int OpenFromA = oldfar::OPEN_PLUGINSMENU;

		oldfar::OpenDlgPluginData DlgData;

		switch (Info->OpenFrom)
		{
		case OPEN_COMMANDLINE:
			OpenFromA = oldfar::OPEN_COMMANDLINE;
			if (Info->Data)
			{
				Buffer.reset(UnicodeToAnsi(reinterpret_cast<const OpenCommandLineInfo*>(Info->Data)->CommandLine));
				Ptr = reinterpret_cast<intptr_t>(Buffer.get());
			}
			break;

		case OPEN_SHORTCUT:
			OpenFromA = oldfar::OPEN_SHORTCUT;
			if (Info->Data)
			{
				const auto SInfo = reinterpret_cast<const OpenShortcutInfo*>(Info->Data);
				const auto shortcutdata = SInfo->ShortcutData? SInfo->ShortcutData : SInfo->HostFile;
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
			OpenFromA = oldfar::OPEN_FROMMACRO|Global->CtrlObject->Macro.GetArea();
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

		ExecuteFunction<iOpen>(es, OpenFromA, Ptr);
	}

	return TranslateResult(es);
}

void* PluginA::OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, size_t DataSize, int OpMode)
{
	ExecuteStruct es = {iOpenFilePlugin};
	if (Load() && has(es.id) && !Global->ProcessException)
	{
		std::unique_ptr<char[]> NameA(Name? UnicodeToAnsi(Name) : nullptr);
		ExecuteFunction<iOpenFilePlugin>(es, NameA.get(), Data, static_cast<int>(DataSize));
	}
	return TranslateResult(es);
}

int PluginA::SetFindList(SetFindListInfo* Info)
{
	ExecuteStruct es = {iSetFindList};
	if (has(es.id) && !Global->ProcessException)
	{
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(Info->PanelItem, PanelItemA, Info->ItemsNumber);
		ExecuteFunction<iSetFindList>(es, Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber));
		FreePanelItemA(PanelItemA, Info->ItemsNumber);
	}
	return es;
}

int PluginA::ProcessEditorInput(ProcessEditorInputInfo* Info)
{
	ExecuteStruct es = {iProcessEditorInput};
	if (Load() && has(es.id) && !Global->ProcessException)
	{
		const INPUT_RECORD *Ptr = &Info->Rec;
		INPUT_RECORD OemRecord;
		if (Ptr->EventType==KEY_EVENT)
		{
			OemRecord = Info->Rec;
			CharToOemBuff(&Info->Rec.Event.KeyEvent.uChar.UnicodeChar,&OemRecord.Event.KeyEvent.uChar.AsciiChar,1);
			Ptr=&OemRecord;
		}
		ExecuteFunction<iProcessEditorInput>(es, Ptr);
	}
	return es;
}

int PluginA::ProcessEditorEvent(ProcessEditorEventInfo* Info)
{
	ExecuteStruct es = {iProcessEditorEvent};
	if (Load() && has(es.id) && !Global->ProcessException)
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
				ExecuteFunction<iProcessEditorEvent>(es, Info->Event, Info->Param);
				break;
		}
	}
	return es;
}

int PluginA::ProcessViewerEvent(ProcessViewerEventInfo* Info)
{
	ExecuteStruct es = {iProcessViewerEvent};
	if (Load() && has(es.id) && !Global->ProcessException)
	{
		switch(Info->Event)
		{
			case VE_CLOSE:
			case VE_GOTFOCUS:
			case VE_KILLFOCUS:
				Info->Param = &Info->ViewerID;
			case VE_READ:
				ExecuteFunction<iProcessViewerEvent>(es, Info->Event, Info->Param);
				break;
		}
	}
	return es;
}

int PluginA::ProcessDialogEvent(ProcessDialogEventInfo* Info)
{
	ExecuteStruct es = {iProcessDialogEvent};
	if (Load() && has(es.id) && !Global->ProcessException)
	{
		ExecuteFunction<iProcessDialogEvent>(es, Info->Event, Info->Param);
	}
	return es;
}

int PluginA::GetVirtualFindData(GetVirtualFindDataInfo* Info)
{
	ExecuteStruct es = {iGetVirtualFindData};
	if (has(es.id) && !Global->ProcessException)
	{
		pVFDPanelItemA = nullptr;
		size_t Size = wcslen(Info->Path) + 1;
		char_ptr PathA(Size);
		UnicodeToOEM(Info->Path, PathA.get(), Size);
		int ItemsNumber = 0;
		ExecuteFunction<iGetVirtualFindData>(es, Info->hPanel, &pVFDPanelItemA, &ItemsNumber, PathA.get());
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
	if (has(es.id) && !Global->ProcessException && pVFDPanelItemA)
	{
		ExecuteFunction<iFreeVirtualFindData>(es, Info->hPanel, pVFDPanelItemA, static_cast<int>(Info->ItemsNumber));
		pVFDPanelItemA = nullptr;
	}
}

int PluginA::GetFiles(GetFilesInfo* Info)
{
	ExecuteStruct es = {iGetFiles, -1};
	if (has(es.id) && !Global->ProcessException)
	{
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(Info->PanelItem, PanelItemA, Info->ItemsNumber);
		char DestA[oldfar::NM];
		UnicodeToOEM(Info->DestPath, DestA);
		ExecuteFunction<iGetFiles>(es, Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber), Info->Move, DestA, Info->OpMode);
		static wchar_t DestW[oldfar::NM];
		OEMToUnicode(DestA, DestW);
		Info->DestPath=DestW;
		FreePanelItemA(PanelItemA, Info->ItemsNumber);
	}
	return es;
}

int PluginA::PutFiles(PutFilesInfo* Info)
{
	ExecuteStruct es = {iPutFiles, -1};
	if (has(es.id) && !Global->ProcessException)
	{
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(Info->PanelItem, PanelItemA, Info->ItemsNumber);
		ExecuteFunction<iPutFiles>(es, Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber), Info->Move, Info->OpMode);
		FreePanelItemA(PanelItemA, Info->ItemsNumber);
	}
	return es;
}

int PluginA::DeleteFiles(DeleteFilesInfo* Info)
{
	ExecuteStruct es = {iDeleteFiles};
	if (has(es.id) && !Global->ProcessException)
	{
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(Info->PanelItem, PanelItemA, Info->ItemsNumber);
		ExecuteFunction<iDeleteFiles>(es, Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber), Info->OpMode);
		FreePanelItemA(PanelItemA, Info->ItemsNumber);
	}
	return es;
}

int PluginA::MakeDirectory(MakeDirectoryInfo* Info)
{
	ExecuteStruct es = {iMakeDirectory, -1};
	if (has(es.id) && !Global->ProcessException)
	{
		char NameA[oldfar::NM];
		UnicodeToOEM(Info->Name, NameA);
		ExecuteFunction<iMakeDirectory>(es, Info->hPanel, NameA, Info->OpMode);
		static wchar_t NameW[oldfar::NM];
		OEMToUnicode(NameA, NameW);
		Info->Name=NameW;
	}
	return es;
}

int PluginA::ProcessHostFile(ProcessHostFileInfo* Info)
{
	ExecuteStruct es = {iProcessHostFile};
	if (has(es.id) && !Global->ProcessException)
	{
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(Info->PanelItem, PanelItemA, Info->ItemsNumber);
		ExecuteFunction<iProcessHostFile>(es, Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber), Info->OpMode);
		FreePanelItemA(PanelItemA, Info->ItemsNumber);
	}
	return es;
}

int PluginA::ProcessPanelEvent(ProcessPanelEventInfo* Info)
{
	ExecuteStruct es = {iProcessPanelEvent};
	if (has(es.id) && !Global->ProcessException)
	{
		PVOID ParamA = Info->Param;

		if (Info->Param && (Info->Event == FE_COMMAND || Info->Event == FE_CHANGEVIEWMODE))
			ParamA = UnicodeToAnsi((const wchar_t *)Info->Param);

		ExecuteFunction<iProcessPanelEvent>(es, Info->hPanel, Info->Event, ParamA);

		if (ParamA && (Info->Event == FE_COMMAND || Info->Event == FE_CHANGEVIEWMODE))
			delete[] static_cast<char*>(ParamA);
	}
	return es;
}

int PluginA::Compare(CompareInfo* Info)
{
	ExecuteStruct es = {iCompare, -2};
	if (has(es.id) && !Global->ProcessException)
	{
		oldfar::PluginPanelItem *Item1A = nullptr;
		oldfar::PluginPanelItem *Item2A = nullptr;
		ConvertPanelItemsArrayToAnsi(Info->Item1, Item1A, 1);
		ConvertPanelItemsArrayToAnsi(Info->Item2, Item2A, 1);
		ExecuteFunction<iCompare>(es, Info->hPanel, Item1A, Item2A, Info->Mode);
		FreePanelItemA(Item1A,1);
		FreePanelItemA(Item2A,1);
	}
	return es;
}

int PluginA::GetFindData(GetFindDataInfo* Info)
{
	ExecuteStruct es = {iGetFindData};
	if (has(es.id) && !Global->ProcessException)
	{
		pFDPanelItemA = nullptr;
		int ItemsNumber = 0;
		//BUGBUG, translate OpMode
		ExecuteFunction<iGetFindData>(es, Info->hPanel, &pFDPanelItemA, &ItemsNumber, Info->OpMode);

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
	if (has(es.id) && !Global->ProcessException && pFDPanelItemA)
	{
		ExecuteFunction<iFreeFindData>(es, Info->hPanel, pFDPanelItemA, static_cast<int>(Info->ItemsNumber));
		pFDPanelItemA = nullptr;
	}
}

int PluginA::ProcessPanelInput(ProcessPanelInputInfo* Info)
{
	ExecuteStruct es = {iProcessPanelInput};
	if (has(es.id) && !Global->ProcessException)
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

		ExecuteFunction<iProcessPanelInput>(es, Info->hPanel, VirtKey|(Prepocess?PKF_PREPROCESS:0), dwControlState);
	}
	return es;
}

void PluginA::ClosePanel(ClosePanelInfo* Info)
{
	ExecuteStruct es = {iClosePanel};
	if (has(es.id) && !Global->ProcessException)
	{
		ExecuteFunction<iClosePanel>(es, Info->hPanel);
	}
	FreeOpenPanelInfo();
}

int PluginA::SetDirectory(SetDirectoryInfo* Info)
{
	ExecuteStruct es = {iSetDirectory};
	if (has(es.id) && !Global->ProcessException)
	{
		char *DirA = UnicodeToAnsi(Info->Dir);
		ExecuteFunction<iSetDirectory>(es, Info->hPanel, DirA, Info->OpMode);
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
	FreeUnicodeInfoPanelLines(OPI.InfoLines, OPI.InfoLinesNumber);
	DeleteRawArray(OPI.DescrFiles, OPI.DescrFilesNumber);
	FreeUnicodePanelModes(OPI.PanelModesArray, OPI.PanelModesNumber);
	FreeUnicodeKeyBarTitles(const_cast<KeyBarTitles*>(OPI.KeyBar));
	delete OPI.KeyBar;
	delete[] OPI.ShortcutData;
	ClearStruct(OPI);
}

void PluginA::ConvertOpenPanelInfo(const oldfar::OpenPanelInfo &Src, OpenPanelInfo *Dest)
{
	FreeOpenPanelInfo();
	OPI.StructSize = sizeof(OPI);
	OPI.Flags = OPIF_NONE;

	static const std::pair<oldfar::OPENPANELINFO_FLAGS, OPENPANELINFO_FLAGS> PanelInfoFlagsMap[] =
	{
		OLDFAR_TO_FAR_MAP(OPIF_ADDDOTS),
		OLDFAR_TO_FAR_MAP(OPIF_RAWSELECTION),
		OLDFAR_TO_FAR_MAP(OPIF_REALNAMES),
		OLDFAR_TO_FAR_MAP(OPIF_SHOWNAMESONLY),
		OLDFAR_TO_FAR_MAP(OPIF_SHOWRIGHTALIGNNAMES),
		OLDFAR_TO_FAR_MAP(OPIF_SHOWPRESERVECASE),
		OLDFAR_TO_FAR_MAP(OPIF_COMPAREFATTIME),
		OLDFAR_TO_FAR_MAP(OPIF_EXTERNALGET),
		OLDFAR_TO_FAR_MAP(OPIF_EXTERNALPUT),
		OLDFAR_TO_FAR_MAP(OPIF_EXTERNALDELETE),
		OLDFAR_TO_FAR_MAP(OPIF_EXTERNALMKDIR),
		OLDFAR_TO_FAR_MAP(OPIF_USEATTRHIGHLIGHTING),
	};
	FirstFlagsToSecond(Src.Flags, OPI.Flags, PanelInfoFlagsMap);

	if (!(Src.Flags&oldfar::OPIF_USEFILTER)) OPI.Flags |= OPIF_DISABLEFILTER;
	if (!(Src.Flags&oldfar::OPIF_USESORTGROUPS)) OPI.Flags |= OPIF_DISABLESORTGROUPS;
	if (!(Src.Flags&oldfar::OPIF_USEHIGHLIGHTING)) OPI.Flags |= OPIF_DISABLEHIGHLIGHTING;
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
		OPI.DescrFiles = AnsiArrayToUnicode(Src.DescrFiles, Src.DescrFilesNumber);
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
	if (has(es.id) && !Global->ProcessException)
	{
		oldfar::OpenPanelInfo InfoA={};
		ExecuteFunction<iGetOpenPanelInfo>(es, pInfo->hPanel, &InfoA);
		ConvertOpenPanelInfo(InfoA,pInfo);
	}
}

int PluginA::Configure(ConfigureInfo* Info)
{
	ExecuteStruct es = {iConfigure};
	if (Load() && has(es.id) && !Global->ProcessException)
	{
		ExecuteFunction<iConfigure>(es, Info->Guid->Data1);
	}
	return es;
}

void PluginA::FreePluginInfo()
{
	const auto DeleteItems = [](const PluginMenuItem& Item)
	{
		if (Item.Count)
		{
			for (size_t i = 0; i < Item.Count; i++)
				delete[] Item.Strings[i];

			delete[] Item.Guids;
			delete[] Item.Strings;
		}
	};

	DeleteItems(PI.DiskMenu);
	DeleteItems(PI.PluginMenu);
	DeleteItems(PI.PluginConfig);

	delete[] PI.CommandPrefix;

	ClearStruct(PI);
}

void PluginA::ConvertPluginInfo(const oldfar::PluginInfo &Src, PluginInfo *Dest)
{
	FreePluginInfo();
	PI.StructSize = sizeof(PI);

	static const std::pair<oldfar::PLUGIN_FLAGS, PLUGIN_FLAGS> PluginFlagsMap[] =
	{
		OLDFAR_TO_FAR_MAP(PF_PRELOAD),
		OLDFAR_TO_FAR_MAP(PF_DISABLEPANELS),
		OLDFAR_TO_FAR_MAP(PF_EDITOR),
		OLDFAR_TO_FAR_MAP(PF_VIEWER),
		OLDFAR_TO_FAR_MAP(PF_DIALOG),
		OLDFAR_TO_FAR_MAP(PF_FULLCMDLINE),
	};

	PI.Flags = PF_NONE;
	FirstFlagsToSecond(Src.Flags, PI.Flags, PluginFlagsMap);

	const auto CreatePluginMenuItems = [](const char* const* Strings, size_t Size, PluginMenuItem& Item)
	{
		if (Size)
		{
			const auto p = new wchar_t*[Size];
			const auto guid = new GUID[Size]();

			for (size_t i = 0; i != Size; ++i)
			{
				p[i] = AnsiToUnicode(Strings[i]);
				guid[i].Data1 = static_cast<decltype(guid[i].Data1)>(i);
			}

			Item.Guids = guid;
			Item.Strings = p;
			Item.Count = Size;
		}

	};

	#define CREATE_ITEMS(ItemsType) CreatePluginMenuItems(Src.ItemsType##Strings, Src.ItemsType##StringsNumber, PI.ItemsType);

	CREATE_ITEMS(DiskMenu);
	CREATE_ITEMS(PluginMenu);
	CREATE_ITEMS(PluginConfig);

	#undef CREATE_ITEMS

	PI.CommandPrefix = AnsiToUnicode(Src.CommandPrefix);

	*Dest=PI;
}

bool PluginA::GetPluginInfo(PluginInfo *pi)
{
	ClearStruct(*pi);

	ExecuteStruct es = {iGetPluginInfo};
	if (has(es.id) && !Global->ProcessException)
	{
		oldfar::PluginInfo InfoA={sizeof(InfoA)};
		ExecuteFunction<iGetPluginInfo>(es, &InfoA);

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
	if (has(es.id) && !Global->ProcessException)
	{
		// ExitInfo ignored for ansi plugins
		ExecuteFunction<iExitFAR>(es);
	}
}

class AnsiLanguage: public Language
{
public:
	// Don't add Language(Path) to initialiser list and don't move init() to the Language ctor - it calls virtual functions.
	AnsiLanguage(const string& Path) { init(Path); }
	const char* GetMsgA(LNGID nID) const { return CheckMsgId(nID)? m_AnsiMessages[nID].data() : ""; }

private:
	virtual size_t size() const override { return m_AnsiMessages.size(); }
	virtual void reserve(size_t size) override { m_AnsiMessages.reserve(size); }
	virtual void add(string&& str) override { m_AnsiMessages.emplace_back(narrow(str)); }

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
	return static_cast<const AnsiLanguage&>(*PluginLang.get()).GetMsgA(nID);
}

};

#endif // NO_WRAPPER
