#include <algorithm>
#include <mutex>
#include <string>
#include <cwchar>

#include "Proclist.hpp"
#include "perfthread.hpp"
#include "Proclng.hpp"
#include "guid.hpp"
#include <DlgBuilder.hpp>
#include <PluginSettings.hpp>

using namespace std::literals;

class StrTok
{
public:
	StrTok(const wchar_t* str, const wchar_t* token):
		tok(token)
	{
		buf.reset(wcsdup(str));
		ptr = std::wcstok(buf.get(), token);
	}

	operator wchar_t*() const
	{
		return ptr;
	}

	void operator++()
	{
		ptr = std::wcstok({}, tok);
	}

	explicit operator bool() const
	{
		return ptr != nullptr;
	}

private:
	const wchar_t* tok;
	wchar_t* ptr;
	malloc_ptr<wchar_t> buf;
};

ui64Table::ui64Table()
{
	uint64_t n = 1;

	for (auto& i : Table)
	{
		i = n;
		n *= 10;
	}
}

uint64_t ui64Table::tenpow(size_t n)
{
	if (n >= std::size(Table))
		n = std::size(Table) - 1;

	return Table[n];
}

static void ui64toa_width(unsigned __int64 value, wchar_t* buf, unsigned width, bool bThousands)
{
	if (width < 1)
		return;

	const wchar_t* pSuffix = L"";
	const unsigned uDivider = bThousands? 1000 : 1024;

	if (width <= 20)
	{
		if (value >= Ui64Table->tenpow(width))
		{
			value /= uDivider;
			pSuffix = L"K";
		}

		if (value >= Ui64Table->tenpow(width))
		{
			value /= uDivider;
			pSuffix = L"M";
		}
	}

	_ui64tow(value, buf, 10);
	std::wcscat(buf, pSuffix);
}

Plist::Plist():
	dwPluginThread(GetCurrentThreadId())
{
	bInit = false; // force initialize when opening the panel

	{
		PluginSettings settings(MainGuid, Info.SettingsControl);
		SortMode = settings.Get(0, L"SortMode", SM_UNSORTED); //SM_CUSTOM;
		StartPanelMode = settings.Get(0, L"StartPanelMode", 1) + L'0';
	}

	if (SortMode >= SM_PERSEC)
		SortMode &= (SM_PERSEC - 1); // Ахтунг!

	InitializePanelModes();

	pPerfThread = std::make_unique<PerfThread>();
}

int Plist::Menu(unsigned int Flags, const wchar_t* Title, const wchar_t* Bottom, const wchar_t* HelpTopic, const struct FarKey* BreakKeys, const FarMenuItem* Items, size_t ItemsNumber)
{
	return static_cast<int>((*Info.Menu)(&MainGuid, {}, -1, -1, 0, Flags, Title, Bottom, HelpTopic, BreakKeys, {}, Items, ItemsNumber));
}

void Plist::InitializePanelModes()
{
	static const wchar_t
		StatusWidth[] = L"0,8,0,5",
		StatusCols[] = L"N,S,D,T";

	// Default panel modes. Overridable from registry.
	// These modes are translated into PanelModesXX
	static const struct
	{
		const wchar_t* Cols;
		const wchar_t* Width;
	}
	DefaultModes[NPANELMODES]
	{
		/*0*/ {L"N,X15T,X16T,X17T,X18S", L"12,0,0,0,4"}, // I/O
		/*1*/ {L"N,XI,XB,XP,X0S,X6", L"0,6,2,2,3,9"}, // General info
		/*2*/ {L"N,N",L"0,0"},// Names only
		/*3*/ {L"N,XI,XB,XC,D,T",L"0,6,2,6,0,0"},    // Startup: PID/Date/Time
		/*4*/ {L"N,XI,X4,X6",L"0,6,9,9"}, // Memory (basic)
		/*5*/ {L"N,XI,X4,X6,X10,X12,X0,X1,X2",L"12,6,0,0,0,0,8,8,8"},     // Extended Memory/Time
		/*6*/ {L"N,ZD",L"12,0"}, // Descriptions
		/*7*/ {L"N,XP,X0S,X1S,X2S,X11S,X14S,X18S",L"0,2,3,2,2,3,4,3"}, // Dynamic Performance
		/*8*/ {L"N,XI,O",L"0,6,15"}, // Owners (not implemented)
		/*9*/ {L"N,XI,XT,X3,XG,XU",L"0,6,3,4,4,4"} // Resources
	},

	DefaultModesRemote[NPANELMODES]
	{
		/*0*/ {L"N,X15T,X16T,X17T,X18S", L"12,0,0,0,4"}, // I/O
		/*1*/ {L"N,XI,XB,XP,X0S,X6", L"0,6,2,2,3,9"}, // General info
		/*2*/ {L"N,N",L"0,0"},// Names only
		/*3*/ {L"N,XI,XB,XC,D,T",L"0,6,2,6,0,0"},    // Startup: PID/Date/Time
		/*4*/ {L"N,XI,X4,X6",L"0,6,9,9"}, // Memory (basic)
		/*5*/ {L"N,XI,X4,X6,X10,X12,X0,X1,X2",L"12,6,0,0,0,0,8,8,8"},     // Extended Memory/Time
		/*6*/ {L"N,ZD",L"12,0"}, // Descriptions
		/*7*/ {L"N,XP,X0S,X1S,X2S,X11S,X14S,X18S",L"0,2,3,2,2,3,4,3"}, // Dynamic Performance
		/*8*/ {L"N,XI,O",L"0,6,15"}, // Owners (not implemented)
		/*9*/ {L"N,XI,XT,X3",L"0,6,3,4"}
	};
	wchar_t name[20];
	PanelModesLocal[5].Flags |= PMFLAGS_FULLSCREEN;
	PanelModesRemote[5].Flags |= PMFLAGS_FULLSCREEN;

	PluginSettings settings(MainGuid, Info.SettingsControl);

	for (int iMode = 0; iMode < NPANELMODES; iMode++)
	{
		// Set pointers to our buffer
		PanelModesLocal[iMode].ColumnTypes = PanelModeBuffer + iMode * MAX_MODE_STR * 8;
		PanelModesLocal[iMode].ColumnWidths = PanelModeBuffer + iMode * MAX_MODE_STR * 8 + MAX_MODE_STR;
		PanelModesLocal[iMode].StatusColumnTypes = PanelModeBuffer + iMode * MAX_MODE_STR * 8 + MAX_MODE_STR * 2;
		PanelModesLocal[iMode].StatusColumnWidths = PanelModeBuffer + iMode * MAX_MODE_STR * 8 + MAX_MODE_STR * 3;
		PanelModesRemote[iMode].ColumnTypes = PanelModeBuffer + iMode * MAX_MODE_STR * 8 + MAX_MODE_STR * 4;
		PanelModesRemote[iMode].ColumnWidths = PanelModeBuffer + iMode * MAX_MODE_STR * 8 + MAX_MODE_STR * 5;
		PanelModesRemote[iMode].StatusColumnTypes = PanelModeBuffer + iMode * MAX_MODE_STR * 8 + MAX_MODE_STR * 6;
		PanelModesRemote[iMode].StatusColumnWidths = PanelModeBuffer + iMode * MAX_MODE_STR * 8 + MAX_MODE_STR * 7;

		FSF.sprintf(name, L"Mode%d", iMode);
		const auto Root = settings.OpenSubKey(0, name);

		auto FullScreen = settings.Get(Root, L"FullScreenLocal", iMode == 5? 1 : 0);
		if (FullScreen)
			PanelModesLocal[iMode].Flags |= PMFLAGS_FULLSCREEN;
		else
			PanelModesLocal[iMode].Flags &= ~PMFLAGS_FULLSCREEN;

		FullScreen = settings.Get(Root, L"FullScreenRemote", iMode == 5? 1 : 0);
		if (FullScreen)
			PanelModesRemote[iMode].Flags |= PMFLAGS_FULLSCREEN;
		else
			PanelModesRemote[iMode].Flags &= ~PMFLAGS_FULLSCREEN;

		settings.Get(Root, L"ColumnsLocal", ProcPanelModesLocal[iMode], std::size(ProcPanelModesLocal[iMode]), DefaultModes[iMode].Cols);
		settings.Get(Root, L"ColumnsRemote", ProcPanelModesRemote[iMode], std::size(ProcPanelModesRemote[iMode]), DefaultModesRemote[iMode].Cols);
		settings.Get(Root, L"WidthsLocal", const_cast<wchar_t*>(PanelModesLocal[iMode].ColumnWidths), MAX_MODE_STR - 1, DefaultModes[iMode].Width);
		settings.Get(Root, L"WidthsRemote", const_cast<wchar_t*>(PanelModesRemote[iMode].ColumnWidths), MAX_MODE_STR - 1, DefaultModesRemote[iMode].Width);
		//Status line is the same for all modes currently and cannot be changed.
		TranslateMode(StatusCols, const_cast<wchar_t*>(PanelModesLocal[iMode].StatusColumnTypes));
		TranslateMode(StatusCols, const_cast<wchar_t*>(PanelModesRemote[iMode].StatusColumnTypes));
		std::wcscpy(const_cast<wchar_t*>(PanelModesLocal[iMode].StatusColumnWidths), StatusWidth);
		std::wcscpy(const_cast<wchar_t*>(PanelModesRemote[iMode].StatusColumnWidths), StatusWidth);
	}
}

