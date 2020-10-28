/*
PluginA.cpp
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "PluginA.hpp"

#ifndef NO_WRAPPER

// Internal:
#include "plugins.hpp"
#include "encoding.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "panel.hpp"
#include "plclass.hpp"
#include "keyboard.hpp"
#include "interf.hpp"
#include "pathmix.hpp"
#include "mix.hpp"
#include "colormix.hpp"
#include "uuids.far.hpp"
#include "keys.hpp"
#include "language.hpp"
#include "filepanels.hpp"
#include "strmix.hpp"
#include "pluginold.hpp"
#include "string_utils.hpp"
#include "cvtname.hpp"
#include "message.hpp"
#include "lang.hpp"
#include "global.hpp"
#include "plugapi.hpp"
#include "exception_handler.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.memory.hpp"
#include "platform.version.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/function_ref.hpp"
#include "common/null_iterator.hpp"
#include "common/scope_exit.hpp"
#include "common/uuid.hpp"
#include "common/view/select.hpp"
#include "common/view/select.hpp"
#include "common/view/zip.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

#define OLDFAR_TO_FAR_MAP(x) std::pair{ oldfar::x, x }

#define DECLARE_PLUGIN_FUNCTION(name, signature) DECLARE_GEN_PLUGIN_FUNCTION(name, false, signature)

DECLARE_PLUGIN_FUNCTION(iClosePanel,          void   (WINAPI*)(HANDLE hPlugin))
DECLARE_PLUGIN_FUNCTION(iCompare,             int    (WINAPI*)(HANDLE hPlugin, const oldfar::PluginPanelItem *Item1, const oldfar::PluginPanelItem *Item2, unsigned int Mode))
DECLARE_PLUGIN_FUNCTION(iConfigure,           int    (WINAPI*)(int ItemNumber))
DECLARE_PLUGIN_FUNCTION(iDeleteFiles,         int    (WINAPI*)(HANDLE hPlugin, oldfar::PluginPanelItem *PanelItem, int ItemsNumber, int OpMode))
DECLARE_PLUGIN_FUNCTION(iExitFAR,             void   (WINAPI*)())
DECLARE_PLUGIN_FUNCTION(iFreeFindData,        void   (WINAPI*)(HANDLE hPlugin, oldfar::PluginPanelItem *PanelItem, int ItemsNumber))
DECLARE_PLUGIN_FUNCTION(iFreeVirtualFindData, void   (WINAPI*)(HANDLE hPlugin, oldfar::PluginPanelItem *PanelItem, int ItemsNumber))
DECLARE_PLUGIN_FUNCTION(iGetFiles,            int    (WINAPI*)(HANDLE hPlugin, oldfar::PluginPanelItem *PanelItem, int ItemsNumber, int Move, char *DestPath, int OpMode))
DECLARE_PLUGIN_FUNCTION(iGetFindData,         int    (WINAPI*)(HANDLE hPlugin, oldfar::PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode))
DECLARE_PLUGIN_FUNCTION(iGetMinFarVersion,    int    (WINAPI*)())
DECLARE_PLUGIN_FUNCTION(iGetOpenPanelInfo,    void   (WINAPI*)(HANDLE hPlugin, oldfar::OpenPanelInfo *Info))
DECLARE_PLUGIN_FUNCTION(iGetPluginInfo,       void   (WINAPI*)(oldfar::PluginInfo *Info))
DECLARE_PLUGIN_FUNCTION(iGetVirtualFindData,  int    (WINAPI*)(HANDLE hPlugin, oldfar::PluginPanelItem **pPanelItem, int *pItemsNumber, const char *Path))
DECLARE_PLUGIN_FUNCTION(iMakeDirectory,       int    (WINAPI*)(HANDLE hPlugin, char *Name, int OpMode))
DECLARE_PLUGIN_FUNCTION(iOpenFilePlugin,      HANDLE (WINAPI*)(char *Name, const unsigned char *Data, int DataSize))
DECLARE_PLUGIN_FUNCTION(iOpen,                HANDLE (WINAPI*)(int OpenFrom, intptr_t Item))
DECLARE_PLUGIN_FUNCTION(iProcessEditorEvent,  int    (WINAPI*)(int Event, void *Param))
DECLARE_PLUGIN_FUNCTION(iProcessEditorInput,  int    (WINAPI*)(const INPUT_RECORD *Rec))
DECLARE_PLUGIN_FUNCTION(iProcessPanelEvent,   int    (WINAPI*)(HANDLE hPlugin, int Event, void *Param))
DECLARE_PLUGIN_FUNCTION(iProcessHostFile,     int    (WINAPI*)(HANDLE hPlugin, oldfar::PluginPanelItem *PanelItem, int ItemsNumber, int OpMode))
DECLARE_PLUGIN_FUNCTION(iProcessPanelInput,   int    (WINAPI*)(HANDLE hPlugin, int Key, unsigned int ControlState))
DECLARE_PLUGIN_FUNCTION(iPutFiles,            int    (WINAPI*)(HANDLE hPlugin, oldfar::PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode))
DECLARE_PLUGIN_FUNCTION(iSetDirectory,        int    (WINAPI*)(HANDLE hPlugin, const char *Dir, int OpMode))
DECLARE_PLUGIN_FUNCTION(iSetFindList,         int    (WINAPI*)(HANDLE hPlugin, const oldfar::PluginPanelItem *PanelItem, int ItemsNumber))
DECLARE_PLUGIN_FUNCTION(iSetStartupInfo,      void   (WINAPI*)(const oldfar::PluginStartupInfo *Info))
DECLARE_PLUGIN_FUNCTION(iProcessViewerEvent,  int    (WINAPI*)(int Event, void *Param))
DECLARE_PLUGIN_FUNCTION(iProcessDialogEvent,  int    (WINAPI*)(int Event, void *Param))

#undef DECLARE_PLUGIN_FUNCTION

class oem_plugin_module: public native_plugin_module
{
public:
	NONCOPYABLE(oem_plugin_module);

	explicit oem_plugin_module(const string& Name):
		native_plugin_module(Name)
	{
	}

	os::version::file_version m_FileVersion;
};

class oem_plugin_factory: public native_plugin_factory
{
public:
	NONCOPYABLE(oem_plugin_factory);

	explicit oem_plugin_factory(PluginManager* Owner):
		native_plugin_factory(Owner)
	{
		static const export_name ExportsNames[]
		{
			{}, // GetGlobalInfo not used
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
			{}, // ProcessSynchroEvent not used
			{}, // ProcessConsoleEvent not used
			{}, // Analyse not used
			{}, // CloseAnalyse not used
			{}, // GetContentFields not used
			{}, // GetContentData not used
			{}, // FreeContentData not used

			WA("OpenFilePlugin"),
			WA("GetMinFarVersion"),
		};
		static_assert(std::size(ExportsNames) == ExportsCount);
		m_ExportsNames = ExportsNames;
	}

	plugin_module_ptr Create(const string& filename) override
	{
		auto Module = std::make_unique<oem_plugin_module>(filename);
		if (!*Module)
		{
			const auto ErrorState = error_state::fetch();

			Message(MSG_WARNING | MSG_NOPLUGINS, ErrorState,
				msg(lng::MError),
				{
					msg(lng::MPlgLoadPluginError),
					filename
				},
				{ lng::MOk },
				L"ErrLoadPlugin"sv);

			Module.reset();
		}
		return Module;
	}

	std::unique_ptr<Plugin> CreatePlugin(const string& filename) override;

	const std::string& PluginsRootKey()
	{
		if (m_userName.empty())
		{
			m_userName = "Software\\Far Manager"sv;
			if (!Global->strRegUser.empty())
			{
				m_userName.append("\\Users\\"sv).append(encoding::oem::get_bytes(Global->strRegUser));
			}
			m_userName += "\\Plugins"sv;
		}
		return m_userName;
	}

private:
	bool FindExport(const std::string_view ExportName) const override
	{
		// module with ANY known export can be OEM plugin
		return contains(select(m_ExportsNames, [](const export_name& Item) { return Item.AName; }), ExportName);
	}

	std::string m_userName;
};

plugin_factory_ptr CreateOemPluginFactory(PluginManager* Owner)
{
	return std::make_unique<oem_plugin_factory>(Owner);
}

static int IsSpaceA(int x) { return x==' '  || x=='\t'; }
static int IsEolA(int x)   { return x=='\r' || x=='\n'; }
static int IsSlashA(int x) { return x=='\\' || x=='/'; }

static char LowerToUpper[256];
static char UpperToLower[256];
static char UpperOrLower[256];

enum char_case: char
{
	case_none,
	case_lower,
	case_upper,
};

static void LocalUpperInit()
{
	[[maybe_unused]]
	static const auto InitOnce = []
	{
		const auto to_oem   = [](char Char) { CharToOemBuffA(&Char, &Char, 1); return Char; };
		const auto to_ansi  = [](char Char) { OemToCharBuffA(&Char, &Char, 1); return Char; };
		const auto to_upper = [](char Char) { CharUpperBuffA(&Char, 1); return Char; };
		const auto to_lower = [](char Char) { CharLowerBuffA(&Char, 1); return Char; };

		for (size_t I = 0; I != std::size(LowerToUpper); ++I)
		{
			const auto Char = to_ansi(char(I));

			if (IsCharAlphaA(Char) && to_oem(Char) == char(I))
			{
				if (IsCharLowerA(Char))
				{
					UpperOrLower[I] = case_lower;
					LowerToUpper[I] = to_oem(to_upper(Char));
					UpperToLower[I] = char(I);
					continue;
				}

				if (IsCharUpperA(Char))
				{
					UpperOrLower[I] = case_upper;
					LowerToUpper[I] = char(I);
					UpperToLower[I] = to_oem(to_lower(Char));
					continue;
				}
			}

			UpperOrLower[I] = case_none;
			LowerToUpper[I] = UpperToLower[I] = char(I);
		}
		return true;
	}();
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
		if (const auto Result = UpperToLower[static_cast<unsigned>(*s1)] - UpperToLower[static_cast<unsigned>(*s2)])
			return Result < 0? -1 : 1;

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
		if (const auto Result = UpperToLower[static_cast<unsigned>(*s1)] - UpperToLower[static_cast<unsigned>(*s2)])
			return Result < 0? -1 : 1;

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
	}
	while (*String++);

	return nullptr;
}

static void AnsiToUnicodeBin(std::string_view const AnsiString, wchar_t* UnicodeString, uintptr_t CodePage = CP_OEMCP)
{
	if (!AnsiString.empty())
	{
		*UnicodeString = 0;
		// BUGBUG, error checking
		(void)encoding::get_chars(CodePage, AnsiString, { UnicodeString, AnsiString.size() });
	}
}

static wchar_t *AnsiToUnicodeBin(std::string_view const AnsiString, uintptr_t CodePage = CP_OEMCP)
{
	auto Result = std::make_unique<wchar_t[]>(AnsiString.size() + 1);
	AnsiToUnicodeBin(AnsiString, Result.get(), CodePage);
	return Result.release();
}

static wchar_t *AnsiToUnicode(const char* AnsiString)
{
	return AnsiString? AnsiToUnicodeBin(AnsiString, CP_OEMCP) : nullptr;
}

static void UnicodeToAnsiBin(string_view const UnicodeString, char* AnsiString, uintptr_t CodePage = CP_OEMCP)
{
	if (!UnicodeString.empty())
	{
		*AnsiString = 0;
		// BUGBUG, error checking
		(void)encoding::get_bytes(CodePage, UnicodeString, { AnsiString, UnicodeString.size() });
	}
}

static char *UnicodeToAnsiBin(string_view const UnicodeString, uintptr_t CodePage = CP_OEMCP)
{
	/* $ 06.01.2008 TS
	! Увеличил размер выделяемой под строку памяти на 1 байт для нормальной
	работы старых плагинов, которые не знали, что надо смотреть на длину,
	а не на завершающий ноль (например в EditorGetString.StringText).
	*/
	auto Result = std::make_unique<char[]>(UnicodeString.size() + 1);
	UnicodeToAnsiBin(UnicodeString, Result.get(), CodePage);
	return Result.release();
}

static char *UnicodeToAnsi(const wchar_t* UnicodeString)
{
	if (!UnicodeString)
		return nullptr;

	return UnicodeToAnsiBin(UnicodeString, CP_OEMCP);
}

static wchar_t** AnsiArrayToUnicode(span<const char* const> const Strings)
{
	auto Result = std::make_unique<wchar_t*[]>(Strings.size());
	std::transform(ALL_CONST_RANGE(Strings), Result.get(), AnsiToUnicode);
	return Result.release();
}

static wchar_t **AnsiArrayToUnicodeMagic(span<const char* const> const Strings)
{
	auto Result = std::make_unique<wchar_t*[]>(Strings.size() + 1);
	Result[0] = static_cast<wchar_t*>(ToPtr(Strings.size()));
	std::transform(ALL_CONST_RANGE(Strings), Result.get() + 1, AnsiToUnicode);
	return Result.release() + 1;
}

static void FreeUnicodeArrayMagic(const wchar_t* const* Array)
{
	if (!Array)
		return;

	const auto RealPtr = Array - 1;
	const auto Size = reinterpret_cast<size_t>(RealPtr[0]);

	for (const auto& i: span(Array, Size))
	{
		delete[] i;
	}

	delete[] RealPtr;
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
		const auto CleanKey = dOldKey&~KEY_CTRLMASK;

		if (CleanKey>0x80 && CleanKey<0x100)
		{
			const auto OemChar = static_cast<char>(CleanKey);
			wchar_t WideChar = 0;
			if (encoding::oem::get_chars({ &OemChar, 1 }, { &WideChar, 1 }))
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
		const auto CleanKey = dKey&~KEY_CTRLMASK;

		if (CleanKey>0x80 && CleanKey<0x10000)
		{
			const auto WideChar = static_cast<wchar_t>(CleanKey);
			char OemChar = 0;
			if (encoding::oem::get_bytes({ &WideChar, 1 }, { &OemChar, 1 }))
				dKey = (dKey^CleanKey) | OemChar;
		}
	}

	return dKey;
}

template<class F1, class F2, class M>
static void FirstFlagsToSecond(const F1& FirstFlags, F2& SecondFlags, M& Map)
{
	for (const auto& [f1, f2]: Map)
	{
		if (FirstFlags & f1)
		{
			SecondFlags |= f2;
		}
	}
}

template<class F1, class F2, class M>
static void SecondFlagsToFirst(const F2& SecondFlags, F1& FirstFlags, M& Map)
{
	for (const auto& [f1, f2]: Map)
	{
		if (SecondFlags & f2)
		{
			FirstFlags |= f1;
		}
	}
}

static InfoPanelLine* ConvertInfoPanelLinesA(span<const oldfar::InfoPanelLine> const ipl)
{
	auto Result = std::make_unique<InfoPanelLine[]>(ipl.size());

	std::transform(ALL_CONST_RANGE(ipl), Result.get(), [](const auto& Item)
	{
		return InfoPanelLine{ AnsiToUnicode(Item.Text), AnsiToUnicode(Item.Data), Item.Separator? IPLFLAGS_SEPARATOR : 0 };
	});

	return Result.release();
}

static void FreeUnicodeInfoPanelLines(span<const InfoPanelLine> const Lines)
{
	for (const auto& i: Lines)
	{
		delete[] i.Text;
		delete[] i.Data;
	}

	delete[] Lines.data();
}

static void ConvertPanelModeToUnicode(const oldfar::PanelMode& Mode, PanelMode& UnicodeMode)
{
	size_t iColumnCount = 0;
	if (Mode.ColumnTypes)
	{
		const auto Iterator = null_iterator(Mode.ColumnTypes);
		iColumnCount = std::count(Iterator, Iterator.end(), ',') + 1;
	}

	UnicodeMode.ColumnTypes = AnsiToUnicode(Mode.ColumnTypes);
	UnicodeMode.ColumnWidths = AnsiToUnicode(Mode.ColumnWidths);
	if (Mode.ColumnTitles && iColumnCount)
		UnicodeMode.ColumnTitles = AnsiArrayToUnicodeMagic({ Mode.ColumnTitles, iColumnCount });
	UnicodeMode.StatusColumnTypes = AnsiToUnicode(Mode.StatusColumnTypes);
	UnicodeMode.StatusColumnWidths = AnsiToUnicode(Mode.StatusColumnWidths);

	UnicodeMode.Flags =
		(Mode.FullScreen? PMFLAGS_FULLSCREEN : 0) |
		(Mode.DetailedStatus? PMFLAGS_DETAILEDSTATUS : 0) |
		(Mode.AlignExtensions? PMFLAGS_ALIGNEXTENSIONS : 0) |
		(Mode.CaseConversion? PMFLAGS_CASECONVERSION : 0);
}

static void ConvertPanelModesToUnicode(span<const oldfar::PanelMode> const Modes, span<PanelMode> const UnicodeModes)
{
	for (const auto& [m, u]: zip(Modes, UnicodeModes))
	{
		ConvertPanelModeToUnicode(m, u);
	}
}

