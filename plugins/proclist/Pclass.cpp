#include <algorithm>
#include <array>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <cassert>
#include <cmath>
#include <cwchar>
#include <cwctype>

#include "Proclist.hpp"
#include "perfthread.hpp"
#include "Proclng.hpp"
#include "guid.hpp"

#include <DlgBuilder.hpp>
#include <PluginSettings.hpp>

#include <winperf.h>

using namespace std::literals;

const auto FarSortModeSlot = SM_CHTIME;

class StrTok
{
public:
	StrTok(const wchar_t* str, const wchar_t* token):
		tok(token)
	{
		buf.reset(wcsdup(str));
#if defined(_UCRT)
		ptr = std::wcstok(buf.get(), token, &pt);
#else
		ptr = std::wcstok(buf.get(), token);
#endif
	}

	operator wchar_t*() const
	{
		return ptr;
	}

	void operator++()
	{
#if defined(_UCRT)
		ptr = std::wcstok({}, tok, &pt);
#else
		ptr = std::wcstok({}, tok);
#endif
	}

	explicit operator bool() const
	{
		return ptr != nullptr;
	}

private:
	const wchar_t* tok;
	wchar_t* ptr;
	malloc_ptr<wchar_t> buf;
#if defined(_UCRT)
	wchar_t* pt;
#endif
};

// Far: FileSizeToStr
// TODO: API?
static std::wstring ui64toa_width(uint64_t value, unsigned width, bool bThousands)
{
	auto ValueStr = str(value);
	if (ValueStr.size() <= width)
		return ValueStr;

	constexpr std::pair
		BinaryDivider(1024, 10), // 10 == log2(1024)
		DecimalDivider(1000, 3); // 3 == log10(1000)

	const auto& Divider = bThousands? DecimalDivider : BinaryDivider;

	const auto Numerator = value? bThousands? std::log10(value) : std::log2(value) : 0;
	const auto UnitIndex = static_cast<size_t>(Numerator / Divider.second);

	if (!UnitIndex)
		return ValueStr;

	const auto SizeInUnits = static_cast<double>(value) / std::pow(Divider.first, UnitIndex);

	double Parts[2];
	Parts[1] = std::modf(SizeInUnits, &Parts[0]);
	auto Integral = static_cast<int>(Parts[0]);

	if (const auto NumDigits = Integral < 10? 2 : Integral < 100? 1 : 0)
	{
		const auto AdjustedParts = [&]
		{
			const auto Multiplier = static_cast<unsigned long long>(std::pow(10, NumDigits));
			const auto Value = Parts[1] * static_cast<double>(Multiplier);
			const auto UseRound = true;
			const auto Fractional = static_cast<unsigned long long>(UseRound? std::round(Value) : Value);
			return Fractional == Multiplier? std::make_pair(Integral + 1, 0ull) : std::make_pair(Integral, Fractional);
		}();

		ValueStr = far::format(L"{0}.{1:0{2}}"sv, AdjustedParts.first, AdjustedParts.second, NumDigits);
	}
	else
	{
		ValueStr = str(static_cast<int>(std::round(SizeInUnits)));
	}

	return ValueStr.append(1, L' ').append(1, L"BKMGTPE"[UnitIndex]);
}

static std::wstring PrintTitle(std::wstring_view const Msg)
{
	return far::format(L"{0:<21} "sv, far::format(L"{0}:"sv, Msg));
}

static std::wstring PrintTitle(int MsgId)
{
	return PrintTitle(GetMsg(MsgId));
}

Plist::Plist():
	dwPluginThread(GetCurrentThreadId())
{
	{
		PluginSettings settings(MainGuid, PsInfo.SettingsControl);
		SortMode = settings.Get(0, L"SortMode", SM_UNSORTED); //SM_CUSTOM;
		StartPanelMode = settings.Get(0, L"StartPanelMode", 1) + L'0';
	}

	if (SortMode >= SM_PROCLIST_PERSEC)
		SortMode &= (SM_PROCLIST_PERSEC - 1); // Ахтунг!

	InitializePanelModes();

	pPerfThread = std::make_unique<PerfThread>(this);
}

int Plist::Menu(unsigned int Flags, const wchar_t* Title, const wchar_t* Bottom, const wchar_t* HelpTopic, const FarKey* BreakKeys, const FarMenuItem* Items, size_t ItemsNumber)
{
	return static_cast<int>((*PsInfo.Menu)(&MainGuid, {}, -1, -1, 0, Flags, Title, Bottom, HelpTopic, BreakKeys, {}, Items, ItemsNumber));
}

static wchar_t upper(wchar_t Char)
{
	CharUpperBuff(&Char, 1);
	return Char;
}

static bool TranslateMode(const std::wstring& Src, std::wstring& Destination)
{
	std::wstring dest;

	int iCustomMode = L'0';
	bool bWasDesc = false;

	auto src = Src.c_str();
	while (*src)
	{
		switch (upper(*src))
		{
		case L'Z':
			switch (*++src)
			{
			case L'P':
			case L'W':
			case L'D':
			case L'C':
				++src;
				break;

			default:
				return false;
			}

			if (bWasDesc)
				return false;

			dest.push_back(L'Z');
			bWasDesc = true;

			if (*src && *src != L',')
				return false;
			break;

		case L'X':

			switch (*++src)
			{
			case L'P':
			case L'I':
			case L'C':
			case L'T':
			case L'B':
			case L'G':
			case L'U':
				src++;
				break;

			default:
				auto endptr = src;
				while (std::iswdigit(*endptr))
					++endptr;

				if (endptr == src)
					return false;

				src = endptr;
			}

			dest.push_back(L'C');
			dest.push_back(iCustomMode++);

			/*if(*src && *src!=L',')
			return false;*/
			while (*src && *src != L',') ++src;

			break;
		default:

			while (*src && *src != L',')
				dest.push_back(*src++);
		}

		if (*src == L',')
			dest.push_back(*src++);
	}

	Destination = std::move(dest);
	return true;
}

void Plist::InitializePanelModes()
{
	static const wchar_t
		StatusCols[] = L"N,S,D,T",
		StatusWidths[] = L"0,8,0,5";

	// Default panel modes. Overridable from config.
	// These modes are translated into PanelModesXX
	m_PanelModesDataLocal =
	{
		/*0*/ { { L"N,X15F,X16F,X17F,X18S",             L"12,0,0,0,4",         }, {}, PMFLAGS_NONE,       }, // I/O
		/*1*/ { { L"N,XI,XB,XP,X0S,X6F",                L"0,6,2,2,3,9",        }, {}, PMFLAGS_NONE,       }, // General info
		/*2*/ { { L"N,N",                               L"0,0",                }, {}, PMFLAGS_NONE,       }, // Names only
		/*3*/ { { L"N,XI,XB,XC,D,T",                    L"0,6,2,6,0,0",        }, {}, PMFLAGS_NONE,       }, // Startup: PID/Date/Time
		/*4*/ { { L"N,XI,X4F,X6F",                      L"0,6,9,9",            }, {}, PMFLAGS_NONE,       }, // Memory (basic)
		/*5*/ { { L"N,XI,X4F,X6F,X10F,X12F,X0,X1,X2",   L"0,6,9,9,9,9,8,8,8",  }, {}, PMFLAGS_FULLSCREEN, }, // Extended Memory/Time
		/*6*/ { { L"N,ZD",                              L"12,0",               }, {}, PMFLAGS_NONE,       }, // Descriptions
		/*7*/ { { L"N,XP,X0S,X1S,X2S,X11S,X14FS,X18S",  L"0,2,3,2,2,3,4,3",    }, {}, PMFLAGS_NONE,       }, // Dynamic Performance
		/*8*/ { { L"N,XI,O",                            L"0,6,15",             }, {}, PMFLAGS_NONE,       }, // Owners (not implemented)
		/*9*/ { { L"N,XI,XT,X3,XG,XU",                  L"0,6,3,4,4,4",        }, {}, PMFLAGS_NONE,       }, // Resources
	};

	m_PanelModesDataRemote =
	{
		/*0*/ { { L"N,X15F,X16F,X17F,X18S",             L"12,0,0,0,4",         }, {}, PMFLAGS_NONE,       }, // I/O
		/*1*/ { { L"N,XI,XB,XP,X0S,X6F",                L"0,6,2,2,3,9",        }, {}, PMFLAGS_NONE,       }, // General info
		/*2*/ { { L"N,N",                               L"0,0",                }, {}, PMFLAGS_NONE,       }, // Names only
		/*3*/ { { L"N,XI,XB,XC,D,T",                    L"0,6,2,6,0,0",        }, {}, PMFLAGS_NONE,       }, // Startup: PID/Date/Time
		/*4*/ { { L"N,XI,X4F,X6F",                      L"0,6,9,9",            }, {}, PMFLAGS_NONE,       }, // Memory (basic)
		/*5*/ { { L"N,XI,X4F,X6F,X10F,X12F,X0,X1,X2",   L"0,6,9,9,9,9,8,8,8",  }, {}, PMFLAGS_FULLSCREEN, }, // Extended Memory/Time
		/*6*/ { { L"N,ZD",                              L"12,0",               }, {}, PMFLAGS_NONE,       }, // Descriptions
		/*7*/ { { L"N,XP,X0S,X1S,X2S,X11S,X14FS,X18S",  L"0,2,3,2,2,3,4,3",    }, {}, PMFLAGS_NONE,       }, // Dynamic Performance
		/*8*/ { { L"N,XI,O",                            L"0,6,15",             }, {}, PMFLAGS_NONE,       }, // Owners (not implemented)
		/*9*/ { { L"N,XI,XT,X3",                        L"0,6,3,4",            }, {}, PMFLAGS_NONE,       }, // Resources
	};

	PluginSettings settings(MainGuid, PsInfo.SettingsControl);

	for (size_t i = 0; i != NPANELMODES; ++i)
	{
		auto& ModeLocal = m_PanelModesDataLocal[i];
		auto& ModeRemote = m_PanelModesDataRemote[i];

		auto& ColsLocal = ModeLocal.panel_columns;
		auto& ColsRemote = ModeRemote.panel_columns;

		if (const auto Root = settings.OpenSubKey(0, far::format(L"Mode{0}"sv, i).c_str()))
		{
			if (settings.Get(Root, L"FullScreenLocal", (ModeLocal.Flags & PMFLAGS_FULLSCREEN) != 0))
				ModeLocal.Flags |= PMFLAGS_FULLSCREEN;
			else
				ModeLocal.Flags &= ~PMFLAGS_FULLSCREEN;

			if (settings.Get(Root, L"FullScreenRemote", (ModeRemote.Flags & PMFLAGS_FULLSCREEN) != 0))
				ModeRemote.Flags |= PMFLAGS_FULLSCREEN;
			else
				ModeRemote.Flags &= ~PMFLAGS_FULLSCREEN;


			ColsLocal.internal_types = settings.Get(Root, L"ColumnsLocal", ColsLocal.internal_types.c_str());
			ColsRemote.internal_types = settings.Get(Root, L"ColumnsRemote", ColsRemote.internal_types.c_str());
			ColsLocal.widths = settings.Get(Root, L"WidthsLocal", ColsLocal.widths.c_str());
			ColsRemote.widths = settings.Get(Root, L"WidthsRemote", ColsRemote.widths.c_str());
		}

		TranslateMode(ColsLocal.internal_types, ColsLocal.far_types);
		TranslateMode(ColsRemote.internal_types, ColsRemote.far_types);

		auto& StatusLocal = ModeLocal.status_columns;
		auto& StatusRemote = ModeRemote.status_columns;

		//Status line is the same for all modes currently and cannot be changed.
		StatusLocal.internal_types = StatusCols;
		StatusRemote.internal_types = StatusCols;
		StatusLocal.widths = StatusWidths;
		StatusRemote.widths = StatusWidths;

		TranslateMode(StatusLocal.internal_types, StatusLocal.far_types);
		TranslateMode(StatusRemote.internal_types, StatusRemote.far_types);
	}
}

