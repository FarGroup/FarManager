#include "Proclist.hpp"
#include "perfthread.hpp"
#include "Proclng.hpp"
#include "guid.hpp"
#include <DlgBuilder.hpp>
#include <PluginSettings.hpp>

class StrTok
{
	private:
		LPCTSTR tok;
		LPTSTR  ptr;
		LPTSTR  buf;

	public:
		StrTok(LPCTSTR str, LPCTSTR token) : tok(token)
		{
			buf = wcsdup(str);
			ptr = wcstok(buf, token);
		}
		~StrTok() { free(buf); }

	public:
		operator wchar_t*() { return ptr; }
		void operator ++ () { ptr = _tcstok(NULL, tok); }
		operator bool() { return ptr!=NULL; }
};

ui64Table::ui64Table()
{
	unsigned __int64 n = 1;

	for (size_t i=0; i<ARRAYSIZE(Table); i++,n*=10)
		Table[i] = n;
}

unsigned __int64 ui64Table::tenpow(unsigned n)
{
	if (n>=ARRAYSIZE(Table))
		n = ARRAYSIZE(Table) - 1;

	return Table[n];
}

static void ui64toa_width(unsigned __int64 value, wchar_t* buf, unsigned width, bool bThousands)
{
	if (width < 1)
		return;

	const wchar_t* pSuffix = L"";
	unsigned uDivider = bThousands ? 1000 : 1024;

	if (width<=20)
	{
		if (value >= _ui64Table->tenpow(width))
		{
			value /= uDivider;
			pSuffix = L"K";
		}

		if (value >= _ui64Table->tenpow(width))
		{
			value /= uDivider;
			pSuffix = L"M";
		}
	}

	_ui64tot(value, buf, 10);
	lstrcat(buf,pSuffix);
}

Plist::Plist()
{
	pWMI = 0;
	LastUpdateTime=0;
	bInit = false; // force initialize when opening the panel
	*HostName = 0;
	pPerfThread = 0;
	dwPluginThread = GetCurrentThreadId();
	PluginSettings settings(MainGuid, Info.SettingsControl);
	SortMode = settings.Get(0,L"SortMode", SM_UNSORTED); //SM_CUSTOM;

	if (SortMode >= SM_PERSEC)
		SortMode &= (SM_PERSEC-1); // ÄÂ‚„≠£!

	StartPanelMode = settings.Get(0,L"StartPanelMode", 1)+L'0';
	InitializePanelModes();

	pPerfThread = new PerfThread(*this);
}

	int Plist::Menu(unsigned int Flags, const wchar_t *Title, const wchar_t *Bottom, wchar_t *HelpTopic, const struct FarKey *BreakKeys, FarMenuItem *Item, int ItemsNumber)
	{
		return (*Info.Menu)(&MainGuid, nullptr, -1, -1, 0, Flags, Title, Bottom, HelpTopic, BreakKeys, 0, Item, ItemsNumber);
	}

void Plist::InitializePanelModes()
{
	static const wchar_t
		StatusWidth[] = L"0,8,0,5",
		StatusCols[]    = L"N,S,D,T";

	// Default panel modes. Overridable from registry.
	// These modes are translated into PanelModesXX
	static const struct
	{
		const wchar_t *Cols;
		const wchar_t *Width;
	}
	DefaultModes[NPANELMODES] =
	{
		/*0*/ {L"N,X15T,X16T,X17T,X18S", L"12,0,0,0,4"}, // I/O
		/*1*/ {L"N,XI,XP,X0S,X6", L"0,4,2,3,9"}, // General info
		/*2*/ {L"N,N",L"0,0"},// Names only
		/*3*/ {L"N,XI,XC,D,T",L"0,4,4,0,0"},    // Startup: PID/Date/Time
		/*4*/ {L"N,XI,X4,X6",L"0,4,9,9"}, // Memory (basic)
		/*5*/ {L"N,XI,X4,X6,X10,X12,X0,X1,X2",L"12,4,0,0,0,0,8,8,8"},     // Extended Memory/Time
		/*6*/ {L"N,ZD",L"12,0"}, // Descriptions
		/*7*/ {L"N,XP,X0S,X1S,X2S,X11S,X14S,X18S",L"0,2,3,2,2,3,4,3"}, // Dynamic Performance
		/*8*/ {L"N,XI,O",L"0,5,15"}, // Owners (not implemented)
		/*9*/ {L"N,XI,XT,X3,XG,XU",L"0,4,3,4,4,4"} // Resources
	},

	DefaultModesRemote[NPANELMODES] =
	{
		/*0*/ {L"N,X15T,X16T,X17T,X18S", L"12,0,0,0,4"}, // I/O
		/*1*/ {L"N,XI,XP,X0S,X6", L"0,4,2,3,9"}, // General info
		/*2*/ {L"N,N",L"0,0"},// Names only
		/*3*/ {L"N,XI,XC,D,T",L"0,4,4,0,0"},    // Startup: PID/Date/Time
		/*4*/ {L"N,XI,X4,X6",L"0,4,9,9"}, // Memory (basic)
		/*5*/ {L"N,XI,X4,X6,X10,X12,X0,X1,X2",L"12,4,0,0,0,0,8,8,8"},     // Extended Memory/Time
		/*6*/ {L"N,ZD",L"12,0"}, // Descriptions
		/*7*/ {L"N,XP,X0S,X1S,X2S,X11S,X14S,X18S",L"0,2,3,2,2,3,4,3"}, // Dynamic Performance
		/*8*/ {L"N,XI,O",L"0,5,15"}, // Owners (not implemented)
		/*9*/ {L"N,XI,XT,X3",L"0,4,3,4"}
	};
#define MAXCOLS MAX_CUSTOM_COLS+4
	wchar_t name[20];
	PanelModesLocal[5].Flags |= PMFLAGS_FULLSCREEN;
	PanelModesRemote[5].Flags |= PMFLAGS_FULLSCREEN;

	PluginSettings settings(MainGuid, Info.SettingsControl);

	for (int iMode=0; iMode<NPANELMODES; iMode++)
	{
		// Set pointers to our buffer
		PanelModesLocal[iMode].ColumnTypes = PanelModeBuffer + iMode*MAX_MODE_STR*8;
		PanelModesLocal[iMode].ColumnWidths = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR;
		PanelModesLocal[iMode].StatusColumnTypes = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR*2;
		PanelModesLocal[iMode].StatusColumnWidths = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR*3;
		PanelModesRemote[iMode].ColumnTypes = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR*4;
		PanelModesRemote[iMode].ColumnWidths = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR*5;
		PanelModesRemote[iMode].StatusColumnTypes = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR*6;
		PanelModesRemote[iMode].StatusColumnWidths = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR*7;

		FSF.sprintf(name, L"Mode%d", iMode);
		int Root=settings.OpenSubKey(0,name);

		int FullScreen;
		FullScreen=settings.Get(Root, L"FullScreenLocal", iMode==5 ? 1 : 0);
		if (FullScreen)
			PanelModesLocal[iMode].Flags |= PMFLAGS_FULLSCREEN;
		else
			PanelModesLocal[iMode].Flags &= ~PMFLAGS_FULLSCREEN;

		FullScreen=settings.Get(Root, L"FullScreenRemote", iMode==5 ? 1 : 0);
		if (FullScreen)
			PanelModesRemote[iMode].Flags |= PMFLAGS_FULLSCREEN;
		else
			PanelModesRemote[iMode].Flags &= ~PMFLAGS_FULLSCREEN;

		settings.Get(Root, L"ColumnsLocal", ProcPanelModesLocal[iMode], ARRAYSIZE(ProcPanelModesLocal[iMode]), DefaultModes[iMode].Cols);
		settings.Get(Root, L"ColumnsRemote", ProcPanelModesRemote[iMode], ARRAYSIZE(ProcPanelModesRemote[iMode]), DefaultModesRemote[iMode].Cols);
		settings.Get(Root, L"WidthsLocal", (LPTSTR)PanelModesLocal[iMode].ColumnWidths, MAX_MODE_STR-1, DefaultModes[iMode].Width);
		settings.Get(Root, L"WidthsRemote", (LPTSTR)PanelModesRemote[iMode].ColumnWidths, MAX_MODE_STR-1, DefaultModesRemote[iMode].Width);
		//Status line is the same for all modes currently and cannot be changed.
		TranslateMode(StatusCols, (LPTSTR)PanelModesLocal[iMode].StatusColumnTypes);
		TranslateMode(StatusCols, (LPTSTR)PanelModesRemote[iMode].StatusColumnTypes);
		lstrcpy((wchar_t*)PanelModesLocal[iMode].StatusColumnWidths, StatusWidth);
		lstrcpy((wchar_t*)PanelModesRemote[iMode].StatusColumnWidths, StatusWidth);

	}
}