Plist::~Plist()
{
	pPerfThread.reset();
	DisconnectWMI();
}

void Plist::SavePanelModes()
{
	wchar_t name[20];
	PluginSettings settings(MainGuid, Info.SettingsControl);

	for (int iMode = 0; iMode < NPANELMODES; iMode++)
	{
		FSF.sprintf(name, L"Mode%d", iMode);
		const auto Root = settings.CreateSubKey(0, name);
		settings.Set(Root, L"ColumnsLocal", ProcPanelModesLocal[iMode]);
		settings.Set(Root, L"ColumnsRemote", ProcPanelModesRemote[iMode]);
		settings.Set(Root, L"WidthsLocal", PanelModesLocal[iMode].ColumnWidths);
		settings.Set(Root, L"WidthsRemote", PanelModesRemote[iMode].ColumnWidths);
		settings.Set(Root, L"FullScreenLocal", PanelModesLocal[iMode].Flags & PMFLAGS_FULLSCREEN);
		settings.Set(Root, L"FullScreenRemote", PanelModesRemote[iMode].Flags & PMFLAGS_FULLSCREEN);
	}
}

bool Plist::TranslateMode(const wchar_t* src, wchar_t* dest)
{
	if (!dest) return true;

	if (!src) { *dest = 0; return true; }

	int iCustomMode = L'0';
	bool bWasDesc = false;

	while (*src)
	{
		switch (*src)
		{
		case L'z':
		case L'Z':

			switch (*++src)
			{
			case L'p':case L'w':case L'd':case L'c':
			case L'P':case L'W':case L'D':case L'C':
				break;
			default:
				return false;
			}

			if (bWasDesc)
				return false;

			*dest++ = L'Z';
			bWasDesc = true;

			//*dest++ = iCustomMode++;
			// BUGBUG
			if (*src && *src != L',')
				return false;

			break;
		case L'x':
		case L'X':

			switch (*++src)
			{
			case L'P':case L'I':case L'C'://case L'W':
			case L'T':case L'B':case L'G':case L'U':
			case L'p':case L'i':case L'c'://case L'w':
			case L't':case L'b':case L'g':case L'u':
				src++;
				break;
			default:
				auto endptr = src;
				while (*endptr >= L'0' && *endptr <= L'9')
					++endptr;

				if (endptr == src)
					return false;

				src = endptr;
			}

			*dest++ = L'C';
			*dest++ = iCustomMode++;

			/*if(*src && *src!=L',')
			return false;*/
			while (*src && *src != L',') ++src;

			break;
		default:

			while (*src && *src != L',') *dest++ = *src++;
		}

		if (*src == L',') *dest++ = *src++;
	}

	*dest = 0;
	return true;
}

void Plist::GeneratePanelModes()
{
	for (int iMode = 0; iMode < NPANELMODES; iMode++)
	{
		TranslateMode(ProcPanelModesLocal[iMode], const_cast<wchar_t*>(PanelModesLocal[iMode].ColumnTypes));
		TranslateMode(ProcPanelModesRemote[iMode], const_cast<wchar_t*>(PanelModesRemote[iMode].ColumnTypes));
	}
}

int DescIDs[] = { MColFullPathname, MColumnTitle,
				  MTitleFileDesc, MCommandLine
};

#define CANBE_PERSEC(n) ((n)<3 || (n)==11 || (n)>=14)