Plist::~Plist()
{
	pPerfThread.reset();
	DisconnectWMI();
}

void Plist::SavePanelModes()
{
	PluginSettings settings(MainGuid, PsInfo.SettingsControl);

	for (size_t i = 0; i != NPANELMODES; ++i)
	{
		const auto Root = settings.CreateSubKey(0, far::format(L"Mode{0}"sv, i).c_str());

		auto& ModeLocal = m_PanelModesDataLocal[i];
		auto& ModeRemote = m_PanelModesDataRemote[i];
		settings.Set(Root, L"FullScreenLocal", (ModeLocal.Flags & PMFLAGS_FULLSCREEN) != 0);
		settings.Set(Root, L"FullScreenRemote", (ModeRemote.Flags & PMFLAGS_FULLSCREEN) != 0);

		auto& ColsLocal = ModeLocal.panel_columns;
		auto& ColsRemote = ModeRemote.panel_columns;
		settings.Set(Root, L"ColumnsLocal", ColsLocal.internal_types.c_str());
		settings.Set(Root, L"ColumnsRemote", ColsRemote.internal_types.c_str());
		settings.Set(Root, L"WidthsLocal", ColsLocal.widths.c_str());
		settings.Set(Root, L"WidthsRemote", ColsRemote.widths.c_str());
	}
}

static bool can_be_per_sec(size_t const Counter)
{
	return Counter < 3 || Counter == 11 || Counter >= 14;
}

using panel_modes_array = std::array<PanelMode, NPANELMODES>;

static void generate_titles(const std::wstring& Str, PanelMode& PanelMode)
{
	int ii = 0;

	for (StrTok tok(Str.data(), L","); tok; ++tok)
	{
		int id = 0;

		switch (upper(*tok))
		{
		case L'N': id = MColumnModule; break;

		case L'Z':
			switch (upper(tok[1]))
			{
			case L'P': id = MTitleFullPath; break;
			case L'W': id = MColumnTitle; break;
			case L'D': id = MTitleFileDesc; break;
			case L'C': id = MCommandLine; break;
			}
			break;

		case L'X':
			switch (upper(tok[1]))
			{
			case L'P': id = MColumnPriority; break;
			case L'I': id = MTitlePID; break;
			case L'C': id = MColumnParentPID; break;
			case L'T': id = MTitleThreads; break;
			case L'B': id = MColumnBits; break;
			case L'G': id = MColumnGDI; break;
			case L'U': id = MColumnUSER; break;
			default:
				const auto n = FSF.atoi(&tok[1]);

				if (n >= 0 && n < NCOUNTERS)
				{
					id = Counters[n].idCol;
					if (wcspbrk(&tok[1], L"Ss") && can_be_per_sec(n))
						++id;
				}

				break;
			}

			break;
		}

		if (id)
			const_cast<const wchar_t*&>(PanelMode.ColumnTitles[ii++]) = GetMsg(id);
	}
}

static void convert_panel_modes(const std::vector<mode>& Data, panel_modes_array& PanelModes)
{
	for (size_t i = 0, size = Data.size(); i != size; ++i)
	{
		const auto& From = Data[i];
		auto& To = PanelModes[i];

		To.ColumnTypes = From.panel_columns.far_types.c_str();
		To.ColumnWidths = From.panel_columns.widths.c_str();
		To.StatusColumnTypes = From.status_columns.far_types.c_str();
		To.StatusColumnWidths = From.status_columns.widths.c_str();
		To.Flags = From.Flags;

		generate_titles(From.panel_columns.internal_types, To);
	}
}

// Obtains the current array of panel modes. Called from OpenPluginInfo.
PanelMode* Plist::PanelModes(size_t& nModes)
{
	using custom_titles_array = std::array<wchar_t*, MAXCOLS>;
	static std::array<custom_titles_array, NPANELMODES> TitlesLocal, TitlesRemote;
	static std::array<PanelMode, NPANELMODES> PanelModesLocal, PanelModesRemote;

	if (static bool ArraysInited = false; !ArraysInited)
	{
		for (size_t i = 0; i != NPANELMODES; ++i)
		{
			PanelModesLocal[i].ColumnTitles = TitlesLocal[i].data();
			PanelModesRemote[i].ColumnTitles = TitlesRemote[i].data();
		}
		ArraysInited = true;
	}

	convert_panel_modes(m_PanelModesDataLocal, PanelModesLocal);
	convert_panel_modes(m_PanelModesDataRemote, PanelModesRemote);

	nModes = NPANELMODES;
	return HostName.empty()? PanelModesLocal.data() : PanelModesRemote.data();
}

void Plist::GetOpenPanelInfo(OpenPanelInfo* Info)
{
	const std::scoped_lock b(m_RefreshLock);

	Info->StructSize = sizeof(*Info);
	Info->Flags = OPIF_ADDDOTS | OPIF_SHOWNAMESONLY | OPIF_USEATTRHIGHLIGHTING;
	Info->CurDir = L"";
	static std::wstring Title;

	if (HostName.empty())
		Title = far::format(L" {} "sv, GetMsg(MPlistPanel));
	else
		Title = far::format(L"{}: {} "sv, HostName, GetMsg(MPlistPanel));

	Info->PanelTitle = Title.c_str();
	Info->PanelModesArray = PanelModes(Info->PanelModesNumber);
	Info->StartPanelMode = StartPanelMode;
	Info->StartSortMode = SortMode >= SM_PROCLIST_CUSTOM? FarSortModeSlot : static_cast<OPENPANELINFO_SORTMODES>(SortMode); //SM_UNSORTED;


	static WORD FKeys[] =
	{
		VK_F6,0,MFRemote,
		VK_F7,0,0,
		VK_F8,0,MFKill,
		VK_F6,LEFT_ALT_PRESSED,0,
		VK_F1,SHIFT_PRESSED,MFPriorMinus,
		VK_F2,SHIFT_PRESSED,MFPriorPlus,
		VK_F3,SHIFT_PRESSED,MViewDDD,
		VK_F5,SHIFT_PRESSED,0,
		VK_F6,SHIFT_PRESSED,MFLocal,
		VK_F7,SHIFT_PRESSED,0,
		VK_F8,SHIFT_PRESSED,MFKill,
	};

	static KeyBarLabel kbl[std::size(FKeys) / 3];
	static KeyBarTitles kbt = { std::size(kbl), kbl };

	for (size_t j = 0, i = 0; i < std::size(FKeys); i += 3, ++j)
	{
		kbl[j].Key.VirtualKeyCode = FKeys[i];
		kbl[j].Key.ControlKeyState = FKeys[i + 1];

		if (FKeys[i + 2])
		{
			kbl[j].Text = kbl[j].LongText = GetMsg(FKeys[i + 2]);
		}
		else
		{
			kbl[j].Text = kbl[j].LongText = L"";
		}
	}

	Info->KeyBar = &kbt;
}