Plist::~Plist()
{
	delete pPerfThread;
	DisconnectWMI();
}

void Plist::SavePanelModes()
{
	wchar_t name[20];
	PluginSettings settings(MainGuid, Info.SettingsControl);

	for (int iMode=0; iMode<NPANELMODES; iMode++)
	{
		FSF.sprintf(name, L"Mode%d", iMode);
		int Root=settings.CreateSubKey(0,name);
		settings.Set(Root,L"ColumnsLocal", ProcPanelModesLocal[iMode]);
		settings.Set(Root,L"ColumnsRemote", ProcPanelModesRemote[iMode]);
		settings.Set(Root,L"WidthsLocal", PanelModesLocal[iMode].ColumnWidths);
		settings.Set(Root,L"WidthsRemote", PanelModesRemote[iMode].ColumnWidths);
		settings.Set(Root,L"FullScreenLocal", PanelModesLocal[iMode].Flags&PMFLAGS_FULLSCREEN);
		settings.Set(Root,L"FullScreenRemote", PanelModesRemote[iMode].Flags&PMFLAGS_FULLSCREEN);
	}
}

bool Plist::TranslateMode(LPCTSTR src, LPTSTR dest)
{
	if (!dest) return true;

	if (!src) { *dest=0; return true; }

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
				if (*src && *src!=L',')
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
						wchar_t* endptr;

						if (*src<L'0' || *src>L'9')
							return false;

						_tcstol(src,&endptr,10);

						if (endptr==src) return false;

						src = endptr;
				}

				*dest++ = L'C';
				*dest++ = iCustomMode++;

				/*if(*src && *src!=L',')
				return false;*/
				while (*src && *src!=L',') ++src;

				break;
			default:

				while (*src && *src!=L',') *dest++ = *src++;
		}

		if (*src==L',') *dest++ = *src++;
	}

	*dest = 0;
	return true;
}

void Plist::GeneratePanelModes()
{
	for (int iMode=0; iMode<NPANELMODES; iMode++)
	{
		TranslateMode(ProcPanelModesLocal[iMode], (LPTSTR)PanelModesLocal[iMode].ColumnTypes);
		TranslateMode(ProcPanelModesRemote[iMode], (LPTSTR)PanelModesRemote[iMode].ColumnTypes);
		/*TranslateMode(ProcPanelStModesNT[iMode], PanelModesNT[iMode].StatusColumnTypes);
		if(!NT) TranslateMode(ProcPanelStModes9x[iMode], PanelModes9x[iMode].StatusColumnTypes);*/
	}
}

int DescIDs[] = { MColFullPathname, MColumnTitle,
                  MTitleFileDesc, MCommandLine
                };

#define CANBE_PERSEC(n) ((n)<3 || (n)==11 || (n)>=14)

static void GenerateTitles(wchar_t ProcPanelModes[][MAX_MODE_STR],PanelMode* PanelModes, wchar_t* Titles[][MAXCOLS])
{
	wchar_t buf[80];

	for (int i=0; i<NPANELMODES; i++)
	{
		if (*ProcPanelModes[i])
		{
			lstrcpyn(buf, ProcPanelModes[i], ARRAYSIZE(buf));
			int ii=0;

			for (StrTok tok(buf,L","); tok; ++tok)
			{
				int id=0;

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
								int n = FSF.atoi(&tok[1]);

								if (n>=0 && n<NCOUNTERS)
								{
									id = Counters[n].idCol;

									if (_tcspbrk(&tok[1], L"Ss") && CANBE_PERSEC(n))
										++id;
								}

								break;
						}

						break;
				}

				Titles[i][ii++] = id ? (wchar_t*)GetMsg(id) : 0;
			}

			PanelModes[i].ColumnTitles = Titles[i];
		}
		else
			PanelModes[i].ColumnTitles = 0;
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
	return *HostName ? PanelModesRemote : PanelModesLocal;
}

void Plist::GetOpenPanelInfo(OpenPanelInfo *Info)
{
	Info->StructSize = sizeof(*Info);
	Info->Flags = OPIF_ADDDOTS|OPIF_SHOWNAMESONLY|OPIF_USEATTRHIGHLIGHTING;
	Info->CurDir = L"";
	static wchar_t Title[100];

	if (*HostName)
		FSF.snprintf(Title,ARRAYSIZE(Title),L"%s: %s ", HostName, GetMsg(MPlistPanel));
	else
		FSF.snprintf(Title,ARRAYSIZE(Title),L" %s ",GetMsg(MPlistPanel));

	Info->PanelTitle=Title;
	Info->PanelModesArray = PanelModes(Info->PanelModesNumber);
	Info->StartPanelMode = StartPanelMode;
	Info->StartSortMode = (OPENPANELINFO_SORTMODES)(SortMode >= SM_CUSTOM ? SM_CTIME : SortMode); //SM_UNSORTED;


	static WORD FKeys[]=
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

	static struct KeyBarLabel kbl[ARRAYSIZE(FKeys)/3];
	static struct KeyBarTitles kbt = {ARRAYSIZE(kbl), kbl};

	for (size_t j=0,i=0; i < ARRAYSIZE(FKeys); i+=3, ++j)
	{
		kbl[j].Key.VirtualKeyCode = FKeys[i];
		kbl[j].Key.ControlKeyState = FKeys[i+1];

		if (FKeys[i+2])
		{
			kbl[j].Text = kbl[j].LongText = GetMsg(FKeys[i+2]);
		}
		else
		{
			kbl[j].Text = kbl[j].LongText = L"";
		}
	}

	Info->KeyBar=&kbt;
}

struct EnumWndData {DWORD dwPID; HWND hWnd; };

BOOL CALLBACK EnumWndProc(HWND hWnd,LPARAM lParam)
{
	DWORD dwProcID;
	GetWindowThreadProcessId(hWnd, &dwProcID);

	if (dwProcID==((EnumWndData*)lParam)->dwPID && GetParent(hWnd)==NULL)
	{
		BOOL bVisible = IsWindowVisible(hWnd) ||
		                (IsIconic(hWnd) && (GetWindowLong(hWnd,GWL_STYLE) & WS_DISABLED)==0);

		if (!((EnumWndData*)lParam)->hWnd || bVisible)
			((EnumWndData*)lParam)->hWnd = hWnd;

		return !bVisible;
	}

	return TRUE;
}