static void GenerateTitles(wchar_t ProcPanelModes[][MAX_MODE_STR], PanelMode* PanelModes, wchar_t* Titles[][MAXCOLS])
{
	wchar_t buf[80];

	for (int i = 0; i < NPANELMODES; i++)
	{
		if (*ProcPanelModes[i])
		{
			std::wcsncpy(buf, ProcPanelModes[i], std::size(buf));
			int ii = 0;

			for (StrTok tok(buf, L","); tok; ++tok)
			{
				int id = 0;

				switch (*tok)
				{
				case L'n':
				case L'N':
					id = MColumnModule;
					break;
				case L'z':
				case L'Z':

					switch (tok[1])
					{
					case L'P': case L'p': id = MTitleFullPath; break;
					case L'W': case L'w': id = MColumnTitle; break;
					case L'D': case L'd': id = MTitleFileDesc; break;
					case L'C': case L'c': id = MCommandLine; break;
					}

					break;
				case L'x':
				case L'X':

					switch (tok[1])
					{
					case L'P': case L'p': id = MColumnPriority; break;
					case L'I': case L'i': id = MTitlePID; break;
					case L'C': case L'c': id = MColumnParentPID; break;
						//	case L'W': case L'w': id = ; break;
					case L'T': case L't': id = MTitleThreads; break;
					case L'B': case L'b': id = MColumnBits; break;
					case L'G': case L'g': id = MColumnGDI; break;
					case L'U': case L'u': id = MColumnUSER; break;
					default:
						const auto n = FSF.atoi(&tok[1]);

						if (n >= 0 && n < NCOUNTERS)
						{
							id = Counters[n].idCol;

							if (wcspbrk(&tok[1], L"Ss") && CANBE_PERSEC(n))
								++id;
						}

						break;
					}

					break;
				}

				Titles[i][ii++] = id? const_cast<wchar_t*>(GetMsg(id)) : nullptr;
			}

			PanelModes[i].ColumnTitles = Titles[i];
		}
		else
			PanelModes[i].ColumnTitles = {};
	}
}

// Obtains the current array of panel modes. Called from OpenPluginInfo.
PanelMode* Plist::PanelModes(size_t& nModes)
{
	static wchar_t* TitlesLocal[NPANELMODES][MAXCOLS];
	static wchar_t* TitlesRemote[NPANELMODES][MAXCOLS];
	static const wchar_t* OldMsg0;

	if (OldMsg0 != GetMsg(0)) // language changed
	{
		bInit = false;
		OldMsg0 = GetMsg(0);
	}

	if (!bInit)
	{
		GeneratePanelModes();
		GenerateTitles(ProcPanelModesLocal, PanelModesLocal, TitlesLocal);
		GenerateTitles(ProcPanelModesRemote, PanelModesRemote, TitlesRemote);
		bInit = true;
	}

	nModes = NPANELMODES;
	return HostName.empty()? PanelModesLocal : PanelModesRemote;
}