static BOOL CALLBACK EnumWndProc(HWND hWnd, LPARAM lParam)
{
	DWORD dwProcID;
	GetWindowThreadProcessId(hWnd, &dwProcID);

	auto& Data = *reinterpret_cast<std::unordered_map<DWORD, HWND>*>(lParam);

	const auto Iterator = Data.find(dwProcID);

	if (Iterator != Data.end() && !GetParent(hWnd))
	{
		const auto bVisible = IsWindowVisible(hWnd) || (IsIconic(hWnd) && !(GetWindowLongPtr(hWnd, GWL_STYLE) & WS_DISABLED));

		if (!Iterator->second || bVisible)
			Iterator->second = hWnd;
	}

	return TRUE;
}

struct custom_data
{
	static auto make(std::vector<std::wstring>&& Strings)
	{
		const auto Data = new custom_data;
		Data->m_Strings = std::move(Strings);
		Data->m_Pointers.resize(Data->m_Strings.size() + 1);
		std::transform(Data->m_Strings.cbegin(), Data->m_Strings.cend(), Data->m_Pointers.begin(), [](const std::wstring& Str) { return Str.c_str(); });
		Data->m_Pointers.back() = reinterpret_cast<const wchar_t*>(Data);
		return Data->m_Pointers.data();
	}

	static void destroy(const wchar_t* const* const Ptr, size_t const Size)
	{
		delete reinterpret_cast<const custom_data*>(Ptr[Size]);
	}

	custom_data(const custom_data&) = delete;
	custom_data& operator=(const custom_data&) = delete;

private:
	custom_data() = default;
	~custom_data() = default;

	std::vector<std::wstring> m_Strings;
	std::vector<const wchar_t*> m_Pointers;
};


int Plist::GetFindData(PluginPanelItem*& pPanelItem, size_t& ItemsNumber, OPERATION_MODES OpMode)
{
	const std::scoped_lock b(m_RefreshLock);
	const std::scoped_lock l(*pPerfThread);

	if (!GetList(pPanelItem, ItemsNumber, *pPerfThread))
		return FALSE;

	PanelInfo pi = { sizeof(PanelInfo) };
	PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &pi);
	auto& ProcPanelModes = HostName.empty()? m_PanelModesDataLocal : m_PanelModesDataRemote;
	wchar_t cDescMode = 0;

	if (HostName.empty())
	{
		if (const auto p = wcschr(ProcPanelModes[pi.ViewMode].panel_columns.internal_types.c_str(), L'Z'))
			cDescMode = p[1];
	}

	std::unordered_map<DWORD, HWND> Windows;

	for (size_t i = 0; i != ItemsNumber; ++i)
	{
		PluginPanelItem& CurItem = pPanelItem[i];
		auto& pdata = *static_cast<ProcessData*>(CurItem.UserData.Data);
		Windows[pdata.dwPID] = {};
	}

	EnumWindows(EnumWndProc, (LPARAM)&Windows);

	for (size_t i = 0; i != ItemsNumber; ++i)
	{
		PluginPanelItem& CurItem = pPanelItem[i];
		auto& pdata = *static_cast<ProcessData*>(CurItem.UserData.Data);
		// Make descriptions
		wchar_t Title[MAX_PATH]{};
		std::unique_ptr<char[]> Buffer;
		pdata.hwnd = Windows[pdata.dwPID];
		const wchar_t* pDesc = {};

		switch (upper(cDescMode))
		{
		case L'P':
			pDesc = pdata.FullPath.c_str();
			break;

		case L'W':
			if (pdata.hwnd)
				GetWindowText(pdata.hwnd, Title, static_cast<int>(std::size(Title)));
			pDesc = Title;
			break;

		case L'D':
			const wchar_t* pVersion;
			GetVersionInfo(pdata.FullPath.c_str(), Buffer, pVersion, pDesc);
			break;

		case L'C':
			pDesc = static_cast<ProcessData*>(CurItem.UserData.Data)->CommandLine.c_str();
			break;

		default:
			break;
		}

		if (pDesc)
		{
			CurItem.Description = new wchar_t[std::wcslen(pDesc) + 1];
			std::wcscpy(const_cast<wchar_t*>(CurItem.Description), pDesc);
		}

		const auto pd = pPerfThread->GetProcessData(pdata.dwPID, (DWORD)CurItem.NumberOfLinks);

		int Widths[MAX_CUSTOM_COLS]{};
		int nCols = 0;

		{
			const size_t Size = PsInfo.PanelControl(this, FCTL_GETCOLUMNWIDTHS, 0, {});
			const auto ColumnWidths = std::make_unique<wchar_t[]>(Size);
			PsInfo.PanelControl(this, FCTL_GETCOLUMNWIDTHS, Size, ColumnWidths.get());

			for (StrTok tokn(ColumnWidths.get(), L","); tokn && nCols < MAX_CUSTOM_COLS; ++tokn)
			{
				Widths[nCols++] = FSF.atoi(tokn);
			}

		}

		if (nCols)
		{
			std::vector<std::wstring> CustomDataStrings;
			nCols = 0;

			for (StrTok tok(ProcPanelModes[pi.ViewMode].panel_columns.internal_types.data(), L", "); tok; ++tok, ++nCols)
			{
				if (upper(*tok) != L'X')
					continue;

				// Custom column
				bool bCol = true;
				DWORD dwData = 0;
				int iCounter = -1;
				int nColWidth = Widths[nCols];

				if (nColWidth == 0)
					continue;

				bool bPerSec = false, bThousands = false, bFloat = false;

				const auto c = upper(tok[1]);
				switch (c)
				{
				case L'P': dwData = pdata.dwPrBase; break;
				case L'I': dwData = pdata.dwPID;
					break;

				case L'C': dwData = pdata.dwParentPID;
					break;

				case L'T': dwData = (DWORD)CurItem.NumberOfLinks; break;
				case L'B': dwData = pdata.Bitness; break;

				case L'G': if (pd) dwData = pd->dwGDIObjects; break;

				case L'U': if (pd) dwData = pd->dwUSERObjects; break;

				default:
					if (std::iswdigit(c))
					{
						iCounter = FSF.atoi(&tok[1]);

						if (wcspbrk(&tok[1], L"Ss") && can_be_per_sec(iCounter))
							bPerSec = true;

						if (wcspbrk(&tok[1], L"Tt"))
							bThousands = true;

						if (wcspbrk(&tok[1], L"Ff"))
							bFloat = true;
					}
					else
						bCol = false;
				}

				if (!bCol)
					continue;

				std::wstring Str;

				if (c >= L'A') // Not a performance counter
					Str = str(dwData);
				else if (pd && iCounter >= 0)     // Format performance counters
				{
					if (iCounter < 3 && !bPerSec) // first 3 are date/time
						Str = DurationToText(pd->qwCounters[iCounter]);
					else
						Str = ui64toa_width(bPerSec? pd->qwResults[iCounter] : pd->qwCounters[iCounter], bFloat? 0 : nColWidth, bThousands);
				}

				int nVisibleDigits = static_cast<int>(Str.size());

				if (nVisibleDigits > nColWidth) nVisibleDigits = nColWidth;

				Str.insert(0, nColWidth - nVisibleDigits, L' ');
				CustomDataStrings.emplace_back(std::move(Str));

				if (CustomDataStrings.size() == MAX_CUSTOM_COLS)
					break;
			}

			CurItem.CustomColumnNumber = CustomDataStrings.size();
			CurItem.CustomColumnData = custom_data::make(std::move(CustomDataStrings));
		}
	}

	LastUpdateTime = GetTickCount();
	return TRUE;
}

void Plist::FreeFindData(PluginPanelItem* PanelItem, size_t ItemsNumber)
{
	for (size_t i = 0; i != ItemsNumber; ++i)
	{
		auto& item = PanelItem[i];

		delete[] item.Description;
		delete[] item.Owner;
		delete[] item.FileName;
		delete[] item.AlternateFileName;
		custom_data::destroy(item.CustomColumnData, item.CustomColumnNumber);
	}

	delete PanelItem;
}