int Plist::GetFindData(PluginPanelItem*& pPanelItem,size_t &ItemsNumber,OPERATION_MODES OpMode)
{
	Lock l(pPerfThread);
	int RetCode = GetList(pPanelItem,ItemsNumber,*pPerfThread);

	if (!RetCode) return FALSE;

	PanelInfo pi = {sizeof(PanelInfo)};
	Info.PanelControl(this,FCTL_GETPANELINFO,0,(void*)&pi);
	wchar_t(* ProcPanelModes)[MAX_MODE_STR] = *HostName ? ProcPanelModesRemote : ProcPanelModesLocal;
	int cDescMode = 0;

	if (!*HostName)
	{
		wchar_t* p = _tcschr(ProcPanelModes[pi.ViewMode], L'Z');

		if (p)
			cDescMode = p[1];
	}

	for (size_t i = 0; i < ItemsNumber; i++)
	{
		PluginPanelItem &CurItem = pPanelItem[i];
		ProcessData & pdata = *((ProcessData *)CurItem.UserData);
		// Make descriptions
		wchar_t Title[MAX_PATH];
		*Title=0;
		wchar_t* pDesc=(wchar_t *)L"";
		LPBYTE pBuf=0;
		EnumWndData ewdata = { pdata.dwPID, 0 };
		EnumWindows((WNDENUMPROC)EnumWndProc, (LPARAM)&ewdata);
		pdata.hwnd = ewdata.hWnd;

		if (cDescMode)
		{
			switch (cDescMode)
			{
				case L'p':
				case L'P':

					if (*pdata.FullPath)
						pDesc = pdata.FullPath;

					break;
				case L'w':
				case L'W':

					if (ewdata.hWnd)
						GetWindowText(ewdata.hWnd, Title, ARRAYSIZE(Title));

					pDesc = Title;
					break;
				case L'd':
				case L'D':
					wchar_t *pVersion;

					if (!Plist::GetVersionInfo(pdata.FullPath, pBuf, pVersion, pDesc))
						pDesc = (wchar_t *)L"";

					break;
				case L'c':
				case L'C':
					pDesc = ((ProcessDataNT *)CurItem.UserData)->CommandLine;

					break;
				default:
					cDescMode = 0;
			}

			if (cDescMode)
			{
				CurItem.Description = new wchar_t[lstrlen(pDesc)+1];
				lstrcpy((wchar_t*)CurItem.Description, pDesc);
			}

			delete[] pBuf;
		}

		ProcessPerfData* pd = 0;

		if (pPerfThread)
			pd = pPerfThread->GetProcessData(pdata.dwPID, CurItem.NumberOfLinks);

		const int DataOffset = sizeof(wchar_t*) * MAX_CUSTOM_COLS;
		int Widths[MAX_CUSTOM_COLS];
		memset(Widths, 0, sizeof(Widths));
		unsigned uCustomColSize = 0;
		int nCols=0;
		size_t Size=Info.PanelControl(this,FCTL_GETCOLUMNWIDTHS,0,NULL);
		wchar_t *ColumnWidths=new wchar_t[Size];
		Info.PanelControl(this,FCTL_GETCOLUMNWIDTHS,static_cast<int>(Size),(void*)ColumnWidths);

		for (StrTok tokn(ColumnWidths, L", "); (bool)tokn && nCols<MAX_CUSTOM_COLS; ++tokn)
		{
			uCustomColSize += (unsigned int)(((Widths[nCols++] = FSF.atoi(tokn)) + 1)*sizeof(wchar_t));
		}

		delete[] ColumnWidths;

		if (nCols)
		{
			CurItem.CustomColumnData = (wchar_t**)new char[DataOffset + uCustomColSize];
			wchar_t* pData = (wchar_t*)((PCH)CurItem.CustomColumnData+DataOffset); // Start offset of column data aftet ptrs
			int nCustomCols;
			nCustomCols = nCols = 0;

			for (StrTok tok(ProcPanelModes[pi.ViewMode], L", "); tok; ++tok, ++nCols)
			{
				if (*tok==L'X' || *tok==L'x')  // Custom column
				{
					bool bCol = true;
					DWORD dwData = 0;
					int nBase = 10;
					int iCounter = -1;
					int nColWidth = Widths[nCols];

					if (nColWidth==0)
						continue;

					bool bPerSec = false, bThousands = false;
					wchar_t c = tok[1];

					switch (c)
					{
						case L'P': case L'p': dwData = pdata.dwPrBase; break;
						case L'I': case L'i': dwData = pdata.dwPID;

							if (!pPerfThread) nBase = 16;

							break;
						case L'C': case L'c': dwData = pdata.dwParentPID;

							if (!pPerfThread) nBase = 16;

							break;
					//	case L'W': case L'w': dwData = hwnd; nBase = 16; break;
						case L'T': case L't': dwData = CurItem.NumberOfLinks; break;
						case L'B': case L'b': dwData = pdata.uAppType; break;

						case L'G': case L'g': if (pd) dwData = pd->dwGDIObjects; break;

						case L'U': case L'u': if (pd) dwData = pd->dwUSERObjects; break;

						default:

							if (c<L'0' || c>L'9')
								bCol = false;
							else
							{
								iCounter = FSF.atoi(&tok[1]);

								if (_tcspbrk(&tok[1], L"Ss") && CANBE_PERSEC(iCounter))
									bPerSec = true;

								if (_tcspbrk(&tok[1], L"Tt"))
									bThousands = true;
							}
					}

					if (!bCol)
						continue;

					((wchar_t**)(CurItem.CustomColumnData))[nCustomCols] = pData;
					int nBufSize = Max(nColWidth+1, 16);  // to provide space for itoa
					Array<wchar_t> buf(nBufSize);

					if ( c >= L'A') // Not a performance counter
						FSF.itoa(dwData, buf, nBase);
					else if (pd && iCounter>=0)     // Format performance counters
					{
						if (iCounter<3 && !bPerSec) // first 3 are date/time
							lstrcpyn(buf, PrintTime(pd->qwCounters[iCounter], false), nBufSize);
						else
							ui64toa_width(bPerSec ? pd->qwResults[iCounter] : pd->qwCounters[iCounter],
							              buf, nColWidth, bThousands);
					}
					else
						*buf=0;

					int nVisibleDigits = lstrlen(buf);

					if (nVisibleDigits > nColWidth) nVisibleDigits = nColWidth;

					wmemset(pData, L' ', nColWidth-nVisibleDigits);
					pData += nColWidth-nVisibleDigits;
					lstrcpyn(pData,buf,nVisibleDigits+1);
					pData += nVisibleDigits+1;

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

void Plist::FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber)
{
	for (int i=0; i<ItemsNumber; i++)
	{
		PluginPanelItem& item = PanelItem[i];
		delete[] item.Description;
		delete[] item.Owner;
		delete[] item.CustomColumnData;
		delete(ProcessData *)item.UserData;
		delete[] item.FileName;
		delete[] item.AlternateFileName;
	}

	delete PanelItem;
}

int Plist::GetFiles(PluginPanelItem *PanelItem,int ItemsNumber, int Move,WCONST WTYPE DestPath,OPERATION_MODES OpMode, _Opt& Opt)
{
	static const wchar_t invalid_chars[] = L":*?\\/\"<>;|";

	if (ItemsNumber==0)
		return 0;

	for (int I=0; I<ItemsNumber; I++)
	{
		PluginPanelItem &CurItem = PanelItem[I];
		ProcessData *pdata=(ProcessData *)CurItem.UserData;
		ProcessDataNT PData;

		if (!pdata)
		{
			PData.dwPID = FSF.atoi(CurItem.AlternateFileName);
			ProcessPerfData* ppd = pPerfThread->GetProcessData(PData.dwPID, CurItem.NumberOfLinks);

			if (ppd && GetPDataNT(PData, *ppd))
				pdata = &PData;

			if (!pdata)
				return 0;
		}

		// may be 0 if called from FindFile
		wchar_t FileName[MAX_PATH];
		lstrcpyn(FileName, WDEREF DestPath, ARRAYSIZE(FileName));

		if (!(OpMode&0x10000))
		{
			FSF.AddEndSlash(FileName);
			_tcsncat(FileName,CurItem.FileName,ARRAYSIZE(FileName)-lstrlen(FileName)-1);
			_tcsncat(FileName,L".txt",ARRAYSIZE(FileName)-lstrlen(FileName)-1);
		}

		// Replace "invalid" chars by underscores
		wchar_t* pname = _tcsrchr(FileName, L'\\');
		if (!pname) pname = FileName; else pname++;

		for (; *pname; pname++)
		{
			if (_tcschr(invalid_chars, *pname) || *pname < L' ')
				*pname = L'_';
		}

		HANDLE InfoFile = CreateFile(FileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);

		if (InfoFile==INVALID_HANDLE_VALUE)
			return 0;

		fputc(0xFEFF, InfoFile);
		wchar_t AppType[100];

		if (!pPerfThread && pdata->uAppType)
			FSF.sprintf(AppType,L", %d%s",pdata->uAppType,GetMsg(MBits));
		else
			*AppType=0;

		wchar_t ModuleName[MAX_PATH];
		lstrcpy(ModuleName, CurItem.FileName);
		fprintf(InfoFile,L"%s %s%s\n",PrintTitle(MTitleModule),ModuleName,AppType);

		if (pdata && *pdata->FullPath)
		{
			fprintf(InfoFile,L"%s %s\n",PrintTitle(MTitleFullPath),OUT_STRING(pdata->FullPath));
			PrintVersionInfo(InfoFile, pdata->FullPath);
		}

		fprintf(InfoFile, pPerfThread ? L"%s %d\n":L"%s %08X\n",PrintTitle(MTitlePID),pdata->dwPID);
		fprintf(InfoFile, L"%s ", PrintTitle(MTitleParentPID));

		if (pPerfThread)
		{
			Lock l(pPerfThread);
			ProcessPerfData* pParentData = pPerfThread->GetProcessData(pdata->dwParentPID, 0);
			wchar_t* pName = pdata->dwParentPID && pParentData ? pParentData->ProcessName : 0;
			fprintf(InfoFile, pName ? L"%u  (%s)\n" : L"%u\n",pdata->dwParentPID, pName);
		}
		else
			fprintf(InfoFile, L"%08X\n",pdata->dwParentPID);

		fprintf(InfoFile,L"%s %d\n",PrintTitle(MTitlePriority),pdata->dwPrBase);
		fprintf(InfoFile,L"%s %u\n",PrintTitle(MTitleThreads),CurItem.NumberOfLinks);

		PrintOwnerInfo(InfoFile, pdata->dwPID);

		// Time information

		if (CurItem.CreationTime.dwLowDateTime || CurItem.CreationTime.dwHighDateTime)
		{
			FILETIME CurFileTime;
			GetSystemTimeAsFileTime(&CurFileTime);
			SYSTEMTIME Current,Compare;
			GetLocalTime(&Current);
			FileTimeToSystemTime(&CurItem.CreationTime,&Compare);
			SystemTimeToTzSpecificLocalTime(NULL,&Compare,&Compare);
			wchar_t DateText[MAX_DATETIME],TimeText[MAX_DATETIME];
			ConvertDate(CurItem.CreationTime,DateText,TimeText);

			if (Current.wYear!=Compare.wYear || Current.wMonth!=Compare.wMonth || Current.wDay!=Compare.wDay)
			{
				fprintf(InfoFile,L"\n%s %s %s\n",PrintTitle(MTitleStarted),DateText,TimeText);
			}
			else
			{
				fprintf(InfoFile,L"\n%s %s\n",PrintTitle(MTitleStarted),TimeText);
			}

			//fprintf(InfoFile,L"%s %s\n",PrintTitle(MTitleUptime),PrintNTUptime((void*)CurItem.UserData));
			FileTimeToText(&CurFileTime,&CurItem.CreationTime,TimeText);
			fprintf(InfoFile,L"%s %s\n",PrintTitle(MTitleUptime),TimeText);
		}

		HANDLE hProcess = 0;

		if (!*HostName) // local only
		{
			if (*((ProcessDataNT*)pdata)->CommandLine)
			{
				fprintf(InfoFile, L"\n%s:\n%s\n", GetMsg(MCommandLine), OUT_STRING(((ProcessDataNT*)pdata)->CommandLine));
			}

			DebugToken token;
			hProcess = OpenProcessForced(&token, PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|READ_CONTROL,pdata->dwPID);

			if (hProcess)
			{
				PrintNTCurDirAndEnv(InfoFile, hProcess, Opt.ExportEnvironment);
				CloseHandle(hProcess);
			}

			if (hProcess)
			{
				Lock l(pPerfThread);
				ProcessPerfData* pd = pPerfThread->GetProcessData(pdata->dwPID, CurItem.NumberOfLinks);

				if (pd)
				{
					if (pd->dwGDIObjects)
					{
						fprintf(InfoFile,L"\n%s %u\n",PrintTitle(MGDIObjects), pd->dwGDIObjects);
					}

					if (pd->dwUSERObjects)
					{
						fprintf(InfoFile,L"%s %u\n",PrintTitle(MUSERObjects), pd->dwUSERObjects);
					}
				}
			}
		}// NT && !*HostName

		if (Opt.ExportPerformance && pPerfThread)
			DumpNTCounters(InfoFile, *pPerfThread, pdata->dwPID, CurItem.NumberOfLinks);

		if (!*HostName && pdata->hwnd)
		{
			wchar_t Title[MAX_PATH]; *Title=0;
			GetWindowText(pdata->hwnd, Title, ARRAYSIZE(Title));
			fprintf(InfoFile,L"\n%s %s\n",PrintTitle(MTitleWindow), Title);
			fprintf(InfoFile,L"%-22s %p\n",L"HWND:",pdata->hwnd);
			LONG Style=0,ExtStyle=0;

			if (pdata->hwnd!=NULL)
			{
				Style=GetWindowLong(pdata->hwnd,GWL_STYLE);
				ExtStyle=GetWindowLong(pdata->hwnd,GWL_EXSTYLE);
			}

			static const int Styles[]=
			{
				WS_POPUP,WS_CHILD,WS_MINIMIZE,WS_VISIBLE,WS_DISABLED,
				WS_CLIPSIBLINGS,WS_CLIPCHILDREN,WS_MAXIMIZE,WS_BORDER,WS_DLGFRAME,
				WS_VSCROLL,WS_HSCROLL,WS_SYSMENU,WS_THICKFRAME,WS_MINIMIZEBOX,
				WS_MAXIMIZEBOX
			};
			static wchar_t const * const StrStyles[]=
			{
				L"WS_POPUP",L"WS_CHILD",L"WS_MINIMIZE",L"WS_VISIBLE",
				L"WS_DISABLED",L"WS_CLIPSIBLINGS",L"WS_CLIPCHILDREN",
				L"WS_MAXIMIZE",L"WS_BORDER",L"WS_DLGFRAME",L"WS_VSCROLL",
				L"WS_HSCROLL",L"WS_SYSMENU",L"WS_THICKFRAME",
				L"WS_MINIMIZEBOX",L"WS_MAXIMIZEBOX"
			};
			static const int ExtStyles[]=
			{
				WS_EX_DLGMODALFRAME,WS_EX_NOPARENTNOTIFY,WS_EX_TOPMOST,
				WS_EX_ACCEPTFILES,WS_EX_TRANSPARENT,WS_EX_MDICHILD,
				WS_EX_TOOLWINDOW,WS_EX_WINDOWEDGE,WS_EX_CLIENTEDGE,WS_EX_CONTEXTHELP,
				WS_EX_RIGHT,WS_EX_RTLREADING,WS_EX_LEFTSCROLLBAR,WS_EX_CONTROLPARENT,
				WS_EX_STATICEDGE,WS_EX_APPWINDOW,
				0x00080000, 0x00100000L, 0x00400000L, 0x08000000L
				/*WS_EX_LAYERED,WS_EX_NOINHERITLAYOUT,
				  WS_EX_LAYOUTRTL,WS_EX_NOACTIVATE*/
			};
			static wchar_t const * const StrExtStyles[]=
			{
				L"WS_EX_DLGMODALFRAME",L"WS_EX_NOPARENTNOTIFY",L"WS_EX_TOPMOST",
				L"WS_EX_ACCEPTFILES",L"WS_EX_TRANSPARENT",L"WS_EX_MDICHILD",
				L"WS_EX_TOOLWINDOW",L"WS_EX_WINDOWEDGE",L"WS_EX_CLIENTEDGE",
				L"WS_EX_CONTEXTHELP",L"WS_EX_RIGHT",L"WS_EX_RTLREADING",
				L"WS_EX_LEFTSCROLLBAR",L"WS_EX_CONTROLPARENT",
				L"WS_EX_STATICEDGE",L"WS_EX_APPWINDOW",L"WS_EX_LAYERED",
				L"WS_EX_NOINHERITLAYOUT",L"WS_EX_LAYOUTRTL",L"WS_EX_NOACTIVATE"
			};
			wchar_t StyleStr[1024], ExtStyleStr[1024];
			*StyleStr = *ExtStyleStr=0;
			size_t i;

			for (i=0; i<ARRAYSIZE(Styles); i++)
				if (Style & Styles[i])
				{
					lstrcat(StyleStr, L" ");
					lstrcat(StyleStr, StrStyles[i]);
				}

			for (i=0; i<ARRAYSIZE(ExtStyles); i++)
				if (Style & ExtStyles[i])
				{
					lstrcat(ExtStyleStr, L" ");
					lstrcat(ExtStyleStr, StrExtStyles[i]);
				}

			fprintf(InfoFile,L"%-22s %08X %s\n",PrintTitle(MTitleStyle),Style,StyleStr);
			fprintf(InfoFile,L"%-22s %08X %s\n",PrintTitle(MTitleExtStyle),ExtStyle,ExtStyleStr);
		}

		if (!*HostName && Opt.ExportModuleInfo && pdata->dwPID!=8)
		{
			fprintf(InfoFile,L"\n%s\n%s%s\n",  GetMsg(MTitleModules), GetMsg(MColBaseSize),
			        Opt.ExportModuleVersion ? GetMsg(MColPathVerDesc) : GetMsg(MColPathVerDescNotShown));

			PrintModules(InfoFile, pdata->dwPID, Opt);
		}

		if (!*HostName && (Opt.ExportHandles|Opt.ExportHandlesUnnamed) && pdata->dwPID /*&& pdata->dwPID!=8*/)
			PrintHandleInfo(pdata->dwPID, InfoFile, Opt.ExportHandlesUnnamed?true:false, pPerfThread);

		CloseHandle(InfoFile);
	}

	return 1;
}


int Plist::DeleteFiles(PluginPanelItem *PanelItem,int ItemsNumber,OPERATION_MODES OpMode)
{
	if (ItemsNumber==0)
		return FALSE;

//    if(Info.AdvControl(Info.ModuleNumber, ACTL_GETPOLICIES, 0) & FFPOL_KILLTASK)
//        ;

	if (*HostName && !Opt.EnableWMI)
	{
		//cannot kill remote process
		const wchar_t *MsgItems[]={GetMsg(MCannotDeleteProc),GetMsg(MCannotKillRemote),GetMsg(MOk)};
		Message(FMSG_WARNING,NULL,MsgItems,ARRAYSIZE(MsgItems));
		return FALSE;
	}

	const wchar_t *MsgItems[]={GetMsg(MDeleteTitle),GetMsg(MDeleteProcesses),
	                         GetMsg(MDeleteDelete),GetMsg(MCancel)
	                        };

	wchar_t Msg[512];

	if (ItemsNumber==1)
	{
		FSF.sprintf(Msg,GetMsg(MDeleteProcess),PanelItem[0].FileName);
		MsgItems[1]=Msg;
	}

	if (Message(0,NULL,MsgItems,ARRAYSIZE(MsgItems),2)!=0)
		return FALSE;

	if (ItemsNumber>1)
	{
		wchar_t Msg[512];
		FSF.sprintf(Msg,GetMsg(MDeleteNumberOfProcesses),ItemsNumber);
		MsgItems[1]=Msg;

		if (Message(FMSG_WARNING,NULL,MsgItems,ARRAYSIZE(MsgItems),2)!=0)
			return FALSE;
	}

	for (int I=0; I<ItemsNumber; I++)
	{
		PluginPanelItem& CurItem = PanelItem[I];
		ProcessData *pdata=(ProcessData *)CurItem.UserData;
		BOOL Success;
		int MsgId = 0;

		if (*HostName)  // try WMI
		{
			if (!ConnectWMI())
			{
				WmiError();
				Success = FALSE;
				break;
			}

			Success = FALSE;

			switch (pWMI->TerminateProcess(pdata->dwPID))
			{
				case -1:
					WmiError();
					continue;
				case 0: Success = TRUE; break;
				case 2: MsgId = MTAccessDenied; break;
				case 3: MsgId = MTInsufficientPrivilege; break;
				default: MsgId = MTUnknownFailure; break;
			}
		}
		else
		{
			Success = KillProcess(pdata->dwPID,pdata->hwnd);
		}


		if (!Success)
		{
			wchar_t Msg[512];
			FSF.sprintf(Msg,GetMsg(MCannotDelete),CurItem.FileName);
			const wchar_t *MsgItems[]={GetMsg(MDeleteTitle),Msg, 0, GetMsg(MOk)};
			int nItems = ARRAYSIZE(MsgItems);

			if (MsgId)
				MsgItems[2] = GetMsg(MsgId);
			else
			{
				MsgItems[2] = MsgItems[3];
				nItems--;
			}

			Message(FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,nItems);
			return FALSE;
		}
	}

	if (pPerfThread)
		pPerfThread->SmartReread();

	return TRUE;
}


int Plist::ProcessEvent(int Event,void *Param)
{
	if (Event==FE_IDLE && (pPerfThread && pPerfThread->Updated() /*|| !pPerfThread&&GetTickCount()-LastUpdateTime>1000*/))
		Reread();

	if (Event==FE_CLOSE)
	{
		PanelInfo pi = {sizeof(PanelInfo)};
		Info.PanelControl(this,FCTL_GETPANELINFO,0,(void*)&pi);
		PluginSettings settings(MainGuid, Info.SettingsControl);

		settings.Set(0,L"StartPanelMode", pi.ViewMode);
		settings.Set(0,L"SortMode", pi.SortMode==SM_CTIME ? SortMode : pi.SortMode);
	}

	if (Event==FE_CHANGEVIEWMODE)
	{
		if (/*pPerfThread || */_tcschr((wchar_t*)Param,L'Z') || _tcschr((wchar_t*)Param,L'C'))
			Reread();
	}

	return FALSE;
}

void Plist::Reread()
{
	Info.PanelControl(this,FCTL_UPDATEPANEL,1,NULL);
	Info.PanelControl(this,FCTL_REDRAWPANEL,0,NULL);
	PanelInfo PInfo = {sizeof(PanelInfo)};
	Info.PanelControl(PANEL_PASSIVE, FCTL_GETPANELINFO,0,(void*)&PInfo);

	if (PInfo.PanelType==PTYPE_QVIEWPANEL)
	{
		Info.PanelControl(PANEL_PASSIVE, FCTL_UPDATEPANEL,1,NULL);
		Info.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL,0,NULL);
	}
}

void Plist::PutToCmdLine(wchar_t* tmp)
{
	unsigned l = lstrlen(tmp);
	wchar_t* tmp1 = 0;

	if (_tcscspn(tmp,L" &^")!=l)
	{
		tmp1 = new wchar_t[l+3];
		memcpy(tmp1+1,tmp,l*sizeof(wchar_t));
		tmp1[0] = tmp1[l+1] = L'\"';
		tmp1[l+2] = 0;
		tmp = tmp1;
	}

	Info.PanelControl(this,FCTL_INSERTCMDLINE,0,(void*)tmp);
	Info.PanelControl(this,FCTL_INSERTCMDLINE,0,(void*)L" ");
	delete[] tmp1;
}

bool Plist::Connect(LPCTSTR pMachine, LPCTSTR pUser, LPCTSTR pPasw)
{
	wchar_t Machine[ARRAYSIZE(HostName)];
	lstrcpyn(Machine, pMachine, ARRAYSIZE(Machine));

	// Add "\\" if missing
	if (!NORM_M_PREFIX(Machine))
	{
		memmove(Machine+2, Machine, (lstrlen(Machine)+1)*sizeof(wchar_t));
		Machine[0] = L'\\';
		Machine[1] = L'\\';
	}

	//Try to connect...
	LPCTSTR ConnectItems[] = {L"",GetMsg(MConnect)};
	HANDLE hScreen = Info.SaveScreen(0,0,-1,-1);
	Message(0,0,ConnectItems,2,0);

	if (pUser && *pUser)
	{
		static NETRESOURCE nr =
		    { RESOURCE_GLOBALNET, RESOURCETYPE_DISK, RESOURCEDISPLAYTYPE_SERVER,
		      RESOURCEUSAGE_CONTAINER, NULL, NULL, (wchar_t *)L"", NULL
		    };
		nr.lpRemoteName = Machine;
		DWORD dwErr = WNetAddConnection2(&nr,pPasw,pUser,0);

		if (dwErr==ERROR_SESSION_CREDENTIAL_CONFLICT)
		{
			WNetCancelConnection2(Machine, 0, 0);
			dwErr = WNetAddConnection2(&nr,pPasw,pUser,0);
		}

		if (dwErr!=NO_ERROR)
		{
			SetLastError(dwErr);
			WinError();
			return false;
		}
	}

	PerfThread* pNewPerfThread = new PerfThread(*this, Machine, pUser?pUser:0, pUser?pPasw:0);

	if (!pNewPerfThread->IsOK())
	{
		WinError();
		Info.RestoreScreen(hScreen);
		delete pNewPerfThread;
	}
	else
	{
		Info.RestoreScreen(hScreen);
		delete pPerfThread;
		DisconnectWMI();
		pPerfThread = pNewPerfThread;
		lstrcpy(HostName, Machine);
		return true;
	}

	return false;
}

int Plist::ProcessKey(const INPUT_RECORD *Rec)
{
	if (!(Rec->EventType == KEY_EVENT || Rec->EventType == FARMACRO_KEY_EVENT))
		return FALSE;

	int Key=Rec->Event.KeyEvent.wVirtualKeyCode;
	unsigned int ControlState=Rec->Event.KeyEvent.dwControlKeyState;

	if ((ControlState&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)) && Key==L'R')
	{
		if (pPerfThread)
			pPerfThread->SmartReread();

		return FALSE;
	}

	if (ControlState==0 && Key==VK_RETURN)
	{
		//check for the command line; if it's not empty, don't process Enter
		if (Info.PanelControl(this,FCTL_GETCMDLINE,0,NULL)>1)
			return FALSE;

		PanelInfo PInfo = {sizeof(PanelInfo)};
		Info.PanelControl(this,FCTL_GETPANELINFO,0,(void*)&PInfo);

		if (PInfo.CurrentItem < PInfo.ItemsNumber)
		{
			size_t Size=Info.PanelControl(this,FCTL_GETPANELITEM,static_cast<int>(PInfo.CurrentItem),0);
			PluginPanelItem* CurItem=static_cast<PluginPanelItem*>(malloc(Size));
			if (CurItem)
			{
				FarGetPluginPanelItem gpi={Size, CurItem};
				Info.PanelControl(this,FCTL_GETPANELITEM,static_cast<int>(PInfo.CurrentItem),&gpi);

				if (!CurItem->UserData)
				{
					free(CurItem);
					return FALSE;
				}
			}
			else
				return FALSE;

			HWND hWnd = ((ProcessData *)CurItem->UserData)->hwnd;
			free(CurItem);


			if (hWnd!=NULL && (IsWindowVisible(hWnd) || (IsIconic(hWnd) && (GetWindowLong(hWnd,GWL_STYLE) & WS_DISABLED)==0)) )
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
					bSPI = SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, 0, 0);

				SetForegroundWindow(hWnd);

				if (bSPI) // Restore the old value
					SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)(SIZE_T)dwMs, 0);

				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);

				if (!GetWindowPlacement(hWnd,&wp) || wp.showCmd!=SW_SHOWMAXIMIZED)
					ShowWindowAsync(hWnd,SW_RESTORE);
			}
		}

		return TRUE;
	}
	else if (ControlState==SHIFT_PRESSED && Key==VK_F3)
	{
		PanelInfo pi = {sizeof(PanelInfo)};
		Info.PanelControl(this,FCTL_GETPANELINFO,0,(void*)&pi);

		size_t Size=Info.PanelControl(this,FCTL_GETPANELITEM,static_cast<int>(pi.CurrentItem),0);
		PluginPanelItem* PPI=static_cast<PluginPanelItem*>(malloc(Size));
		if (PPI)
		{
			FarGetPluginPanelItem gpi={Size, PPI};
			Info.PanelControl(this,FCTL_GETPANELITEM,static_cast<int>(pi.CurrentItem),&gpi);
			bool Exit=pi.CurrentItem >= pi.ItemsNumber || !lstrcmp(PPI->FileName,L"..");
			free(PPI);

			if (Exit)
				return TRUE;
		}
		else
			return FALSE; //???


		_Opt LocalOpt = Opt;

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
		FSF.MkTemp(FileName, ARRAYSIZE(FileName), L"prc");
		WCONST wchar_t *lpFileName=FileName;
		Size=Info.PanelControl(this,FCTL_GETPANELITEM,static_cast<int>(pi.CurrentItem),0);
		PPI=(PluginPanelItem*)new char[Size];
		if (PPI)
		{
			FarGetPluginPanelItem gpi={Size, PPI};
			Info.PanelControl(this,FCTL_GETPANELITEM,static_cast<int>(pi.CurrentItem),&gpi);

			if (GetFiles(PPI, 1, 0, WADDR lpFileName, OPM_VIEW|0x10000, LocalOpt))
			{
				//TODO: viewer crashed on exit!
				Info.Viewer(FileName,PPI->FileName, 0,0,-1,-1, VF_NONMODAL|VF_DELETEONCLOSE,CP_AUTODETECT);
			}
			free(PPI);
		}
		else
			return FALSE; //???


		return TRUE;
	}
	else if (ControlState==0 && Key==VK_F6)
	{
		{
			wchar_t Username[1024] = {};
			wchar_t Password[1024] = {};
			PluginDialogBuilder Builder(Info, MainGuid, ConfigDialogGuid, MSelectComputer, L"Contents"); // ConfigDialogGuid ???
			Builder.AddText(MComputer);
			Builder.AddEditField(HostName, ARRAYSIZE(HostName), 65, L"ProcessList.Computer");
			Builder.AddText(MEmptyForLocal);
			Builder.AddSeparator();
			Builder.StartColumns();
			Builder.AddText(MUsername);
			Builder.AddEditField(Username, ARRAYSIZE(Username), 18, L"ProcessList.Username");
			Builder.ColumnBreak();
			Builder.AddText(MPaswd);
			Builder.AddPasswordField(Password, ARRAYSIZE(Password), 18);
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
					if (*HostName==0 || !lstrcmp(HostName, L"\\\\"))
					{
						//go to local computer
						delete pPerfThread;
						DisconnectWMI();
						pPerfThread = new PerfThread(*this/*, GetPtr(_REF,7), GetPtr(_REF,8)*/);
						*HostName = 0;
						stop = true;
					}
					else if (Connect(HostName, Username, Password))
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
	else if (ControlState==SHIFT_PRESSED && Key==VK_F6)
	{
		// go to local host
		delete pPerfThread;
		DisconnectWMI();
		pPerfThread = new PerfThread(*this);
		*HostName = 0;
		Reread();
		return TRUE;
	}

#if 0
	else if ((ControlState&(RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED)) && Key==VK_F6)
	{
		if (!Opt.EnableWMI)
			return TRUE;

		const wchar_t *MsgItems[]={/*GetMsg(MAttachDebugger)*/L"Attach Debugger",/*GetMsg(MConfirmAttachDebugger)*/L"Do you want to attach debugger to this process?",GetMsg(MYes),GetMsg(MNo)};

		if (Message(0,NULL,MsgItems,ARRAYSIZE(MsgItems),2)!=0)
			return TRUE;

		PanelInfo pi = {sizeof(PanelInfo)};
		Control(FCTL_GETPANELINFO,&pi);
		PluginPanelItem& item = pi.PanelItems[pi.CurrentItem];

		if (!lstrcmp(item.FileName, L".."))
			return TRUE;

		int i;

		if (ConnectWMI() && pWMI && item.UserData)
			switch (i=pWMI->AttachDebuggerToProcess(((ProcessData*)item.UserData)->dwPID))
			{
				case -1: WmiError(); break;
				case 0: break;
				case 2: SetLastError(i); WinError(); break;
				default:
					wchar_t buf[80];
					FSF.sprintf(buf,L"Return code: %d", i);
					const wchar_t *MsgItems[]={/*GetMsg(MAttachDebugger)*/L"Attach Debugger",buf,GetMsg(MOk)};
					Message(FMSG_WARNING,0,MsgItems,ARRAYSIZE(MsgItems));
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
	else if (ControlState==SHIFT_PRESSED && (Key==VK_F1||Key==VK_F2) && (!*HostName||Opt.EnableWMI))
	{
		//lower/raise priority class
		PanelInfo PInfo = {sizeof(PanelInfo)};
		Info.PanelControl(this,FCTL_GETPANELINFO,0,(void*)&PInfo);

		if (PInfo.SelectedItemsNumber>1)
		{
			const wchar_t *MsgItems[]={GetMsg(MChangePriority),GetMsg(MConfirmChangePriority),GetMsg(MYes),GetMsg(MNo)};

			if (Message(0,NULL,MsgItems,ARRAYSIZE(MsgItems),2)!=0)
			{
				return TRUE;
			}
		}

		if (*HostName && Opt.EnableWMI && !ConnectWMI())
		{
			WmiError();
			return TRUE;
		}

		static const USHORT PrClasses[] =
		    { IDLE_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS,
		      NORMAL_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS,
		      HIGH_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS
		    };
		const int N = ARRAYSIZE(PrClasses);
		DebugToken token;

		for (size_t i=0; i<PInfo.SelectedItemsNumber; i++)
		{
			size_t Size = Info.PanelControl(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, 0, nullptr);
			if(Size)
			{
				PluginPanelItem* PPI = reinterpret_cast<PluginPanelItem*>(new char[Size]);
				if(PPI)
				{
					FarGetPluginPanelItem gpi = {Size, PPI};
					Info.PanelControl(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, static_cast<int>(i), &gpi);
					SetLastError(0);

					if (((ProcessData*)PPI->UserData)->dwPID)
					{
						if (!*HostName)
						{
							HANDLE hProcess=OpenProcessForced(&token, PROCESS_QUERY_INFORMATION|PROCESS_SET_INFORMATION,((ProcessData*)PPI->UserData)->dwPID);

							if (hProcess)
							{
								DWORD dwPriorityClass = GetPriorityClass(hProcess);

								if (dwPriorityClass==0)
									WinError();
								else
								{
									for (int i=0; i<N; i++)
										if (dwPriorityClass==PrClasses[i])
										{
											bool bChange = false;

											if (Key==VK_F1 && i>0)
											{
												i--; bChange = true;
											}
											else if (Key==VK_F2 && i<N-1)
											{
												i++;
												bChange = true;
											}

											if (bChange && !SetPriorityClass(hProcess, PrClasses[i]))
												WinError();

											//else
												//Item.Flags &= ~PPIF_SELECTED;
											break;
										}
								}

								CloseHandle(hProcess);
							}
							else
								WinError();
						}
						else if (pWMI)  //*HostName
						{
							DWORD dwPriorityClass = pWMI->GetProcessPriority(((ProcessData*)PPI->UserData)->dwPID);

							if (!dwPriorityClass)
							{
								WmiError();
								continue;
							}

							static const BYTE Pr[ARRAYSIZE(PrClasses)] = {4,6,8,10,13,24};

							for (int i=0; i<N; i++)
								if (dwPriorityClass==Pr[i])
								{
									bool bChange = false;

									if (Key==VK_F1 && i>0)
									{
										i--;
										bChange = true;
									}
									else if (Key==VK_F2 && i<N-1)
									{
										i++;
										bChange = true;
									}
									if (bChange && pWMI->SetProcessPriority(((ProcessData*)PPI->UserData)->dwPID, PrClasses[i])!=0)
										WmiError();
									break;
								}
						}
					} // if dwPID

					delete[] reinterpret_cast<char*>(PPI);
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
		if (pPerfThread)
			pPerfThread->SmartReread();

		Reread();
		return TRUE;
		/*  } else if ((ControlState&(RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED)) && (ControlState&SHIFT_PRESSED) && Key==VK_F9) {
		Config();
		return TRUE;*/
	}
	else if ((ControlState&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)) && Key==L'F')
	{
		PanelInfo pi = {sizeof(PanelInfo)};
		Info.PanelControl(this,FCTL_GETPANELINFO,0,(void*)&pi);

		if (pi.CurrentItem < pi.ItemsNumber)
		{
			size_t Size=Info.PanelControl(this,FCTL_GETPANELITEM,static_cast<int>(pi.CurrentItem),0);
			PluginPanelItem* PPI=static_cast<PluginPanelItem*>(malloc(Size));
			if (PPI)
			{
				FarGetPluginPanelItem gpi={Size, PPI};
				Info.PanelControl(this,FCTL_GETPANELITEM,static_cast<int>(pi.CurrentItem),&gpi);

				ProcessData* pData = (ProcessData *)PPI->UserData;
				if (pData)
					PutToCmdLine(pData->FullPath);

				free(PPI);
			}
			else
				return FALSE;

		}

		return TRUE;
	}
	else if ((ControlState&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)) && Key==VK_F12)
	{
		struct {int id, mode;} StaticItems[] =
		{
			{MSortByName,SM_NAME},
			{MSortByExt,SM_EXT},
			{MSortByTime,SM_MTIME},
			{MSortBySize,SM_SIZE},
			{MSortByUnsorted,SM_UNSORTED},
			{MSortByDescriptions,SM_DESCR},
			{MSortByOwner,SM_OWNER},
			//      {MPageFileBytes,SM_COMPRESSEDSIZE},
			{MTitlePID,SM_PID},
			{MTitleParentPID,SM_PARENTPID},
			{MTitleThreads,SM_NUMLINKS},
			{MTitlePriority,SM_PRIOR},
			//      {0,-1},
			//      {MUseSortGroups,0}, {MShowSelectedFirst,-1}
		};
#define NSTATICITEMS (ARRAYSIZE(StaticItems))
		int nMoreData = pPerfThread ? ARRAYSIZE(Counters) + 1 : 0;
		PanelInfo pi = {sizeof(PanelInfo)};
		Info.PanelControl(this,FCTL_GETPANELINFO,0,(void*)&pi);
		wchar_t cIndicator = pi.Flags&PFLAGS_REVERSESORTORDER ? L'-' : L'+';
		Array<FarMenuItem> Items(NSTATICITEMS + nMoreData*2);
		struct
		{
			BYTE  m;
			BYTE  a;
		} Flags[ARRAYSIZE(Counters)*2 + NSTATICITEMS + 1] = { {0} };
		DWORD i;

		for (i=0; i<NSTATICITEMS; i++)
			if (StaticItems[i].id==0)
				Items[i].Flags |= MIF_SEPARATOR;
			else
			{
				Items[i].Text = GetMsg(StaticItems[i].id);
				Flags[i].m = StaticItems[i].mode;
				int sm = pi.SortMode;

				if (sm==SM_CTIME)
					sm = SortMode;

				if (sm==StaticItems[i].mode)
				{
					if (cIndicator)
						Items[i].Flags |= MIF_CHECKED;
					else
						Items[i].Flags &= ~MIF_CHECKED;
				}
			}

		int nItems = NSTATICITEMS;

		if (pPerfThread)
		{
			Items[nItems++].Flags |= MIF_SEPARATOR;
			const PerfLib* pl = pPerfThread->GetPerfLib();

			for (i=0; i < ARRAYSIZE(Counters); i++)
				if (pl->dwCounterTitles[i])
				{
					Items[nItems].Text = GetMsg(Counters[i].idName);
					Flags[nItems].m = (BYTE)(SM_PERFCOUNTER+i);
					if (SM_PERFCOUNTER+i==SortMode)
					{
						if (cIndicator)
							Items[nItems].Flags |= MIF_CHECKED;
						else
							Items[nItems].Flags &= ~MIF_CHECKED;
					}
					nItems++;

					if (CANBE_PERSEC(i))
					{
						wchar_t tmpStr[512];
						if (i<3)
							FSF.sprintf(tmpStr, L"%% %s", GetMsg(Counters[i].idName));
						else
							FSF.sprintf(tmpStr, L"%s %s", GetMsg(Counters[i].idName), GetMsg(MperSec));

						Items[nItems].Text = wcsdup(tmpStr);
						Flags[nItems].m = (BYTE)((SM_PERFCOUNTER+i) | SM_PERSEC);
						Flags[nItems].a = 1;

						if (((SM_PERFCOUNTER+i) | SM_PERSEC) == SortMode)
						{
							if (cIndicator)
								Items[nItems].Flags |= MIF_CHECKED;
							else
								Items[nItems].Flags &= ~MIF_CHECKED;
						}

						nItems++;
					}
				}
		}

		// Show sort menu
		int rc= Menu(FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE, GetMsg(MSortBy), 0, 0, 0, Items, nItems);

		if (rc != -1)
		{
			unsigned mode;
			mode = Flags[rc].m;
			SortMode = (OPENPANELINFO_SORTMODES)mode;

			if (mode >= SM_CUSTOM)
				mode = SM_CTIME;

			Info.PanelControl(this,FCTL_SETSORTMODE,mode,NULL);
			/*
			else if(rc==NSTATICITEMS-2)
			Control(FCTL_SETSORTORDER, (void*)&items[rc].mode);
			else if(rc==NSTATICITEMS-1)
			Control(FCTL_SETSORTMODE, (void*)&items[rc].mode);
			*/
		}


		while (--nItems > (int)NSTATICITEMS)
			if (Flags[nItems].a) free((wchar_t*)Items[nItems].Text);

		return TRUE;
	}

	return FALSE;
}

wchar_t *Plist::PrintTitle(int MsgId)
{
	static wchar_t FullStr[256];
	wchar_t Str[256];
	FSF.snprintf(Str,ARRAYSIZE(Str),L"%s:",GetMsg(MsgId));
	FSF.snprintf(FullStr,ARRAYSIZE(FullStr),L"%-22s",Str);
	return FullStr;
}

void Plist::FileTimeToText(FILETIME *CurFileTime,FILETIME *SrcTime,wchar_t *TimeText)
{
	FILETIME Uptime;

	if (CurFileTime==NULL)
	{
		Uptime.dwHighDateTime=SrcTime->dwHighDateTime;
		Uptime.dwLowDateTime=SrcTime->dwLowDateTime;
	}
	else
	{
		Uptime.dwHighDateTime=CurFileTime->dwHighDateTime-SrcTime->dwHighDateTime;

		if (CurFileTime->dwLowDateTime<SrcTime->dwLowDateTime)
			Uptime.dwHighDateTime--;

		Uptime.dwLowDateTime=CurFileTime->dwLowDateTime-SrcTime->dwLowDateTime;
	}

	SYSTEMTIME st;

	if (FileTimeToSystemTime(&Uptime,&st))
	{
		int Days=st.wDay-1;

		for (int I=1; I<st.wMonth; I++)
		{
			static const int MonthDays[12]={31,29,31,30,31,30,31,31,30,31,30,31};
			Days+=MonthDays[I-1];
		}

		if (Days>0)
			FSF.sprintf(TimeText,L"%d %02d:%02d:%02d",Days,st.wHour,st.wMinute,st.wSecond);
		else
			FSF.sprintf(TimeText,L"%02d:%02d:%02d",st.wHour,st.wMinute,st.wSecond);
	}
	else // failed
		lstrcpy(TimeText, L"???");
}

bool Plist::GetVersionInfo(wchar_t* pFullPath, LPBYTE &pBuffer, wchar_t* &pVersion, wchar_t* &pDesc)
{
	static const wchar_t WSFI[] = L"StringFileInfo";

	if (!memcmp(pFullPath,L"\\??\\",4*sizeof(wchar_t)))
		pFullPath+=4;

	DWORD size = GetFileVersionInfoSizeW((wchar_t*)pFullPath, &size);

	if (!size) return false;

	pBuffer = new BYTE[size];
	GetFileVersionInfoW((wchar_t*)pFullPath, 0, size, pBuffer);

	//Find StringFileInfo
	DWORD ofs;

	for (ofs = 92; ofs < size; ofs += *(WORD*)(pBuffer+ofs))
		if ( !lstrcmpiW((wchar_t*)(pBuffer+ofs+6), WSFI) )
			break;

	if (ofs >= size)
	{
		delete[] pBuffer;
		return false;
	}

	wchar_t *langcode;

	langcode = (wchar_t*)(pBuffer + ofs + 42);

	wchar_t blockname[48];
	unsigned dsize;
	FSF.sprintf(blockname, L"\\%s\\%s\\FileVersion", WSFI, langcode);

	if (!VerQueryValue(pBuffer, blockname, (void**)&pVersion, &dsize))
		pVersion = 0;

	FSF.sprintf(blockname, L"\\%s\\%s\\FileDescription", WSFI, langcode);

	if (!VerQueryValue(pBuffer, blockname, (void**)&pDesc, &dsize))
		pDesc = 0;

	return true;
}

void Plist::PrintVersionInfo(HANDLE InfoFile, wchar_t* pFullPath)
{
	wchar_t   *pVersion, *pDesc;
	LPBYTE  pBuf;

	if (!GetVersionInfo(pFullPath, pBuf, pVersion, pDesc))
		return;

	if (pVersion)
	{
		fprintf(InfoFile,L"%s %s\n",PrintTitle(MTitleFileVersion), pVersion);
	}

	if (pDesc)
	{
		fprintf(InfoFile,L"%s %s\n",PrintTitle(MTitleFileDesc), pDesc);
	}

	delete[] pBuf;
}

bool Plist::ConnectWMI()
{
	if (!Opt.EnableWMI || dwPluginThread != GetCurrentThreadId())
		return false;

	if (!pWMI)
		pWMI = new WMIConnection();

	if (*pWMI)
		return true;

	return pWMI->Connect(HostName, pPerfThread->UserName, pPerfThread->Password);
}

void Plist::DisconnectWMI()
{
	if (pWMI) { delete pWMI; pWMI = 0; }
}

void Plist::PrintOwnerInfo(HANDLE InfoFile, DWORD dwPid)
{
	wchar_t User[MAX_USERNAME_LENGTH];
	wchar_t UserSid[MAX_USERNAME_LENGTH];
	wchar_t Domain[MAX_USERNAME_LENGTH];

	if (!Opt.EnableWMI || !pPerfThread->IsWMIConnected() || !ConnectWMI())
		return;

	if (!*pWMI) // exists, but counld not connect
		return;

	pWMI->GetProcessOwner(dwPid, User, Domain);
	pWMI->GetProcessUserSid(dwPid, UserSid);

	if (*User || *Domain || *UserSid)
	{
		fprintf(InfoFile,L"%s ", PrintTitle(MTitleUsername));

		if (*Domain)
			fprintf(InfoFile, L"%s\\", Domain);

		if (*User)
			fprintf(InfoFile, L"%s", User);

		if (*UserSid)
			fprintf(InfoFile,L" (%s)", UserSid);

		fputc(L'\n',InfoFile);
	}

	int nSession = pWMI->GetProcessSessionId(dwPid);

	if (nSession!=-1)
		fprintf(InfoFile,L"%s %d\n", PrintTitle(MTitleSessionId), nSession);
}

int Plist::Compare(const PluginPanelItem *Item1, const PluginPanelItem *Item2,
                   unsigned int Mode)
{
	if (Mode!=SM_CTIME || SortMode<SM_CUSTOM)
		return -2;

	int diff;

	switch (SortMode)
	{
		case SM_PID: diff = ((ProcessData*)Item1->UserData)->dwPID - ((ProcessData*)Item2->UserData)->dwPID;
			break;
		case SM_PARENTPID: diff = ((ProcessData*)Item1->UserData)->dwParentPID - ((ProcessData*)Item2->UserData)->dwParentPID;
			break;
		case SM_PRIOR: diff = ((ProcessData*)Item2->UserData)->dwPrBase - ((ProcessData*)Item1->UserData)->dwPrBase;
			break;
		default:
		{
			Lock l(pPerfThread);
			ProcessPerfData* data1 = pPerfThread->GetProcessData(
			                             ((ProcessData*)Item1->UserData)->dwPID,Item1->NumberOfLinks);
			ProcessPerfData* data2 = pPerfThread->GetProcessData(
			                             ((ProcessData*)Item2->UserData)->dwPID,Item2->NumberOfLinks);

			if (data1==0) return data2 ? 1 : 0;

			if (data2==0) return -1;

			bool bPerSec = false;
			unsigned smode = SortMode;

			if (smode >= SM_PERSEC)
			{
				bPerSec = true;
				smode &= ~SM_PERSEC;
			}

			int i = smode - SM_PERFCOUNTER;
			//if((DWORD)i >= (DWORD)NCOUNTERS){
			//  i=0; //????
			//}
			LONGLONG diff = bPerSec ? data2->qwResults[i] - data1->qwResults[i] : data2->qwCounters[i] - data1->qwCounters[i];
			return diff<0 ? -1 : diff==0 ? 0 : 1;
		}
	}

	if (diff==0)
		diff = (DWORD)Item1->UserData - (DWORD)Item2->UserData; // unsorted

	return diff<0 ? -1 : diff==0 ? 0 : 1;
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
Control(FCTL_REDRAWPANEL, NULL);
return true;
}
*/
void Plist::WmiError()
{
	if (pWMI)
	{
		HRESULT hr = pWMI->GetLastHResult();
		SetLastError(hr);
		WinError(hr<(HRESULT)0x80040000 ? 0 : (wchar_t *)L"wbemcomn");
	}
}