void Plist::GetOpenPanelInfo(OpenPanelInfo* Info)
{
	Info->StructSize = sizeof(*Info);
	Info->Flags = OPIF_ADDDOTS | OPIF_SHOWNAMESONLY | OPIF_USEATTRHIGHLIGHTING;
	Info->CurDir = L"";
	static wchar_t Title[100];

	if (HostName.empty())
		FSF.snprintf(Title, std::size(Title), L" %s ", GetMsg(MPlistPanel));
	else
		FSF.snprintf(Title, std::size(Title), L"%s: %s ", HostName.c_str(), GetMsg(MPlistPanel));

	Info->PanelTitle = Title;
	Info->PanelModesArray = PanelModes(Info->PanelModesNumber);
	Info->StartPanelMode = StartPanelMode;
	Info->StartSortMode = (OPENPANELINFO_SORTMODES)(SortMode >= SM_CUSTOM? SM_CTIME : SortMode); //SM_UNSORTED;


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

	static struct KeyBarLabel kbl[std::size(FKeys) / 3];
	static struct KeyBarTitles kbt = { std::size(kbl), kbl };

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

struct EnumWndData
{
	DWORD dwPID;
	HWND hWnd;
};

BOOL CALLBACK EnumWndProc(HWND hWnd, LPARAM lParam)
{
	DWORD dwProcID;
	GetWindowThreadProcessId(hWnd, &dwProcID);

	auto& Data = *reinterpret_cast<EnumWndData*>(lParam);

	if (dwProcID == Data.dwPID && !GetParent(hWnd))
	{
		const auto bVisible = IsWindowVisible(hWnd) || (IsIconic(hWnd) && !(GetWindowLongPtr(hWnd, GWL_STYLE) & WS_DISABLED));

		if (!Data.hWnd || bVisible)
			Data.hWnd = hWnd;

		return !bVisible;
	}

	return TRUE;
}

int Plist::GetFindData(PluginPanelItem*& pPanelItem, size_t& ItemsNumber, OPERATION_MODES OpMode)
{
	const std::scoped_lock l(*pPerfThread);

	if (!GetList(pPanelItem, ItemsNumber, *pPerfThread))
		return FALSE;

	PanelInfo pi = { sizeof(PanelInfo) };
	Info.PanelControl(this, FCTL_GETPANELINFO, 0, &pi);
	auto& ProcPanelModes = HostName.empty()? ProcPanelModesLocal : ProcPanelModesRemote;
	int cDescMode = 0;

	if (HostName.empty())
	{
		if (const auto p = wcschr(ProcPanelModes[pi.ViewMode], L'Z'))
			cDescMode = p[1];
	}

	for (size_t i = 0; i < ItemsNumber; i++)
	{
		PluginPanelItem& CurItem = pPanelItem[i];
		ProcessData& pdata = *((ProcessData*)CurItem.UserData.Data);
		// Make descriptions
		wchar_t Title[MAX_PATH];
		*Title = 0;
		const wchar_t* pDesc = L"";
		std::unique_ptr<char[]> Buffer;
		EnumWndData ewdata = { pdata.dwPID, {} };
		EnumWindows((WNDENUMPROC)EnumWndProc, (LPARAM)&ewdata);
		pdata.hwnd = ewdata.hWnd;

		if (cDescMode)
		{
			switch (cDescMode)
			{
			case L'p':
			case L'P':
				pDesc = pdata.FullPath.c_str();
				break;

			case L'w':
			case L'W':
				if (ewdata.hWnd)
					GetWindowText(ewdata.hWnd, Title, static_cast<int>(std::size(Title)));
				pDesc = Title;
				break;

			case L'd':
			case L'D':
				const wchar_t* pVersion;
				if (!GetVersionInfo(pdata.FullPath.c_str(), Buffer, pVersion, pDesc))
					pDesc = L"";
				break;

			case L'c':
			case L'C':
				pDesc = static_cast<ProcessData*>(CurItem.UserData.Data)->CommandLine.c_str();
				break;

			default:
				cDescMode = 0;
				break;
			}

			if (cDescMode)
			{
				CurItem.Description = new wchar_t[std::wcslen(pDesc) + 1];
				std::wcscpy(const_cast<wchar_t*>(CurItem.Description), pDesc);
			}
		}

		const auto pd = pPerfThread->GetProcessData(pdata.dwPID, (DWORD)CurItem.NumberOfLinks);

		const int DataOffset = sizeof(wchar_t*) * MAX_CUSTOM_COLS;
		int Widths[MAX_CUSTOM_COLS]{};
		unsigned uCustomColSize = 0;
		int nCols = 0;

		{
			const size_t Size = Info.PanelControl(this, FCTL_GETCOLUMNWIDTHS, 0, {});
			const auto ColumnWidths = std::make_unique<wchar_t[]>(Size);
			Info.PanelControl(this, FCTL_GETCOLUMNWIDTHS, Size, ColumnWidths.get());

			for (StrTok tokn(ColumnWidths.get(), L", "); tokn && nCols < MAX_CUSTOM_COLS; ++tokn)
			{
				uCustomColSize += (unsigned int)(((Widths[nCols++] = FSF.atoi(tokn)) + 1) * sizeof(wchar_t));
			}

		}

		if (nCols)
		{
			CurItem.CustomColumnData = (wchar_t**)new char[DataOffset + uCustomColSize];
			wchar_t* pData = (wchar_t*)((PCH)CurItem.CustomColumnData + DataOffset); // Start offset of column data aftet ptrs
			int nCustomCols;
			nCustomCols = nCols = 0;

			for (StrTok tok(ProcPanelModes[pi.ViewMode], L", "); tok; ++tok, ++nCols)
			{
				if (*tok == L'X' || *tok == L'x')  // Custom column
				{
					bool bCol = true;
					DWORD dwData = 0;
					int nBase = 10;
					int iCounter = -1;
					int nColWidth = Widths[nCols];

					if (nColWidth == 0)
						continue;

					bool bPerSec = false, bThousands = false;
					wchar_t c = tok[1];

					switch (c)
					{
					case L'P': case L'p': dwData = pdata.dwPrBase; break;
					case L'I': case L'i': dwData = pdata.dwPID;
						break;

					case L'C': case L'c': dwData = pdata.dwParentPID;
						break;

						//	case L'W': case L'w': dwData = hwnd; nBase = 16; break;
					case L'T': case L't': dwData = (DWORD)CurItem.NumberOfLinks; break;
					case L'B': case L'b': dwData = pdata.Bitness; break;

					case L'G': case L'g': if (pd) dwData = pd->dwGDIObjects; break;

					case L'U': case L'u': if (pd) dwData = pd->dwUSERObjects; break;

					default:

						if (c < L'0' || c>L'9')
							bCol = false;
						else
						{
							iCounter = FSF.atoi(&tok[1]);

							if (wcspbrk(&tok[1], L"Ss") && CANBE_PERSEC(iCounter))
								bPerSec = true;

							if (wcspbrk(&tok[1], L"Tt"))
								bThousands = true;
						}
					}

					if (!bCol)
						continue;

					((wchar_t**)(CurItem.CustomColumnData))[nCustomCols] = pData;
					int nBufSize = std::max(nColWidth + 1, 16);  // to provide space for itoa
					std::vector<wchar_t> buf(nBufSize);

					if (c >= L'A') // Not a performance counter
						FSF.itoa(dwData, buf.data(), nBase);
					else if (pd && iCounter >= 0)     // Format performance counters
					{
						if (iCounter < 3 && !bPerSec) // first 3 are date/time
							std::wcsncpy(buf.data(), PrintTime(pd->qwCounters[iCounter], false), nBufSize);
						else
							ui64toa_width(bPerSec? pd->qwResults[iCounter] : pd->qwCounters[iCounter],
								buf.data(), nColWidth, bThousands);
					}
					else
						buf[0] = L'\0';

					int nVisibleDigits = static_cast<int>(buf.size());

					if (nVisibleDigits > nColWidth) nVisibleDigits = nColWidth;

					wmemset(pData, L' ', nColWidth - nVisibleDigits);
					pData += nColWidth - nVisibleDigits;
					std::wcsncpy(pData, buf.data(), nVisibleDigits + 1);
					pData += nVisibleDigits + 1;

					if (++nCustomCols >= MAX_CUSTOM_COLS/* || ...>=MAX_CUSTOM_COL_SIZE*/)
						break;
				}
			}

			CurItem.CustomColumnNumber = /*Modes[pi.ViewMode].*/nCustomCols;
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
		delete[] item.CustomColumnData;
		delete[] item.FileName;
		delete[] item.AlternateFileName;
	}

	delete PanelItem;
}

#define CODE_STR(Code) \
	{ Code, L ## # Code }

static auto window_style(HWND Hwnd)
{
	const auto Style = GetWindowLongPtr(Hwnd, GWL_STYLE);

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

int Plist::GetFiles(PluginPanelItem* PanelItem, size_t ItemsNumber, int Move, const wchar_t** DestPath, OPERATION_MODES OpMode, options& Opt)
{
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
			ProcessPerfData* ppd = pPerfThread->GetProcessData(PData.dwPID, (DWORD)CurItem.NumberOfLinks);

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

		PrintToFile(InfoFile.get(), 0xFEFF);
		wchar_t AppType[100];
		FSF.sprintf(AppType, L", %d%s", pdata->Bitness, GetMsg(MBits));

		wchar_t ModuleName[MAX_PATH];
		std::wcscpy(ModuleName, CurItem.FileName);
		PrintToFile(InfoFile.get(), L"%s %s%s\n", PrintTitle(MTitleModule), ModuleName, AppType);

		if (!pdata->FullPath.empty())
		{
			PrintToFile(InfoFile.get(), L"%s %s\n", PrintTitle(MTitleFullPath), pdata->FullPath.c_str());
			PrintVersionInfo(InfoFile.get(), pdata->FullPath.c_str());
		}

		PrintToFile(InfoFile.get(), L"%s %d\n", PrintTitle(MTitlePID), pdata->dwPID);
		PrintToFile(InfoFile.get(), L"%s ", PrintTitle(MTitleParentPID));

		{
			const std::scoped_lock l(*pPerfThread);
			ProcessPerfData* pParentData = pPerfThread->GetProcessData(pdata->dwParentPID, 0);
			const auto pName = pdata->dwParentPID && pParentData? pParentData->ProcessName.c_str() : nullptr;
			PrintToFile(InfoFile.get(), pName? L"%u  (%s)\n" : L"%u\n", pdata->dwParentPID, pName);
		}

		PrintToFile(InfoFile.get(), L"%s %d\n", PrintTitle(MTitlePriority), pdata->dwPrBase);
		PrintToFile(InfoFile.get(), L"%s %u\n", PrintTitle(MTitleThreads), CurItem.NumberOfLinks);

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
				PrintToFile(InfoFile.get(), L"\n%s %s %s\n", PrintTitle(MTitleStarted), DateText, TimeText);
			}
			else
			{
				PrintToFile(InfoFile.get(), L"\n%s %s\n", PrintTitle(MTitleStarted), TimeText);
			}

			//PrintToFile(InfoFile,L"%s %s\n",PrintTitle(MTitleUptime),PrintNTUptime(CurItem.UserData));
			FileTimeToText(CurFileTime, CurItem.CreationTime, TimeText);
			PrintToFile(InfoFile.get(), L"%s %s\n", PrintTitle(MTitleUptime), TimeText);
		}

		if (HostName.empty()) // local only
		{
			if (!pdata->CommandLine.empty())
			{
				PrintToFile(InfoFile.get(), L"\n%s:\n%s\n", GetMsg(MCommandLine), pdata->CommandLine.c_str());
			}

			DebugToken token;

			if (const auto Process = handle(OpenProcessForced(&token, PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | READ_CONTROL, pdata->dwPID)))
			{
				PrintNTCurDirAndEnv(InfoFile.get(), Process.get(), Opt.ExportEnvironment);

				const std::scoped_lock l(*pPerfThread);
				ProcessPerfData* pd = pPerfThread->GetProcessData(pdata->dwPID, (DWORD)CurItem.NumberOfLinks);

				if (pd)
				{
					if (pd->dwGDIObjects)
					{
						PrintToFile(InfoFile.get(), L"\n%s %u\n", PrintTitle(MGDIObjects), pd->dwGDIObjects);
					}

					if (pd->dwUSERObjects)
					{
						PrintToFile(InfoFile.get(), L"%s %u\n", PrintTitle(MUSERObjects), pd->dwUSERObjects);
					}
				}
			}
		}

		if (Opt.ExportPerformance)
			DumpNTCounters(InfoFile.get(), *pPerfThread, pdata->dwPID, (DWORD)CurItem.NumberOfLinks);

		if (HostName.empty() && pdata->hwnd)
		{
			wchar_t Title[MAX_PATH]; *Title = 0;
			GetWindowText(pdata->hwnd, Title, static_cast<int>(std::size(Title)));
			PrintToFile(InfoFile.get(), L"\n%s %s\n", PrintTitle(MTitleWindow), Title);
			PrintToFile(InfoFile.get(), L"%-22s %p\n", L"HWND:", pdata->hwnd);

			const auto& [Style, StyleStr] = window_style(pdata->hwnd);
			PrintToFile(InfoFile.get(), L"%-22s %08X %s\n", PrintTitle(MTitleStyle), Style, StyleStr.c_str());
			const auto& [ExStyle, ExStyleStr] = window_ex_style(pdata->hwnd);
			PrintToFile(InfoFile.get(), L"%-22s %08X %s\n", PrintTitle(MTitleExtStyle), ExStyle, ExStyleStr.c_str());
		}

		if (HostName.empty() && Opt.ExportModuleInfo && pdata->dwPID != 8)
		{
			PrintToFile(InfoFile.get(), L"\n%s\n%s%s\n", GetMsg(MTitleModules), GetMsg(MColBaseSize),
				Opt.ExportModuleVersion? GetMsg(MColPathVerDesc) : GetMsg(MColPathVerDescNotShown));

			PrintModules(InfoFile.get(), pdata->dwPID, Opt);
		}

		if (HostName.empty() && (Opt.ExportHandles | Opt.ExportHandlesUnnamed) && pdata->dwPID /*&& pdata->dwPID!=8*/)
			PrintHandleInfo(pdata->dwPID, InfoFile.get(), Opt.ExportHandlesUnnamed? true : false, pPerfThread.get());
	}

	return true;
}


int Plist::DeleteFiles(PluginPanelItem* PanelItem, size_t ItemsNumber, OPERATION_MODES OpMode)
{
	if (ItemsNumber == 0)
		return false;

	if (!HostName.empty() && !Opt.EnableWMI)
	{
		//cannot kill remote process
		const wchar_t* MsgItems[] = { GetMsg(MCannotDeleteProc),GetMsg(MCannotKillRemote),GetMsg(MOk) };
		Message(FMSG_WARNING, {}, MsgItems, std::size(MsgItems));
		return false;
	}

	const wchar_t* MsgItems[]
	{
		GetMsg(MDeleteTitle),
		GetMsg(MDeleteProcesses),
		GetMsg(MDeleteDelete),
		GetMsg(MCancel),
	};

	{
		wchar_t Msg[512];

		if (ItemsNumber == 1)
		{
			FSF.sprintf(Msg, GetMsg(MDeleteProcess), PanelItem[0].FileName);
			MsgItems[1] = Msg;
		}

		if (Message(0, {}, MsgItems, std::size(MsgItems), 2) != 0)
			return FALSE;

		if (ItemsNumber > 1)
		{
			wchar_t Msg[512];
			FSF.sprintf(Msg, GetMsg(MDeleteNumberOfProcesses), ItemsNumber);
			MsgItems[1] = Msg;

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
			wchar_t Msg[512];
			FSF.sprintf(Msg, GetMsg(MCannotDelete), CurItem.FileName);
			const wchar_t* MsgItems[]
			{
				GetMsg(MDeleteTitle),
				Msg,
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
	if (Event == FE_IDLE && (pPerfThread->Updated() /*|| !pPerfThread&&GetTickCount()-LastUpdateTime>1000*/))
		Reread();

	if (Event == FE_CLOSE)
	{
		PanelInfo pi = { sizeof(PanelInfo) };
		Info.PanelControl(this, FCTL_GETPANELINFO, 0, &pi);
		PluginSettings settings(MainGuid, Info.SettingsControl);

		settings.Set(0, L"StartPanelMode", pi.ViewMode);
		settings.Set(0, L"SortMode", pi.SortMode == SM_CTIME? SortMode : pi.SortMode);
	}

	if (Event == FE_CHANGEVIEWMODE)
	{
		if (/*pPerfThread || */wcschr((wchar_t*)Param, L'Z') || wcschr((wchar_t*)Param, L'C'))
			Reread();
	}

	return FALSE;
}

void Plist::Reread()
{
	Info.PanelControl(this, FCTL_UPDATEPANEL, 1, {});
	Info.PanelControl(this, FCTL_REDRAWPANEL, 0, {});
	PanelInfo PInfo = { sizeof(PanelInfo) };
	Info.PanelControl(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, &PInfo);

	if (PInfo.PanelType == PTYPE_QVIEWPANEL)
	{
		Info.PanelControl(PANEL_PASSIVE, FCTL_UPDATEPANEL, 1, {});
		Info.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, {});
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

	Info.PanelControl(this, FCTL_INSERTCMDLINE, 0, const_cast<wchar_t*>(tmp));
	Info.PanelControl(this, FCTL_INSERTCMDLINE, 0, const_cast<wchar_t*>(L" "));
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
	const auto hScreen = Info.SaveScreen(0, 0, -1, -1);
	Message(0, {}, ConnectItems, 2, 0);

	if (pUser && *pUser)
	{
		static NETRESOURCE nr =
		{ RESOURCE_GLOBALNET, RESOURCETYPE_DISK, RESOURCEDISPLAYTYPE_SERVER,
		  RESOURCEUSAGE_CONTAINER, {}, {}, (wchar_t*)L"", {}
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

	auto pNewPerfThread = std::make_unique<PerfThread>(Machine.c_str(), pUser? pUser : nullptr, pUser? pPasw : nullptr);

	if (!pNewPerfThread->IsOK())
	{
		WinError();
		Info.RestoreScreen(hScreen);
	}
	else
	{
		Info.RestoreScreen(hScreen);
		pPerfThread = std::move(pNewPerfThread);
		DisconnectWMI();
		HostName = std::move(Machine);
		return true;
	}

	return false;
}

int Plist::ProcessKey(const INPUT_RECORD* Rec)
{
	if (Rec->EventType != KEY_EVENT)
		return FALSE;

	int Key = Rec->Event.KeyEvent.wVirtualKeyCode;
	unsigned int ControlState = Rec->Event.KeyEvent.dwControlKeyState;

	if ((ControlState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) && Key == L'R')
	{
		pPerfThread->SmartReread();
		return FALSE;
	}

	if (ControlState == 0 && Key == VK_RETURN)
	{
		//check for the command line; if it's not empty, don't process Enter
		if (Info.PanelControl(this, FCTL_GETCMDLINE, 0, {}) > 1)
			return false;

		PanelInfo PInfo = { sizeof(PanelInfo) };
		Info.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);

		if (PInfo.CurrentItem < PInfo.ItemsNumber)
		{
			const size_t Size = Info.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, {});
			const auto CurItem = make_malloc<PluginPanelItem>(Size);
			if (!CurItem)
				return false;

			FarGetPluginPanelItem gpi{ sizeof(FarGetPluginPanelItem), Size, CurItem.get() };
			Info.PanelControl(this, FCTL_GETPANELITEM, PInfo.CurrentItem, &gpi);

			if (!CurItem->UserData.Data)
				return false;

			const auto hWnd = static_cast<ProcessData*>(CurItem->UserData.Data)->hwnd;

			if (hWnd && (IsWindowVisible(hWnd) || (IsIconic(hWnd) && (GetWindowLongPtr(hWnd, GWL_STYLE) & WS_DISABLED) == 0)))
			{
#ifndef SPI_GETFOREGROUNDLOCKTIMEOUT
#define SPI_GETFOREGROUNDLOCKTIMEOUT        0x2000
#define SPI_SETFOREGROUNDLOCKTIMEOUT        0x2001
#endif
				// Allow SetForegroundWindow on Win98+.
				DWORD dwMs;
				// Remember the current value.
				BOOL bSPI = SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &dwMs, 0);

				if (bSPI) // Reset foreground lock timeout
					bSPI = SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, {}, 0);

				SetForegroundWindow(hWnd);

				if (bSPI) // Restore the old value
					SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)(SIZE_T)dwMs, 0);

				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);

				if (!GetWindowPlacement(hWnd, &wp) || wp.showCmd != SW_SHOWMAXIMIZED)
					ShowWindowAsync(hWnd, SW_RESTORE);
			}
		}

		return TRUE;
	}
	else if (ControlState == SHIFT_PRESSED && Key == VK_F3)
	{
		PanelInfo pi{ sizeof(PanelInfo) };
		Info.PanelControl(this, FCTL_GETPANELINFO, 0, &pi);

		size_t Size = Info.PanelControl(this, FCTL_GETPANELITEM, pi.CurrentItem, {});
		if (const auto PPI = make_malloc<PluginPanelItem>(Size))
		{
			FarGetPluginPanelItem gpi = { sizeof(FarGetPluginPanelItem), Size, PPI.get() };
			Info.PanelControl(this, FCTL_GETPANELITEM, pi.CurrentItem, &gpi);
			bool Exit = pi.CurrentItem >= pi.ItemsNumber || !lstrcmp(PPI->FileName, L"..");

			if (Exit)
				return TRUE;
		}
		else
			return FALSE; //???


		auto LocalOpt = Opt;

		PluginDialogBuilder Builder(Info, MainGuid, ConfigDialogGuid, MViewWithOptions, L"Config");
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
		Size = Info.PanelControl(this, FCTL_GETPANELITEM, pi.CurrentItem, {});
		if (const auto PPI = make_malloc<PluginPanelItem>(Size))
		{
			FarGetPluginPanelItem gpi{ sizeof(FarGetPluginPanelItem), Size, PPI.get() };
			Info.PanelControl(this, FCTL_GETPANELITEM, pi.CurrentItem, &gpi);

			if (GetFiles(PPI.get(), 1, 0, &lpFileName, OPM_VIEW | 0x10000, LocalOpt))
			{
				//TODO: viewer crashed on exit!
				Info.Viewer(FileName, PPI->FileName, 0, 0, -1, -1, VF_NONMODAL | VF_DELETEONCLOSE, CP_DEFAULT);
			}
		}
		else
			return FALSE; //???


		return TRUE;
	}
	else if (ControlState == 0 && Key == VK_F6)
	{
		{
			wchar_t Host[1024] = {};
			std::wcsncpy(Host, HostName.c_str(), std::size(Host));
			wchar_t Username[1024] = {};
			wchar_t Password[1024] = {};
			PluginDialogBuilder Builder(Info, MainGuid, ConfigDialogGuid, MSelectComputer, L"Contents"); // ConfigDialogGuid ???
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
						pPerfThread = std::make_unique<PerfThread>();
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
	else if (ControlState == SHIFT_PRESSED && Key == VK_F6)
	{
		// go to local host
		pPerfThread = {};
		DisconnectWMI();
		pPerfThread = std::make_unique<PerfThread>();
		HostName.clear();
		Reread();
		return TRUE;
	}

#if 0
	else if ((ControlState & (RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED)) && Key == VK_F6)
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
	else if (ControlState == SHIFT_PRESSED && (Key == VK_F1 || Key == VK_F2) && (HostName.empty() || Opt.EnableWMI))
	{
		//lower/raise priority class
		PanelInfo PInfo = { sizeof(PanelInfo) };
		Info.PanelControl(this, FCTL_GETPANELINFO, 0, &PInfo);

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
			const size_t Size = Info.PanelControl(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, 0, {});
			if (!Size)
				continue;

			const auto PPI = make_malloc<PluginPanelItem>(Size);
			if (!PPI)
				continue;

			FarGetPluginPanelItem gpi{ sizeof(FarGetPluginPanelItem), Size, PPI.get() };
			Info.PanelControl(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, i, &gpi);
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
	else if ((ControlState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) && Key == L'F')
	{
		PanelInfo pi = { sizeof(PanelInfo) };
		Info.PanelControl(this, FCTL_GETPANELINFO, 0, &pi);

		if (pi.CurrentItem < pi.ItemsNumber)
		{
			const size_t Size = Info.PanelControl(this, FCTL_GETPANELITEM, pi.CurrentItem, {});
			const auto PPI = make_malloc<PluginPanelItem>(Size);
			if (PPI)
			{
				FarGetPluginPanelItem gpi{ sizeof(FarGetPluginPanelItem), Size, PPI.get() };
				Info.PanelControl(this, FCTL_GETPANELITEM, pi.CurrentItem, &gpi);

				if (const auto pData = static_cast<ProcessData*>(PPI->UserData.Data))
					PutToCmdLine(pData->FullPath.c_str());
			}
			else
				return FALSE;

		}

		return TRUE;
	}
	else if ((ControlState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) && Key == VK_F12)
	{
		struct
		{
			unsigned id;
			unsigned mode;
		}
		StaticItems[]
		{
			{MSortByName,SM_NAME},
			{MSortByExt,SM_EXT},
			{MSortByTime,SM_MTIME},
			{MSortBySize,SM_SIZE},
			{MSortByUnsorted,SM_UNSORTED},
			{MSortByDescriptions,SM_DESCR},
			{MSortByOwner,SM_OWNER},
			//{MPageFileBytes,SM_COMPRESSEDSIZE},
			{MTitlePID,SM_PID},
			{MTitleParentPID,SM_PARENTPID},
			{MTitleThreads,SM_NUMLINKS},
			{MTitlePriority,SM_PRIOR},
			//{0,-1},
			//{MUseSortGroups,0},
			//{MShowSelectedFirst,-1}
		};

		int nMoreData = static_cast<int>(std::size(Counters) + 1);

		PanelInfo pi{ sizeof(PanelInfo) };
		Info.PanelControl(this, FCTL_GETPANELINFO, 0, &pi);
		const auto cIndicator = pi.Flags & PFLAGS_REVERSESORTORDER? L'-' : L'+';

		std::vector<FarMenuItem> Items(std::size(StaticItems) + nMoreData * 2);

		struct
		{
			BYTE  m;
			BYTE  a;
		}
		Flags[std::size(Counters) * 2 + std::size(StaticItems) + 1]{};

		for (size_t i = 0; i < std::size(StaticItems); i++)
			if (StaticItems[i].id == 0)
				Items[i].Flags |= MIF_SEPARATOR;
			else
			{
				Items[i].Text = GetMsg(StaticItems[i].id);
				Flags[i].m = StaticItems[i].mode;
				unsigned sm = pi.SortMode;

				if (sm == SM_CTIME)
					sm = SortMode;

				if (sm == StaticItems[i].mode)
				{
					Items[i].Flags |= MIF_CHECKED | cIndicator;
				}
			}

		auto nItems = std::size(StaticItems);

		Items[nItems++].Flags |= MIF_SEPARATOR;
		const PerfLib* pl = pPerfThread->GetPerfLib();

		for (size_t i = 0; i < std::size(Counters); i++)
			if (pl->dwCounterTitles[i])
			{
				Items[nItems].Text = GetMsg(Counters[i].idName);
				Flags[nItems].m = (BYTE)(SM_PERFCOUNTER + i);
				if (SM_PERFCOUNTER + i == SortMode)
				{
					Items[nItems].Flags |= MIF_CHECKED | cIndicator;
				}
				nItems++;

				if (CANBE_PERSEC(i))
				{
					wchar_t tmpStr[512];
					if (i < 3)
						FSF.sprintf(tmpStr, L"%% %s", GetMsg(Counters[i].idName));
					else
						FSF.sprintf(tmpStr, L"%s %s", GetMsg(Counters[i].idName), GetMsg(MperSec));

					Items[nItems].Text = wcsdup(tmpStr);
					Flags[nItems].m = (BYTE)((SM_PERFCOUNTER + i) | SM_PERSEC);
					Flags[nItems].a = 1;

					if (((SM_PERFCOUNTER + i) | SM_PERSEC) == SortMode)
					{
						Items[nItems].Flags |= MIF_CHECKED | cIndicator;
					}

					nItems++;
				}
			}

		// Show sort menu
		int rc = Menu(FMENU_AUTOHIGHLIGHT | FMENU_WRAPMODE, GetMsg(MSortBy), {}, {}, {}, Items.data(), nItems);

		if (rc != -1)
		{
			unsigned mode = Flags[rc].m;
			SortMode = static_cast<OPENPANELINFO_SORTMODES>(mode);

			if (mode >= SM_CUSTOM)
				mode = SM_CTIME;

			Info.PanelControl(this, FCTL_SETSORTMODE, mode, {});
			/*
			else if(rc==std::size(StaticItems)-2)
			Control(FCTL_SETSORTORDER, &items[rc].mode);
			else if(rc==std::size(StaticItems)-1)
			Control(FCTL_SETSORTMODE, &items[rc].mode);
			*/
		}


		while (--nItems > std::size(StaticItems))
			if (Flags[nItems].a)
				free(const_cast<wchar_t*>(Items[nItems].Text));

		return TRUE;
	}

	return FALSE;
}

wchar_t* Plist::PrintTitle(int MsgId)
{
	static wchar_t FullStr[256];
	wchar_t Str[256];
	FSF.snprintf(Str, std::size(Str), L"%s:", GetMsg(MsgId));
	FSF.snprintf(FullStr, std::size(FullStr), L"%-22s", Str);
	return FullStr;
}

void Plist::FileTimeToText(const FILETIME& CurFileTime, const FILETIME& SrcTime, wchar_t* TimeText)
{
	ULARGE_INTEGER Cur, Src;
	Cur.HighPart = CurFileTime.dwHighDateTime;
	Cur.LowPart = CurFileTime.dwLowDateTime;
	Src.HighPart = SrcTime.dwHighDateTime;
	Src.LowPart = SrcTime.dwLowDateTime;

	const auto Duration = Cur.QuadPart - Src.QuadPart;

	const auto
		TicksPerS = 10'000'000ull,
		TicksPerM = TicksPerS * 60,
		TicksPerH = TicksPerM * 60,
		TicksPerD = TicksPerH * 24;

	const auto
		Days    = Duration / TicksPerD,
		Hours   = Duration % TicksPerD / TicksPerH,
		Minutes = Duration % TicksPerD % TicksPerH / TicksPerM,
		Seconds = Duration % TicksPerD % TicksPerH % TicksPerM / TicksPerS;

	if (Days > 0)
		FSF.sprintf(TimeText, L"%d %02d:%02d:%02d", Days, Hours, Minutes, Seconds);
	else
		FSF.sprintf(TimeText, L"%02d:%02d:%02d", Hours, Minutes, Seconds);
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

	wchar_t BlockPath[64];
	FSF.sprintf(BlockPath, L"\\StringFileInfo\\%04X%04X\\", LOWORD(*Translation), HIWORD(*Translation));

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
		PrintToFile(InfoFile, L"%s %s\n", PrintTitle(MTitleFileVersion), pVersion);
	}

	if (pDesc)
	{
		PrintToFile(InfoFile, L"%s %s\n", PrintTitle(MTitleFileDesc), pDesc);
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

	return pWMI->Connect(HostName.c_str(), pPerfThread->GetUserName(), pPerfThread->GetPassword());
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
		PrintToFile(InfoFile, L"%s ", PrintTitle(MTitleUsername));

		if (!Domain.empty())
			PrintToFile(InfoFile, L"%s\\", Domain.c_str());

		if (!User.empty())
			PrintToFile(InfoFile, L"%s", User.c_str());

		if (!Sid.empty())
			PrintToFile(InfoFile, L" (%s)", Sid.c_str());

		PrintToFile(InfoFile, L'\n');
	}

	const auto nSession = pWMI->GetProcessSessionId(dwPid);

	if (nSession != -1)
		PrintToFile(InfoFile, L"%s %d\n", PrintTitle(MTitleSessionId), nSession);
}

int Plist::Compare(const PluginPanelItem* Item1, const PluginPanelItem* Item2, unsigned int Mode) const
{
	if (Mode != SM_CTIME || SortMode < SM_CUSTOM)
		return -2;

	ptrdiff_t diff;

	const auto& pd1 = *static_cast<const ProcessData*>(Item1->UserData.Data);
	const auto& pd2 = *static_cast<const ProcessData*>(Item2->UserData.Data);

	switch (SortMode)
	{
	case SM_PID:
		diff = pd1.dwPID - pd2.dwPID;
		break;

	case SM_PARENTPID:
		diff = pd1.dwParentPID - pd2.dwParentPID;
		break;

	case SM_PRIOR:
		diff = pd2.dwPrBase - pd1.dwPrBase;
		break;

	default:
		{
			const std::scoped_lock l(*pPerfThread);
			const auto data1 = pPerfThread->GetProcessData(pd1.dwPID, static_cast<DWORD>(Item1->NumberOfLinks));
			const auto data2 = pPerfThread->GetProcessData(pd2.dwPID, static_cast<DWORD>(Item2->NumberOfLinks));

			if (!data1)
				return data2? 1 : 0;

			if (!data2)
				return -1;

			bool bPerSec = false;
			unsigned smode = SortMode;

			if (smode >= SM_PERSEC)
			{
				bPerSec = true;
				smode &= ~SM_PERSEC;
			}

			const auto i = smode - SM_PERFCOUNTER;
			//if((DWORD)i >= (DWORD)NCOUNTERS){
			//  i=0; //????
			//}
			diff = bPerSec? data2->qwResults[i] - data1->qwResults[i] : data2->qwCounters[i] - data1->qwCounters[i];
		}
		break;
	}

	if (diff == 0)
		diff = reinterpret_cast<uintptr_t>(Item1->UserData.Data) - reinterpret_cast<uintptr_t>(Item2->UserData.Data); // unsorted

	return diff < 0? -1 : diff == 0? 0 : 1;
}

bool Plist::bInit;
PanelMode Plist::PanelModesLocal[], Plist::PanelModesRemote[];
wchar_t Plist::PanelModeBuffer[];
wchar_t Plist::ProcPanelModesLocal[NPANELMODES][MAX_MODE_STR], Plist::ProcPanelModesRemote[NPANELMODES][MAX_MODE_STR];
/*
bool Plist::PostUpdate()
{
PanelInfo pi = {sizeof(PanelInfo)};
if(!Control(FCTL_GETPANELINFO, &pi))
return false;

DWORD dwCtrlR = KEY_CTRL | L'R';
KeySequence ks = { 0, 1, &dwCtrlR };
(*Info.AdvControl)(Info.ModuleNumber, ACTL_POSTKEYSEQUENCE, &ks);
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