#define CODE_STR(Code) \
	{ Code, L ## # Code }

static auto window_style(HWND Hwnd)
{
	const auto Style = static_cast<DWORD>(GetWindowLongPtr(Hwnd, GWL_STYLE));

	static const std::pair<int, const wchar_t*> Styles[]
	{
		CODE_STR(WS_POPUP),
		CODE_STR(WS_CHILD),
		CODE_STR(WS_MINIMIZE),
		CODE_STR(WS_VISIBLE),
		CODE_STR(WS_DISABLED),
		CODE_STR(WS_CLIPSIBLINGS),
		CODE_STR(WS_CLIPCHILDREN),
		CODE_STR(WS_MAXIMIZE),
		CODE_STR(WS_BORDER),
		CODE_STR(WS_DLGFRAME),
		CODE_STR(WS_VSCROLL),
		CODE_STR(WS_HSCROLL),
		CODE_STR(WS_SYSMENU),
		CODE_STR(WS_THICKFRAME),
		CODE_STR(WS_MINIMIZEBOX),
		CODE_STR(WS_MAXIMIZEBOX),
	};

	std::wstring StyleStr;

	for (const auto& [Code, Str]: Styles)
	{
		if (!(Style & Code))
			continue;

		StyleStr += L' ';
		StyleStr += Str;
	}

	return std::pair{ Style, StyleStr };
}

static auto window_ex_style(HWND Hwnd)
{
	const auto ExStyle = GetWindowLongPtr(Hwnd, GWL_EXSTYLE);

	static const std::pair<int, const wchar_t*> ExtStyles[]
	{
		CODE_STR(WS_EX_DLGMODALFRAME),
		CODE_STR(WS_EX_NOPARENTNOTIFY),
		CODE_STR(WS_EX_TOPMOST),
		CODE_STR(WS_EX_ACCEPTFILES),
		CODE_STR(WS_EX_TRANSPARENT),
		CODE_STR(WS_EX_MDICHILD),
		CODE_STR(WS_EX_TOOLWINDOW),
		CODE_STR(WS_EX_WINDOWEDGE),
		CODE_STR(WS_EX_CLIENTEDGE),
		CODE_STR(WS_EX_CONTEXTHELP),
		CODE_STR(WS_EX_RIGHT),
		CODE_STR(WS_EX_RTLREADING),
		CODE_STR(WS_EX_LEFTSCROLLBAR),
		CODE_STR(WS_EX_CONTROLPARENT),
		CODE_STR(WS_EX_STATICEDGE),
		CODE_STR(WS_EX_APPWINDOW),
		CODE_STR(WS_EX_LAYERED),
		CODE_STR(WS_EX_NOINHERITLAYOUT),
		CODE_STR(WS_EX_LAYOUTRTL),
		CODE_STR(WS_EX_NOACTIVATE),
	};

	std::wstring ExStyleStr;

	for (const auto& [Code, Str]: ExtStyles)
	{
		if (!(ExStyle & Code))
			continue;

		ExStyleStr += L' ';
		ExStyleStr += Str;
	}

	return std::pair{ ExStyle, ExStyleStr };
}

#undef CODE_STR

static void DumpNTCounters(HANDLE InfoFile, PerfThread& Thread, DWORD dwPid, DWORD dwThreads)
{
	WriteToFile(InfoFile, L'\n');
	const std::scoped_lock l(Thread);
	const auto pdata = Thread.GetProcessData(dwPid, dwThreads);
	if (!pdata)
		return;

	const PerfLib* pf = Thread.GetPerfLib();

	for (size_t i = 0; i != std::size(Counters); i++)
	{
		if (!pf->dwCounterTitles[i]) // counter is absent
			continue;

		WriteToFile(InfoFile, PrintTitle(GetMsg(Counters[i].idName)));

		switch (pf->CounterTypes[i])
		{
		case PERF_COUNTER_RAWCOUNT:
		case PERF_COUNTER_LARGE_RAWCOUNT:
			// Display as is.  No Display Suffix.
			WriteToFile(InfoFile, far::format(L"{:20}\n"sv, pdata->qwResults[i]));
			break;

		case PERF_100NSEC_TIMER:
			// 64-bit Timer in 100 nsec units. Display delta divided by delta time. Display suffix: "%"
			WriteToFile(InfoFile, far::format(L"{:>20} {:7}%\n"sv, DurationToText(pdata->qwCounters[i]), pdata->qwResults[i]));
			break;

		case PERF_COUNTER_COUNTER:
			// 32-bit Counter.  Divide delta by delta time.  Display suffix: " / sec"
		case PERF_COUNTER_BULK_COUNT:
			// 64-bit Counter.  Divide delta by delta time. Display Suffix: " / sec"
			WriteToFile(InfoFile, far::format(L"{:20}  {:7}{}\n"sv, pdata->qwCounters[i], pdata->qwResults[i], GetMsg(MPerSec)));
			break;

		default:
			WriteToFile(InfoFile, L'\n');
			break;
		}
	}
}

int Plist::GetFiles(PluginPanelItem* PanelItem, size_t ItemsNumber, int Move, const wchar_t** DestPath, OPERATION_MODES OpMode, options& LocalOpt)
{
	const std::scoped_lock b(m_RefreshLock);

	static const wchar_t invalid_chars[] = L":*?\\/\"<>;|";

	if (ItemsNumber == 0)
		return 0;

	for (size_t I = 0; I != ItemsNumber; ++I)
	{
		PluginPanelItem& CurItem = PanelItem[I];
		auto pdata = static_cast<ProcessData*>(CurItem.UserData.Data);
		ProcessData PData;

		if (!pdata)
		{
			PData.dwPID = FSF.atoi(CurItem.AlternateFileName);
			const auto ppd = pPerfThread->GetProcessData(PData.dwPID, (DWORD)CurItem.NumberOfLinks);

			if (ppd && GetPData(PData, *ppd))
				pdata = &PData;

			if (!pdata)
				return 0;
		}

		// may be 0 if called from FindFile
		std::wstring FileName = *DestPath;

		if (!(OpMode & 0x10000))
		{
			if (FileName.back() != L'\\')
				FileName += L'\\';

			FileName += CurItem.FileName;
			FileName += L".txt";
		}

		// Replace "invalid" chars by underscores
		const auto From = FileName.rfind(L'\\') + 1;

		for (auto i = FileName.begin() + From; i != FileName.end(); ++i)
		{
			if (wcschr(invalid_chars, *i) || *i < L' ')
				*i = L'_';
		}

		const handle InfoFile(normalise_handle(CreateFile(FileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, {}, CREATE_ALWAYS, 0, {})));
		if (!InfoFile)
			return 0;

		WriteToFile(InfoFile.get(), L'\xfeff');

		WriteToFile(InfoFile.get(), far::format(L"{}{}, {}{}\n"sv, PrintTitle(MTitleModule), CurItem.FileName, pdata->Bitness, GetMsg(MBits)));

		if (!pdata->FullPath.empty())
		{
			WriteToFile(InfoFile.get(), far::format(L"{}{}\n"sv, PrintTitle(MTitleFullPath), pdata->FullPath));
			PrintVersionInfo(InfoFile.get(), pdata->FullPath.c_str());
		}

		WriteToFile(InfoFile.get(), far::format(L"{}{}\n"sv, PrintTitle(MTitlePID), pdata->dwPID));
		WriteToFile(InfoFile.get(), PrintTitle(MTitleParentPID));

		{
			const std::scoped_lock l(*pPerfThread);
			const auto pParentData = pPerfThread->GetProcessData(pdata->dwParentPID, 0);
			const auto pName = pdata->dwParentPID && pParentData? pParentData->ProcessName.c_str() : nullptr;

			if (pName)
				WriteToFile(InfoFile.get(), far::format(L"{}  ({})\n"sv, pdata->dwParentPID, pName));
			else
				WriteToFile(InfoFile.get(), far::format(L"{}\n"sv, pdata->dwParentPID));
		}

		WriteToFile(InfoFile.get(), far::format(L"{}{}\n"sv, PrintTitle(MTitlePriority), pdata->dwPrBase));
		WriteToFile(InfoFile.get(), far::format(L"{}{}\n"sv, PrintTitle(MTitleThreads), CurItem.NumberOfLinks));

		PrintOwnerInfo(InfoFile.get(), pdata->dwPID);

		// Time information

		if (CurItem.CreationTime.dwLowDateTime || CurItem.CreationTime.dwHighDateTime)
		{
			FILETIME CurFileTime;
			GetSystemTimeAsFileTime(&CurFileTime);
			SYSTEMTIME Current, Compare;
			GetLocalTime(&Current);
			FileTimeToSystemTime(&CurItem.CreationTime, &Compare);
			SystemTimeToTzSpecificLocalTime({}, &Compare, &Compare);
			wchar_t DateText[MAX_DATETIME], TimeText[MAX_DATETIME];
			ConvertDate(CurItem.CreationTime, DateText, TimeText);

			if (Current.wYear != Compare.wYear || Current.wMonth != Compare.wMonth || Current.wDay != Compare.wDay)
			{
				WriteToFile(InfoFile.get(), far::format(L"\n{}{} {}\n"sv, PrintTitle(MTitleStarted), DateText, TimeText));
			}
			else
			{
				WriteToFile(InfoFile.get(), far::format(L"\n{}{}\n"sv, PrintTitle(MTitleStarted), TimeText));
			}

			WriteToFile(InfoFile.get(), far::format(L"{}{}\n"sv, PrintTitle(MTitleUptime), FileTimeDifferenceToText(CurFileTime, CurItem.CreationTime)));
		}

		if (HostName.empty()) // local only
		{
			if (!pdata->CommandLine.empty())
			{
				WriteToFile(InfoFile.get(), far::format(L"\n{}:\n{}\n"sv, GetMsg(MCommandLine), pdata->CommandLine));
			}

			DebugToken token;

			if (const auto Process = handle(OpenProcessForced(&token, PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | READ_CONTROL, pdata->dwPID)))
			{
				PrintNTCurDirAndEnv(InfoFile.get(), Process.get(), LocalOpt.ExportEnvironment);

				const std::scoped_lock l(*pPerfThread);

				if (const auto pd = pPerfThread->GetProcessData(pdata->dwPID, (DWORD)CurItem.NumberOfLinks))
				{
					if (pd->dwGDIObjects)
					{
						WriteToFile(InfoFile.get(), far::format(L"\n{}{}\n"sv, PrintTitle(MTitleGDIObjects), pd->dwGDIObjects));
					}

					if (pd->dwUSERObjects)
					{
						WriteToFile(InfoFile.get(), far::format(L"{}{}\n"sv, PrintTitle(MTitleUSERObjects), pd->dwUSERObjects));
					}
				}
			}
		}

		if (LocalOpt.ExportPerformance)
			DumpNTCounters(InfoFile.get(), *pPerfThread, pdata->dwPID, (DWORD)CurItem.NumberOfLinks);

		if (HostName.empty() && pdata->hwnd)
		{
			wchar_t Title[MAX_PATH]; *Title = 0;
			GetWindowText(pdata->hwnd, Title, static_cast<int>(std::size(Title)));
			WriteToFile(InfoFile.get(), far::format(L"\n{}{}\n"sv, PrintTitle(MTitleWindow), Title));
			WriteToFile(InfoFile.get(), far::format(L"{}{}\n"sv, PrintTitle(L"HWND"sv), static_cast<const void*>(pdata->hwnd)));

			const auto& [Style, StyleStr] = window_style(pdata->hwnd);
			WriteToFile(InfoFile.get(), far::format(L"{}{:08X} {}\n"sv, PrintTitle(MTitleStyle), Style, StyleStr));
			const auto& [ExStyle, ExStyleStr] = window_ex_style(pdata->hwnd);
			WriteToFile(InfoFile.get(), far::format(L"{}{:08X} {}\n"sv, PrintTitle(MTitleExtStyle), ExStyle, ExStyleStr));
		}

		if (HostName.empty() && LocalOpt.ExportModuleInfo && pdata->dwPID != 8)
		{
			WriteToFile(InfoFile.get(), far::format(L"\n{}:\n{:<{}} {:<8} {}\n"sv,
				GetMsg(MTitleModules),
				GetMsg(MColBase),
#ifdef _WIN64
				16,
#else
				pdata->Bitness == 64? 16 : 8,
#endif

				GetMsg(MColSize),
				GetMsg(LocalOpt.ExportModuleVersion? MColPathVerDesc : MColPathVerDescNotShown)
			));

			PrintModules(InfoFile.get(), pdata->dwPID, LocalOpt);
		}

		if (HostName.empty() && (LocalOpt.ExportHandles | LocalOpt.ExportHandlesUnnamed) && pdata->dwPID /*&& pdata->dwPID!=8*/)
			PrintHandleInfo(pdata->dwPID, InfoFile.get(), LocalOpt.ExportHandlesUnnamed? true : false, pPerfThread.get());
	}

	return true;
}


int Plist::DeleteFiles(PluginPanelItem* PanelItem, size_t ItemsNumber, OPERATION_MODES OpMode)
{
	const std::scoped_lock b(m_RefreshLock);

	if (ItemsNumber == 0)
		return false;

	if (!HostName.empty() && !Opt.EnableWMI)
	{
		//cannot kill remote process
		const wchar_t* MsgItems[] = { GetMsg(MCannotDeleteProc),GetMsg(MCannotKillRemote),GetMsg(MOk) };
		Message(FMSG_WARNING, {}, MsgItems, std::size(MsgItems));
		return false;
	}

	{
		std::wstring Msg;

		const wchar_t* MsgItems[]
		{
			GetMsg(MDeleteTitle),
			GetMsg(MDeleteProcesses),
			GetMsg(MDeleteDelete),
			GetMsg(MCancel),
		};

		if (ItemsNumber == 1)
		{
			Msg = far::vformat(GetMsg(MDeleteProcess), PanelItem[0].FileName);
			MsgItems[1] = Msg.c_str();
		}

		if (Message(0, {}, MsgItems, std::size(MsgItems), 2) != 0)
			return FALSE;

		if (ItemsNumber > 1)
		{
			Msg = far::vformat(GetMsg(MDeleteNumberOfProcesses), ItemsNumber);
			MsgItems[1] = Msg.c_str();

			if (Message(FMSG_WARNING, {}, MsgItems, std::size(MsgItems), 2) != 0)
				return FALSE;
		}
	}

	for (size_t I = 0; I != ItemsNumber; ++I)
	{
		PluginPanelItem& CurItem = PanelItem[I];
		const auto pdata = static_cast<ProcessData*>(CurItem.UserData.Data);
		bool Success = false;
		int MsgId = 0;

		if (!HostName.empty())  // try WMI
		{
			if (!ConnectWMI())
			{
				WmiError();
				Success = false;
				break;
			}

			Success = false;

			switch (pWMI->TerminateProcess(pdata->dwPID))
			{
			case -1:
				WmiError();
				continue;
			case 0: Success = true; break;
			case 2: MsgId = MTAccessDenied; break;
			case 3: MsgId = MTInsufficientPrivilege; break;
			default: MsgId = MTUnknownFailure; break;
			}
		}
		else
		{
			Success = KillProcess(pdata->dwPID, pdata->hwnd);
		}


		if (!Success)
		{
			const auto Msg = far::vformat(GetMsg(MCannotDelete), CurItem.FileName);
			const wchar_t* MsgItems[]
			{
				GetMsg(MDeleteTitle),
				Msg.c_str(),
				{},
				GetMsg(MOk)
			};
			auto nItems = std::size(MsgItems);

			if (MsgId)
				MsgItems[2] = GetMsg(MsgId);
			else
			{
				MsgItems[2] = MsgItems[3];
				nItems--;
			}

			Message(FMSG_WARNING | FMSG_ERRORTYPE, {}, MsgItems, nItems);
			return false;
		}
	}

	pPerfThread->SmartReread();

	return true;
}


int Plist::ProcessEvent(intptr_t Event, void* Param)
{
	const std::scoped_lock b(m_RefreshLock);

	if (Event == FE_CLOSE)
	{
		PanelInfo pi = { sizeof(PanelInfo) };
		PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &pi);
		PluginSettings settings(MainGuid, PsInfo.SettingsControl);

		settings.Set(0, L"StartPanelMode", pi.ViewMode);
		settings.Set(0, L"SortMode", pi.SortMode == FarSortModeSlot? static_cast<OPENPANELINFO_SORTMODES>(SortMode) : pi.SortMode);
	}

	if (Event == FE_CHANGEVIEWMODE)
	{
		if (wcspbrk(static_cast<const wchar_t*>(Param), L"ZC"))
			Reread();
	}

	return FALSE;
}