static void FreeUnicodePanelModes(span<PanelMode const> const Modes)
{
	for (const auto& i: Modes)
	{
		delete[] i.ColumnTypes;
		delete[] i.ColumnWidths;
		FreeUnicodeArrayMagic(i.ColumnTitles);
		delete[] i.StatusColumnTypes;
		delete[] i.StatusColumnWidths;
	}
	delete[] Modes.data();
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

		const auto Extract = [&](const auto& Item, size_t i)
		{
			return std::invoke(Item.first, kbtA)[i];
		};

		for (size_t i = 0; i != 12; ++i)
		{
			const auto CheckLabel = [&](const auto& Item) { return Extract(Item, i) != nullptr; };

			kbtW->CountLabels += std::count_if(ALL_CONST_RANGE(LabelsMap), CheckLabel);

			if (FullStruct)
			{
				kbtW->CountLabels += std::count_if(ALL_CONST_RANGE(LabelsMapEx), CheckLabel);
			}
		}

		if (kbtW->CountLabels)
		{
			auto WideLabels = std::make_unique<KeyBarLabel[]>(kbtW->CountLabels);

			for (size_t i = 0, j = 0; i != 12; ++i)
			{
				const auto ProcessLabel = [&](const auto& Item)
				{
					if (const auto& Text = Extract(Item, i))
					{
						WideLabels[j].Text = AnsiToUnicode(Text);
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

static void FreeUnicodeKeyBarTitles(const KeyBarTitles* kbtW)
{
	if (!kbtW)
		return;

	for (const auto& Item: span(kbtW->Labels, kbtW->CountLabels))
	{
		delete[] Item.Text;
	}

	delete[] kbtW->Labels;
}

static void WINAPI FreeUserData(void* UserData, const FarPanelItemFreeInfo*)
{
	delete[] static_cast<char*>(UserData);
}

static const std::array PluginPanelItemFlagsMap
{
	OLDFAR_TO_FAR_MAP(PPIF_PROCESSDESCR),
	OLDFAR_TO_FAR_MAP(PPIF_SELECTED),
	// PPIF_USERDATA is handled manually
};

static PluginPanelItem* ConvertAnsiPanelItemsToUnicode(span<const oldfar::PluginPanelItem> const PanelItemA)
{
	auto Result = std::make_unique<PluginPanelItem[]>(PanelItemA.size());
	const span DstSpan(Result.get(), PanelItemA.size());
	for(const auto& [Src, Dst]: zip(PanelItemA, DstSpan))
	{
		// Plugin can keep its own flags in the low word
		Dst.Flags = LOWORD(Src.Flags);
		FirstFlagsToSecond(Src.Flags, Dst.Flags, PluginPanelItemFlagsMap);

		Dst.NumberOfLinks = Src.NumberOfLinks;

		Dst.Description = AnsiToUnicode(Src.Description);
		Dst.Owner = AnsiToUnicode(Src.Owner);

		if (Src.CustomColumnData && Src.CustomColumnNumber)
		{
			Dst.CustomColumnNumber = Src.CustomColumnNumber;
			Dst.CustomColumnData = AnsiArrayToUnicode({ Src.CustomColumnData, static_cast<size_t>(Src.CustomColumnNumber) });
		}

		if (Src.Flags&oldfar::PPIF_USERDATA)
		{
			const auto UserData = reinterpret_cast<const void*>(Src.UserData);
			const auto Size = *static_cast<const DWORD*>(UserData);
			Dst.UserData.Data = new char[Size];
			copy_memory(UserData, Dst.UserData.Data, Size);
			Dst.UserData.FreeData = FreeUserData;
		}
		else
		{
			Dst.UserData.Data = reinterpret_cast<void*>(Src.UserData);
			Dst.UserData.FreeData = nullptr;
		}
		Dst.CRC32 = Src.CRC32;
		Dst.FileAttributes = Src.FindData.dwFileAttributes;
		Dst.CreationTime = Src.FindData.ftCreationTime;
		Dst.LastAccessTime = Src.FindData.ftLastAccessTime;
		Dst.LastWriteTime = Src.FindData.ftLastWriteTime;
		Dst.FileSize = static_cast<unsigned long long>(Src.FindData.nFileSizeLow) + (static_cast<unsigned long long>(Src.FindData.nFileSizeHigh) << 32);
		Dst.AllocationSize = static_cast<unsigned long long>(Src.PackSize) + (static_cast<unsigned long long>(Src.PackSizeHigh) << 32);
		Dst.FileName = AnsiToUnicode(Src.FindData.cFileName);
		Dst.AlternateFileName = AnsiToUnicode(Src.FindData.cAlternateFileName);
	}
	return Result.release();
}

static void ConvertPanelItemToAnsi(const PluginPanelItem &PanelItem, oldfar::PluginPanelItem &PanelItemA, size_t PathOffset = 0)
{
	// Plugin can keep its own flags in the low word
	PanelItemA.Flags = LOWORD(PanelItem.Flags);

	SecondFlagsToFirst(PanelItem.Flags, PanelItemA.Flags, PluginPanelItemFlagsMap);

	if (PanelItem.UserData.FreeData == FreeUserData)
		PanelItemA.Flags |= oldfar::PPIF_USERDATA;

	PanelItemA.NumberOfLinks = PanelItem.NumberOfLinks;
	PanelItemA.Description = UnicodeToAnsi(PanelItem.Description);
	PanelItemA.Owner = UnicodeToAnsi(PanelItem.Owner);

	if (PanelItem.CustomColumnNumber)
	{
		PanelItemA.CustomColumnNumber = static_cast<int>(PanelItem.CustomColumnNumber);
		auto Data = std::make_unique<char*[]>(PanelItem.CustomColumnNumber);
		std::transform(PanelItem.CustomColumnData, PanelItem.CustomColumnData + PanelItem.CustomColumnNumber, Data.get(), UnicodeToAnsi);
		PanelItemA.CustomColumnData = Data.release();
	}

	if (PanelItem.UserData.Data&&PanelItem.UserData.FreeData == FreeUserData)
	{
		const auto Size = *static_cast<const DWORD*>(PanelItem.UserData.Data);
		auto Data = std::make_unique<char[]>(Size);
		copy_memory(PanelItem.UserData.Data, Data.get(), Size);
		PanelItemA.UserData = reinterpret_cast<intptr_t>(Data.release());
	}
	else
		PanelItemA.UserData = reinterpret_cast<intptr_t>(PanelItem.UserData.Data);

	PanelItemA.CRC32 = PanelItem.CRC32;
	PanelItemA.FindData.dwFileAttributes = PanelItem.FileAttributes;
	PanelItemA.FindData.ftCreationTime = PanelItem.CreationTime;
	PanelItemA.FindData.ftLastAccessTime = PanelItem.LastAccessTime;
	PanelItemA.FindData.ftLastWriteTime = PanelItem.LastWriteTime;
	PanelItemA.FindData.nFileSizeLow = static_cast<DWORD>(PanelItem.FileSize & 0xFFFFFFFF);
	PanelItemA.FindData.nFileSizeHigh = static_cast<DWORD>(PanelItem.FileSize >> 32);
	PanelItemA.PackSize = static_cast<DWORD>(PanelItem.AllocationSize & 0xFFFFFFFF);
	PanelItemA.PackSizeHigh = static_cast<DWORD>(PanelItem.AllocationSize >> 32);
	(void)encoding::oem::get_bytes(PanelItem.FileName + PathOffset, PanelItemA.FindData.cFileName);
	(void)encoding::oem::get_bytes(PanelItem.AlternateFileName, PanelItemA.FindData.cAlternateFileName);
}

static oldfar::PluginPanelItem* ConvertPanelItemsArrayToAnsi(const PluginPanelItem *PanelItemW, size_t ItemsNumber)
{
	auto Result = std::make_unique<oldfar::PluginPanelItem[]>(ItemsNumber);

	for (size_t i = 0; i != ItemsNumber; i++)
	{
		ConvertPanelItemToAnsi(PanelItemW[i], Result[i]);
	}

	return Result.release();
}

static void FreeUnicodePanelItem(PluginPanelItem *PanelItem, size_t ItemsNumber)
{
	FreePluginPanelItemsData(span(PanelItem, ItemsNumber));

	delete[] PanelItem;
}

static void FreePanelItemA(span<const oldfar::PluginPanelItem> const PanelItem)
{
	for (const auto& Item: PanelItem)
	{
		delete[] Item.Description;
		delete[] Item.Owner;

		DeleteRawArray(span(Item.CustomColumnData, Item.CustomColumnNumber));

		if (Item.Flags & oldfar::PPIF_USERDATA)
		{
			delete[] reinterpret_cast<char*>(Item.UserData);
		}
	}

	delete[] PanelItem.data();
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
		Str[l++] = '"';
		Str[l] = 0;
	}

	return Str;
}

static auto GetPluginUuid(intptr_t n)
{
	return &reinterpret_cast<Plugin*>(n)->Id();
}

struct DialogData
{
	oldfar::FARWINDOWPROC DlgProc;
	HANDLE hDlg;
	oldfar::FarDialogItem *diA;
	FarDialogItem *di;
	FarList *l;
};

static auto& Dialogs()
{
	static std::unordered_map<HANDLE, DialogData> s_Dialogs;
	return s_Dialogs;
}

static auto FindDialogData(HANDLE hDlg)
{
	const auto ItemIterator = Dialogs().find(hDlg);
	return ItemIterator == Dialogs().end()? nullptr : &ItemIterator->second;
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

	for (const auto& [Src, Dst]: zip(span(VBufA, Size), span(VBuf, Size)))
	{
		if (NoCvt)
		{
			Dst.Char = Src.Char.UnicodeChar;
		}
		else
		{
			AnsiToUnicodeBin({ &Src.Char.AsciiChar, 1 }, &Dst.Char);
		}
		Dst.Attributes = colors::ConsoleColorToFarColor(Src.Attributes);
	}
}

static FAR_CHAR_INFO* AnsiVBufToUnicode(const oldfar::FarDialogItem &diA)
{
	if (!diA.VBuf)
		return nullptr;

	const auto Size = GetAnsiVBufSize(diA);
	// + 1 потому что там храним поинтер на анси vbuf.
	auto VBuf = std::make_unique<FAR_CHAR_INFO[]>(Size + 1);
	AnsiVBufToUnicode(diA.VBuf, VBuf.get(), Size, (diA.Flags&oldfar::DIF_NOTCVTUSERCONTROL) == oldfar::DIF_NOTCVTUSERCONTROL);
	SetAnsiVBufPtr(VBuf.get(), diA.VBuf, Size);
	return VBuf.release();
}

static const std::array ListFlagsMap
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
	(void)encoding::oem::get_bytes(li->Text, liA->Text);
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

static const std::array DialogItemFlagsMap
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
	di = {};
	AnsiDialogItemToUnicodeSafe(diA, di);

	switch (di.Type)
	{
	case DI_LISTBOX:
	case DI_COMBOBOX:
	{
		if (diA.ListItems && os::memory::is_pointer(diA.ListItems))
		{
			auto Items = std::make_unique<FarListItem[]>(diA.ListItems->ItemsNumber);
			for (int i = 0; i != diA.ListItems->ItemsNumber; ++i)
			{
				AnsiListItemToUnicode(&diA.ListItems->Items[i], &Items[i]);
			}
			l.Items = Items.release();
			l.ItemsNumber = diA.ListItems->ItemsNumber;
			di.ListItems = &l;
		}

		break;
	}
	case DI_USERCONTROL:
		di.VBuf = AnsiVBufToUnicode(diA);
		break;
	case DI_EDIT:
	case DI_FIXEDIT:
	{
		if (diA.Flags&oldfar::DIF_HISTORY)
			di.History = AnsiToUnicode(diA.History);
		else if (diA.Flags&oldfar::DIF_MASKEDIT)
			di.Mask = AnsiToUnicode(diA.Mask);

		break;
	}
	default:
		break;
	}

	if (diA.Type == oldfar::DI_USERCONTROL)
	{
		auto Data = std::make_unique<wchar_t[]>(std::size(diA.Data));
		copy_memory(diA.Data, Data.get(), sizeof(diA.Data));
		di.Data = Data.release();
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
				for (const auto& i: span(di.ListItems->Items, di.ListItems->ItemsNumber))
				{
					delete[] i.Text;
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

	if (diA.Flags&oldfar::DIF_SETCOLOR)
	{
		diA.Flags = oldfar::DIF_SETCOLOR | (diA.Flags & oldfar::DIF_COLORMASK);
	}
	else
	{
		diA.Flags = 0;
	}

	if (di.Flags)
	{
		SecondFlagsToFirst(di.Flags, diA.Flags, DialogItemFlagsMap);
	}

	diA.DefaultButton = (di.Flags&DIF_DEFAULTBUTTON) != 0;
}

static oldfar::FarDialogItem* UnicodeDialogItemToAnsi(FarDialogItem &di, HANDLE hDlg, int ItemNumber)
{
	auto diA = CurrentDialogItemA(hDlg, ItemNumber);
	if (!diA)
	{
		static oldfar::FarDialogItem OneDialogItem;
		diA = &OneDialogItem;
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
		diA->ListPos = static_cast<int>(pluginapi::apiSendDlgMessage(hDlg, DM_LISTGETCURPOS, ItemNumber, nullptr));
		break;
	}

	if (diA->Type == oldfar::DI_USERCONTROL)
	{
		if (di.Data)
			copy_memory(di.Data, diA->Data, sizeof(diA->Data));
	}
	else if ((diA->Type == oldfar::DI_EDIT || diA->Type == oldfar::DI_COMBOBOX) && diA->Flags&oldfar::DIF_VAREDIT)
	{
		const auto Length = wcslen(di.Data);
		diA->Ptr.PtrLength = static_cast<int>(Length);
		auto Data = std::make_unique<char[]>(Length + 1);
		(void)encoding::oem::get_bytes({ di.Data, Length }, { Data.get(), Length });
		Data[Length] = {};
		diA->Ptr.PtrData = Data.release();
	}
	else
		(void)encoding::oem::get_bytes(di.Data, diA->Data);

	return diA;
}

static void ConvertUnicodePanelInfoToAnsi(const PanelInfo* PIW, oldfar::PanelInfo* PIA)
{
	PIA->PanelType = oldfar::PTYPE_FILEPANEL;

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
	PIA->Visible = (PIW->Flags&PFLAGS_VISIBLE)? 1 : 0;
	PIA->Focus = (PIW->Flags&PFLAGS_FOCUS)? 1 : 0;
	PIA->ViewMode = PIW->ViewMode;
	PIA->ShortNames = (PIW->Flags&PFLAGS_ALTERNATIVENAMES)? 1 : 0;

	switch (PIW->SortMode)
	{
	default:
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
	}

	PIA->Flags = 0;

	static const std::array FlagsMap
	{
		OLDFAR_TO_FAR_MAP(PFLAGS_SHOWHIDDEN),
		OLDFAR_TO_FAR_MAP(PFLAGS_HIGHLIGHT),
		OLDFAR_TO_FAR_MAP(PFLAGS_REVERSESORTORDER),
		OLDFAR_TO_FAR_MAP(PFLAGS_USESORTGROUPS),
		OLDFAR_TO_FAR_MAP(PFLAGS_SELECTEDFIRST),
		OLDFAR_TO_FAR_MAP(PFLAGS_REALNAMES),
		OLDFAR_TO_FAR_MAP(PFLAGS_PANELLEFT),
	};

	SecondFlagsToFirst(PIW->Flags, PIA->Flags, FlagsMap);
}

static void FreeAnsiPanelInfo(oldfar::PanelInfo* PIA)
{
	if (PIA->PanelItems)
		FreePanelItemA({ PIA->PanelItems, static_cast<size_t>(PIA->ItemsNumber) });

	if (PIA->SelectedItems)
		FreePanelItemA({ PIA->SelectedItems, static_cast<size_t>(PIA->SelectedItemsNumber) });

	*PIA = {};
}

struct oldPanelInfoContainer: noncopyable
{
	oldPanelInfoContainer(): Info() {}
	~oldPanelInfoContainer() { FreeAnsiPanelInfo(&Info); }

	oldfar::PanelInfo Info;
};

static long long GetSetting(FARSETTINGS_SUBFOLDERS Root, const wchar_t* Name)
{
	long long result = 0;
	FarSettingsCreate settings = { sizeof(FarSettingsCreate), FarUuid, INVALID_HANDLE_VALUE };
	const auto Settings = pluginapi::apiSettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &settings)? settings.Handle : nullptr;
	if (Settings)
	{
		FarSettingsItem item = { sizeof(FarSettingsItem), static_cast<size_t>(Root), Name, FST_UNKNOWN, {} };
		if (pluginapi::apiSettingsControl(Settings, SCTL_GET, 0, &item) && FST_QWORD == item.Type)
		{
			result = item.Number;
		}
		pluginapi::apiSettingsControl(Settings, SCTL_FREE, 0, nullptr);
	}
	return result;
}

static uintptr_t GetEditorCodePageA()
{
	EditorInfo info = { sizeof(EditorInfo) };
	pluginapi::apiEditorControl(-1, ECTL_GETINFO, 0, &info);
	auto CodePage = info.CodePage;
	CPINFO cpi;

	if (!GetCPInfo(static_cast<unsigned>(CodePage), &cpi) || cpi.MaxCharSize > 1)
		CodePage = encoding::codepage::ansi();

	return CodePage;
}

static int GetEditorCodePageFavA()
{
	const auto CodePage = GetEditorCodePageA();

	if (encoding::codepage::oem() == CodePage)
		return 0;

	if (encoding::codepage::ansi() == CodePage)
		return 1;

	auto result = -(static_cast<int>(CodePage) + 2);
	DWORD FavIndex = 2;

	for (const auto& [Name, Value]: codepages::GetFavoritesEnumerator())
	{
		if (!(Value & CPST_FAVORITE))
			continue;

		if (Name == CodePage)
		{
			result = FavIndex;
			break;
		}

		FavIndex++;
	}

	return result;
}

static void MultiByteRecode(uintptr_t const CPin, uintptr_t const CPout, span<char> const Buffer)
{
	if (!Buffer.empty())
	{
		(void)encoding::get_bytes(CPout, encoding::get_chars(CPin, { Buffer.data(), Buffer.size() }), Buffer);
	}
}

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
		case 0:
			nCP = encoding::codepage::oem();
			break;

		case 1:
			nCP = encoding::codepage::ansi();
			break;

		default:
			{
				int FavIndex = 2;
				for (const auto& [Name, Value]: codepages::GetFavoritesEnumerator())
				{
					if (!(Value & CPST_FAVORITE))
						continue;

					if (FavIndex == Command)
					{
						nCP = Name;
						break;
					}

					FavIndex++;
				}
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

static const char* GetPluginMsg(const Plugin* PluginInstance, int MsgId);

namespace oldpluginapi
{
static void WINAPI qsort(void *base, size_t nelem, size_t width, comparer cmp) noexcept
{
	return cpp_try(
	[&]
	{
		return pluginapi::apiQsort(base, nelem, width, comparer_wrapper, reinterpret_cast<void*>(cmp));
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
	});
}

static void WINAPI qsortex(void *base, size_t nelem, size_t width, comparer_ex cmp, void *userparam) noexcept
{
	return cpp_try(
	[&]
	{
		comparer_helper helper = { cmp, userparam };
		return pluginapi::apiQsort(base, nelem, width, comparer_ex_wrapper, &helper);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
	});
}

static void* WINAPI bsearch(const void *key, const void *base, size_t nelem, size_t width, comparer cmp) noexcept
{
	return cpp_try(
	[&]
	{
		return pluginapi::apiBsearch(key, base, nelem, width, comparer_wrapper, reinterpret_cast<void*>(cmp));
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return nullptr;
	});
}

static int WINAPI LocalIslower(unsigned Ch) noexcept
{
	// noexcept
	return Ch < 256 && UpperOrLower[Ch] == case_lower;
}

static int WINAPI LocalIsupper(unsigned Ch) noexcept
{
	// noexcept
	return Ch < 256 && UpperOrLower[Ch] == case_upper;
}

static int WINAPI LocalIsalpha(unsigned Ch) noexcept
{
	// noexcept
	if (Ch >= 256)
		return FALSE;

	char CvtCh = Ch;
	OemToCharBuffA(&CvtCh, &CvtCh, 1);
	return IsCharAlphaA(CvtCh);
}

static int WINAPI LocalIsalphanum(unsigned Ch) noexcept
{
	// noexcept
	if (Ch >= 256)
		return FALSE;

	char CvtCh = Ch;
	OemToCharBuffA(&CvtCh, &CvtCh, 1);
	return IsCharAlphaNumericA(CvtCh);
}

static unsigned WINAPI LocalUpper(unsigned LowerChar) noexcept
{
	// noexcept
	return LowerChar < 256 ? LowerToUpper[LowerChar] : LowerChar;
}

static void WINAPI LocalUpperBuf(char *Buf, int Length) noexcept
{
	return cpp_try(
	[&]
	{
		for (auto& i: span(Buf, Length))
		{
			i = LowerToUpper[static_cast<size_t>(i)];
		}
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
	});
}

static unsigned WINAPI LocalLower(unsigned UpperChar) noexcept
{
	// noexcept
	return UpperChar < 256 ? UpperToLower[UpperChar] : UpperChar;
}

static void WINAPI LocalLowerBuf(char *Buf, int Length) noexcept
{
	return cpp_try(
	[&]
	{
		for (auto& i: span(Buf, Length))
		{
			i = UpperToLower[static_cast<size_t>(i)];
		}
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
	});
}

static void WINAPI LocalStrupr(char *s1) noexcept
{
	return cpp_try(
	[&]
	{
		const auto Iterator = null_iterator(s1);

		for (auto& i: range(Iterator, Iterator.end()))
		{
			i = LowerToUpper[static_cast<size_t>(i)];
		}
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
	});
}

static void WINAPI LocalStrlwr(char *s1) noexcept
{
	return cpp_try(
	[&]
	{
		const auto Iterator = null_iterator(s1);

		for (auto& i: range(Iterator, Iterator.end()))
		{
			i = UpperToLower[static_cast<size_t>(i)];
		}
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
	});
}

static int WINAPI LStricmp(const char *s1, const char *s2) noexcept
{
	return cpp_try(
	[&]
	{
		return LocalStricmp(s1, s2);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return -1;
	});
}

static int WINAPI LStrnicmp(const char *s1, const char *s2, int n) noexcept
{
	return cpp_try(
	[&]
	{
		return LocalStrnicmp(s1, s2, n);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return -1;
	});
}

static char* WINAPI RemoveTrailingSpacesA(char *Str) noexcept
{
	return cpp_try(
	[&]
	{
		if (!Str || !*Str)
			return Str;

		size_t I;
		for (auto ChPtr = Str + (I = strlen(Str)) - 1; I > 0; I--, ChPtr--)
		{
			if (IsSpaceA(*ChPtr) || IsEolA(*ChPtr))
				*ChPtr = 0;
			else
				break;
		}

		return Str;
	},
	[&]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return Str;
	});
}

static char *WINAPI FarItoaA(int value, char *string, int radix) noexcept
{
	// noexcept
	return _itoa(value, string, radix);
}

static char *WINAPI FarItoa64A(long long value, char *string, int radix) noexcept
{
	// noexcept
	return _i64toa(value, string, radix);
}

static int WINAPI FarAtoiA(const char *s) noexcept
{
	// noexcept
	return std::strtol(s, nullptr, 10);
}

static long long WINAPI FarAtoi64A(const char *s) noexcept
{
	// noexcept
	return std::strtoll(s, nullptr, 10);
}

static char* WINAPI PointToNameA(char *Path) noexcept
{
	return cpp_try(
	[&]
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
	},
	[&]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return Path;
	});
}

static void WINAPI UnquoteA(char *Str) noexcept
{
	// noexcept
	char *Dst = Str;

	while (*Str)
	{
		if (*Str != '"')
			*Dst++ = *Str;

		Str++;
	}

	*Dst = 0;
}

static char* WINAPI RemoveLeadingSpacesA(char *Str) noexcept
{
	return cpp_try(
	[&]
	{
		if (!Str || !*Str)
			return Str;

		auto ChPtr = Str;

		for (; IsSpaceA(*ChPtr); ChPtr++)
			;

		if (ChPtr != Str)
			std::copy_n(ChPtr, strlen(ChPtr) + 1, Str);

		return Str;
	},
	[&]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return Str;
	});
}

static char* WINAPI RemoveExternalSpacesA(char *Str) noexcept
{
	return RemoveTrailingSpacesA(RemoveLeadingSpacesA(Str));
}

static char* WINAPI TruncStrA(char *Str, int MaxLength) noexcept
{
	return cpp_try(
	[&]
	{
		if (MaxLength < 0)
			MaxLength = 0;

		size_t Length;
		if ((Length = strlen(Str)) > static_cast<size_t>(MaxLength))
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
	},
	[&]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return Str;
	});
}

static char* WINAPI TruncPathStrA(char *Str, int MaxLength) noexcept
{
	return cpp_try(
	[&]
	{
		const auto nLength = strlen(Str);

		if (nLength > static_cast<size_t>(MaxLength))
		{
			char *Start = nullptr;

			if (*Str && Str[1] == ':' && IsSlash(Str[2]))
				Start = Str + 3;
			else
			{
				if (Str[0] == '\\' && Str[1] == '\\')
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
	},
	[&]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return Str;
	});
}

static char* WINAPI QuoteSpaceOnlyA(char *Str) noexcept
{
	return cpp_try(
	[&]
	{
		if (contains(Str, ' '))
			InsertQuoteA(Str);
		return Str;
	},
	[&]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return Str;
	});
}

static BOOL WINAPI AddEndSlashA(char *Path) noexcept
{
	return cpp_try(
	[&]
	{
		if (!Path)
			return FALSE;
		/* $ 06.12.2000 IS
		! Теперь функция работает с обоими видами слешей, также происходит
		изменение уже существующего конечного слеша на такой, который
		встречается чаще.
		*/
		int Slash = 0, BackSlash = 0;

		auto end = Path;

		while (*end)
		{
			Slash += (*end == '\\');
			BackSlash += (*end == '/');
			end++;
		}

		const auto Length = end - Path;
		const auto c = (Slash < BackSlash) ? '/' : '\\';

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
		return TRUE;
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static void WINAPI GetPathRootA(const char *Path, char *Root) noexcept
{
	return cpp_try(
	[&]
	{
		wchar_t Buffer[MAX_PATH];
		const auto Size = pluginapi::apiGetPathRoot(encoding::oem::get_chars(Path).c_str(), Buffer, std::size(Buffer));
		if (Size)
			(void)encoding::oem::get_bytes({ Buffer, Size - 1 }, { Root, std::size(Buffer) });
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
	});
}

static int WINAPI CopyToClipboardA(const char *Data) noexcept
{
	return cpp_try(
	[&]
	{
		return pluginapi::apiCopyToClipboard(FCT_STREAM, Data? encoding::oem::get_chars(Data).c_str() : nullptr);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static char* WINAPI PasteFromClipboardA() noexcept
{
	return cpp_try(
	[&]() -> char*
	{
		if (const auto Size = pluginapi::apiPasteFromClipboard(FCT_ANY, nullptr, 0))
		{
			std::vector<wchar_t> p(Size);
			pluginapi::apiPasteFromClipboard(FCT_STREAM, p.data(), p.size());
			return UnicodeToAnsiBin({ p.data(), p.size() });
		}
		return nullptr;
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return nullptr;
	});
}

static void WINAPI DeleteBufferA(void* Buffer) noexcept
{
	// noexcept
	delete[] static_cast<char*>(Buffer);
}

static int WINAPI ProcessNameA(const char *Param1, char *Param2, DWORD Flags) noexcept
{
	return cpp_try(
	[&]
	{
		const auto strP1 = encoding::oem::get_chars(Param1), strP2 = encoding::oem::get_chars(Param2);
		const auto size = static_cast<int>(strP1.size() + strP2.size() + oldfar::NM) + 1; //а хрен ещё как угадать скока там этот Param2 для PN_GENERATENAME
		const wchar_t_ptr_n<os::default_buffer_size> p(size);
		*copy_string(strP2, p.data()) = {};

		auto newFlags = PN_NONE;

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

		const auto ret = static_cast<int>(pluginapi::apiProcessName(strP1.c_str(), p.data(), size, newFlags));

		if (ret && (newFlags & PN_GENERATENAME))
			(void)encoding::oem::get_bytes({ p.data(), static_cast<size_t>(ret - 1) }, { Param2, static_cast<size_t>(size) });

		return ret;
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static int WINAPI KeyNameToKeyA(const char *Name) noexcept
{
	return cpp_try(
	[&]
	{
		const auto Key = KeyNameToKey(encoding::oem::get_chars(Name));
		return Key? KeyToOldKey(Key) : -1;
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return -1;
	});
}

static BOOL WINAPI FarKeyToNameA(int Key, char *KeyText, int Size) noexcept
{
	return cpp_try(
	[&]
	{
		const auto strKT = KeyToText(OldKeyToKey(Key));
		if (strKT.empty())
			return FALSE;

		(void)encoding::oem::get_bytes(strKT, { KeyText, static_cast<size_t>(Size > 0? Size + 1 : 32) });
		return TRUE;
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static int WINAPI InputRecordToKeyA(const INPUT_RECORD *r) noexcept
{
	return cpp_try(
	[&]
	{
		return KeyToOldKey(InputRecordToKey(r));
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return 0;
	});
}

static char* WINAPI FarMkTempA(char *Dest, const char *Prefix) noexcept
{
	return cpp_try(
	[&]
	{
		wchar_t D[oldfar::NM]{};
		const auto Size = pluginapi::apiMkTemp(D, std::size(D), encoding::oem::get_chars(Prefix).c_str());
		if (Size)
			(void)encoding::oem::get_bytes({ D, Size - 1 }, { Dest, std::size(D) });
		return Dest;
	},
	[&]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return Dest;
	});
}

static int WINAPI FarMkLinkA(const char *Src, const char *Dest, DWORD OldFlags) noexcept
{
	return cpp_try(
	[&]
	{
		auto Type = LINK_HARDLINK;

		switch (OldFlags & 0xf)
		{
		case oldfar::FLINK_HARDLINK:    Type = LINK_HARDLINK; break;
		case oldfar::FLINK_JUNCTION:    Type = LINK_JUNCTION; break;
		case oldfar::FLINK_VOLMOUNT:    Type = LINK_VOLMOUNT; break;
		case oldfar::FLINK_SYMLINKFILE: Type = LINK_SYMLINKFILE; break;
		case oldfar::FLINK_SYMLINKDIR:  Type = LINK_SYMLINKDIR; break;
		}

		auto Flags = MLF_NONE;
		if (OldFlags&oldfar::FLINK_SHOWERRMSG)
			Flags |= MLF_SHOWERRMSG;

		if (OldFlags&oldfar::FLINK_DONOTUPDATEPANEL)
			Flags |= MLF_DONOTUPDATEPANEL;

		return pluginapi::apiMkLink(encoding::oem::get_chars(Src).c_str(), encoding::oem::get_chars(Dest).data(), Type, Flags);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static int WINAPI GetNumberOfLinksA(const char *Name) noexcept
{
	return cpp_try(
	[&]
	{
		return static_cast<int>(pluginapi::apiGetNumberOfLinks(encoding::oem::get_chars(Name).c_str()));
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return 0;
	});
}

static int WINAPI ConvertNameToRealA(const char *Src, char *Dest, int DestSize) noexcept
{
	return cpp_try(
	[&]
	{
		const auto strDest = ConvertNameToReal(encoding::oem::get_chars(Src));

		if (!Dest)
			return static_cast<int>(strDest.size());

		(void)encoding::oem::get_bytes(strDest, { Dest, static_cast<size_t>(DestSize) });
		return std::min(static_cast<int>(strDest.size()), DestSize);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return 0;
	});
}

static int WINAPI FarGetReparsePointInfoA(const char *Src, char *Dest, int DestSize) noexcept
{
	return cpp_try(
	[&]
	{
		const auto strSrc = encoding::oem::get_chars(Src);
		wchar_t Buffer[MAX_PATH];
		auto Result = pluginapi::apiGetReparsePointInfo(strSrc.c_str(), Buffer, std::size(Buffer));
		if (Result && DestSize && Dest)
		{
			if (Result > MAX_PATH)
			{
				std::vector<wchar_t> Tmp(DestSize);
				Result = pluginapi::apiGetReparsePointInfo(strSrc.c_str(), Tmp.data(), Tmp.size());
				(void)encoding::oem::get_bytes({ Tmp.data(), Result - 1 }, { Dest, static_cast<size_t>(DestSize) });
			}
			else
			{
				(void)encoding::oem::get_bytes({ Buffer, Result - 1 }, { Dest, static_cast<size_t>(DestSize) });
			}
		}
		return static_cast<int>(Result);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static int WINAPI FarRecursiveSearchA_Callback(const PluginPanelItem *FData, const wchar_t *FullName, void *param) noexcept
{
	return cpp_try(
	[&]
	{
		const auto pCallbackParam = static_cast<const FAR_SEARCH_A_CALLBACK_PARAM*>(param);
		WIN32_FIND_DATAA FindData = {};
		FindData.dwFileAttributes = FData->FileAttributes;
		FindData.ftCreationTime = FData->CreationTime;
		FindData.ftLastAccessTime = FData->LastAccessTime;
		FindData.ftLastWriteTime = FData->LastWriteTime;
		FindData.nFileSizeLow = static_cast<DWORD>(FData->FileSize);
		FindData.nFileSizeHigh = static_cast<DWORD>(FData->FileSize >> 32);
		(void)encoding::oem::get_bytes(FData->FileName, FindData.cFileName);
		(void)encoding::oem::get_bytes(FData->AlternateFileName, FindData.cAlternateFileName);
		char FullNameA[oldfar::NM];
		(void)encoding::oem::get_bytes(FullName, FullNameA);
		return pCallbackParam->Func(&FindData, FullNameA, pCallbackParam->Param);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static void WINAPI FarRecursiveSearchA(const char *InitDir, const char *Mask, oldfar::FRSUSERFUNC Func, DWORD Flags, void *Param) noexcept
{
	return cpp_try(
	[&]
	{
		FAR_SEARCH_A_CALLBACK_PARAM CallbackParam;
		CallbackParam.Func = Func;
		CallbackParam.Param = Param;

		static const std::array FlagsMap
		{
			OLDFAR_TO_FAR_MAP(FRS_RETUPDIR),
			OLDFAR_TO_FAR_MAP(FRS_RECUR),
			OLDFAR_TO_FAR_MAP(FRS_SCANSYMLINK),
		};

		auto NewFlags = FRS_NONE;
		FirstFlagsToSecond(Flags, NewFlags, FlagsMap);

		pluginapi::apiRecursiveSearch(encoding::oem::get_chars(InitDir).c_str(), encoding::oem::get_chars(Mask).data(), FarRecursiveSearchA_Callback, NewFlags, &CallbackParam);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
	});
}

static DWORD WINAPI ExpandEnvironmentStrA(const char *src, char *dest, size_t size) noexcept
{
	return cpp_try(
	[&]
	{
		const auto strD = os::env::expand(encoding::oem::get_chars(src));
		const auto len = std::min(strD.size(), size - 1);
		(void)encoding::oem::get_bytes(strD, { dest, len + 1 });
		return static_cast<DWORD>(len);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return 0;
	});
}

static int WINAPI FarViewerA(const char *FileName, const char *Title, int X1, int Y1, int X2, int Y2, DWORD Flags) noexcept
{
	return cpp_try(
	[&]
	{
		return pluginapi::apiViewer(encoding::oem::get_chars(FileName).c_str(), encoding::oem::get_chars(Title).c_str(), X1, Y1, X2, Y2, Flags, CP_DEFAULT);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static int WINAPI FarEditorA(const char *FileName, const char *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, int StartLine, int StartChar) noexcept
{
	return cpp_try(
	[&]
	{
		return pluginapi::apiEditor(encoding::oem::get_chars(FileName).c_str(), encoding::oem::get_chars(Title).c_str(), X1, Y1, X2, Y2, Flags, StartLine, StartChar, CP_DEFAULT);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return EEC_OPEN_ERROR;
	});
}

static int WINAPI FarCmpNameA(const char *pattern, const char *str, int skippath) noexcept
{
	// noexcept
	return ProcessNameA(pattern, const_cast<char*>(str), oldfar::PN_CMPNAME | (skippath ? oldfar::PN_SKIPPATH : 0));
}

static void WINAPI FarTextA(int X, int Y, int ConColor, const char *Str) noexcept
{
	return cpp_try(
	[&]
	{
		const auto Color = colors::ConsoleColorToFarColor(ConColor);
		return pluginapi::apiText(X, Y, &Color, Str? encoding::oem::get_chars(Str).c_str() : nullptr);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
	});
}

static BOOL WINAPI FarShowHelpA(const char *ModuleName, const char *HelpTopic, DWORD Flags) noexcept
{
	return cpp_try(
	[&]
	{
		return pluginapi::apiShowHelp(encoding::oem::get_chars(ModuleName).c_str(), (HelpTopic ? encoding::oem::get_chars(HelpTopic).c_str() : nullptr), Flags);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static int WINAPI FarInputBoxA(const char *Title, const char *Prompt, const char *HistoryName, const char *SrcText, char *DestText, int DestLength, const char *HelpTopic, DWORD Flags) noexcept
{
	return cpp_try(
	[&]
	{
		static const std::array FlagsMap
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

		const wchar_t_ptr_n<256> Buffer(DestLength);

		const auto ret = pluginapi::apiInputBox(
			&FarUuid,
			&FarUuid,
			Title? encoding::oem::get_chars(Title).c_str() : nullptr,
			Prompt? encoding::oem::get_chars(Prompt).c_str() : nullptr,
			HistoryName? encoding::oem::get_chars(HistoryName).c_str() : nullptr,
			SrcText? encoding::oem::get_chars(SrcText).c_str() : nullptr,
			Buffer.data(),
			Buffer.size(),
			HelpTopic? encoding::oem::get_chars(HelpTopic).c_str() : nullptr,
			NewFlags
		);

		if (ret && DestText)
			(void)encoding::oem::get_bytes(Buffer.data(), { DestText, static_cast<size_t>(DestLength) + 1 });

		return ret;
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static int WINAPI FarMessageFnA(intptr_t PluginNumber, DWORD Flags, const char *HelpTopic, const char * const *Items, int ItemsNumber, int ButtonsNumber) noexcept
{
	return cpp_try(
	[&]
	{
		Flags &= ~oldfar::FMSG_DOWN;

		std::unique_ptr<wchar_t[]> AllInOneAnsiItem;
		std::vector<std::unique_ptr<wchar_t[]>> AnsiItems;

		if (Flags&oldfar::FMSG_ALLINONE)
		{
			AllInOneAnsiItem.reset(AnsiToUnicode(reinterpret_cast<const char*>(Items)));
		}
		else
		{
			AnsiItems.reserve(ItemsNumber);
			std::transform(Items, Items + ItemsNumber, std::back_inserter(AnsiItems), [](const char* Item){ return std::unique_ptr<wchar_t[]>(AnsiToUnicode(Item)); });
		}

		static const std::array FlagsMap
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

		return pluginapi::apiMessageFn(
			GetPluginUuid(PluginNumber),
			&FarUuid,
			NewFlags,
			HelpTopic? encoding::oem::get_chars(HelpTopic).c_str() : nullptr,
			AnsiItems.empty()? reinterpret_cast<const wchar_t* const *>(AllInOneAnsiItem.get()) : reinterpret_cast<const wchar_t* const *>(AnsiItems.data()),
			ItemsNumber,
			ButtonsNumber
		);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return -1;
	});
}

static const char * WINAPI FarGetMsgFnA(intptr_t PluginHandle, int MsgId) noexcept
{
	return cpp_try(
	[&]
	{
		//BUGBUG, надо проверять, что PluginHandle - плагин
		const auto pPlugin = reinterpret_cast<Plugin*>(PluginHandle);
		string_view Path = pPlugin->ModuleName();
		CutToSlash(Path);

		if (pPlugin->InitLang(Path, Global->Opt->strLanguage))
			return GetPluginMsg(pPlugin, MsgId);

		return "";
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return "";
	});
}

static int WINAPI FarMenuFnA(intptr_t PluginNumber, int X, int Y, int MaxHeight, DWORD Flags, const char *Title, const char *Bottom, const char *HelpTopic, const int *BreakKeys, int *BreakCode, const oldfar::FarMenuItem *Item, int ItemsNumber) noexcept
{
	return cpp_try(
	[&]
	{
		static const std::array FlagsMap
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

			static const std::array ItemFlagsMap
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
						AnsiToUnicodeBin({ reinterpret_cast<const char*>(&Item[i].Checked), 1 }, reinterpret_cast<wchar_t*>(&mi[i].Flags));
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
			while (BreakKeys[BreakKeysCount])
				++BreakKeysCount;

			if (BreakKeysCount)
			{
				NewBreakKeys.resize(BreakKeysCount);
				std::transform(BreakKeys, BreakKeys + BreakKeysCount, NewBreakKeys.begin(), [](int i)
				{
					FarKey NewItem;
					NewItem.VirtualKeyCode = i & 0xffff;
					NewItem.ControlKeyState = 0;
					const auto ItemFlags = i >> 16;
					if (ItemFlags & oldfar::PKF_CONTROL) NewItem.ControlKeyState |= LEFT_CTRL_PRESSED;
					if (ItemFlags & oldfar::PKF_ALT) NewItem.ControlKeyState |= LEFT_ALT_PRESSED;
					if (ItemFlags & oldfar::PKF_SHIFT) NewItem.ControlKeyState |= SHIFT_PRESSED;
					return NewItem;
				});
			}
		}

		intptr_t NewBreakCode;

		const auto ret = pluginapi::apiMenuFn(GetPluginUuid(PluginNumber), &FarUuid, X, Y, MaxHeight, NewFlags,
			Title? encoding::oem::get_chars(Title).c_str() : nullptr,
			Bottom? encoding::oem::get_chars(Bottom).c_str() : nullptr,
			HelpTopic? encoding::oem::get_chars(HelpTopic).c_str() : nullptr,
			BreakKeys? NewBreakKeys.data() : nullptr,
			&NewBreakCode, mi.data(), ItemsNumber);

		if (BreakCode)
			*BreakCode = NewBreakCode;

		for (const auto& i: mi)
		{
			delete[] i.Text;
		}

		return ret;
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return -1;
	});
}

static intptr_t WINAPI FarDefDlgProcA(HANDLE hDlg, int Msg, int Param1, void* Param2) noexcept
{
	return cpp_try(
	[&]
	{
		auto& TopEvent = OriginalEvents().top();
		auto Result = pluginapi::apiDefDlgProc(TopEvent.hDlg, TopEvent.Msg, TopEvent.Param1, TopEvent.Param2);
		switch (Msg)
		{
		case DN_CTLCOLORDIALOG:
		case DN_CTLCOLORDLGITEM:
			Result = reinterpret_cast<intptr_t>(Param2);
			break;
		}
		return Result;
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return 0;
	});
}

static intptr_t WINAPI CurrentDlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) noexcept
{
	return cpp_try(
	[&]
	{
		const auto Data = FindDialogData(hDlg);
		return (Data->DlgProc ? Data->DlgProc : FarDefDlgProcA)(hDlg, Msg, Param1, Param2);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return 0;
	});
}

static intptr_t WINAPI DlgProcA(HANDLE hDlg, intptr_t NewMsg, intptr_t Param1, void* Param2) noexcept
{
	return cpp_try(
	[&]() -> intptr_t
	{
		const FarDialogEvent e = {sizeof(FarDialogEvent), hDlg, NewMsg, Param1, Param2};

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

					const auto Result = static_cast<DWORD>(CurrentDlgProc(hDlg, oldfar::DN_CTLCOLORDLGITEM, Param1, ToPtr(MAKELONG(
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
					const auto Result = CurrentDlgProc(hDlg, oldfar::DN_CTLCOLORDLGLIST, Param1, &lcA);
					if(Result)
					{
						lc->ColorsCount = lcA.ColorCount;
						std::transform(lcA.Colors, lcA.Colors + lcA.ColorCount, lc->Colors, colors::ConsoleColorToFarColor);
					}
					return Result != 0;
				}

			case DN_DRAWDLGITEM:
			{
				Msg=oldfar::DN_DRAWDLGITEM;
				const auto di = static_cast<FarDialogItem*>(Param2);
				const auto FarDiA = UnicodeDialogItemToAnsi(*di, hDlg, Param1);
				const auto ret = CurrentDlgProc(hDlg, Msg, Param1, FarDiA);
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
				const std::unique_ptr<char[]> HelpTopicA(UnicodeToAnsi(static_cast<const wchar_t*>(Param2)));
				auto ret = CurrentDlgProc(hDlg, oldfar::DN_HELP, Param1, HelpTopicA.get());
				if (ret && ret != reinterpret_cast<intptr_t>(Param2)) // changed
				{
					static std::unique_ptr<wchar_t[]> HelpTopic;
					HelpTopic.reset(AnsiToUnicode(reinterpret_cast<const char*>(ret)));
					ret = reinterpret_cast<intptr_t>(HelpTopic.get());
				}
				return ret;
			}
			case DN_HOTKEY:
				Msg=oldfar::DN_HOTKEY;
				Param2 = ToPtr(KeyToOldKey(static_cast<DWORD>(InputRecordToKey(static_cast<const INPUT_RECORD *>(Param2)))));
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
				return pluginapi::apiDefDlgProc(hDlg, NewMsg, Param1, Param2);
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
						Param2=ToPtr(KeyToOldKey(static_cast<DWORD>(InputRecordToKey(record))));
						break;
					}
				}
				return pluginapi::apiDefDlgProc(hDlg, NewMsg, Param1, Param2);
			default:
				break;
		}
		return CurrentDlgProc(hDlg, Msg, Param1, Param2);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return 0;
	});
}

static intptr_t WINAPI FarSendDlgMessageA(HANDLE hDlg, int OldMsg, int Param1, void* Param2) noexcept
{
	return cpp_try(
	[&]() -> intptr_t
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
				size_t item_size = pluginapi::apiSendDlgMessage(hDlg, DM_GETDLGITEM, Param1, nullptr);

				if (item_size)
				{
					block_ptr<FarDialogItem> Buffer(item_size);
					FarGetDialogItem gdi = {sizeof(FarGetDialogItem), item_size, Buffer.data()};

					if (gdi.Item)
					{
						pluginapi::apiSendDlgMessage(hDlg, DM_GETDLGITEM, Param1, &gdi);
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
					return pluginapi::apiSendDlgMessage(hDlg, DM_GETTEXT, Param1, nullptr);

				const auto didA = static_cast<oldfar::FarDialogItemData*>(Param2);
				if (!didA->PtrLength) //вот такой хреновый API!!!
					didA->PtrLength = static_cast<int>(pluginapi::apiSendDlgMessage(hDlg, DM_GETTEXT, Param1, nullptr));
				std::vector<wchar_t> text(didA->PtrLength + 1);
				//BUGBUG: если didA->PtrLength=0, то вернётся с учётом '\0', в Энц написано, что без, хз как правильно.
				FarDialogItemData did = {sizeof(FarDialogItemData), static_cast<size_t>(didA->PtrLength), text.data()};
				intptr_t ret = pluginapi::apiSendDlgMessage(hDlg, DM_GETTEXT, Param1, &did);
				didA->PtrLength = static_cast<int>(did.PtrLength);
				(void)encoding::oem::get_bytes({ text.data(), did.PtrLength }, { didA->PtrData, static_cast<size_t>(didA->PtrLength + 1) });
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
				return pluginapi::apiSendDlgMessage(hDlg, DM_KEY, Param1, KeysW.data());
			}
			case oldfar::DM_MOVEDIALOG: Msg = DM_MOVEDIALOG; break;
			case oldfar::DM_SETDLGDATA: Msg = DM_SETDLGDATA; break;
			case oldfar::DM_SETDLGITEM:
			{
				if (!Param2)
					return FALSE;

				auto& di = CurrentDialogItem(hDlg, Param1);

				if (di.Type==DI_LISTBOX || di.Type==DI_COMBOBOX)
					di.ListItems = &CurrentList(hDlg,Param1);

				FreeUnicodeDialogItem(di);
				const auto diA = static_cast<const oldfar::FarDialogItem*>(Param2);
				AnsiDialogItemToUnicode(*diA, di, *di.ListItems);

				// save color info
				if(diA->Flags&oldfar::DIF_SETCOLOR)
				{
					const auto diA_Copy = CurrentDialogItemA(hDlg, Param1);
					diA_Copy->Flags = diA->Flags;
				}

				return pluginapi::apiSendDlgMessage(hDlg, DM_SETDLGITEM, Param1, &di);
			}
			case oldfar::DM_SETFOCUS: Msg = DM_SETFOCUS; break;
			case oldfar::DM_REDRAW:   Msg = DM_REDRAW; break;
			case oldfar::DM_SETTEXT:
			{
				if (!Param2)return 0;

				const auto didA = static_cast<const oldfar::FarDialogItemData*>(Param2);

				if (!didA->PtrData) return 0;

				//BUGBUG - PtrLength ни на что не влияет.
				const auto text(encoding::oem::get_chars(didA->PtrData));
				FarDialogItemData di = {sizeof(FarDialogItemData), text.size(), UNSAFE_CSTR(text)};
				return pluginapi::apiSendDlgMessage(hDlg, DM_SETTEXT, Param1, &di);
			}
			case oldfar::DM_SETMAXTEXTLENGTH: Msg = DM_SETMAXTEXTLENGTH; break;
			case oldfar::DM_SHOWDIALOG:       Msg = DM_SHOWDIALOG; break;
			case oldfar::DM_GETFOCUS:         Msg = DM_GETFOCUS; break;
			case oldfar::DM_GETCURSORPOS:     Msg = DM_GETCURSORPOS; break;
			case oldfar::DM_SETCURSORPOS:     Msg = DM_SETCURSORPOS; break;
			case oldfar::DM_GETTEXTPTR:
			{
				size_t length = pluginapi::apiSendDlgMessage(hDlg, DM_GETTEXT, Param1, nullptr);

				if (!Param2) return length;

				std::vector<wchar_t> text(length + 1);
				FarDialogItemData item = {sizeof(FarDialogItemData), static_cast<size_t>(length), text.data()};
				length = pluginapi::apiSendDlgMessage(hDlg, DM_GETTEXT, Param1, &item);
				(void)encoding::oem::get_bytes({ text.data(), length }, { static_cast<char*>(Param2), length + 1 });
				return length;
			}
			case oldfar::DM_SETTEXTPTR:
			{
				return Param2? pluginapi::apiSendDlgMessage(hDlg, DM_SETTEXTPTR, Param1, UNSAFE_CSTR(encoding::oem::get_chars(static_cast<const char*>(Param2)))) : FALSE;
			}
			case oldfar::DM_SHOWITEM: Msg = DM_SHOWITEM; break;
			case oldfar::DM_ADDHISTORY:
			{
				return Param2? pluginapi::apiSendDlgMessage(hDlg, DM_ADDHISTORY, Param1, UNSAFE_CSTR(encoding::oem::get_chars(static_cast<const char*>(Param2)))) : FALSE;
			}
			case oldfar::DM_GETCHECK:
			{
				intptr_t ret = pluginapi::apiSendDlgMessage(hDlg, DM_GETCHECK, Param1, nullptr);
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
				return pluginapi::apiSendDlgMessage(hDlg, DM_SETCHECK, Param1, ToPtr(State));
			}
			case oldfar::DM_SET3STATE: Msg = DM_SET3STATE; break;
			case oldfar::DM_LISTSORT:  Msg = DM_LISTSORT; break;
			case oldfar::DM_LISTGETITEM: //BUGBUG, недоделано в фаре
			{
				if (!Param2) return FALSE;

				const auto lgiA = static_cast<oldfar::FarListGetItem*>(Param2);
				FarListGetItem lgi = {sizeof(FarListGetItem),lgiA->ItemIndex};
				intptr_t ret = pluginapi::apiSendDlgMessage(hDlg, DM_LISTGETITEM, Param1, &lgi);
				UnicodeListItemToAnsi(&lgi.Item, &lgiA->Item);
				return ret;
			}
			case oldfar::DM_LISTGETCURPOS:

				if (Param2)
				{
					FarListPos lp={sizeof(FarListPos)};
					intptr_t ret=pluginapi::apiSendDlgMessage(hDlg, DM_LISTGETCURPOS, Param1, &lp);
					const auto lpA = static_cast<oldfar::FarListPos*>(Param2);
					lpA->SelectPos=lp.SelectPos;
					lpA->TopPos=lp.TopPos;
					return ret;
				}
				else return pluginapi::apiSendDlgMessage(hDlg, DM_LISTGETCURPOS, Param1, nullptr);

			case oldfar::DM_LISTSETCURPOS:
			{
				if (!Param2) return FALSE;

				const auto lpA = static_cast<const oldfar::FarListPos*>(Param2);
				FarListPos lp = {sizeof(FarListPos),lpA->SelectPos,lpA->TopPos};
				return pluginapi::apiSendDlgMessage(hDlg, DM_LISTSETCURPOS, Param1, &lp);
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

				return pluginapi::apiSendDlgMessage(hDlg, DM_LISTDELETE, Param1, Param2? &ld : nullptr);
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

				const auto ret = pluginapi::apiSendDlgMessage(hDlg, DM_LISTADD, Param1, Param2? &newlist : nullptr);

				for (const auto& i: Items)
				{
					delete[] i.Text;
				}

				return ret;
			}
			case oldfar::DM_LISTADDSTR:
			{
				std::unique_ptr<wchar_t[]> newstr(AnsiToUnicode(static_cast<const char*>(Param2)));
				return pluginapi::apiSendDlgMessage(hDlg, DM_LISTADDSTR, Param1, newstr.get());
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

				intptr_t ret = pluginapi::apiSendDlgMessage(hDlg, DM_LISTUPDATE, Param1, Param2? &newui : nullptr);

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

				intptr_t ret = pluginapi::apiSendDlgMessage(hDlg, DM_LISTINSERT, Param1, Param2? &newli : nullptr);

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

				intptr_t ret = pluginapi::apiSendDlgMessage(hDlg, DM_LISTFINDSTRING, Param1, Param2? &newlf : nullptr);

				delete[] newlf.Pattern;

				return ret;
			}
			case oldfar::DM_LISTINFO:
			{
				if (!Param2) return FALSE;

				FarListInfo li={sizeof(FarListInfo)};
				intptr_t Result=pluginapi::apiSendDlgMessage(hDlg, DM_LISTINFO, Param1, &li);
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
				intptr_t Size = pluginapi::apiSendDlgMessage(hDlg, DM_LISTGETDATASIZE, Param1, Param2);
				intptr_t Data = pluginapi::apiSendDlgMessage(hDlg, DM_LISTGETDATA, Param1, Param2);
				if(Size<=4) Data=Data?*reinterpret_cast<unsigned*>(Data):0;
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
						newlid.DataSize = (wcslen(static_cast<wchar_t*>(oldlid->Data)) + 1) * sizeof(wchar_t);
					}
					else if(newlid.DataSize<=4)
					{
						newlid.Data=&oldlid->Data;
					}
				}

				return pluginapi::apiSendDlgMessage(hDlg, DM_LISTSETDATA, Param1, Param2? &newlid : nullptr);
			}
			case oldfar::DM_LISTSETTITLES:
			{
				if (!Param2)
					return FALSE;

				const auto ltA = static_cast<const oldfar::FarListTitles*>(Param2);
				const std::unique_ptr<wchar_t[]> Title(AnsiToUnicode(ltA->Title)), Bottom(AnsiToUnicode(ltA->Bottom));
				FarListTitles lt = {sizeof(FarListTitles), 0, Title.get(), 0 , Bottom.get()};
				return pluginapi::apiSendDlgMessage(hDlg, DM_LISTSETTITLES, Param1, &lt);
			}
			case oldfar::DM_LISTGETTITLES:
			{
				if (Param2)
				{
					const auto OldListTitle = static_cast<const oldfar::FarListTitles*>(Param2);
					FarListTitles ListTitle={sizeof(FarListTitles)};
					std::unique_ptr<wchar_t[]> Title, Bottom;

					if (OldListTitle->Title)
					{
						ListTitle.TitleSize = OldListTitle->TitleLen + 1;
						Title = std::make_unique<wchar_t[]>(ListTitle.TitleSize);
						ListTitle.Title = Title.get();
					}

					if (OldListTitle->Bottom)
					{
						ListTitle.BottomSize=OldListTitle->BottomLen+1;
						Bottom = std::make_unique<wchar_t[]>(ListTitle.BottomSize);
						ListTitle.Bottom = Bottom.get();
					}

					const auto Ret = pluginapi::apiSendDlgMessage(hDlg, DM_LISTGETTITLES, Param1, &ListTitle);

					if (Ret)
					{
						if (OldListTitle->Title)
							(void)encoding::oem::get_bytes(ListTitle.Title, { OldListTitle->Title, static_cast<size_t>(OldListTitle->TitleLen) });

						if (OldListTitle->Bottom)
							(void)encoding::oem::get_bytes(ListTitle.Bottom, { OldListTitle->Bottom, static_cast<size_t>(OldListTitle->BottomLen) });
					}

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
					return pluginapi::apiSendDlgMessage(hDlg, DM_SETHISTORY, Param1, nullptr);
				else
				{
					FarDialogItem& di = CurrentDialogItem(hDlg,Param1);
					delete[] di.History;
					di.History = AnsiToUnicode(static_cast<const char*>(Param2));
					return pluginapi::apiSendDlgMessage(hDlg, DM_SETHISTORY, Param1, const_cast<wchar_t*>(di.History));
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
						auto Items = std::make_unique<FarListItem[]>(newlist.ItemsNumber);
						for (size_t i = 0; i != newlist.ItemsNumber; ++i)
						{
							AnsiListItemToUnicode(&oldlist->Items[i], &Items[i]);
						}
						newlist.Items = Items.release();
					}
				}

				const auto ret = pluginapi::apiSendDlgMessage(hDlg, DM_LISTSET, Param1, Param2? &newlist : nullptr);

				if (newlist.Items)
				{
					for (const auto& i: span(newlist.Items, newlist.ItemsNumber))
					{
						delete[] i.Text;
					}

					delete[] newlist.Items;
				}

				return ret;
			}
			case oldfar::DM_LISTSETMOUSEREACTION:
			{
				FarDialogItem DlgItem = {};
				pluginapi::apiSendDlgMessage(hDlg, DM_GETDLGITEMSHORT, Param1, &DlgItem);
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
				pluginapi::apiSendDlgMessage(hDlg, DM_SETDLGITEMSHORT, Param1, &DlgItem);
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
				intptr_t ret=pluginapi::apiSendDlgMessage(hDlg, DM_GETSELECTION, Param1, &es);
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
				return pluginapi::apiSendDlgMessage(hDlg, DM_SETSELECTION, Param1, &es);
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
		return pluginapi::apiSendDlgMessage(hDlg, Msg, Param1, Param2);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return 0;
	});
}

static int WINAPI FarDialogExA(intptr_t PluginNumber, int X1, int Y1, int X2, int Y2, const char *HelpTopic, oldfar::FarDialogItem *Items, int ItemsNumber, DWORD, DWORD Flags, oldfar::FARWINDOWPROC DlgProc, void* Param) noexcept
{
	return cpp_try(
	[&]() -> intptr_t
	{
		span ItemsSpan(Items, ItemsNumber);

		std::vector<oldfar::FarDialogItem> diA(ItemsSpan.size());

		// to save DIF_SETCOLOR state
		for (const auto& [a, w]: zip(diA, ItemsSpan))
		{
			a.Flags = w.Flags;
		}

		std::vector<FarDialogItem> di(ItemsSpan.size());
		std::vector<FarList> l(di.size());

		for (const auto& i: zip(ItemsSpan, di, l))
		{
			std::apply(AnsiDialogItemToUnicode, i);
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

		const auto hDlg = pluginapi::apiDialogInit(GetPluginUuid(PluginNumber), &FarUuid, X1, Y1, X2, Y2, (HelpTopic? encoding::oem::get_chars(HelpTopic).c_str() : nullptr), di.data(), di.size(), 0, DlgFlags, DlgProcA, Param);
		if (hDlg == INVALID_HANDLE_VALUE)
			return -1;

		DialogData NewDialogData;
		NewDialogData.DlgProc = DlgProc;
		NewDialogData.hDlg = hDlg;
		NewDialogData.diA = diA.data();
		NewDialogData.di = di.data();
		NewDialogData.l = l.data();

		Dialogs().emplace(hDlg, NewDialogData);
		SCOPE_EXIT{ Dialogs().erase(hDlg); };

		const auto ret = pluginapi::apiDialogRun(hDlg);

		for (int i = 0; i != ItemsNumber; ++i)
		{
			size_t const Size = pluginapi::apiSendDlgMessage(hDlg, DM_GETDLGITEM, i, nullptr);
			block_ptr<FarDialogItem> Buffer(Size);
			FarGetDialogItem gdi = {sizeof(FarGetDialogItem), Size, Buffer.data()};

			if (gdi.Item)
			{
				pluginapi::apiSendDlgMessage(hDlg, DM_GETDLGITEM, i, &gdi);
				UnicodeDialogItemToAnsiSafe(*gdi.Item, ItemsSpan[i]);
				const auto res = NullToEmpty(gdi.Item->Data);

				if ((di[i].Type==DI_EDIT || di[i].Type==DI_COMBOBOX) && ItemsSpan[i].Flags&oldfar::DIF_VAREDIT)
					(void)encoding::oem::get_bytes(res, { ItemsSpan[i].Ptr.PtrData, static_cast<size_t>(ItemsSpan[i].Ptr.PtrLength + 1) });
				else
					(void)encoding::oem::get_bytes(res, ItemsSpan[i].Data);

				if (gdi.Item->Type==DI_USERCONTROL)
				{
					di[i].VBuf=gdi.Item->VBuf;
					ItemsSpan[i].VBuf=GetAnsiVBufPtr(gdi.Item->VBuf,GetAnsiVBufSize(ItemsSpan[i]));
				}

				if (gdi.Item->Type==DI_COMBOBOX || gdi.Item->Type==DI_LISTBOX)
				{
					ItemsSpan[i].ListPos = static_cast<int>(pluginapi::apiSendDlgMessage(hDlg, DM_LISTGETCURPOS, i, nullptr));
				}
			}

			FreeAnsiDialogItem(diA[i]);
		}

		pluginapi::apiDialogFree(hDlg);

		for (int i=0; i<ItemsNumber; i++)
		{
			if (di[i].Type==DI_LISTBOX || di[i].Type==DI_COMBOBOX)
				di[i].ListItems = &CurrentList(hDlg,i);

			FreeUnicodeDialogItem(di[i]);
		}

		return ret;
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return -1;
	});
}

static int WINAPI FarDialogFnA(intptr_t PluginNumber, int X1, int Y1, int X2, int Y2, const char *HelpTopic, oldfar::FarDialogItem *Item, int ItemsNumber) noexcept
{
	// noexcept
	return FarDialogExA(PluginNumber, X1, Y1, X2, Y2, HelpTopic, Item, ItemsNumber, 0, 0, nullptr, nullptr);
}

static int WINAPI FarPanelControlA(HANDLE hPlugin, int Command, void *Param) noexcept
{
	return cpp_try(
	[&]
	{
		static oldPanelInfoContainer PanelInfoA, AnotherPanelInfoA;

		if (!hPlugin || hPlugin==INVALID_HANDLE_VALUE)
			hPlugin=PANEL_ACTIVE;

		switch (Command)
		{
			case oldfar::FCTL_CHECKPANELSEXIST:
				return static_cast<int>(pluginapi::apiPanelControl(hPlugin,FCTL_CHECKPANELSEXIST,0,Param));
			case oldfar::FCTL_CLOSEPLUGIN:
			{
				std::unique_ptr<wchar_t[]> ParamW(AnsiToUnicode(static_cast<const char*>(Param)));
				return static_cast<int>(pluginapi::apiPanelControl(hPlugin,FCTL_CLOSEPANEL,0,ParamW.get()));
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
				const auto ret = static_cast<int>(pluginapi::apiPanelControl(hPlugin,FCTL_GETPANELINFO,0,&PI));

				if (ret)
				{
					FreeAnsiPanelInfo(OldPI);
					ConvertUnicodePanelInfoToAnsi(&PI,OldPI);

					const auto CreatePanelItems = [hPlugin](FILE_CONTROL_COMMANDS ControlCode, oldfar::PluginPanelItem*& Dest, size_t ItemsNumber)
					{
						if (!ItemsNumber)
							return;

						auto Items = std::make_unique<oldfar::PluginPanelItem[]>(ItemsNumber);

						block_ptr<PluginPanelItem> PPI;
						size_t PPISize = 0;

						for (size_t i = 0; i != ItemsNumber; ++i)
						{
							const auto NewPPISize = static_cast<size_t>(pluginapi::apiPanelControl(hPlugin, ControlCode, i, nullptr));

							if (NewPPISize > PPISize)
							{
								PPI.reset(NewPPISize);
								PPISize = NewPPISize;
							}
							FarGetPluginPanelItem gpi { sizeof(FarGetPluginPanelItem), PPISize, PPI.data() };
							pluginapi::apiPanelControl(hPlugin, ControlCode, i, &gpi);
							if (PPI)
							{
								ConvertPanelItemToAnsi(*PPI, Items[i]);
							}
						}
						Dest = Items.release();
					};

					CreatePanelItems(FCTL_GETPANELITEM, OldPI->PanelItems, OldPI->ItemsNumber);
					CreatePanelItems(FCTL_GETSELECTEDPANELITEM, OldPI->SelectedItems, OldPI->SelectedItemsNumber);

					if(const size_t dirSize = pluginapi::apiPanelControl(hPlugin, FCTL_GETPANELDIRECTORY, 0, nullptr))
					{
						block_ptr<FarPanelDirectory> dirInfo(dirSize);
						dirInfo->StructSize=sizeof(FarPanelDirectory);
						pluginapi::apiPanelControl(hPlugin, FCTL_GETPANELDIRECTORY, dirSize, dirInfo.data());
						(void)encoding::oem::get_bytes(dirInfo->Name,OldPI->CurDir);
					}

					wchar_t ColumnTypes[sizeof(OldPI->ColumnTypes)];
					pluginapi::apiPanelControl(hPlugin, FCTL_GETCOLUMNTYPES, std::size(ColumnTypes),ColumnTypes);
					(void)encoding::oem::get_bytes(ColumnTypes,OldPI->ColumnTypes);

					wchar_t ColumnWidths[sizeof(OldPI->ColumnWidths)];
					pluginapi::apiPanelControl(hPlugin, FCTL_GETCOLUMNWIDTHS, std::size(ColumnWidths), ColumnWidths);
					(void)encoding::oem::get_bytes(ColumnWidths,OldPI->ColumnWidths);

					*static_cast<oldfar::PanelInfo*>(Param) = *OldPI;
				}
				else
				{
					*static_cast<oldfar::PanelInfo*>(Param) = {};
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
				*OldPI = {};

				if (Command==oldfar::FCTL_GETANOTHERPANELSHORTINFO)
					hPlugin=PANEL_PASSIVE;

				PanelInfo PI = {sizeof(PanelInfo)};
				const auto ret = static_cast<int>(pluginapi::apiPanelControl(hPlugin,FCTL_GETPANELINFO,0,&PI));

				if (ret)
				{
					ConvertUnicodePanelInfoToAnsi(&PI,OldPI);
					size_t dirSize = pluginapi::apiPanelControl(hPlugin, FCTL_GETPANELDIRECTORY, 0, nullptr);
					if(dirSize)
					{
						block_ptr<FarPanelDirectory> dirInfo(dirSize);
						dirInfo->StructSize=sizeof(FarPanelDirectory);
						pluginapi::apiPanelControl(hPlugin, FCTL_GETPANELDIRECTORY, dirSize, dirInfo.data());
						(void)encoding::oem::get_bytes(dirInfo->Name, OldPI->CurDir);
					}
					wchar_t ColumnTypes[sizeof(OldPI->ColumnTypes)];
					pluginapi::apiPanelControl(hPlugin,FCTL_GETCOLUMNTYPES, std::size(OldPI->ColumnTypes),ColumnTypes);
					(void)encoding::oem::get_bytes(ColumnTypes, OldPI->ColumnTypes);
					wchar_t ColumnWidths[sizeof(OldPI->ColumnWidths)];
					pluginapi::apiPanelControl(hPlugin,FCTL_GETCOLUMNWIDTHS,sizeof(OldPI->ColumnWidths),ColumnWidths);
					(void)encoding::oem::get_bytes(ColumnWidths, OldPI->ColumnWidths);
				}

				return ret;
			}

		case oldfar::FCTL_SETANOTHERSELECTION:
			hPlugin=PANEL_PASSIVE;
			[[fallthrough]];
		case oldfar::FCTL_SETSELECTION:
			{
				if (!Param)
					return FALSE;

				const auto OldPI = static_cast<const oldfar::PanelInfo*>(Param);
				pluginapi::apiPanelControl(hPlugin, FCTL_BEGINSELECTION, 0, nullptr);

				for (int i=0; i<OldPI->ItemsNumber; i++)
				{
					pluginapi::apiPanelControl(hPlugin,FCTL_SETSELECTION,i,ToPtr(OldPI->PanelItems[i].Flags & oldfar::PPIF_SELECTED));
				}

				pluginapi::apiPanelControl(hPlugin, FCTL_ENDSELECTION, 0, nullptr);
				return TRUE;
			}

		case oldfar::FCTL_REDRAWANOTHERPANEL:
			hPlugin = PANEL_PASSIVE;
			[[fallthrough]];
		case oldfar::FCTL_REDRAWPANEL:
			{
				if (!Param)
					return static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_REDRAWPANEL, 0, nullptr));

				const auto priA = static_cast<const oldfar::PanelRedrawInfo*>(Param);
				PanelRedrawInfo pri = {sizeof(PanelRedrawInfo), static_cast<size_t>(priA->CurrentItem),static_cast<size_t>(priA->TopPanelItem)};
				return static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_REDRAWPANEL,0,&pri));
			}

		case oldfar::FCTL_SETANOTHERPANELDIR:
				hPlugin = PANEL_PASSIVE;
				[[fallthrough]];
		case oldfar::FCTL_SETPANELDIR:
			{
				if (!Param)
					return FALSE;

				const auto Dir = encoding::oem::get_chars(static_cast<const char*>(Param));
				FarPanelDirectory dirInfo = { sizeof(FarPanelDirectory), Dir.c_str(), nullptr, FarUuid, nullptr };
				return static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_SETPANELDIRECTORY, 0, &dirInfo));
			}

			case oldfar::FCTL_SETANOTHERSORTMODE:
				hPlugin = PANEL_PASSIVE;
				[[fallthrough]];
			case oldfar::FCTL_SETSORTMODE:

				if (!Param)
					return FALSE;

				return static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_SETSORTMODE, *static_cast<int*>(Param), nullptr));

			case oldfar::FCTL_SETANOTHERSORTORDER:
				hPlugin = PANEL_PASSIVE;
				[[fallthrough]];
			case oldfar::FCTL_SETSORTORDER:
				return static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_SETSORTORDER, Param && *static_cast<int*>(Param), nullptr));

			case oldfar::FCTL_SETANOTHERVIEWMODE:
				hPlugin = PANEL_PASSIVE;
				[[fallthrough]];
			case oldfar::FCTL_SETVIEWMODE:
				return static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_SETVIEWMODE, Param? *static_cast<int *>(Param) : 0, nullptr));

			case oldfar::FCTL_UPDATEANOTHERPANEL:
				hPlugin = PANEL_PASSIVE;
				[[fallthrough]];
			case oldfar::FCTL_UPDATEPANEL:
				return static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_UPDATEPANEL, Param? 1 : 0, nullptr));

			case oldfar::FCTL_GETCMDLINE:
			case oldfar::FCTL_GETCMDLINESELECTEDTEXT:
			{
				if (Param)
				{
					const size_t Size = 1024;
					wchar_t _s[Size], *s=_s;
					pluginapi::apiPanelControl(hPlugin, FCTL_GETCMDLINE, Size, s);
					if(Command==oldfar::FCTL_GETCMDLINESELECTEDTEXT)
					{
						CmdLineSelect cls={sizeof(CmdLineSelect)};
						pluginapi::apiPanelControl(hPlugin,FCTL_GETCMDLINESELECTION, 0, &cls);
						if(cls.SelStart >=0 && cls.SelEnd > cls.SelStart)
						{
							s[cls.SelEnd] = 0;
							s += cls.SelStart;
						}
					}
					(void)encoding::oem::get_bytes(s, { static_cast<char*>(Param), Size });
					return TRUE;
				}

				return FALSE;
			}
			case oldfar::FCTL_GETCMDLINEPOS:

				if (!Param)
					return FALSE;

				return static_cast<int>(pluginapi::apiPanelControl(hPlugin,FCTL_GETCMDLINEPOS,0,Param));
			case oldfar::FCTL_GETCMDLINESELECTION:
			{
				if (!Param)
					return FALSE;

				CmdLineSelect cls={sizeof(CmdLineSelect)};
				const auto ret = static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_GETCMDLINESELECTION,0,&cls));

				if (ret)
				{
					const auto clsA = static_cast<oldfar::CmdLineSelect*>(Param);
					clsA->SelStart = cls.SelStart;
					clsA->SelEnd = cls.SelEnd;
				}

				return ret;
			}

			case oldfar::FCTL_INSERTCMDLINE:
				return Param ? static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_INSERTCMDLINE, 0, UNSAFE_CSTR(encoding::oem::get_chars(static_cast<const char*>(Param))))) : FALSE;

			case oldfar::FCTL_SETCMDLINE:
				return Param ? static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_SETCMDLINE, 0, UNSAFE_CSTR(encoding::oem::get_chars(static_cast<const char*>(Param))))) : FALSE;

			case oldfar::FCTL_SETCMDLINEPOS:
				return Param ? static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_SETCMDLINEPOS, *static_cast<int*>(Param), nullptr)) : FALSE;

			case oldfar::FCTL_SETCMDLINESELECTION:
			{
				if (!Param)
					return FALSE;

				const auto clsA = static_cast<const oldfar::CmdLineSelect*>(Param);
				CmdLineSelect cls = {sizeof(CmdLineSelect),clsA->SelStart,clsA->SelEnd};
				return static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_SETCMDLINESELECTION,0,&cls));
			}
			case oldfar::FCTL_GETUSERSCREEN:
				return static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_GETUSERSCREEN, 0, nullptr));
			case oldfar::FCTL_SETUSERSCREEN:
				return static_cast<int>(pluginapi::apiPanelControl(hPlugin, FCTL_SETUSERSCREEN, 0, nullptr));
		}
		return FALSE;
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static HANDLE WINAPI FarSaveScreenA(int X1, int Y1, int X2, int Y2) noexcept
{
	return cpp_try(
	[&]
	{
		return pluginapi::apiSaveScreen(X1, Y1, X2, Y2);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return nullptr;
	});
}

static void WINAPI FarRestoreScreenA(HANDLE Screen) noexcept
{
	return cpp_try(
	[&]
	{
		return pluginapi::apiRestoreScreen(Screen);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
	});
}

static int GetDirListGeneric(oldfar::PluginPanelItem*& PanelItems, int& ItemsSize, function_ref<int(PluginPanelItem*&, size_t&, size_t&)> const Getter)
{
	PanelItems = nullptr;
	ItemsSize = 0;

	PluginPanelItem* Items;
	size_t Size;
	size_t PathOffset;

	const auto Result = Getter(Items, Size, PathOffset);

	if (Result && Size)
	{
		// + 1 чтобы хранить размер ибо в FarFreeDirListA как-то надо знать
		auto AnsiItems = std::make_unique<oldfar::PluginPanelItem[]>(Size + 1);
		AnsiItems[0].Reserved[0] = Size;

		for (size_t i = 0; i != Size; i++)
		{
			ConvertPanelItemToAnsi(Items[i], AnsiItems[i + 1], PathOffset);
		}

		pluginapi::apiFreeDirList(Items, Size);

		ItemsSize = static_cast<int>(Size);
		PanelItems = AnsiItems.release() + 1;
	}

	return Result;
}


static int WINAPI FarGetDirListA(const char *Dir, oldfar::PluginPanelItem **pPanelItem, int *pItemsNumber) noexcept
{
	return cpp_try(
	[&]
	{
		return GetDirListGeneric(*pPanelItem, *pItemsNumber, [Dir](PluginPanelItem*& Items, size_t& Size, size_t& PathOffset)
		{
			auto strDir = encoding::oem::get_chars(Dir);
			DeleteEndSlash(strDir);
			PathOffset = ExtractFilePath(strDir).size() + 1;
			return pluginapi::apiGetDirList(strDir.c_str(), &Items, &Size);
		});
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static int WINAPI FarGetPluginDirListA(intptr_t PluginNumber, HANDLE hPlugin, const char *Dir, oldfar::PluginPanelItem **pPanelItem, int *pItemsNumber) noexcept
{
	return cpp_try(
	[&]
	{
		return GetDirListGeneric(*pPanelItem, *pItemsNumber, [&](PluginPanelItem*& Items, size_t& Size, size_t& PathOffset)
		{
			PathOffset = 0;
			return pluginapi::apiGetPluginDirList(GetPluginUuid(PluginNumber), hPlugin, encoding::oem::get_chars(Dir).c_str(), &Items, &Size);
		});
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static void WINAPI FarFreeDirListA(const oldfar::PluginPanelItem *PanelItem) noexcept
{
	return cpp_try(
	[&]
	{
		//Тут хранится ItemsNumber полученный в FarGetDirListA или FarGetPluginDirListA
		--PanelItem;
		const size_t count = PanelItem->Reserved[0];
		FreePanelItemA({ PanelItem, count });
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
	});
}

static intptr_t WINAPI FarAdvControlA(intptr_t ModuleNumber, oldfar::ADVANCED_CONTROL_COMMANDS Command, void *Param) noexcept
{
	return cpp_try(
	[&]() -> intptr_t
	{
		switch (Command)
		{
			case oldfar::ACTL_GETFARVERSION:
			{
				VersionInfo Info;
				pluginapi::apiAdvControl(GetPluginUuid(ModuleNumber), ACTL_GETFARMANAGERVERSION, 0, &Info);
				DWORD FarVer = Info.Major<<8|Info.Minor|Info.Build<<16;

				if (Param)
					*static_cast<DWORD*>(Param) = FarVer;

				return FarVer;
			}
			case oldfar::ACTL_CONSOLEMODE:
				return IsConsoleFullscreen();

			case oldfar::ACTL_GETSYSWORDDIV:
			{
				intptr_t Result = 0;
				FarSettingsCreate settings = { sizeof(FarSettingsCreate), FarUuid, INVALID_HANDLE_VALUE };
				HANDLE Settings = pluginapi::apiSettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &settings)? settings.Handle : nullptr;
				if(Settings)
				{
					FarSettingsItem item={sizeof(FarSettingsItem),FSSF_EDITOR,L"WordDiv",FST_UNKNOWN,{}};
					if(pluginapi::apiSettingsControl(Settings,SCTL_GET,0,&item)&&FST_STRING==item.Type)
					{
						const auto Length = wcslen(item.String);
						Result = Length + 1;
						if (Param)
							(void)encoding::oem::get_bytes({ item.String, Length }, { static_cast<char*>(Param), static_cast<size_t>(oldfar::NM) });
					}
					pluginapi::apiSettingsControl(Settings, SCTL_FREE, 0, nullptr);
				}
				return Result;
			}
			case oldfar::ACTL_WAITKEY:
				{
					INPUT_RECORD input = {};
					KeyToInputRecord(OldKeyToKey(static_cast<int>(reinterpret_cast<intptr_t>(Param))),&input);
					return pluginapi::apiAdvControl(GetPluginUuid(ModuleNumber), ACTL_WAITKEY, 0, &input);
				}

			case oldfar::ACTL_GETCOLOR:
				{
					FarColor Color;
					int ColorIndex = static_cast<int>(reinterpret_cast<intptr_t>(Param));

					// there was a reserved position after COL_VIEWERARROWS in Far 1.x.
					if(ColorIndex > COL_VIEWERARROWS)
					{
						ColorIndex--;
					}
					return pluginapi::apiAdvControl(GetPluginUuid(ModuleNumber), ACTL_GETCOLOR, ColorIndex, &Color)? colors::FarColorToConsoleColor(Color) :-1;
				}

			case oldfar::ACTL_GETARRAYCOLOR:
				{
					const auto PaletteSize = pluginapi::apiAdvControl(GetPluginUuid(ModuleNumber), ACTL_GETARRAYCOLOR, 0, nullptr);
					if(Param)
					{
						std::vector<FarColor> Color(PaletteSize);
						pluginapi::apiAdvControl(GetPluginUuid(ModuleNumber), ACTL_GETARRAYCOLOR, 0, Color.data());
						const auto OldColors = static_cast<LPBYTE>(Param);
						std::transform(ALL_CONST_RANGE(Color), OldColors, colors::FarColorToConsoleColor);
					}
					return PaletteSize;
				}

			case oldfar::ACTL_EJECTMEDIA:
				return FALSE;

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
					res = pluginapi::apiMacroControl(nullptr, MacroCommand, Param1, &mtW);

					if (MacroCommand == MCTL_SENDSTRING)
					{
						switch (Param1)
						{
							case MSSC_CHECK:
								kmA->MacroResult.ErrMsg1 = "";
								kmA->MacroResult.ErrMsg2 = "";
								kmA->MacroResult.ErrMsg3 = "";
								[[fallthrough]];
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

				auto strSequence = L"Keys(\""s;
				for (const auto& Key: span(ksA->Sequence, ksA->Count))
				{
					if (const auto KeyText = KeyToText(OldKeyToKey(Key)); !KeyText.empty())
					{
						append(strSequence, L' ', KeyText);
					}
				}
				append(strSequence, L"\")"sv);

				return Global->CtrlObject->Macro.PostNewMacro(strSequence.c_str(), Flags);
			}
			case oldfar::ACTL_GETSHORTWINDOWINFO:
			case oldfar::ACTL_GETWINDOWINFO:
			{
				if (!Param)
					return FALSE;

				const auto wiA = static_cast<oldfar::WindowInfo*>(Param);
				WindowInfo wi={sizeof(wi)};
				wi.Pos = wiA->Pos;
				intptr_t ret = pluginapi::apiAdvControl(GetPluginUuid(ModuleNumber), ACTL_GETWINDOWINFO, 0, &wi);

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
							pluginapi::apiAdvControl(GetPluginUuid(ModuleNumber),ACTL_GETWINDOWINFO, 0, &wi);
							(void)encoding::oem::get_bytes(wi.TypeName, wiA->TypeName);
							(void)encoding::oem::get_bytes(wi.Name, wiA->Name);
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
				return pluginapi::apiAdvControl(GetPluginUuid(ModuleNumber), ACTL_GETWINDOWCOUNT, 0, nullptr);
			case oldfar::ACTL_SETCURRENTWINDOW:
				return pluginapi::apiAdvControl(GetPluginUuid(ModuleNumber), ACTL_SETCURRENTWINDOW, reinterpret_cast<intptr_t>(Param), nullptr);
			case oldfar::ACTL_COMMIT:
				return pluginapi::apiAdvControl(GetPluginUuid(ModuleNumber), ACTL_COMMIT, 0, nullptr);
			case oldfar::ACTL_GETFARHWND:
				return pluginapi::apiAdvControl(GetPluginUuid(ModuleNumber), ACTL_GETFARHWND, 0, nullptr);

			case oldfar::ACTL_GETSYSTEMSETTINGS:
			{
				return oldfar::FSS_CLEARROATTRIBUTE |
					(GetSetting(FSSF_SYSTEM, L"DeleteToRecycleBin")? oldfar::FSS_DELETETORECYCLEBIN : 0) |
					(GetSetting(FSSF_SYSTEM, L"CopyOpened")? oldfar::FSS_COPYFILESOPENEDFORWRITING : 0) |
					(GetSetting(FSSF_SYSTEM, L"ScanJunction")? oldfar::FSS_SCANSYMLINK : 0);
			}

			case oldfar::ACTL_GETPANELSETTINGS:
			{
				return
					(GetSetting(FSSF_PANEL, L"ShowHidden")? oldfar::FPS_SHOWHIDDENANDSYSTEMFILES : 0) |
					(GetSetting(FSSF_PANELLAYOUT, L"ColumnTitles")? oldfar::FPS_SHOWCOLUMNTITLES : 0) |
					(GetSetting(FSSF_PANELLAYOUT, L"StatusLine")? oldfar::FPS_SHOWSTATUSLINE : 0) |
					(GetSetting(FSSF_PANELLAYOUT, L"SortMode")? oldfar::FPS_SHOWSORTMODELETTER : 0);
			}

			case oldfar::ACTL_GETINTERFACESETTINGS:
			{
				return
					(GetSetting(FSSF_SCREEN, L"KeyBar")? oldfar::FIS_SHOWKEYBAR : 0) |
					(GetSetting(FSSF_INTERFACE, L"ShowMenuBar")? oldfar::FIS_ALWAYSSHOWMENUBAR : 0);
			}

			case oldfar::ACTL_GETCONFIRMATIONS:
			{
				return
					(GetSetting(FSSF_CONFIRMATIONS, L"Copy")? oldfar::FCS_COPYOVERWRITE : 0) |
					(GetSetting(FSSF_CONFIRMATIONS, L"Move")? oldfar::FCS_MOVEOVERWRITE : 0) |
					(GetSetting(FSSF_CONFIRMATIONS, L"Drag")? oldfar::FCS_DRAGANDDROP : 0) |
					(GetSetting(FSSF_CONFIRMATIONS, L"Delete")? oldfar::FCS_DELETE : 0) |
					(GetSetting(FSSF_CONFIRMATIONS, L"DeleteFolder")? oldfar::FCS_DELETENONEMPTYFOLDERS : 0) |
					(GetSetting(FSSF_CONFIRMATIONS, L"Esc")? oldfar::FCS_INTERRUPTOPERATION : 0) |
					(GetSetting(FSSF_CONFIRMATIONS, L"RemoveConnection")? oldfar::FCS_DISCONNECTNETWORKDRIVE : 0) |
					(GetSetting(FSSF_CONFIRMATIONS, L"HistoryClear")? oldfar::FCS_CLEARHISTORYLIST : 0) |
					(GetSetting(FSSF_CONFIRMATIONS, L"Exit")? oldfar::FCS_EXIT : 0);
			}

			case oldfar::ACTL_GETDESCSETTINGS:
			{
				const auto& DizSettings = Global->Opt->Diz;
				return
					(DizSettings.SetHidden? oldfar::FDS_SETHIDDEN : 0) |
					(DizSettings.UpdateMode == DIZ_UPDATE_ALWAYS? oldfar::FDS_UPDATEALWAYS : 0) |
					(DizSettings.UpdateMode == DIZ_UPDATE_IF_DISPLAYED? oldfar::FDS_UPDATEIFDISPLAYED : 0) |
					(DizSettings.ROUpdate? oldfar::FDS_UPDATEREADONLY : 0);
			}
			case oldfar::ACTL_SETARRAYCOLOR:
			{
				if (!Param)
					return FALSE;

				const auto scA = static_cast<const oldfar::FarSetColors*>(Param);
				std::vector<FarColor> Colors(scA->ColorCount);
				std::transform(scA->Colors, scA->Colors + scA->ColorCount, Colors.begin(), colors::ConsoleColorToFarColor);
				FarSetColors sc = {sizeof(FarSetColors), 0, static_cast<size_t>(scA->StartIndex), Colors.size(), Colors.data()};
				if (scA->Flags&oldfar::FCLR_REDRAW)
					sc.Flags|=FSETCLR_REDRAW;
				return pluginapi::apiAdvControl(GetPluginUuid(ModuleNumber), ACTL_SETARRAYCOLOR, 0, &sc);
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
			case oldfar::ACTL_REDRAWALL:
				return pluginapi::apiAdvControl(GetPluginUuid(ModuleNumber), ACTL_REDRAWALL, 0, nullptr);
		}
		return FALSE;
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static int WINAPI FarEditorControlA(oldfar::EDITOR_CONTROL_COMMANDS OldCommand, void* Param) noexcept
{
	return cpp_try(
	[&]
	{
		intptr_t et;
		EDITOR_CONTROL_COMMANDS Command;
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
					ec.Owner = FarUuid;
					EditorDeleteColor edc={sizeof(edc)};
					edc.Owner = FarUuid;
					edc.StringNumber = ecA->StringNumber;
					edc.StartPos = ecA->StartPos;
					return static_cast<int>(ecA->Color?pluginapi::apiEditorControl(-1, ECTL_ADDCOLOR, 0, &ec):pluginapi::apiEditorControl(-1, ECTL_DELCOLOR, 0, &edc));
				}
				return FALSE;
			case oldfar::ECTL_GETCOLOR:
				if(Param)
				{
					const auto ecA = static_cast<oldfar::EditorColor*>(Param);
					EditorColor ec={sizeof(ec)};
					ec.StringNumber = ecA->StringNumber;
					ec.ColorItem = ecA->ColorItem;
					const auto Result = static_cast<int>(pluginapi::apiEditorControl(-1, ECTL_GETCOLOR, 0, &ec));
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
				const auto ret = static_cast<int>(pluginapi::apiEditorControl(-1,ECTL_GETSTRING,0,&egs));

				if (ret)
				{
					oegs->StringNumber=egs.StringNumber;
					oegs->StringLength=egs.StringLength;
					oegs->SelStart=egs.SelStart;
					oegs->SelEnd=egs.SelEnd;

					const auto CodePage = GetEditorCodePageA();
					static std::unique_ptr<char[]> gt, geol;
					gt.reset(UnicodeToAnsiBin({ egs.StringText, static_cast<size_t>(egs.StringLength) }, CodePage));
					geol.reset(UnicodeToAnsiBin(egs.StringEOL, CodePage));
					oegs->StringText = gt.get();
					oegs->StringEOL = geol.get();
					return TRUE;
				}

				return FALSE;
			}
			case oldfar::ECTL_INSERTTEXT:
			{
				return Param ? static_cast<int>(pluginapi::apiEditorControl(-1, ECTL_INSERTTEXT, 0, UNSAFE_CSTR(encoding::oem::get_chars(static_cast<const char*>(Param))))) : FALSE;
			}
			case oldfar::ECTL_GETINFO:
			{
				EditorInfo ei={sizeof(EditorInfo)};
				const auto oei = static_cast<oldfar::EditorInfo*>(Param);

				if (!oei)
					return FALSE;

				const auto ret = static_cast<int>(pluginapi::apiEditorControl(-1,ECTL_GETINFO,0,&ei));

				if (ret)
				{
					*oei = {};
					if (const size_t FileNameSize = pluginapi::apiEditorControl(-1, ECTL_GETFILENAME, 0, nullptr))
					{
						wchar_t_ptr_n<os::default_buffer_size> FileName(FileNameSize);
						pluginapi::apiEditorControl(-1, ECTL_GETFILENAME, FileNameSize, FileName.data());
						static std::unique_ptr<char[]> fn;
						fn.reset(UnicodeToAnsi(FileName.data()));
						oei->FileName = fn.get();
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
					oei->BookMarkCount = static_cast<int>(ei.BookmarkCount);
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
				const auto CodePage = GetEditorCodePageA();
				MultiByteRecode(
					OldCommand == oldfar::ECTL_OEMTOEDITOR? CP_OEMCP : CodePage,
					OldCommand == oldfar::ECTL_OEMTOEDITOR? CodePage : CP_OEMCP,
					{ ect->Text, static_cast<size_t>(ect->TextLength) });
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
						FileName = encoding::oem::get_chars(oldsf->FileName);
						newsf.FileName = FileName.c_str();
					}
					if (oldsf->FileEOL)
					{
						EOL = encoding::oem::get_chars(oldsf->FileEOL);
						newsf.FileEOL = EOL.c_str();
					}
					newsf.CodePage = CP_DEFAULT;
				}

				return static_cast<int>(pluginapi::apiEditorControl(-1, ECTL_SAVEFILE, 0, Param? &newsf : nullptr));
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
							if (encoding::oem::get_chars({ &pIR->Event.KeyEvent.uChar.AsciiChar, 1 }, { &res, 1 }))
								pIR->Event.KeyEvent.uChar.UnicodeChar=res;
							break;
						}
						default:
							break;
					}
				}

				return static_cast<int>(pluginapi::apiEditorControl(-1,ECTL_PROCESSINPUT, 0, Param));
			}
			case oldfar::ECTL_PROCESSKEY:
			{
				INPUT_RECORD r={};
				KeyToInputRecord(OldKeyToKey(static_cast<int>(reinterpret_cast<intptr_t>(Param))),&r);
				return static_cast<int>(pluginapi::apiEditorControl(-1,ECTL_PROCESSINPUT, 0, &r));
			}
			case oldfar::ECTL_READINPUT:	//BUGBUG?
			{
				const auto ret = static_cast<int>(pluginapi::apiEditorControl(-1,ECTL_READINPUT, 0, Param));

				if (Param)
				{
					const auto pIR = static_cast<INPUT_RECORD*>(Param);

					switch (pIR->EventType)
					{
						case KEY_EVENT:
						{
							char res;
							if (encoding::oem::get_bytes({ &pIR->Event.KeyEvent.uChar.UnicodeChar, 1 }, { &res, 1 }))
								pIR->Event.KeyEvent.uChar.UnicodeChar=res;
						}
					}
				}

				return ret;
			}
			case oldfar::ECTL_SETKEYBAR:
			{
				switch (reinterpret_cast<intptr_t>(Param))
				{
					case 0:
					case -1:
						return static_cast<int>(pluginapi::apiEditorControl(-1,ECTL_SETKEYBAR, 0, Param));
					default:
						const auto oldkbt = static_cast<const oldfar::KeyBarTitles*>(Param);
						KeyBarTitles newkbt;
						FarSetKeyBarTitles newfskbt={sizeof(FarSetKeyBarTitles),&newkbt};
						ConvertKeyBarTitlesA(oldkbt, &newkbt);
						const auto ret = static_cast<int>(pluginapi::apiEditorControl(-1,ECTL_SETKEYBAR, 0, &newfskbt));
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
									newsp.iParam = encoding::codepage::oem();
									break;
								case 2:
									newsp.iParam = encoding::codepage::ansi();
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
								case oldfar::EXPAND_NOTABS:     newsp.iParam = EXPAND_NOTABS;   break;
								case oldfar::EXPAND_ALLTABS:    newsp.iParam = EXPAND_ALLTABS;  break;
								case oldfar::EXPAND_NEWTABS:    newsp.iParam = EXPAND_NEWTABS;  break;
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
								cParam = encoding::oem::get_chars(oldsp->cParam);
								newsp.wszParam = UNSAFE_CSTR(cParam);
							}
							return static_cast<int>(pluginapi::apiEditorControl(-1,ECTL_SETPARAM, 0, &newsp));
						}
						case oldfar::ESPT_GETWORDDIV:
						{
							if (!oldsp->cParam)
								return FALSE;

							newsp.Type = ESPT_GETWORDDIV;
							newsp.Size = pluginapi::apiEditorControl(-1,ECTL_SETPARAM, 0, &newsp);
							std::vector<wchar_t> Buffer(newsp.Size);
							newsp.wszParam = Buffer.data();
							const auto ret = static_cast<int>(pluginapi::apiEditorControl(-1,ECTL_SETPARAM, 0, &newsp));
							xstrncpy(oldsp->cParam, encoding::oem::get_bytes(newsp.wszParam).c_str(), 256);
							return ret;
						}
						default:
							return FALSE;
					}
				}

				return static_cast<int>(pluginapi::apiEditorControl(-1, ECTL_SETPARAM, 0, Param? &newsp : nullptr));
			}
			case oldfar::ECTL_SETSTRING:
			{
				EditorSetString newss = {sizeof(EditorSetString)};

				if (Param)
				{
					const auto oldss = static_cast<const oldfar::EditorSetString*>(Param);
					newss.StringNumber=oldss->StringNumber;
					const auto CodePage = GetEditorCodePageA();
					newss.StringText = oldss->StringText? AnsiToUnicodeBin({ oldss->StringText, static_cast<size_t>(oldss->StringLength) }, CodePage) : nullptr;
					newss.StringEOL = oldss->StringEOL? AnsiToUnicodeBin(oldss->StringEOL, CodePage) : nullptr;
					newss.StringLength=oldss->StringLength;
				}

				const auto ret = static_cast<int>(pluginapi::apiEditorControl(-1, ECTL_SETSTRING, 0, Param? &newss : nullptr));

				delete[] newss.StringText;
				delete[] newss.StringEOL;

				return ret;
			}
			case oldfar::ECTL_SETTITLE:
			{
				string newtit;

				if (Param)
				{
					newtit = encoding::oem::get_chars(static_cast<const char*>(Param));
				}
				return static_cast<int>(pluginapi::apiEditorControl(-1,ECTL_SETTITLE, 0, Param? UNSAFE_CSTR(newtit) : nullptr));
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
					if (!pluginapi::apiEditorControl(-1,ECTL_GETINFO,0,&ei)) return FALSE;
					return static_cast<int>(ei.SessionBookmarkCount);
				}
				Command = bStack ? ECTL_GETSESSIONBOOKMARKS : ECTL_GETBOOKMARKS;
				intptr_t size = pluginapi::apiEditorControl(-1,Command,0,nullptr);
				if (!size) return FALSE;
				block_ptr<EditorBookmarks> newbm(size);
				newbm->StructSize = sizeof(*newbm);
				newbm->Size = size;
				if (!pluginapi::apiEditorControl(-1, Command, 0, newbm.data()))
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
				return bStack? static_cast<int>(newbm->Count) : TRUE;
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
				const auto ret = static_cast<int>(pluginapi::apiEditorControl(-1, OldCommand == oldfar::ECTL_REALTOTAB ? ECTL_REALTOTAB : ECTL_TABTOREAL, 0, &newecp));
				oldecp->DestPos=newecp.DestPos;
				return ret;
			}
			case oldfar::ECTL_SELECT:
			{
				const auto oldes = static_cast<const oldfar::EditorSelect*>(Param);
				EditorSelect newes={sizeof(EditorSelect),oldes->BlockType,oldes->BlockStartLine,oldes->BlockStartPos,oldes->BlockWidth,oldes->BlockHeight};
				return static_cast<int>(pluginapi::apiEditorControl(-1, ECTL_SELECT, 0, &newes));
			}
			case oldfar::ECTL_REDRAW:				Command = ECTL_REDRAW; break;
			case oldfar::ECTL_SETPOSITION:
			{
				const auto oldsp = static_cast<const oldfar::EditorSetPosition*>(Param);
				EditorSetPosition newsp={sizeof(EditorSetPosition),oldsp->CurLine,oldsp->CurPos,oldsp->CurTabPos,oldsp->TopScreenLine,oldsp->LeftPos,oldsp->Overtype};
				return static_cast<int>(pluginapi::apiEditorControl(-1, ECTL_SETPOSITION, 0, &newsp));
			}
			case oldfar::ECTL_ADDSTACKBOOKMARK:			Command = ECTL_ADDSESSIONBOOKMARK; break;
			case oldfar::ECTL_PREVSTACKBOOKMARK:		Command = ECTL_PREVSESSIONBOOKMARK; break;
			case oldfar::ECTL_NEXTSTACKBOOKMARK:		Command = ECTL_NEXTSESSIONBOOKMARK; break;
			case oldfar::ECTL_CLEARSTACKBOOKMARKS:	Command = ECTL_CLEARSESSIONBOOKMARKS; break;
			case oldfar::ECTL_DELETESTACKBOOKMARK:	Command = ECTL_DELETESESSIONBOOKMARK; break;
			default:
				return FALSE;
		}
		return static_cast<int>(pluginapi::apiEditorControl(-1, Command, 0, Param));
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static int WINAPI FarViewerControlA(int Command, void* Param) noexcept
{
	return cpp_try(
	[&]
	{
		switch (Command)
		{
			case oldfar::VCTL_GETINFO:
			{
				if (!Param) return FALSE;

				const auto viA = static_cast<oldfar::ViewerInfo*>(Param);

				if (!viA->StructSize) return FALSE;

				ViewerInfo viW = {sizeof(viW)};

				if (pluginapi::apiViewerControl(-1,VCTL_GETINFO, 0, &viW) == FALSE) return FALSE;

				viA->ViewerID = viW.ViewerID;

				if (const size_t FileNameSize = pluginapi::apiViewerControl(-1, VCTL_GETFILENAME, 0, nullptr))
				{
					const wchar_t_ptr_n<os::default_buffer_size> FileName(FileNameSize);
					pluginapi::apiViewerControl(-1, VCTL_GETFILENAME, FileNameSize, FileName.data());
					static std::unique_ptr<char[]> filename;
					filename.reset(UnicodeToAnsi(FileName.data()));
					viA->FileName = filename.get();
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
				viA->CurMode.AnsiMode       = viW.CurMode.CodePage == encoding::codepage::ansi();
				viA->CurMode.Unicode        = IsUnicodeCodePage(viW.CurMode.CodePage);
				viA->CurMode.Wrap           = (viW.CurMode.Flags&VMF_WRAP)?1:0;
				viA->CurMode.WordWrap       = (viW.CurMode.Flags&VMF_WORDWRAP)?1:0;
				viA->CurMode.Hex            = viW.CurMode.ViewMode;
				viA->LeftPos = static_cast<int>(viW.LeftPos);
				viA->Reserved3 = 0;
				break;
			}
			case oldfar::VCTL_QUIT:
				return static_cast<int>(pluginapi::apiViewerControl(-1, VCTL_QUIT, 0, nullptr));
			case oldfar::VCTL_REDRAW:
				return static_cast<int>(pluginapi::apiViewerControl(-1, VCTL_REDRAW, 0, nullptr));
			case oldfar::VCTL_SETKEYBAR:
			{
				switch (reinterpret_cast<intptr_t>(Param))
				{
					case 0:
					case -1:
						return static_cast<int>(pluginapi::apiViewerControl(-1,VCTL_SETKEYBAR,0, Param));
					default:
						const auto kbtA = static_cast<const oldfar::KeyBarTitles*>(Param);
						KeyBarTitles kbt;
						FarSetKeyBarTitles newfskbt={sizeof(FarSetKeyBarTitles),&kbt};
						ConvertKeyBarTitlesA(kbtA, &kbt);
						const auto ret = static_cast<int>(pluginapi::apiViewerControl(-1,VCTL_SETKEYBAR,0, &newfskbt));
						FreeUnicodeKeyBarTitles(&kbt);
						return ret;
				}
			}
			case oldfar::VCTL_SETPOSITION:
			{
				if (!Param) return FALSE;

				const auto vspA = static_cast<oldfar::ViewerSetPosition*>(Param);
				ViewerSetPosition vsp={sizeof(ViewerSetPosition)};

				static const std::array PluginFlagsMap
				{
					OLDFAR_TO_FAR_MAP(VSP_NOREDRAW),
					OLDFAR_TO_FAR_MAP(VSP_PERCENT),
					OLDFAR_TO_FAR_MAP(VSP_RELATIVE),
					OLDFAR_TO_FAR_MAP(VSP_NORETNEWPOS),
				};

				vsp.Flags = VSP_NONE;
				FirstFlagsToSecond(vspA->Flags, vsp.Flags, PluginFlagsMap);

				vsp.StartPos = vspA->StartPos;
				vsp.LeftPos = vspA->LeftPos;
				const auto ret = static_cast<int>(pluginapi::apiViewerControl(-1,VCTL_SETPOSITION,0, &vsp));
				vspA->StartPos = vsp.StartPos;
				return ret;
			}
			case oldfar::VCTL_SELECT:
			{
				if (!Param)
					return static_cast<int>(pluginapi::apiViewerControl(-1, VCTL_SELECT, 0, nullptr));

				const auto vsA = static_cast<const oldfar::ViewerSelect*>(Param);
				ViewerSelect vs = {sizeof(ViewerSelect),vsA->BlockStartPos,vsA->BlockLen};
				return static_cast<int>(pluginapi::apiViewerControl(-1,VCTL_SELECT,0, &vs));
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

				return static_cast<int>(pluginapi::apiViewerControl(-1,VCTL_SETMODE,0, &vsm));
			}
		}
		return TRUE;
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
}

static int WINAPI FarCharTableA(int Command, char *Buffer, int BufferSize) noexcept
{
	return cpp_try(
	[&]
	{
		if (Command != oldfar::FCT_DETECT)
		{
			if (BufferSize != static_cast<int>(sizeof(oldfar::CharTableSet)))
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

			const auto Info = codepages::GetInfo(nCP);
			if (!Info || Info->MaxCharSize != 1)
				return -1;

			auto sTableName = pad_right(str(nCP), 5);
			append(sTableName, BoxSymbols[BS_V1], L' ', Info->Name);
			(void)encoding::oem::get_bytes(sTableName, TableSet->TableName);
			std::unique_ptr<wchar_t[]> const us(AnsiToUnicodeBin({ reinterpret_cast<char*>(TableSet->DecodeTable), std::size(TableSet->DecodeTable) }, nCP));

			inplace::lower({ us.get(), std::size(TableSet->DecodeTable) });
			(void)encoding::get_bytes(nCP, { us.get(), std::size(TableSet->DecodeTable) }, { reinterpret_cast<char*>(TableSet->LowerTable), std::size(TableSet->DecodeTable) });

			inplace::upper({ us.get(), std::size(TableSet->DecodeTable) });
			(void)encoding::get_bytes(nCP, { us.get(), std::size(TableSet->DecodeTable) }, { reinterpret_cast<char*>(TableSet->UpperTable), std::size(TableSet->DecodeTable) });

			MultiByteRecode(nCP, CP_OEMCP, { reinterpret_cast<char*>(TableSet->DecodeTable), std::size(TableSet->DecodeTable) });
			MultiByteRecode(CP_OEMCP, nCP, { reinterpret_cast<char*>(TableSet->EncodeTable), std::size(TableSet->EncodeTable) });
			return Command;
		}
		return -1;
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return -1;
	});
}

static char* WINAPI XlatA(
	char *Line,                    // исходная строка
	int StartPos,                  // начало переконвертирования
	int EndPos,                    // конец переконвертирования
	const oldfar::CharTableSet*,   // перекодировочная таблица (может быть nullptr)
	DWORD Flags)                   // флаги (см. enum XLATMODE)
{
	return cpp_try(
	[&]
	{
		static const std::array PluginFlagsMap
		{
			OLDFAR_TO_FAR_MAP(XLAT_SWITCHKEYBLAYOUT),
			OLDFAR_TO_FAR_MAP(XLAT_SWITCHKEYBBEEP),
			OLDFAR_TO_FAR_MAP(XLAT_USEKEYBLAYOUTNAME),
			OLDFAR_TO_FAR_MAP(XLAT_CONVERTALLCMDLINE),
		};

		auto NewFlags = XLAT_NONE;
		FirstFlagsToSecond(Flags, NewFlags, PluginFlagsMap);

		auto WideLine = encoding::oem::get_chars(Line);
		pluginapi::apiXlat(WideLine.data(), StartPos, EndPos, NewFlags);
		(void)encoding::oem::get_bytes(WideLine, { Line, WideLine.size() });
		return Line;
	},
	[&]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return Line;
	});
}

static int WINAPI GetFileOwnerA(const char *Computer, const char *Name, char *Owner) noexcept
{
	return cpp_try(
	[&]
	{
		wchar_t wOwner[MAX_PATH];
		const auto Ret = pluginapi::apiGetFileOwner(encoding::oem::get_chars(Computer).c_str(), encoding::oem::get_chars(Name).c_str(), wOwner, std::size(wOwner));
		if (Ret)
		{
			(void)encoding::oem::get_bytes({ wOwner, Ret - 1 }, { Owner, static_cast<size_t>(oldfar::NM) });
		}
		return static_cast<int>(Ret);
	},
	[]
	{
		SAVE_EXCEPTION_TO(GlobalExceptionPtr());
		return FALSE;
	});
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

static bool SendKeyToPluginHook(const Manager::Key& key)
{
	const auto KeyM = key() & ~KEY_CTRLMASK;

	if (!((KeyM >= KEY_MACRO_BASE && KeyM <= KEY_MACRO_ENDBASE) || (KeyM >= KEY_OP_BASE && KeyM <= KEY_OP_ENDBASE))) // пропустим макро-коды
	{
		if (Global->WindowManager->IsPanelsActive())
		{
			if (Global->CtrlObject->Cp()->ActivePanel()->GetMode() == panel_mode::PLUGIN_PANEL)
			{
				const auto ph = Global->CtrlObject->Cp()->ActivePanel()->GetPluginHandle();
				if (ph && ph->plugin()->IsOemPlugin() && Global->CtrlObject->Cp()->ActivePanel()->SendKeyToPlugin(key(), true))
					return true;
			}
		}
	}
	return false;
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

static const std::array OperationModesMap
{
	OLDFAR_TO_FAR_MAP(OPM_SILENT),
	OLDFAR_TO_FAR_MAP(OPM_FIND),
	OLDFAR_TO_FAR_MAP(OPM_VIEW),
	OLDFAR_TO_FAR_MAP(OPM_EDIT),
	OLDFAR_TO_FAR_MAP(OPM_TOPLEVEL),
	OLDFAR_TO_FAR_MAP(OPM_DESCR),
	OLDFAR_TO_FAR_MAP(OPM_QUICKVIEW),
};

static void* TranslateResult(void* hResult)
{
	if (INVALID_HANDLE_VALUE == hResult)
		return nullptr;
	if (hResult == ToPtr(-2))
		return PANEL_STOP;
	return hResult;
}

static void UpdatePluginPanelItemFlags(const oldfar::PluginPanelItem* From, PluginPanelItem* To, size_t Size)
{
	for (size_t i = 0; i != Size; ++i)
	{
		FirstFlagsToSecond(From[i].Flags, To[i].Flags, PluginPanelItemFlagsMap);
	}
}

// TODO: PluginA class shouldn't be derived from Plugin.
// All exports should be provided by oem_plugin_factory.
class PluginA: public Plugin
{
public:
	NONCOPYABLE(PluginA);
	PluginA(plugin_factory* Factory, const string& ModuleName):
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

	~PluginA() override
	{
		FreePluginInfo();
		FreeOpenPanelInfo();
	}

	const char *GetMsgA(int Id) const
	{
		return static_cast<const ansi_plugin_language&>(*PluginLang).GetMsgA(Id);
	}

private:
	bool GetGlobalInfo(GlobalInfo *Info) override
	{
		Info->StructSize = sizeof(GlobalInfo);
		Info->Description = L"Far 1.x plugin";
		Info->Author = L"Unknown";

		const string& Module = ModuleName();
		// Null-terminated, data() is ok
		Info->Title = PointToName(Module).data();

		bool UuidFound = false;

		auto& FileVersion = static_cast<oem_plugin_module*>(m_Instance.get())->m_FileVersion;

		if (FileVersion.read(ModuleName()))
		{
			const wchar_t* Value;
			if (((Value = FileVersion.get_string(L"InternalName"sv)) != nullptr || (Value = FileVersion.get_string(L"OriginalName"sv)) != nullptr) && *Value)
			{
				Info->Title = Value;
			}

			if (((Value = FileVersion.get_string(L"CompanyName"sv)) != nullptr || (Value = FileVersion.get_string(L"LegalCopyright"sv)) != nullptr) && *Value)
			{
				Info->Author = Value;
			}

			if ((Value = FileVersion.get_string(L"FileDescription"sv)) != nullptr && *Value)
			{
				Info->Description = Value;
			}

			if (const auto UuidStr = FileVersion.get_string(L"PluginGUID"sv))
			{
				if (const auto Uuid = uuid::try_parse(string_view(UuidStr)))
				{
					Info->Guid = *Uuid;
					UuidFound = true;
				}
			}

			if (const auto FileInfo = FileVersion.get_fixed_info())
			{
				Info->Version.Major = HIWORD(FileInfo->dwFileVersionMS);
				Info->Version.Minor = LOWORD(FileInfo->dwFileVersionMS);
				Info->Version.Build = HIWORD(FileInfo->dwFileVersionLS);
				Info->Version.Revision = LOWORD(FileInfo->dwFileVersionLS);
			}
		}

		if (equal_icase(Info->Title, L"FarFtp"sv) || equal_icase(Info->Title, L"MultiArc"sv))
			opif_shortcut = true;

		if (!UuidFound)
		{
			int nb = std::min(static_cast<int>(wcslen(Info->Title)), 8);
			while (nb > 0)
			{
				--nb;
				reinterpret_cast<char*>(&Info->Guid)[8 + nb] = static_cast<char>(Info->Title[nb]);
			}
		}

		return true;
	}

	bool SetStartupInfo(PluginStartupInfo*) override
	{
		AnsiExecuteStruct<iSetStartupInfo> es;
		if (has(es) && !Global->ProcessException)
		{
WARNING_PUSH()
WARNING_DISABLE_CLANG("-Wused-but-marked-unused")
			static const oldfar::FarStandardFunctions StandardFunctions =
			{
				sizeof(StandardFunctions),
				oldpluginapi::FarAtoiA,
				oldpluginapi::FarAtoi64A,
				oldpluginapi::FarItoaA,
				oldpluginapi::FarItoa64A,
				sprintf,
				sscanf,
				oldpluginapi::qsort,
				oldpluginapi::bsearch,
				oldpluginapi::qsortex,
				_snprintf,
				{},
				oldpluginapi::LocalIslower,
				oldpluginapi::LocalIsupper,
				oldpluginapi::LocalIsalpha,
				oldpluginapi::LocalIsalphanum,
				oldpluginapi::LocalUpper,
				oldpluginapi::LocalLower,
				oldpluginapi::LocalUpperBuf,
				oldpluginapi::LocalLowerBuf,
				oldpluginapi::LocalStrupr,
				oldpluginapi::LocalStrlwr,
				oldpluginapi::LStricmp,
				oldpluginapi::LStrnicmp,
				oldpluginapi::UnquoteA,
				oldpluginapi::ExpandEnvironmentStrA,
				oldpluginapi::RemoveLeadingSpacesA,
				oldpluginapi::RemoveTrailingSpacesA,
				oldpluginapi::RemoveExternalSpacesA,
				oldpluginapi::TruncStrA,
				oldpluginapi::TruncPathStrA,
				oldpluginapi::QuoteSpaceOnlyA,
				oldpluginapi::PointToNameA,
				oldpluginapi::GetPathRootA,
				oldpluginapi::AddEndSlashA,
				oldpluginapi::CopyToClipboardA,
				oldpluginapi::PasteFromClipboardA,
				oldpluginapi::FarKeyToNameA,
				oldpluginapi::KeyNameToKeyA,
				oldpluginapi::InputRecordToKeyA,
				oldpluginapi::XlatA,
				oldpluginapi::GetFileOwnerA,
				oldpluginapi::GetNumberOfLinksA,
				oldpluginapi::FarRecursiveSearchA,
				oldpluginapi::FarMkTempA,
				oldpluginapi::DeleteBufferA,
				oldpluginapi::ProcessNameA,
				oldpluginapi::FarMkLinkA,
				oldpluginapi::ConvertNameToRealA,
				oldpluginapi::FarGetReparsePointInfoA,
			};
WARNING_POP()

			// NOT constexpr, see VS bug #3103404
			static const oldfar::PluginStartupInfo StartupInfo =
			{
				sizeof(StartupInfo),
				"", // ModuleName, dynamic
				0, // ModuleNumber, dynamic
				{}, // RootKey, dynamic
				oldpluginapi::FarMenuFnA,
				oldpluginapi::FarDialogFnA,
				oldpluginapi::FarMessageFnA,
				oldpluginapi::FarGetMsgFnA,
				oldpluginapi::FarPanelControlA,
				oldpluginapi::FarSaveScreenA,
				oldpluginapi::FarRestoreScreenA,
				oldpluginapi::FarGetDirListA,
				oldpluginapi::FarGetPluginDirListA,
				oldpluginapi::FarFreeDirListA,
				oldpluginapi::FarViewerA,
				oldpluginapi::FarEditorA,
				oldpluginapi::FarCmpNameA,
				oldpluginapi::FarCharTableA,
				oldpluginapi::FarTextA,
				oldpluginapi::FarEditorControlA,
				{}, // FSF, dynamic
				oldpluginapi::FarShowHelpA,
				oldpluginapi::FarAdvControlA,
				oldpluginapi::FarInputBoxA,
				oldpluginapi::FarDialogExA,
				oldpluginapi::FarSendDlgMessageA,
				oldpluginapi::FarDefDlgProcA,
				0,
				oldpluginapi::FarViewerControlA,
			};

			auto InfoCopy = StartupInfo;
			auto FsfCopy = StandardFunctions;
			// скорректируем адреса и плагино-зависимые поля
			InfoCopy.ModuleNumber = reinterpret_cast<intptr_t>(this);
			InfoCopy.FSF = &FsfCopy;
			(void)encoding::oem::get_bytes(ModuleName(), InfoCopy.ModuleName);
			InfoCopy.RootKey = static_cast<oem_plugin_factory*>(m_Factory)->PluginsRootKey().c_str();

			if (Global->strRegUser.empty())
				os::env::del(L"FARUSER"sv);
			else
				os::env::set(L"FARUSER"sv, Global->strRegUser);

			ExecuteFunction(es, &InfoCopy);

			if (bPendingRemove)
			{
				return false;
			}
		}
		return true;
	}

	HANDLE Open(OpenInfo* Info) override
	{
		CheckScreenLock();

		AnsiExecuteStruct<iOpen> es;
		if (Global->ProcessException || !Load() || !has(es))
			return TranslateResult(es);

		std::unique_ptr<char[]> Buffer;
		intptr_t Ptr = 0;

		int OpenFromA = oldfar::OPEN_PLUGINSMENU;

		oldfar::OpenDlgPluginData DlgData{};

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
				const auto shortcutdata = SInfo->ShortcutData ? SInfo->ShortcutData : SInfo->HostFile;
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
			// BUGBUG this is not how it worked in 1.7
			OpenFromA = static_cast<int>(oldfar::OPEN_FROMMACRO) | static_cast<int>(Global->CtrlObject->Macro.GetArea());
			Buffer.reset(UnicodeToAnsi(reinterpret_cast<OpenMacroInfo*>(Info->Data)->Count ? reinterpret_cast<OpenMacroInfo*>(Info->Data)->Values[0].String : L""));
			Ptr = reinterpret_cast<intptr_t>(Buffer.get());
			break;

		case OPEN_DIALOG:
			OpenFromA = oldfar::OPEN_DIALOG;
			DlgData.ItemNumber = Info->Guid->Data1;
			DlgData.hDlg = reinterpret_cast<OpenDlgPluginData*>(Info->Data)->hDlg;
			Ptr = reinterpret_cast<intptr_t>(&DlgData);
			break;

		default:
			break;
		}

		ExecuteFunction(es, OpenFromA, Ptr);
		return TranslateResult(es);
	}

	void ClosePanel(ClosePanelInfo* Info) override
	{
		AnsiExecuteStruct<iClosePanel> es;
		if (Global->ProcessException || !has(es))
			return;

		ExecuteFunction(es, Info->hPanel);
		FreeOpenPanelInfo();
	}

	bool GetPluginInfo(PluginInfo *pi) override
	{
		*pi = {};

		AnsiExecuteStruct<iGetPluginInfo> es;
		if (Global->ProcessException || !has(es))
			return false;

		oldfar::PluginInfo InfoA{ sizeof(InfoA) };
		ExecuteFunction(es, &InfoA);

		if (bPendingRemove)
			return false;

		ConvertPluginInfo(InfoA, pi);
		return true;
	}

	void GetOpenPanelInfo(OpenPanelInfo *Info) override
	{
		Info->StructSize = sizeof(OpenPanelInfo);

		AnsiExecuteStruct<iGetOpenPanelInfo> es;
		if (Global->ProcessException || !has(es))
			return;

		oldfar::OpenPanelInfo InfoA{};
		ExecuteFunction(es, Info->hPanel, &InfoA);
		ConvertOpenPanelInfo(InfoA, Info);
	}

	intptr_t GetFindData(GetFindDataInfo* Info) override
	{
		AnsiExecuteStruct<iGetFindData> es;
		if (Global->ProcessException || !has(es))
			return es;

		pFDPanelItemA = nullptr;
		int ItemsNumber = 0;

		int OpMode = 0;
		SecondFlagsToFirst(Info->OpMode, OpMode, OperationModesMap);

		ExecuteFunction(es, Info->hPanel, &pFDPanelItemA, &ItemsNumber, OpMode);

		Info->ItemsNumber = ItemsNumber;
		if (es && ItemsNumber)
		{
			Info->PanelItem = ConvertAnsiPanelItemsToUnicode({ pFDPanelItemA, static_cast<size_t>(ItemsNumber) });
		}
		return es;
	}

	void FreeFindData(FreeFindDataInfo* Info) override
	{
		FreeUnicodePanelItem(Info->PanelItem, Info->ItemsNumber);

		AnsiExecuteStruct<iFreeFindData> es;
		if (Global->ProcessException || !has(es) || !pFDPanelItemA)
			return;

		ExecuteFunction(es, Info->hPanel, pFDPanelItemA, static_cast<int>(Info->ItemsNumber));
		pFDPanelItemA = nullptr;
	}

	intptr_t GetVirtualFindData(GetVirtualFindDataInfo* Info) override
	{
		AnsiExecuteStruct<iGetVirtualFindData> es;
		if (Global->ProcessException || !has(es))
			return es;

		pVFDPanelItemA = nullptr;
		string_view const Path = Info->Path;
		char_ptr_n<os::default_buffer_size> const PathA(Path.size() + 1);
		(void)encoding::oem::get_bytes(Path, PathA);
		int ItemsNumber = 0;
		ExecuteFunction(es, Info->hPanel, &pVFDPanelItemA, &ItemsNumber, PathA.data());
		Info->ItemsNumber = ItemsNumber;

		if (es && ItemsNumber)
		{
			Info->PanelItem = ConvertAnsiPanelItemsToUnicode({ pVFDPanelItemA, static_cast<size_t>(ItemsNumber) });
		}

		return es;
	}

	void FreeVirtualFindData(FreeFindDataInfo* Info) override
	{
		FreeUnicodePanelItem(Info->PanelItem, Info->ItemsNumber);

		AnsiExecuteStruct<iFreeVirtualFindData> es;
		if (Global->ProcessException || !has(es) || !pVFDPanelItemA)
			return;

		ExecuteFunction(es, Info->hPanel, pVFDPanelItemA, static_cast<int>(Info->ItemsNumber));
		pVFDPanelItemA = nullptr;
	}

	intptr_t SetDirectory(SetDirectoryInfo* Info) override
	{
		AnsiExecuteStruct<iSetDirectory> es;
		if (Global->ProcessException || !has(es))
			return es;

		std::unique_ptr<char[]> const DirA(UnicodeToAnsi(Info->Dir));
		int OpMode = 0;
		SecondFlagsToFirst(Info->OpMode, OpMode, OperationModesMap);
		ExecuteFunction(es, Info->hPanel, DirA.get(), OpMode);
		return es;
	}

	intptr_t GetFiles(GetFilesInfo* Info) override
	{
		AnsiExecuteStruct<iGetFiles> es(-1);
		if (Global->ProcessException || !has(es))
			return es;

		const auto PanelItemA = ConvertPanelItemsArrayToAnsi(Info->PanelItem, Info->ItemsNumber);
		char DestA[oldfar::NM];
		(void)encoding::oem::get_bytes(Info->DestPath, DestA);
		int OpMode = 0;
		SecondFlagsToFirst(Info->OpMode, OpMode, OperationModesMap);
		ExecuteFunction(es, Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber), Info->Move, DestA, OpMode);
		UpdatePluginPanelItemFlags(PanelItemA, Info->PanelItem, Info->ItemsNumber);
		static wchar_t DestW[oldfar::NM];
		(void)encoding::oem::get_chars(DestA, DestW);
		Info->DestPath = DestW;
		FreePanelItemA({ PanelItemA, Info->ItemsNumber });
		return es;
	}

	intptr_t PutFiles(PutFilesInfo* Info) override
	{
		AnsiExecuteStruct<iPutFiles> es(-1);
		if (Global->ProcessException || !has(es))
			return es;

		const auto PanelItemA = ConvertPanelItemsArrayToAnsi(Info->PanelItem, Info->ItemsNumber);
		int OpMode = 0;
		SecondFlagsToFirst(Info->OpMode, OpMode, OperationModesMap);
		ExecuteFunction(es, Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber), Info->Move, OpMode);
		UpdatePluginPanelItemFlags(PanelItemA, Info->PanelItem, Info->ItemsNumber);
		FreePanelItemA({ PanelItemA, Info->ItemsNumber });
		return es;
	}

	intptr_t DeleteFiles(DeleteFilesInfo* Info) override
	{
		AnsiExecuteStruct<iDeleteFiles> es;
		if (Global->ProcessException || !has(es))
			return es;

		const auto PanelItemA = ConvertPanelItemsArrayToAnsi(Info->PanelItem, Info->ItemsNumber);
		int OpMode = 0;
		SecondFlagsToFirst(Info->OpMode, OpMode, OperationModesMap);
		ExecuteFunction(es, Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber), OpMode);
		UpdatePluginPanelItemFlags(PanelItemA, Info->PanelItem, Info->ItemsNumber);
		FreePanelItemA({ PanelItemA, Info->ItemsNumber });
		return es;
	}

	intptr_t MakeDirectory(MakeDirectoryInfo* Info) override
	{
		AnsiExecuteStruct<iMakeDirectory> es(-1);
		if (Global->ProcessException || !has(es))
			return es;

		char NameA[oldfar::NM];
		(void)encoding::oem::get_bytes(Info->Name, NameA);
		int OpMode = 0;
		SecondFlagsToFirst(Info->OpMode, OpMode, OperationModesMap);
		ExecuteFunction(es, Info->hPanel, NameA, OpMode);
		static wchar_t NameW[oldfar::NM];
		(void)encoding::oem::get_chars(NameA, NameW);
		Info->Name = NameW;
		return es;
	}

	intptr_t ProcessHostFile(ProcessHostFileInfo* Info) override
	{
		AnsiExecuteStruct<iProcessHostFile> es;
		if (Global->ProcessException || !has(es))
			return es;

		const auto PanelItemA = ConvertPanelItemsArrayToAnsi(Info->PanelItem, Info->ItemsNumber);
		int OpMode = 0;
		SecondFlagsToFirst(Info->OpMode, OpMode, OperationModesMap);
		ExecuteFunction(es, Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber), OpMode);
		FreePanelItemA({ PanelItemA, Info->ItemsNumber });
		return es;
	}

	intptr_t SetFindList(SetFindListInfo* Info) override
	{
		AnsiExecuteStruct<iSetFindList> es;
		if (Global->ProcessException || !has(es))
			return es;

		const auto PanelItemA = ConvertPanelItemsArrayToAnsi(Info->PanelItem, Info->ItemsNumber);
		ExecuteFunction(es, Info->hPanel, PanelItemA, static_cast<int>(Info->ItemsNumber));
		FreePanelItemA({ PanelItemA, Info->ItemsNumber });
		return es;
	}

	intptr_t Configure(ConfigureInfo* Info) override
	{
		AnsiExecuteStruct<iConfigure> es;
		if (Global->ProcessException || !Load() || !has(es))
			return es;

		ExecuteFunction(es, Info->Guid->Data1);
		return es;
	}

	void ExitFAR(ExitInfo*) override
	{
		AnsiExecuteStruct<iExitFAR> es;
		if (Global->ProcessException || !has(es))
			return;

		// ExitInfo ignored for ansi plugins
		ExecuteFunction(es);
	}

	intptr_t ProcessPanelInput(ProcessPanelInputInfo* Info) override
	{
		AnsiExecuteStruct<iProcessPanelInput> es;
		if (Global->ProcessException || !has(es))
			return es;

		const auto Prepocess = (Info->Rec.EventType & 0x4000) != 0;
		Info->Rec.EventType &= ~0x4000;

		if (Info->Rec.EventType != KEY_EVENT)
			return es;

		const auto& KeyEvent = Info->Rec.Event.KeyEvent;

		const DWORD ControlState =
			(KeyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)? oldfar::PKF_CONTROL : 0) |
			(KeyEvent.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)? oldfar::PKF_ALT : 0) |
			(KeyEvent.dwControlKeyState & SHIFT_PRESSED? oldfar::PKF_SHIFT : 0);

		ExecuteFunction(es, Info->hPanel, KeyEvent.wVirtualKeyCode | (Prepocess? oldfar::PKF_PREPROCESS : 0), ControlState);
		return es;
	}

	intptr_t ProcessPanelEvent(ProcessPanelEventInfo* Info) override
	{
		AnsiExecuteStruct<iProcessPanelEvent> es;
		if (Global->ProcessException || !has(es))
			return es;

		auto Param = Info->Param;
		std::unique_ptr<char[]> ParamA;

		if (Info->Param && (Info->Event == FE_COMMAND || Info->Event == FE_CHANGEVIEWMODE))
		{
			ParamA.reset(UnicodeToAnsi(static_cast<const wchar_t*>(Info->Param)));
			Param = ParamA.get();
		}

		ExecuteFunction(es, Info->hPanel, Info->Event, Param);
		return es;
	}

	intptr_t ProcessEditorEvent(ProcessEditorEventInfo* Info) override
	{
		AnsiExecuteStruct<iProcessEditorEvent> es;
		if (Global->ProcessException || !Load() || !has(es))
			return es;

		switch (Info->Event)
		{
		case EE_CLOSE:
		case EE_GOTFOCUS:
		case EE_KILLFOCUS:
			Info->Param = &Info->EditorID;
			[[fallthrough]];
		case EE_READ:
		case EE_SAVE:
		case EE_REDRAW:
			ExecuteFunction(es, Info->Event, Info->Param);
			break;
		}
		return es;
	}

	intptr_t Compare(CompareInfo* Info) override
	{
		AnsiExecuteStruct<iCompare> es(-2);
		if (Global->ProcessException || !has(es))
			return es;

		const auto Item1A = ConvertPanelItemsArrayToAnsi(Info->Item1, 1);
		const auto Item2A = ConvertPanelItemsArrayToAnsi(Info->Item2, 1);
		ExecuteFunction(es, Info->hPanel, Item1A, Item2A, Info->Mode);
		FreePanelItemA({ Item1A, 1 });
		FreePanelItemA({ Item2A, 1 });
		return es;
	}

	intptr_t ProcessEditorInput(ProcessEditorInputInfo* Info) override
	{
		AnsiExecuteStruct<iProcessEditorInput> es;
		if (Global->ProcessException || !Load() || !has(es))
			return es;

		const INPUT_RECORD *Ptr = &Info->Rec;
		INPUT_RECORD OemRecord;
		if (Ptr->EventType == KEY_EVENT)
		{
			OemRecord = Info->Rec;
			UnicodeToAnsiBin({ &Info->Rec.Event.KeyEvent.uChar.UnicodeChar, 1 }, &OemRecord.Event.KeyEvent.uChar.AsciiChar);
			Ptr = &OemRecord;
		}
		ExecuteFunction(es, Ptr);
		return es;
	}

	intptr_t ProcessViewerEvent(ProcessViewerEventInfo* Info) override
	{
		AnsiExecuteStruct<iProcessViewerEvent> es;
		if (Global->ProcessException || !Load() || !has(es))
			return es;

		switch (Info->Event)
		{
		case VE_CLOSE:
		case VE_GOTFOCUS:
		case VE_KILLFOCUS:
			Info->Param = &Info->ViewerID;
			[[fallthrough]];
		case VE_READ:
			ExecuteFunction(es, Info->Event, Info->Param);
			break;
		}
		return es;
	}

	intptr_t ProcessDialogEvent(ProcessDialogEventInfo* Info) override
	{
		AnsiExecuteStruct<iProcessDialogEvent> es;
		if (Global->ProcessException || !Load() || !has(es))
			return es;

		ExecuteFunction(es, Info->Event, Info->Param);
		return es;
	}

	intptr_t ProcessSynchroEvent(ProcessSynchroEventInfo*) override
	{
		return 0;
	}

	intptr_t ProcessConsoleInput(ProcessConsoleInputInfo*) override
	{
		return 0;
	}

	void* Analyse(AnalyseInfo*) override
	{
		return nullptr;
	}

	void CloseAnalyse(CloseAnalyseInfo*) override
	{
	}

	intptr_t GetContentFields(GetContentFieldsInfo*) override
	{
		return 0;
	}

	intptr_t GetContentData(GetContentDataInfo*) override
	{
		return 0;
	}

	void FreeContentData(GetContentDataInfo*) override
	{
	}

	void* OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, size_t DataSize, int) override
	{
		AnsiExecuteStruct<iOpenFilePlugin> es;
		if (Global->ProcessException || !Load() || !has(es))
			return es;

		std::unique_ptr<char[]> const NameA(UnicodeToAnsi(Name));
		ExecuteFunction(es, NameA.get(), Data, static_cast<int>(DataSize));
		return TranslateResult(es);
	}

	bool CheckMinFarVersion() override
	{
		AnsiExecuteStruct<iGetMinFarVersion> es;
		if (Global->ProcessException || !has(es))
			return true;

		ExecuteFunction(es);
		return !bPendingRemove;
	}

	bool IsOemPlugin() const override
	{
		return true;
	}

	const string& GetHotkeyName() const override
	{
		return CacheName();
	}

	bool InitLang(string_view const Path, string_view Language) override
	{
		if (PluginLang)
			return true;

		try
		{
			PluginLang = std::make_unique<ansi_plugin_language>(Path, Language);
			return true;
		}
		catch (const std::exception&)
		{
			// TODO: log
			return false;
		}
	}

	void Prologue() override
	{
		Plugin::Prologue();
		SetFileApisToOEM();
		++OEMApiCnt;
	}

	void Epilogue() override
	{
		--OEMApiCnt;
		if (!OEMApiCnt)
			SetFileApisToANSI();
		Plugin::Epilogue();
	}

	void FreePluginInfo()
	{
		const auto DeleteItems = [](const PluginMenuItem& Item)
		{
			for (const auto& i: span(Item.Strings, Item.Count))
			{
				delete[] i;
			}

			delete[] Item.Guids;
			delete[] Item.Strings;
		};

		DeleteItems(PI.DiskMenu);
		DeleteItems(PI.PluginMenu);
		DeleteItems(PI.PluginConfig);

		delete[] PI.CommandPrefix;

		PI = {};
	}

	void ConvertPluginInfo(const oldfar::PluginInfo &Src, PluginInfo *Dest)
	{
		FreePluginInfo();
		PI.StructSize = sizeof(PI);

		static const std::array PluginFlagsMap
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
				auto p = std::make_unique<const wchar_t*[]>(Size);
				auto Uuid = std::make_unique<UUID[]>(Size);

				for (size_t i = 0; i != Size; ++i)
				{
					p[i] = AnsiToUnicode(Strings[i]);
					Uuid[i].Data1 = static_cast<decltype(Uuid[i].Data1)>(i);
				}

				Item.Guids = Uuid.release();
				Item.Strings = p.release();
				Item.Count = Size;
			}

		};

#define CREATE_ITEMS(ItemsType) CreatePluginMenuItems(Src.ItemsType##Strings, Src.ItemsType##StringsNumber, PI.ItemsType)

		CREATE_ITEMS(DiskMenu);
		CREATE_ITEMS(PluginMenu);
		CREATE_ITEMS(PluginConfig);

#undef CREATE_ITEMS

		PI.CommandPrefix = AnsiToUnicode(Src.CommandPrefix);

		*Dest = PI;
	}

	void FreeOpenPanelInfo()
	{
		delete[] OPI.CurDir;
		delete[] OPI.HostFile;
		delete[] OPI.Format;
		delete[] OPI.PanelTitle;
		FreeUnicodeInfoPanelLines({ OPI.InfoLines, OPI.InfoLinesNumber });
		DeleteRawArray(span(OPI.DescrFiles, OPI.DescrFilesNumber));
		FreeUnicodePanelModes({ OPI.PanelModesArray, OPI.PanelModesNumber });
		FreeUnicodeKeyBarTitles(OPI.KeyBar);
		delete OPI.KeyBar;
		delete[] OPI.ShortcutData;
		OPI = {};
	}

	void ConvertOpenPanelInfo(const oldfar::OpenPanelInfo &Src, OpenPanelInfo *Dest)
	{
		FreeOpenPanelInfo();
		OPI.StructSize = sizeof(OPI);
		OPI.Flags = OPIF_NONE;

		static const std::array PanelInfoFlagsMap
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
		if (opif_shortcut) OPI.Flags |= OPIF_SHORTCUT;

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
			OPI.InfoLines = ConvertInfoPanelLinesA({ Src.InfoLines, static_cast<size_t>(Src.InfoLinesNumber) });
			OPI.InfoLinesNumber = Src.InfoLinesNumber;
		}

		if (Src.DescrFiles && Src.DescrFilesNumber)
		{
			OPI.DescrFiles = AnsiArrayToUnicode({ Src.DescrFiles, static_cast<size_t>(Src.DescrFilesNumber) });
			OPI.DescrFilesNumber = Src.DescrFilesNumber;
		}

		if (Src.PanelModesArray && Src.PanelModesNumber)
		{
			auto UnicodeModes = std::make_unique<PanelMode[]>(Src.PanelModesNumber);
			ConvertPanelModesToUnicode(
				{ Src.PanelModesArray, static_cast<size_t>(Src.PanelModesNumber) },
				{ UnicodeModes.get(), static_cast<size_t>(Src.PanelModesNumber) }
			);
			OPI.PanelModesArray = UnicodeModes.release();
			OPI.PanelModesNumber = Src.PanelModesNumber;
			OPI.StartPanelMode = Src.StartPanelMode;

			switch (Src.StartSortMode)
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

			OPI.StartSortOrder = Src.StartSortOrder;
		}

		if (Src.KeyBar)
		{
			OPI.KeyBar = new KeyBarTitles;
			ConvertKeyBarTitlesA(Src.KeyBar, const_cast<KeyBarTitles*>(OPI.KeyBar), Src.StructSize >= static_cast<int>(sizeof(oldfar::OpenPanelInfo)));
		}

		if (Src.ShortcutData)
			OPI.ShortcutData = AnsiToUnicode(Src.ShortcutData);

		*Dest = OPI;
	}

	class ansi_language_data final: public i_language_data
	{
	public:
		std::unique_ptr<i_language_data> create() override { return std::make_unique<ansi_language_data>(); }

		void reserve(size_t Size) override { return m_Messages.reserve(Size); }
		void add(string&& Str) override { m_Messages.emplace_back(encoding::oem::get_bytes(Str)); }
		void set_at(size_t Index, string&& Str) override { m_Messages[Index] = encoding::oem::get_bytes(Str); }
		size_t size() const override { return m_Messages.size(); }

		const std::string& ansi_at(size_t Index) const { return m_Messages[Index]; }

	private:
		std::vector<std::string> m_Messages;
	};

	class ansi_plugin_language final: public language
	{
	public:
		explicit ansi_plugin_language(string_view const Path, string_view const Language):
			language(std::make_unique<ansi_language_data>())
		{
			load(Path, Language);
		}

		const char* GetMsgA(int Id) const
		{
			return m_Data->validate(Id)? static_cast<const ansi_language_data&>(*m_Data).ansi_at(Id).c_str() : "";
		}
	};

	template<export_index Export>
	using AnsiExecuteStruct = ExecuteStruct<Export, false>;

	PluginInfo PI;
	OpenPanelInfo OPI;

	oldfar::PluginPanelItem  *pFDPanelItemA;
	oldfar::PluginPanelItem  *pVFDPanelItemA;

	std::atomic_ulong OEMApiCnt;

	bool opif_shortcut;
};

static const char* GetPluginMsg(const Plugin* PluginInstance, int MsgId)
{
	return static_cast<const PluginA*>(PluginInstance)->GetMsgA(MsgId);
}

std::unique_ptr<Plugin> oem_plugin_factory::CreatePlugin(const string& filename)
{
	return IsPlugin(filename)? std::make_unique<PluginA>(this, filename) : nullptr;
}

#undef OLDFAR_TO_FAR_MAP

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("plugin.ansi.tables")
{
	LocalUpperInit();

	static const struct
	{
		char Input;
		char Case;
		char Upper;
		char Lower;
	}
	Tests[]
	{
		{   0, case_none,    0,   0 },
		{ ' ', case_none,  ' ', ' ' },
		{ '0', case_none,  '0', '0' },
		{ '9', case_none,  '9', '9' },
		{ 'a', case_lower, 'A', 'a' },
		{ 'A', case_upper, 'A', 'a' },
		{ 'z', case_lower, 'Z', 'z' },
		{ 'Z', case_upper, 'Z', 'z' },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(UpperOrLower[static_cast<size_t>(i.Input)] == i.Case);
		REQUIRE(LowerToUpper[static_cast<size_t>(i.Input)] == i.Upper);
		REQUIRE(UpperToLower[static_cast<size_t>(i.Input)] == i.Lower);
	}
}

TEST_CASE("plugin.ansi.strcmp")
{
	LocalUpperInit();

	static const struct
	{
		const char* Str1, * Str2;
		int Result;
	}
	Tests[]
	{
		{ "",        "",   0 },
		{ "abc",  "def",  -1 },
		{ "def",  "abc",   1 },
		{ "abc",  "abc",   0 },
		{ "abc",  "abcd", -1 },
		{ "abcd", "abc",   1 },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(LocalStricmp(i.Str1, i.Str2) == i.Result);
	}
}

#endif

#endif // NO_WRAPPER