void Plist::Reread()
{
	PsInfo.PanelControl(this, FCTL_UPDATEPANEL, 1, {});
	PsInfo.PanelControl(this, FCTL_REDRAWPANEL, 0, {});
	PanelInfo PInfo = { sizeof(PanelInfo) };
	PsInfo.PanelControl(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, &PInfo);

	if (PInfo.PanelType == PTYPE_QVIEWPANEL)
	{
		PsInfo.PanelControl(PANEL_PASSIVE, FCTL_UPDATEPANEL, 1, {});
		PsInfo.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, {});
	}
}

void Plist::PutToCmdLine(const wchar_t* tmp)
{
	const auto l = std::wcslen(tmp);
	std::unique_ptr<wchar_t[]> Buffer;

	if (wcscspn(tmp, L" &^") != l)
	{
		Buffer = std::make_unique<wchar_t[]>(l + 3);
		memcpy(Buffer.get() + 1, tmp, l * sizeof(wchar_t));
		Buffer[0] = Buffer[l + 1] = L'\"';
		Buffer[l + 2] = L'\0';
		tmp = Buffer.get();
	}

	PsInfo.PanelControl(this, FCTL_INSERTCMDLINE, 0, const_cast<wchar_t*>(tmp));
	PsInfo.PanelControl(this, FCTL_INSERTCMDLINE, 0, const_cast<wchar_t*>(L" "));
}

bool Plist::Connect(const wchar_t* pMachine, const wchar_t* pUser, const wchar_t* pPasw)
{
	std::wstring Machine = pMachine;

	// Add "\\" if missing
	if (!norm_m_prefix(Machine.c_str()))
	{
		Machine.insert(0, L"\\\\");
	}

	//Try to connect...
	const wchar_t* ConnectItems[] = { L"",GetMsg(MConnect) };
	const auto hScreen = PsInfo.SaveScreen(0, 0, -1, -1);
	Message(0, {}, ConnectItems, 2, 0);

	if (pUser && *pUser)
	{
		static NETRESOURCE nr =
		{ RESOURCE_GLOBALNET, RESOURCETYPE_DISK, RESOURCEDISPLAYTYPE_SERVER,
		  RESOURCEUSAGE_CONTAINER, {}, {}, const_cast<wchar_t*>(L""), {}
		};
		nr.lpRemoteName = const_cast<wchar_t*>(Machine.c_str());
		DWORD dwErr = WNetAddConnection2(&nr, pPasw, pUser, 0);

		if (dwErr == ERROR_SESSION_CREDENTIAL_CONFLICT)
		{
			WNetCancelConnection2(Machine.c_str(), 0, 0);
			dwErr = WNetAddConnection2(&nr, pPasw, pUser, 0);
		}

		if (dwErr != NO_ERROR)
		{
			SetLastError(dwErr);
			WinError();
			return false;
		}
	}

	auto pNewPerfThread = std::make_unique<PerfThread>(this, Machine.c_str(), pUser? pUser : nullptr, pUser? pPasw : nullptr);

	if (!pNewPerfThread->IsOK())
	{
		WinError();
		PsInfo.RestoreScreen(hScreen);
	}
	else
	{
		PsInfo.RestoreScreen(hScreen);
		pPerfThread = std::move(pNewPerfThread);
		DisconnectWMI();
		HostName = std::move(Machine);
		return true;
	}

	return false;
}

// https://blogs.msdn.microsoft.com/oldnewthing/20071008-00/?p=24863/
static bool is_alttab_window(HWND const Window)
{
	if (!IsWindowVisible(Window))
		return false;

	auto Try = GetAncestor(Window, GA_ROOTOWNER);
	HWND Walk = nullptr;
	while (Try != Walk)
	{
		Walk = Try;
		Try = GetLastActivePopup(Walk);
		if (IsWindowVisible(Try))
			break;
	}
	if (Walk != Window)
		return false;

	// Tool windows should not be displayed either, these do not appear in the task bar
	if (GetWindowLongPtr(Window, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
		return false;

	/*if (IsWindows8OrGreater())
	{
		int Cloaked = 0;
		if (SUCCEEDED(imports.DwmGetWindowAttribute(Window, DWMWA_CLOAKED, &Cloaked, sizeof(Cloaked))) && Cloaked)
			return false;
	}*/

	return true;
}

static constexpr auto
	none_pressed = 0,
	any_ctrl_pressed = LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED,
	any_alt_pressed = LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED,
	any_shift_pressed = SHIFT_PRESSED;

constexpr auto check_control(unsigned const ControlState, unsigned const Mask)
{
	constexpr auto ValidMask = any_ctrl_pressed | any_alt_pressed | any_shift_pressed;

	const auto FilteredControlState = ControlState & ValidMask;
	const auto OtherKeys = ValidMask & ~Mask;

	return ((FilteredControlState & Mask) || !Mask) && !(FilteredControlState & OtherKeys);
};

static_assert(check_control(0, 0));
static_assert(check_control(NUMLOCK_ON, 0));
static_assert(check_control(NUMLOCK_ON | SHIFT_PRESSED, any_shift_pressed));
static_assert(check_control(NUMLOCK_ON | SHIFT_PRESSED | LEFT_ALT_PRESSED, any_shift_pressed | any_alt_pressed));
static_assert(!check_control(NUMLOCK_ON | SHIFT_PRESSED | LEFT_ALT_PRESSED, any_shift_pressed));
static_assert(!check_control(0, SHIFT_PRESSED));
static_assert(!check_control(NUMLOCK_ON, SHIFT_PRESSED));
static_assert(!check_control(NUMLOCK_ON | SHIFT_PRESSED, 0));


int Plist::ProcessKey(const INPUT_RECORD* Rec)
{
	const std::scoped_lock b(m_RefreshLock);

	if (Rec->EventType != KEY_EVENT)
		return FALSE;

	const auto Key = Rec->Event.KeyEvent.wVirtualKeyCode;
	const auto ControlState = Rec->Event.KeyEvent.dwControlKeyState;

	const auto
		NonePressed = check_control(ControlState, none_pressed),
		OnlyAnyCtrlPressed = check_control(ControlState, any_ctrl_pressed),
		//OnlyAnyAltPressed = check_control(ControlState, any_alt_pressed),
		OnlyAnyShiftPressed = check_control(ControlState, any_shift_pressed);

	if (OnlyAnyCtrlPressed && Key == L'R')
	{
		pPerfThread->SmartReread();
		return FALSE;
	}

	if (NonePressed && Key == VK_RETURN)
	{
		//check for the command line; if it's not empty, don't process Enter
		if (PsInfo.PanelControl(this, FCTL_GETCMDLINE, 0, {}) > 1)
			return false;

		PanelInfo PInfo = { sizeof(PanelInfo) };
		PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);

		if (PInfo.CurrentItem < PInfo.ItemsNumber)
		{
			const size_t Size = PsInfo.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, {});
			const auto CurItem = make_malloc<PluginPanelItem>(Size);
			if (!CurItem)
				return false;

			FarGetPluginPanelItem gpi{ sizeof(FarGetPluginPanelItem), Size, CurItem.get() };
			PsInfo.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, &gpi);

			if (!CurItem->UserData.Data)
				return false;

			if (const auto hWnd = static_cast<ProcessData*>(CurItem->UserData.Data)->hwnd; hWnd && is_alttab_window(hWnd))
			{
				SetForegroundWindow(hWnd);

				if (IsIconic(hWnd))
					ShowWindowAsync(hWnd, SW_RESTORE);
			}
		}

		return TRUE;
	}

	if (OnlyAnyShiftPressed && Key == VK_F3)
	{
		PanelInfo pi{ sizeof(PanelInfo) };
		PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &pi);

		size_t Size = PsInfo.PanelControl(this, FCTL_GETPANELITEM, pi.CurrentItem, {});
		if (const auto PPI = make_malloc<PluginPanelItem>(Size))
		{
			FarGetPluginPanelItem gpi = { sizeof(FarGetPluginPanelItem), Size, PPI.get() };
			PsInfo.PanelControl(this, FCTL_GETPANELITEM, pi.CurrentItem, &gpi);
			bool Exit = pi.CurrentItem >= pi.ItemsNumber || !lstrcmp(PPI->FileName, L"..");

			if (Exit)
				return TRUE;
		}
		else
			return FALSE; //???


		auto LocalOpt = Opt;

		PluginDialogBuilder Builder(PsInfo, MainGuid, ConfigDialogGuid, MViewWithOptions, L"Config");
		Builder.AddText(MIncludeAdditionalInfo);
		Builder.AddCheckbox(MInclEnvironment, &LocalOpt.ExportEnvironment);
		Builder.AddCheckbox(MInclModuleInfo, &LocalOpt.ExportModuleInfo);
		Builder.AddCheckbox(MInclModuleVersion, &LocalOpt.ExportModuleVersion);
		Builder.AddCheckbox(MInclPerformance, &LocalOpt.ExportPerformance);
		Builder.AddCheckbox(MInclHandles, &LocalOpt.ExportHandles);
		//Builder.AddCheckbox(MInclHandlesUnnamed, &LocalOpt.ExportHandlesUnnamed); // ???
		Builder.AddOKCancel(MOk, MCancel);

		if (!Builder.ShowDialog())
			return TRUE;

		wchar_t FileName[MAX_PATH];
		FSF.MkTemp(FileName, std::size(FileName), L"prc");
		const wchar_t* lpFileName = FileName;
		Size = PsInfo.PanelControl(this, FCTL_GETPANELITEM, pi.CurrentItem, {});
		if (const auto PPI = make_malloc<PluginPanelItem>(Size))
		{
			FarGetPluginPanelItem gpi{ sizeof(FarGetPluginPanelItem), Size, PPI.get() };
			PsInfo.PanelControl(this, FCTL_GETPANELITEM, pi.CurrentItem, &gpi);

			if (GetFiles(PPI.get(), 1, 0, &lpFileName, OPM_VIEW | 0x10000, LocalOpt))
			{
				//TODO: viewer crashed on exit!
				PsInfo.Viewer(FileName, PPI->FileName, 0, 0, -1, -1, VF_NONMODAL | VF_DELETEONCLOSE, CP_DEFAULT);
			}
		}
		else
			return FALSE; //???


		return TRUE;
	}

	if (NonePressed && Key == VK_F6)
	{
		{
			wchar_t Host[1024] = {};
			std::wcsncpy(Host, HostName.c_str(), std::size(Host));
			wchar_t Username[1024] = {};
			wchar_t Password[1024] = {};
			PluginDialogBuilder Builder(PsInfo, MainGuid, ConfigDialogGuid, MSelectComputer, L"Contents"); // ConfigDialogGuid ???
			Builder.AddText(MComputer);
			Builder.AddEditField(Host, static_cast<int>(std::size(Host)), 65, L"ProcessList.Computer");
			Builder.AddText(MEmptyForLocal);
			Builder.AddSeparator();
			Builder.StartColumns();
			Builder.AddText(MUsername);
			Builder.AddEditField(Username, static_cast<int>(std::size(Username)), 18, L"ProcessList.Username");
			Builder.ColumnBreak();
			Builder.AddText(MPaswd);
			Builder.AddPasswordField(Password, static_cast<int>(std::size(Password)), 18);
			Builder.EndColumns();
			Builder.AddText(MEmptyForLocal);
			Builder.AddOKCancel(MOk, MCancel);

			//Loop until successful connect or user cancel in dialog
			for (bool stop = false; ;)
			{
				if (!Builder.ShowDialog())
					stop = true;
				else
				{
					HostName = Host;
					if (HostName.empty() || HostName == L"\\\\")
					{
						//go to local computer
						pPerfThread = {};
						DisconnectWMI();
						pPerfThread = std::make_unique<PerfThread>(this);
						HostName.clear();
						stop = true;
					}
					else if (Connect(HostName.c_str(), Username, Password))
					{
						stop = true;
					}
				}
				if (stop)
					break;

			}
		}

		Reread();
		return TRUE;
	}

	if (OnlyAnyShiftPressed && Key == VK_F6)
	{
		// go to local host
		pPerfThread = {};
		DisconnectWMI();
		pPerfThread = std::make_unique<PerfThread>(this);
		HostName.clear();
		Reread();
		return TRUE;
	}

#if 0
	if (OnlyAnyAltPressed && Key == VK_F6)
	{
		if (!Opt.EnableWMI)
			return TRUE;

		const wchar_t* MsgItems[] = {/*GetMsg(MAttachDebugger)*/L"Attach Debugger",/*GetMsg(MConfirmAttachDebugger)*/L"Do you want to attach debugger to this process?",GetMsg(MYes),GetMsg(MNo) };

		if (Message(0, {}, MsgItems, std::size(MsgItems), 2) != 0)
			return TRUE;

		PanelInfo pi = { sizeof(PanelInfo) };
		Control(FCTL_GETPANELINFO, &pi);
		PluginPanelItem& item = pi.PanelItems[pi.CurrentItem];

		if (!lstrcmp(item.FileName, L".."))
			return TRUE;

		int i;

		if (ConnectWMI() && pWMI && item.UserData)
			switch (i = pWMI->AttachDebuggerToProcess(((ProcessData*)item.UserData)->dwPID))
			{
			case -1: WmiError(); break;
			case 0: break;
			case 2: SetLastError(i); WinError(); break;
			default:
				wchar_t buf[80];
				FSF.sprintf(buf, L"Return code: %d", i);
				const wchar_t* MsgItems[] = {/*GetMsg(MAttachDebugger)*/L"Attach Debugger",buf,GetMsg(MOk) };
				Message(FMSG_WARNING, 0, MsgItems, std::size(MsgItems));
				/*3 The user does not have sufficient privilege.
				8 Unknown failure.
				9 The path specified does not exist.
				21 The specified parameter is invalid.
				*/
				break;
			}

		return TRUE;
	}

#endif

	if (OnlyAnyShiftPressed && (Key == VK_F1 || Key == VK_F2) && (HostName.empty() || Opt.EnableWMI))
	{
		//lower/raise priority class
		PanelInfo PInfo = { sizeof(PanelInfo) };
		PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);

		if (PInfo.SelectedItemsNumber > 1)
		{
			const wchar_t* MsgItems[] = { GetMsg(MChangePriority),GetMsg(MConfirmChangePriority),GetMsg(MYes),GetMsg(MNo) };

			if (Message(0, {}, MsgItems, std::size(MsgItems), 2) != 0)
			{
				return TRUE;
			}
		}

		if (!HostName.empty() && Opt.EnableWMI && !ConnectWMI())
		{
			WmiError();
			return TRUE;
		}

		static const USHORT PrClasses[] =
		{ IDLE_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS,
		  NORMAL_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS,
		  HIGH_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS
		};
		const int N = static_cast<int>(std::size(PrClasses));
		DebugToken token;

		for (size_t i = 0; i < PInfo.SelectedItemsNumber; i++)
		{
			const size_t Size = PsInfo.PanelControl(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, 0, {});
			if (!Size)
				continue;

			const auto PPI = make_malloc<PluginPanelItem>(Size);
			if (!PPI)
				continue;

			FarGetPluginPanelItem gpi{ sizeof(FarGetPluginPanelItem), Size, PPI.get() };
			PsInfo.PanelControl(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, i, &gpi);
			SetLastError(ERROR_SUCCESS);

			if (static_cast<ProcessData*>(PPI->UserData.Data)->dwPID)
			{
				if (HostName.empty())
				{
					if (const auto hProcess = handle(OpenProcessForced(&token, PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION, ((ProcessData*)PPI->UserData.Data)->dwPID)))
					{
						DWORD dwPriorityClass = GetPriorityClass(hProcess.get());

						if (dwPriorityClass == 0)
							WinError();
						else
						{
							for (int j = 0; j != N; ++j)
							{
								if (dwPriorityClass != PrClasses[j])
									continue;

								bool bChange = false;

								if (Key == VK_F1 && j > 0)
								{
									j--; bChange = true;
								}
								else if (Key == VK_F2 && j < N - 1)
								{
									j++;
									bChange = true;
								}

								if (bChange && !SetPriorityClass(hProcess.get(), PrClasses[j]))
									WinError();

								//else
									//Item.Flags &= ~PPIF_SELECTED;
								break;
							}

						}
					}
					else
						WinError();
				}
				else if (pWMI)  //*HostName
				{
					DWORD dwPriorityClass = pWMI->GetProcessPriority(static_cast<ProcessData*>(PPI->UserData.Data)->dwPID);

					if (!dwPriorityClass)
					{
						WmiError();
						continue;
					}

					static const BYTE Pr[std::size(PrClasses)] = { 4,6,8,10,13,24 };

					for (int j = 0; j != N; ++j)
					{
						if (dwPriorityClass != Pr[j])
							continue;

						bool bChange = false;

						if (Key == VK_F1 && j > 0)
						{
							j--;
							bChange = true;
						}
						else if (Key == VK_F2 && j < N - 1)
						{
							j++;
							bChange = true;
						}

						if (bChange && pWMI->SetProcessPriority(static_cast<ProcessData*>(PPI->UserData.Data)->dwPID, PrClasses[j]) != 0)
							WmiError();

						break;
					}
				}
			}
		}

		token.Revert();

		/*    // Copy flags from SelectedItems to PanelItems
		for(int i1=0; i1<PInfo.SelectedItemsNumber; i1++)
		for(int i2=0; i2<PInfo.ItemsNumber; i2++)
		if(((ProcessData*)PInfo.PanelItems[i2].UserData)->dwPID==((ProcessData*)PInfo.SelectedItems[i1].UserData)->dwPID) {
		PInfo.PanelItems[i2].Flags = PInfo.SelectedItems[i1].Flags;
		break;
		}
		Control(FCTL_SETSELECTION,&PInfo);
		*/
		pPerfThread->SmartReread();

		Reread();
		return TRUE;
		/*  } else if ((ControlState&(RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED)) && (ControlState&SHIFT_PRESSED) && Key==VK_F9) {
		Config();
		return TRUE;*/
	}

	if (OnlyAnyCtrlPressed && Key == L'F')
	{
		PanelInfo pi = { sizeof(PanelInfo) };
		PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &pi);

		if (pi.CurrentItem < pi.ItemsNumber)
		{
			const size_t Size = PsInfo.PanelControl(this, FCTL_GETPANELITEM, pi.CurrentItem, {});
			const auto PPI = make_malloc<PluginPanelItem>(Size);
			if (PPI)
			{
				FarGetPluginPanelItem gpi{ sizeof(FarGetPluginPanelItem), Size, PPI.get() };
				PsInfo.PanelControl(this, FCTL_GETPANELITEM, pi.CurrentItem, &gpi);

				if (const auto pData = static_cast<ProcessData*>(PPI->UserData.Data))
					PutToCmdLine(pData->FullPath.c_str());
			}
			else
				return FALSE;

		}

		return TRUE;
	}

	if (OnlyAnyCtrlPressed && (Key >= VK_F3 && Key <= VK_F12))
	{
		if (
			Key == VK_F5 || // Write time
			Key == VK_F9    // Access time
			)
			return TRUE;

		static const struct
		{
			unsigned id;
			unsigned SortMode;
			unsigned Key;
			bool InvertByDefault;
		}
		StaticItems[]
		{
			{MSortByName,           SM_NAME,                 VK_F3,  false, },
			{MSortByExt,            SM_EXT,                  VK_F4,  false, },
			{MSortBySize,           SM_SIZE,                 VK_F6,  true,  },
			{MSortByUnsorted,       SM_UNSORTED,             VK_F7,  false, },
			{MSortByTime,           SM_CTIME,                VK_F8,  true,  },
			{MSortByDescriptions,   SM_DESCR,                VK_F10, false, },
			{MSortByOwner,          SM_OWNER,                VK_F11, false, },
			//{MPageFileBytes,        SM_COMPRESSEDSIZE,       0,      true,  },
			{MTitlePID,             SM_PROCLIST_PID,         0,      true,  },
			{MTitleParentPID,       SM_PROCLIST_PARENTPID,   0,      true,  },
			{MTitleThreads,         SM_NUMLINKS,             0,      true,  },
			{MTitlePriority,        SM_PROCLIST_PRIOR,       0,      true   },
			//{0,-1},
			//{MUseSortGroups,0},
			//{MShowSelectedFirst,-1}
		};

		PanelInfo pi{ sizeof(PanelInfo) };
		PsInfo.PanelControl(this, FCTL_GETPANELINFO, 0, &pi);

		const auto Reversed = (pi.Flags & PFLAGS_REVERSESORTORDER) != 0;

		if (Key != VK_F12)
		{
			const auto CurrentSortMode = SortMode;
			const auto& StaticItem = std::find_if(std::cbegin(StaticItems), std::cend(StaticItems), [Key](const auto& i) { return i.Key == Key; });

			SortMode = StaticItem->SortMode;
			PsInfo.PanelControl(this, FCTL_SETSORTMODE, SortMode, {});

			const auto SameSelected = SortMode == CurrentSortMode;
			PsInfo.PanelControl(this, FCTL_SETSORTORDER, SameSelected? !Reversed : StaticItem->InvertByDefault, {});

			return TRUE;
		}

		const auto cIndicator = Reversed? L'▼' : L'▲';
		const auto KnownSortingSlot =
			pi.SortMode == FarSortModeSlot ||
			std::any_of(std::cbegin(StaticItems), std::cend(StaticItems), [&](const auto& i){ return static_cast<unsigned>(pi.SortMode) == i.SortMode; });

		const auto CheckFlag = [&](size_t const Value)
		{
			return KnownSortingSlot && SortMode == Value? MIF_SELECTED | MIF_CHECKED | cIndicator : 0;
		};

		std::vector<FarMenuItem> Items;
		Items.reserve(std::size(StaticItems) + 1 + std::size(Counters) * 2);

		for (const auto& i: StaticItems)
		{
			Items.push_back({ CheckFlag(i.SortMode), GetMsg(i.id) });
		}

		Items.push_back({ MIF_SEPARATOR });

		const auto pl = pPerfThread->GetPerfLib();

		std::vector<std::wstring> PerSecMessages;
		PerSecMessages.reserve(std::size(Counters));

		std::vector<size_t> DynamicSortModes;
		DynamicSortModes.reserve(std::size(Counters) * 2);

		for (size_t i = 0; i < std::size(Counters); i++)
		{
			if (!pl->dwCounterTitles[i])
				continue;

			Items.push_back({ CheckFlag(SM_PROCLIST_PERFCOUNTER + i), GetMsg(Counters[i].idName) });

			DynamicSortModes.push_back(i);

			if (!can_be_per_sec(i))
				continue;

			PerSecMessages.emplace_back(i < 3?
				far::format(L"% {}"sv, GetMsg(Counters[i].idName)) :
				far::format(L"{}{}"sv, GetMsg(Counters[i].idName), GetMsg(MPerSec))
			);

			Items.push_back({ CheckFlag(SM_PROCLIST_PERSEC | (SM_PROCLIST_PERFCOUNTER + i)), PerSecMessages.back().c_str() });

			DynamicSortModes.push_back(i | SM_PROCLIST_PERSEC);
		}

		// Show sort menu
		const auto rc = Menu(FMENU_AUTOHIGHLIGHT | FMENU_WRAPMODE, GetMsg(MSortBy), {}, {}, {}, Items.data(), Items.size());

		if (rc >= 0)
		{
			const auto IsStatic = static_cast<size_t>(rc) < std::size(StaticItems);
			const auto CurrentSortMode = SortMode;

			SortMode = IsStatic?
				StaticItems[rc].SortMode :
				SM_PROCLIST_PERFCOUNTER + static_cast<unsigned>(DynamicSortModes[rc - std::size(StaticItems) - 1]);

			auto FarSortMode = SortMode < SM_PROCLIST_CUSTOM? static_cast<OPENPANELINFO_SORTMODES>(SortMode) : FarSortModeSlot;

			PsInfo.PanelControl(this, FCTL_SETSORTMODE, FarSortMode, {});

			const auto InvertByDefault = !IsStatic || StaticItems[rc].InvertByDefault;
			const auto SameSelected = SortMode == CurrentSortMode;

			PsInfo.PanelControl(this, FCTL_SETSORTORDER, SameSelected? !Reversed : InvertByDefault, {});
			/*
			else if(rc==std::size(StaticItems)-2)
			Control(FCTL_SETSORTORDER, &items[rc].mode);
			else if(rc==std::size(StaticItems)-1)
			Control(FCTL_SETSORTMODE, &items[rc].mode);
			*/
		}

		return TRUE;
	}

	return FALSE;
}

void Plist::ProcessSynchroEvent()
{
	if (m_RefreshLock.is_busy())
		return;

	Reread();
}

std::wstring DurationToText(uint64_t Duration)
{
	const auto
		TicksPerS = 10'000'000ull,
		TicksPerM = TicksPerS * 60,
		TicksPerH = TicksPerM * 60,
		TicksPerD = TicksPerH * 24;

	const auto
		Days    = Duration / TicksPerD,
		Hours   = Duration % TicksPerD / TicksPerH,
		Minutes = Duration % TicksPerD % TicksPerH / TicksPerM,
		Seconds = Duration % TicksPerD % TicksPerH % TicksPerM / TicksPerS,
		Ticks   = Duration % TicksPerD % TicksPerH % TicksPerM % TicksPerS / 1;

	if (Days > 0)
		return far::format(L"{} {:02}:{:02}:{:02}.{:07}"sv, Days, Hours, Minutes, Seconds, Ticks);
	else
		return far::format(L"{:02}:{:02}:{:02}.{:07}"sv, Hours, Minutes, Seconds, Ticks);
}

std::wstring FileTimeDifferenceToText(const FILETIME& CurFileTime, const FILETIME& SrcTime)
{
	ULARGE_INTEGER Cur, Src;
	Cur.HighPart = CurFileTime.dwHighDateTime;
	Cur.LowPart = CurFileTime.dwLowDateTime;
	Src.HighPart = SrcTime.dwHighDateTime;
	Src.LowPart = SrcTime.dwLowDateTime;

	return DurationToText(Cur.QuadPart - Src.QuadPart);
}

template<class T>
const T* GetValue(const char* Buffer, const wchar_t* SubBlock)
{
	UINT Length;
	T* Result;
	return VerQueryValue(Buffer, SubBlock, reinterpret_cast<void**>(&Result), &Length) && Length? Result : nullptr;
}

bool Plist::GetVersionInfo(const wchar_t* pFullPath, std::unique_ptr<char[]>& Buffer, const wchar_t*& pVersion, const wchar_t*& pDesc)
{
	if (!std::wmemcmp(pFullPath, L"\\??\\", 4))
		pFullPath += 4;

	const auto Size = GetFileVersionInfoSize(pFullPath, nullptr);

	if (!Size)
		return false;

	Buffer = std::make_unique<char[]>(Size);
	if (!GetFileVersionInfo(pFullPath, 0, Size, Buffer.get()))
		return false;

	const auto Translation = GetValue<DWORD>(Buffer.get(), L"\\VarFileInfo\\Translation");
	if (!Translation)
		return false;

	const auto BlockPath = far::format(L"\\StringFileInfo\\{:04X}{:04X}\\"sv, LOWORD(*Translation), HIWORD(*Translation));

	pVersion = GetValue<wchar_t>(Buffer.get(), (BlockPath + L"FileVersion"s).c_str());
	pDesc = GetValue<wchar_t>(Buffer.get(), (BlockPath + L"FileDescription"s).c_str());

	return true;
}

void Plist::PrintVersionInfo(HANDLE InfoFile, const wchar_t* pFullPath)
{
	const wchar_t* pVersion, * pDesc;
	std::unique_ptr<char[]> Buffer;

	if (!GetVersionInfo(pFullPath, Buffer, pVersion, pDesc))
		return;

	if (pVersion)
	{
		WriteToFile(InfoFile, far::format(L"{}{}\n"sv, PrintTitle(MTitleFileVersion), pVersion));
	}

	if (pDesc)
	{
		WriteToFile(InfoFile, far::format(L"{}{}\n"sv, PrintTitle(MTitleFileDesc), pDesc));
	}
}

bool Plist::ConnectWMI()
{
	if (!Opt.EnableWMI || dwPluginThread != GetCurrentThreadId())
		return false;

	if (!pWMI)
		pWMI = std::make_unique<WMIConnection>();

	if (*pWMI)
		return true;

	return pWMI->Connect(
		HostName.c_str(),
		pPerfThread->UserName().c_str(),
		pPerfThread->Password().c_str()
	);
}

void Plist::DisconnectWMI()
{
	pWMI = {};
}

void Plist::PrintOwnerInfo(HANDLE InfoFile, DWORD dwPid)
{
	if (!Opt.EnableWMI || !pPerfThread->IsWMIConnected() || !ConnectWMI())
		return;

	if (!*pWMI) // exists, but could not connect
		return;

	std::wstring Domain;
	const auto User = pWMI->GetProcessOwner(dwPid, &Domain);
	const auto Sid = pWMI->GetProcessUserSid(dwPid);

	if (!User.empty() || !Domain.empty() || !Sid.empty())
	{
		WriteToFile(InfoFile, PrintTitle(MTitleUsername));

		if (!Domain.empty())
			WriteToFile(InfoFile, far::format(L"{}\\"sv, Domain));

		if (!User.empty())
			WriteToFile(InfoFile, User);

		if (!Sid.empty())
			WriteToFile(InfoFile, far::format(L" ({})"sv, Sid));

		WriteToFile(InfoFile, L'\n');
	}

	WriteToFile(InfoFile, far::format(L"{}{}\n"sv, PrintTitle(MTitleSessionId), pWMI->GetProcessSessionId(dwPid)));
}

template<typename T>
auto compare_numbers(T const First, T const Second)
{
	return First < Second? -1 : First != Second;
}

int Plist::Compare(const PluginPanelItem* Item1, const PluginPanelItem* Item2, unsigned int Mode) const
{
	const std::scoped_lock b(m_RefreshLock);

	if (Mode != FarSortModeSlot || SortMode < SM_PROCLIST_CUSTOM)
		return -2;

	int diff;

	const auto& pd1 = *static_cast<const ProcessData*>(Item1->UserData.Data);
	const auto& pd2 = *static_cast<const ProcessData*>(Item2->UserData.Data);

	switch (SortMode)
	{
	case SM_PROCLIST_PID:
		diff = compare_numbers(
			pd1.dwPID,
			pd2.dwPID
		);
		break;

	case SM_PROCLIST_PARENTPID:
		diff = compare_numbers(
			pd1.dwParentPID,
			pd2.dwParentPID
		);
		break;

	case SM_PROCLIST_PRIOR:
		diff = compare_numbers(
			pd1.dwPrBase,
			pd2.dwPrBase
		);
		break;

	default:
		{
			const std::scoped_lock l(*pPerfThread);
			const auto data1 = pPerfThread->GetProcessData(pd1.dwPID, static_cast<DWORD>(Item1->NumberOfLinks));
			const auto data2 = pPerfThread->GetProcessData(pd2.dwPID, static_cast<DWORD>(Item2->NumberOfLinks));

			if (!data1)
				return data2? -1 : 0;

			if (!data2)
				return 1;

			bool bPerSec = false;
			auto smode = SortMode;

			if (smode >= SM_PROCLIST_PERSEC)
			{
				bPerSec = true;
				smode &= ~SM_PROCLIST_PERSEC;
			}

			const auto i = smode - SM_PROCLIST_PERFCOUNTER;
			//if((DWORD)i >= (DWORD)NCOUNTERS){
			//  i=0; //????
			//}

			diff = bPerSec?
				compare_numbers(
					data1->qwResults[i],
					data2->qwResults[i]
				) :
				compare_numbers(
					data1->qwCounters[i],
					data2->qwCounters[i]
			);
		}
		break;
	}

	if (diff == 0)
		diff = compare_numbers(
			reinterpret_cast<uintptr_t>(Item1->UserData.Data),
			reinterpret_cast<uintptr_t>(Item2->UserData.Data)
		); // unsorted

	return diff;
}

/*
bool Plist::PostUpdate()
{
PanelInfo pi = {sizeof(PanelInfo)};
if(!Control(FCTL_GETPANELINFO, &pi))
return false;

DWORD dwCtrlR = KEY_CTRL | L'R';
KeySequence ks = { 0, 1, &dwCtrlR };
(*PsInfo.AdvControl)(PsInfo.ModuleNumber, ACTL_POSTKEYSEQUENCE, &ks);
Control(FCTL_REDRAWPANEL, nullptr);
return true;
}
*/
void Plist::WmiError() const
{
	if (pWMI)
	{
		const auto hr = pWMI->GetLastHResult();
		SetLastError(hr);
		WinError(hr < (HRESULT)0x80040000? nullptr : L"wbemcomn");
	}
}
