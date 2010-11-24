#include "Proclist.hpp"
#include <farkeys.hpp>
#include "perfthread.hpp"
#include "Proclng.hpp"

class StrTok {
    LPCTSTR tok;
    LPTSTR  ptr;
    LPTSTR  buf;
public:
    StrTok(LPCTSTR str, LPCTSTR token) : tok (token) {
        buf = _tcsdup(str);
        ptr = _tcstok(buf, token);
    }
    operator TCHAR* () { return ptr; }
    void operator ++ () { ptr = _tcstok(NULL, tok); }
    operator bool() { return ptr!=NULL; }
    ~StrTok() { delete buf; }
};

ui64Table::ui64Table()
{
    unsigned __int64 n = 1;
    for(size_t i=0; i<ARRAYSIZE(Table); i++,n*=10)
        Table[i] = n;
}

unsigned __int64 ui64Table::tenpow(unsigned n)
{
    if(n>=ARRAYSIZE(Table))
        n = ARRAYSIZE(Table) - 1;
    return Table[n];
}

static void ui64toa_width(unsigned __int64 value, TCHAR* buf, unsigned width, bool bThousands)
{
    if(width < 1)
        return;

    const TCHAR* pSuffix = _T("");
    unsigned uDivider = bThousands ? 1000 : 1024;
    if(width<=20) {
        if(value >= _ui64Table->tenpow(width)) {
            value /= uDivider;
            pSuffix = _T("K");
        }
        if(value >= _ui64Table->tenpow(width)) {
            value /= uDivider;
            pSuffix = _T("M");
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
    SortMode = GetRegKey(0,_T("SortMode"), SM_UNSORTED); //SM_CUSTOM;
    if(SortMode >= SM_PERSEC)
       SortMode &= (SM_PERSEC-1); // ÄÂ‚„≠£!
    StartPanelMode = GetRegKey(0,_T("StartPanelMode"), 1)+'0';

    InitializePanelModes();

    if(NT)
        pPerfThread = new PerfThread(*this);
}

void Plist::InitializePanelModes()
{
    static const TCHAR StatusWidth9x[] = _T("0,8,0,5"),
                       StatusWidthNT[] = _T("0,8,0,5"),
                       StatusCols[] = _T("N,S,D,T");

    // Default panel modes. Overridable from registry.
    // These modes are translated into PanelModesXX
    static const struct {
      const TCHAR *Cols;
      const TCHAR *Width;
    }
      DefaultModesNT[NPANELMODES] = {
        /*0*/ {_T("N,X15T,X16T,X17T,X18S"), _T("12,0,0,0,4")}, // I/O
        /*1*/ {_T("N,XI,XP,X0S,X6"), _T("0,4,2,3,9")}, // General info
        /*2*/ {_T("N,N"),_T("0,0")},// Names only
        /*3*/ {_T("N,XI,XC,D,T"),_T("0,4,4,0,0")},    // Startup: PID/Date/Time
        /*4*/ {_T("N,XI,X4,X6"),_T("0,4,9,9")}, // Memory (basic)
        /*5*/ {_T("N,XI,X4,X6,X10,X12,X0,X1,X2"),_T("12,4,0,0,0,0,8,8,8")},     // Extended Memory/Time
        /*6*/ {_T("N,ZD"),_T("12,0")}, // Descriptions
        /*7*/ {_T("N,XP,X0S,X1S,X2S,X11S,X14S,X18S"),_T("0,2,3,2,2,3,4,3")}, // Dynamic Performance
        /*8*/ {_T("N,XI,O"),_T("0,5,15")}, // Owners (not implemented)
        /*9*/ {_T("N,XI,XT,X3,XG,XU"),_T("0,4,3,4,4,4")} // Resources
    },
      DefaultModes9x[NPANELMODES] = {
        /*0*/ {_T("N,N"),_T("0,0")},// Names only
        /*1*/ {_T("N,XP,XB,XI"),_T("0,2,2,8")},// General info
        /*2*/ {_T("N,N"),_T("0,0")},// Names only
        /*3*/ {_T("N,XP,XI,XC,XT"),_T("0,2,8,8,3")},
        /*4*/ {_T("N,S"),_T("0,8")},
        /*5*/ {_T("N,N"),_T("0,0")},// Names only
        /*6*/ {_T("N,ZP"),_T("12,0")},
        /*7*/ {_T("N,ZD"),_T("12,0")},
        /*8*/ {_T("N,ZW"),_T("12,0")},
        /*9*/ {_T("N,XI,XT"),_T("0,8,3")}
     },
       DefaultModesRemoteNT[NPANELMODES] = {
        /*0*/ {_T("N,X15T,X16T,X17T,X18S"), _T("12,0,0,0,4")}, // I/O
        /*1*/ {_T("N,XI,XP,X0S,X6"), _T("0,4,2,3,9")}, // General info
        /*2*/ {_T("N,N"),_T("0,0")},// Names only
        /*3*/ {_T("N,XI,XC,D,T"),_T("0,4,4,0,0")},    // Startup: PID/Date/Time
        /*4*/ {_T("N,XI,X4,X6"),_T("0,4,9,9")}, // Memory (basic)
        /*5*/ {_T("N,XI,X4,X6,X10,X12,X0,X1,X2"),_T("12,4,0,0,0,0,8,8,8")},     // Extended Memory/Time
        /*6*/ {_T("N,ZD"),_T("12,0")}, // Descriptions
        /*7*/ {_T("N,XP,X0S,X1S,X2S,X11S,X14S,X18S"),_T("0,2,3,2,2,3,4,3")}, // Dynamic Performance
        /*8*/ {_T("N,XI,O"),_T("0,5,15")}, // Owners (not implemented)
        /*9*/ {_T("N,XI,XT,X3"),_T("0,4,3,4")}
      };
#define MAXCOLS MAX_CUSTOM_COLS+4

        TCHAR name[20];
        PanelModesLocal[5].FullScreen = PanelModesRemote[5].FullScreen = 1;

        for(int iMode=0; iMode<NPANELMODES; iMode++) {
            // Set pointers to our buffer
            PanelModesLocal[iMode].ColumnTypes = PanelModeBuffer + iMode*MAX_MODE_STR*8;
            PanelModesLocal[iMode].ColumnWidths = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR;
            PanelModesLocal[iMode].StatusColumnTypes = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR*2;
            PanelModesLocal[iMode].StatusColumnWidths = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR*3;
            PanelModesRemote[iMode].ColumnTypes = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR*4;
            PanelModesRemote[iMode].ColumnWidths = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR*5;
            PanelModesRemote[iMode].StatusColumnTypes = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR*6;
            PanelModesRemote[iMode].StatusColumnWidths = PanelModeBuffer + iMode*MAX_MODE_STR*8 + MAX_MODE_STR*7;

            FSF.sprintf(name, _T("Mode%d"), iMode);

            GetRegKey(name, _T("ColumnsLocal"), ProcPanelModesLocal[iMode], NT ? DefaultModesNT[iMode].Cols : DefaultModes9x[iMode].Cols, ARRAYSIZE(ProcPanelModesLocal[iMode]));
            GetRegKey(name, _T("ColumnsRemote"), ProcPanelModesRemote[iMode], DefaultModesRemoteNT[iMode].Cols, ARRAYSIZE(ProcPanelModesRemote[iMode]));
            GetRegKey(name, _T("WidthsLocal"), (LPTSTR)PanelModesLocal[iMode].ColumnWidths, NT ? DefaultModesNT[iMode].Width : DefaultModes9x[iMode].Width, MAX_MODE_STR-1);
            GetRegKey(name, _T("WidthsRemote"), (LPTSTR)PanelModesRemote[iMode].ColumnWidths, DefaultModesRemoteNT[iMode].Width, MAX_MODE_STR-1);
            GetRegKey(name, _T("FullScreenLocal"), PanelModesLocal[iMode].FullScreen, iMode==5 && NT ? 1 : 0);
            GetRegKey(name, _T("FullScreenRemote"), PanelModesRemote[iMode].FullScreen, iMode==5 ? 1 : 0);

            //Status line is the same for all modes currently and cannot be changed.
            TranslateMode(StatusCols, (LPTSTR)PanelModesLocal[iMode].StatusColumnTypes);
            TranslateMode(StatusCols, (LPTSTR)PanelModesRemote[iMode].StatusColumnTypes);
            lstrcpy((TCHAR*)PanelModesLocal[iMode].StatusColumnWidths, NT ? StatusWidthNT : StatusWidth9x);
            lstrcpy((TCHAR*)PanelModesRemote[iMode].StatusColumnWidths, StatusWidthNT);
        }
}

Plist::~Plist()
{
    delete pPerfThread;
    DisconnectWMI();
}

void Plist::SavePanelModes()
{
    TCHAR name[20];
    for(int iMode=0; iMode<NPANELMODES; iMode++) {
        FSF.sprintf(name, _T("Mode%d"), iMode);
        SetRegKey(name, _T("ColumnsLocal"), ProcPanelModesLocal[iMode]);
        SetRegKey(name, _T("ColumnsRemote"), ProcPanelModesRemote[iMode]);
        SetRegKey(name, _T("WidthsLocal"), PanelModesLocal[iMode].ColumnWidths);
        SetRegKey(name, _T("WidthsRemote"), PanelModesRemote[iMode].ColumnWidths);
        SetRegKey(name, _T("FullScreenLocal"), PanelModesLocal[iMode].FullScreen);
        SetRegKey(name, _T("FullScreenRemote"), PanelModesRemote[iMode].FullScreen);
    }
}

bool Plist::TranslateMode(LPCTSTR src, LPTSTR dest)
{
    if(!dest) return true;
    if(!src) { *dest=0; return true; }
    int iCustomMode = _T('0');
    bool bWasDesc = false;
    while(*src) {
      switch(*src & ~0x20) {
        case _T('Z'):
          switch(*++src & ~0x20) {
            case _T('P'):case _T('W'):case _T('D'):case _T('C'):
              break;
            default:
              return false;
          }
          if(bWasDesc)
            return false;
          *dest++ = _T('Z');
          bWasDesc = true;
          //*dest++ = iCustomMode++;
          if(*src && *src!=_T(','))
            return false;
          break;
        case _T('X'):
          switch(*++src & ~0x20) {
            case _T('P'):case _T('I'):case _T('C')://case _T('W'):
            case _T('T'):case _T('B'):case _T('G'):case _T('U'):
              src++;
              break;
            default:
              TCHAR* endptr;
              if(*src<_T('0') || *src>_T('9'))
                return false;
              _tcstol(src,&endptr,10);
              if(endptr==src) return false;
              src = endptr;
          }
          *dest++ = _T('C');
          *dest++ = iCustomMode++;
          /*if(*src && *src!=_T(','))
          return false;*/
          while(*src && *src!=_T(',')) ++src;
          break;
        default:
          while(*src && *src!=_T(',')) *dest++ = *src++;
      }
      if(*src==_T(',')) *dest++ = *src++;
    }
    *dest = 0;
    return true;
}

void Plist::GeneratePanelModes()
{
    for(int iMode=0; iMode<NPANELMODES; iMode++) {
        TranslateMode(ProcPanelModesLocal[iMode], (LPTSTR)PanelModesLocal[iMode].ColumnTypes);
        TranslateMode(ProcPanelModesRemote[iMode], (LPTSTR)PanelModesRemote[iMode].ColumnTypes);
        /*TranslateMode(ProcPanelStModesNT[iMode], PanelModesNT[iMode].StatusColumnTypes);
        if(!NT) TranslateMode(ProcPanelStModes9x[iMode], PanelModes9x[iMode].StatusColumnTypes);*/
    }
}

int DescIDs[] = { MColFullPathname, MColumnTitle,
MTitleFileDesc, MCommandLine };

#define CANBE_PERSEC(n) ((n)<3 || (n)==11 || (n)>=14)

static void GenerateTitles(TCHAR ProcPanelModes[][MAX_MODE_STR],
                           PanelMode* PanelModes, TCHAR* Titles[][MAXCOLS])
{
    TCHAR buf[80];
    for(int i=0; i<NPANELMODES; i++) {
        if(*ProcPanelModes[i]) {
            lstrcpyn(buf, ProcPanelModes[i], ARRAYSIZE(buf));
            int ii=0;
            for(StrTok tok(buf,_T(",")); tok; ++tok) {
                int id=0;
                switch(*tok&~0x20) {
                  case _T('N'): id = MColumnModule; break;
                  case _T('Z'):
                    switch(tok[1]&~0x20) {
                      case _T('P'): id = MTitleFullPath; break;
                      case _T('W'): id = MColumnTitle; break;
                      case _T('D'): id = MTitleFileDesc; break;
                      case _T('C'): id = MCommandLine; break;
                    }
                    break;
                  case _T('X'):
                    switch(tok[1]&~0x20) {
                      case _T('P'): id = MColumnPriority; break;
                      case _T('I'): id = MTitlePID; break;
                      case _T('C'): id = MColumnParentPID; break;
                      //case _T('W'): id = ; break;
                      case _T('T'): id = MTitleThreads; break;
                      case _T('B'): id = MColumnBits; break;
                      case _T('G'): id = MColumnGDI; break;
                      case _T('U'): id = MColumnUSER; break;
                      default:
                        int n = FSF.atoi(&tok[1]);
                        if(n>=0 && n<NCOUNTERS) {
                          id = Counters[n].idCol;
                          if(_tcspbrk(&tok[1], _T("Ss")) && CANBE_PERSEC(n))
                          ++id;
                        }
                        break;
                    }
                    break;
                }
                Titles[i][ii++] = id ? GetMsg(id) : 0;
            }
            PanelModes[i].ColumnTitles = Titles[i];
        }
        else
            PanelModes[i].ColumnTitles = 0;
    }
}

// Obtains the current array of panel modes. Called from OpenPluginInfo.
PanelMode* Plist::PanelModes(int& nModes)
{
    static TCHAR* TitlesLocal[NPANELMODES][MAXCOLS];
    static TCHAR* TitlesRemote[NPANELMODES][MAXCOLS];

    static TCHAR* OldMsg0;

    if(OldMsg0 != GetMsg(0)) {// language changed
        bInit = false;
        OldMsg0 = GetMsg(0);
    }

    if(!bInit) {
        GeneratePanelModes();
        GenerateTitles(ProcPanelModesLocal, PanelModesLocal, TitlesLocal);
        GenerateTitles(ProcPanelModesRemote, PanelModesRemote, TitlesRemote);
        bInit = true;
    }
    nModes = NPANELMODES;
    return *HostName ? PanelModesRemote : PanelModesLocal;
}

void Plist::GetOpenPluginInfo(OpenPluginInfo *Info)
{
    Info->StructSize = sizeof(*Info);
    Info->Flags = OPIF_ADDDOTS|OPIF_SHOWNAMESONLY|OPIF_USEATTRHIGHLIGHTING;

    Info->CurDir = _T("");

    static TCHAR Title[100];
    if(*HostName)
        FSF.sprintf(Title,_T("%s: %s "), HostName, GetMsg(MPlistPanel));
    else
        FSF.sprintf(Title,_T(" %s "),GetMsg(MPlistPanel));
    Info->PanelTitle=Title;

    Info->PanelModesArray = PanelModes(Info->PanelModesNumber);

    Info->StartPanelMode = StartPanelMode;
    Info->StartSortMode = SortMode >= SM_CUSTOM ? SM_CTIME : SortMode; //SM_UNSORTED;

    static KeyBarTitles keybartitles = {
        { 0, 0, 0, 0, 0, (TCHAR*)_T(""), (TCHAR*)_T(""), }, { 0, }, { 0, 0, 0, 0, 0, (TCHAR*)_T(""),},
        { (TCHAR*)_T(""), (TCHAR*)_T(""), (TCHAR*)_T(""), 0, (TCHAR*)_T(""), (TCHAR*)_T(""), (TCHAR*)_T(""), }
    };
    keybartitles.Titles[5] = GetMsg(MFRemote);
    keybartitles.ShiftTitles[0] = GetMsg(MFPriorMinus);
    keybartitles.ShiftTitles[1] = GetMsg(MFPriorPlus);
    keybartitles.ShiftTitles[2] = GetMsg(MViewDDD);
    keybartitles.ShiftTitles[5] = GetMsg(MFLocal);
    keybartitles.Titles[7] = keybartitles.ShiftTitles[7] = GetMsg(MFKill);
    Info->KeyBar = &keybartitles;
}

struct EnumWndData {DWORD dwPID; HWND hWnd; };

BOOL CALLBACK EnumWndProc(HWND hWnd,LPARAM lParam)
{
    DWORD dwProcID;
    GetWindowThreadProcessId(hWnd, &dwProcID);
    if (dwProcID==((EnumWndData*)lParam)->dwPID && GetParent(hWnd)==NULL )
    {
        BOOL bVisible = IsWindowVisible(hWnd) ||
            (IsIconic(hWnd) && (GetWindowLong(hWnd,GWL_STYLE) & WS_DISABLED)==0);
        if(!((EnumWndData*)lParam)->hWnd || bVisible)
            ((EnumWndData*)lParam)->hWnd = hWnd;
        return !bVisible;
    }
    return TRUE;
}

int Plist::GetFindData(PluginPanelItem*& pPanelItem,int &ItemsNumber,int OpMode)
{
    Lock l(pPerfThread);
    int RetCode = pPerfThread ? GetListNT(pPanelItem,ItemsNumber,*pPerfThread) :
                                GetList95(pPanelItem,ItemsNumber);
    if(!RetCode) return FALSE;

    PanelInfo pi;
#ifndef UNICODE
    Info.Control(this,FCTL_GETPANELINFO, &pi);
#else
    Info.Control(this,FCTL_GETPANELINFO,0,(LONG_PTR)&pi);
#endif
    TCHAR (* ProcPanelModes)[MAX_MODE_STR] = *HostName ? ProcPanelModesRemote : ProcPanelModesLocal;
    int cDescMode = 0;
    if(!*HostName) {
        TCHAR* p = _tcschr(ProcPanelModes[pi.ViewMode], _T('Z') );
        if(p)
            cDescMode = p[1];
    }
    for(int i = 0; i < ItemsNumber; i++)
    {
        PluginPanelItem &CurItem = pPanelItem[i];
        ProcessData & pdata = *((ProcessData *)CurItem.UserData);

        // Make descriptions
        TCHAR Title[MAX_PATH]; *Title=0;
        TCHAR* pDesc=(TCHAR *)_T("");
        LPBYTE pBuf=0;
        EnumWndData ewdata = { pdata.dwPID, 0 };
        EnumWindows((WNDENUMPROC)EnumWndProc, (LPARAM)&ewdata);
        pdata.hwnd = ewdata.hWnd;

        if(cDescMode) {
            switch(cDescMode&~0x20) {
            case _T('P'):
                if(*pdata.FullPath)
                    pDesc = pdata.FullPath;
                break;
            case _T('W'):
                if(ewdata.hWnd)
                    GetWindowText(ewdata.hWnd, Title, ARRAYSIZE(Title));
                pDesc = Title;
                break;
            case _T('D'):
                TCHAR *pVersion;
                if(!Plist::GetVersionInfo(pdata.FullPath, pBuf, pVersion, pDesc))
                    pDesc = (TCHAR *)_T("");
                break;
            case _T('C'):
                if(NT)
                    pDesc = ((ProcessDataNT *)CurItem.UserData)->CommandLine;
                break;
            default:
                cDescMode = 0;
            }
            if(cDescMode)
            {
                CurItem.Description = new TCHAR[lstrlen(pDesc)+1];
                lstrcpy((TCHAR*)CurItem.Description, pDesc);
#ifndef UNICODE
                CharToOem(CurItem.Description, CurItem.Description);
#endif
            }
            delete pBuf;
        }
        ProcessPerfData* pd = 0;
        if(pPerfThread)
            pd = pPerfThread->GetProcessData(pdata.dwPID, CurItem.NumberOfLinks);
        const int DataOffset = sizeof(TCHAR*) * MAX_CUSTOM_COLS;
        int Widths[MAX_CUSTOM_COLS]; memset(Widths, 0, sizeof(Widths));

        unsigned uCustomColSize = 0;

        int nCols=0;
#ifndef UNICODE
#define ColumnWidths pi.ColumnWidths
#else
        int Size=Info.Control(this,FCTL_GETCOLUMNWIDTHS,0,NULL);
        wchar_t *ColumnWidths=new wchar_t[Size];
        Info.Control(this,FCTL_GETCOLUMNWIDTHS,Size,(LONG_PTR)ColumnWidths);
#endif
        for(StrTok tokn(ColumnWidths, _T(", ")); (bool)tokn && nCols<MAX_CUSTOM_COLS; ++tokn) {
#undef ColumnWidths
            uCustomColSize += (unsigned int)(((Widths[nCols++] = FSF.atoi(tokn)) + 1)*sizeof(TCHAR));
        }
#ifdef UNICODE
        delete[] ColumnWidths;
#endif

        if(nCols) {
            CurItem.CustomColumnData = (TCHAR**)new char[DataOffset + uCustomColSize];
            TCHAR* pData = (TCHAR*)((PCH)CurItem.CustomColumnData+DataOffset); // Start offset of column data aftet ptrs

            int nCustomCols;
            nCustomCols = nCols = 0;
            for(StrTok tok(ProcPanelModes[pi.ViewMode], _T(", ")); tok; ++tok, ++nCols) {
                if((*tok&~0x20)==_T('X')) { // Custom column
                    bool bCol = true;
                    DWORD dwData = 0;
                    int nBase = 10;
                    int iCounter = -1;
                    int nColWidth = Widths[nCols];
                    if(nColWidth==0)
                        continue;
                    bool bPerSec = false, bThousands = false;
                    TCHAR c = tok[1];
                    switch(c&~0x20) {
                        case _T('P'): dwData = pdata.dwPrBase; break;
                        case _T('I'): dwData = pdata.dwPID;
                            if(!pPerfThread) nBase = 16;
                            break;
                        case _T('C'): dwData = pdata.dwParentPID;
                            if(!pPerfThread) nBase = 16;
                            break;
                            //case _T('W'): dwData = hwnd; nBase = 16; break;
                        case _T('T'): dwData = CurItem.NumberOfLinks; break;
                        case _T('B'): dwData = pdata.uAppType; break;
                        case _T('G'): if(pd) dwData = pd->dwGDIObjects; break;
                        case _T('U'): if(pd) dwData = pd->dwUSERObjects; break;
                        default:
                            if(c<_T('0') || c>_T('9'))
                                bCol = false;
                            else {
                                iCounter = FSF.atoi(&tok[1]);
                                if(_tcspbrk(&tok[1], _T("Ss")) && CANBE_PERSEC(iCounter))
                                    bPerSec = true;
                                if(_tcspbrk(&tok[1], _T("Tt")))
                                    bThousands = true;
                            }
                    }
                    if(!bCol)
                        continue;

                    ((TCHAR**)(CurItem.CustomColumnData))[nCustomCols] = pData;

                    int nBufSize = max( nColWidth+1, 16); // to provide space for itoa
                    Array<TCHAR> buf(nBufSize);
                    if(c>=_T('A')) // Not a performance counter
                        FSF.itoa(dwData, buf, nBase);
                    else if(pd && iCounter>=0) {    // Format performance counters
                        if(iCounter<3 && !bPerSec)  // first 3 are date/time
                            lstrcpyn(buf, PrintTime(pd->qwCounters[iCounter], false), nBufSize);
                        else
                            ui64toa_width(bPerSec ? pd->qwResults[iCounter] : pd->qwCounters[iCounter],
                            buf, nColWidth, bThousands);
                    }
                    else
                        *buf=0;
                    int nVisibleDigits = lstrlen(buf);
                    if(nVisibleDigits > nColWidth) nVisibleDigits = nColWidth;
                    _tmemset(pData, _T(' '), nColWidth-nVisibleDigits);
                    pData += nColWidth-nVisibleDigits;
                    lstrcpyn(pData,buf,nVisibleDigits+1);
                    pData += nVisibleDigits+1;

                    if(++nCustomCols >= MAX_CUSTOM_COLS/* || ...>=MAX_CUSTOM_COL_SIZE*/)
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
        delete item.Description;
        delete item.Owner;
        delete item.CustomColumnData;
        delete (ProcessData *)item.UserData;
#ifdef UNICODE
        delete item.FindData.lpwszFileName;
        delete item.FindData.lpwszAlternateFileName;
#endif
    }
    delete PanelItem;
}

int Plist::GetFiles(PluginPanelItem *PanelItem,int ItemsNumber, int Move,WCONST WTYPE DestPath,int OpMode, _Opt& Opt)
{
    static const TCHAR invalid_chars[] = _T(":*?\\/\"<>;|");
    if (ItemsNumber==0)
        return 0;

    for (int I=0;I<ItemsNumber;I++)
    {
        PluginPanelItem &CurItem = PanelItem[I];
        ProcessData *pdata=(ProcessData *)CurItem.UserData;
        ProcessDataNT PData;
        if(!pdata) {
            if(!pPerfThread) {
                memset(&PData, 0, sizeof(PData));
#ifdef UNICODE
#define cAlternateFileName lpwszAlternateFileName
#endif
                PData.dwPID = _tcstoul(CurItem.FindData.cAlternateFileName, 0, 16);
                if(GetPData95(PData))
                    pdata = &PData;
            }
            else {
                PData.dwPID = FSF.atoi(CurItem.FindData.cAlternateFileName);
#undef cAlternateFileName
                ProcessPerfData* ppd = pPerfThread->GetProcessData(PData.dwPID, CurItem.NumberOfLinks);
                if(ppd && GetPDataNT(PData, *ppd))
                    pdata = &PData;
            }
            if(!pdata)
                return 0;
        }
        // may be 0 if called from FindFile
        TCHAR FileName[MAX_PATH];
        lstrcpyn(FileName, WDEREF DestPath, ARRAYSIZE(FileName));
        if(!(OpMode&0x10000)) {
            FSF.AddEndSlash(FileName);
#ifdef UNICODE
#define cFileName lpwszFileName
#endif
            _tcsncat(FileName,CurItem.FindData.cFileName,ARRAYSIZE(FileName)-lstrlen(FileName)-1);
            _tcsncat(FileName,_T(".txt"),ARRAYSIZE(FileName)-lstrlen(FileName)-1);
        }
        // Replace "invalid" chars by underscores
        TCHAR* pname = _tcsrchr(FileName, _T('\\'));
        if(!pname) pname = FileName; else pname++;
        for(; *pname; pname++) {
            if(_tcschr(invalid_chars, *pname) || *pname<_T(' '))
                *pname = _T('_');
        }

        HANDLE InfoFile = CreateFile(FileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
        if (InfoFile==INVALID_HANDLE_VALUE)
            return 0;
#ifdef UNICODE
        fputc(0xFEFF, InfoFile);
#endif
        TCHAR AppType[100];
        if (!pPerfThread && pdata->uAppType)
            FSF.sprintf(AppType,_T(", %d%s"),pdata->uAppType,GetMsg(MBits));
        else
            *AppType=0;

        TCHAR ModuleName[MAX_PATH];
        lstrcpy(ModuleName, CurItem.FindData.cFileName);
#undef cFileName
        fprintf(InfoFile,_T("%s %s%s\n"),PrintTitle(MTitleModule),ModuleName,AppType);
        if (pdata && pdata->FullPath && *pdata->FullPath) {
            fprintf(InfoFile,_T("%s %s\n"),PrintTitle(MTitleFullPath),OUT_STRING(pdata->FullPath));
            PrintVersionInfo(InfoFile, pdata->FullPath);
        }
        fprintf(InfoFile, pPerfThread ? _T("%s %d\n"):_T("%s %08X\n"),PrintTitle(MTitlePID),pdata->dwPID);
        fprintf(InfoFile, _T("%s "), PrintTitle(MTitleParentPID));
        if(pPerfThread) {
            Lock l(pPerfThread);
            ProcessPerfData* pParentData = pPerfThread->GetProcessData(pdata->dwParentPID, 0);
            TCHAR* pName = pdata->dwParentPID && pParentData ? pParentData->ProcessName : 0;
            fprintf(InfoFile, pName ? _T("%u  (%s)\n") : _T("%u\n"),pdata->dwParentPID, pName);
        }
        else
            fprintf(InfoFile, _T("%08X\n"),pdata->dwParentPID);

        fprintf(InfoFile,_T("%s %d\n"),PrintTitle(MTitlePriority),pdata->dwPrBase);
        fprintf(InfoFile,_T("%s %u\n"),PrintTitle(MTitleThreads),CurItem.NumberOfLinks);

        if(!pPerfThread) {
#ifndef UNICODE
            fprintf(InfoFile,_T("%s %8u\n"),PrintTitle(MTitleModuleSize),CurItem.PackSize);
#else
            fprintf(InfoFile,_T("%s %8u\n"),PrintTitle(MTitleModuleSize),(DWORD)CurItem.FindData.nPackSize);
#endif
            fprintf(InfoFile,_T("%s %8u\n"),PrintTitle(MTitleHeapSize),GetHeapSize(pdata->dwPID));
        }
        else
            PrintOwnerInfo(InfoFile, pdata->dwPID);

        // Time information

        if(CurItem.FindData.ftCreationTime.dwLowDateTime || CurItem.FindData.ftCreationTime.dwHighDateTime)
        {
            FILETIME CurFileTime;
            GetSystemTimeAsFileTime(&CurFileTime);

            SYSTEMTIME Current,Compare;
            GetLocalTime(&Current);
            FileTimeToSystemTime(&CurItem.FindData.ftCreationTime,&Compare);
            SystemTimeToTzSpecificLocalTime(NULL,&Compare,&Compare);

            TCHAR DateText[MAX_DATETIME],TimeText[MAX_DATETIME];
            ConvertDate(CurItem.FindData.ftCreationTime,DateText,TimeText);

            if (Current.wYear!=Compare.wYear || Current.wMonth!=Compare.wMonth || Current.wDay!=Compare.wDay)
            {
                fprintf(InfoFile,_T("\n%s %s %s\n"),PrintTitle(MTitleStarted),DateText,TimeText);
            }
            else
            {
                fprintf(InfoFile,_T("\n%s %s\n"),PrintTitle(MTitleStarted),TimeText);
            }
            //fprintf(InfoFile,_T("%s %s\n"),PrintTitle(MTitleUptime),PrintNTUptime((void*)CurItem.UserData));
            FileTimeToText  (&CurFileTime,&CurItem.FindData.ftCreationTime,TimeText);
            fprintf(InfoFile,_T("%s %s\n"),PrintTitle(MTitleUptime),TimeText);
        }

        HANDLE hProcess = 0;

        if (NT && !*HostName) // local only
        {
            if (*((ProcessDataNT*)pdata)->CommandLine)
            {
                fprintf(InfoFile, _T("\n%s:\n%s\n"), GetMsg(MCommandLine), OUT_STRING(((ProcessDataNT*)pdata)->CommandLine));
            }

            DebugToken token;
            hProcess = OpenProcessForced(&token, PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|READ_CONTROL,pdata->dwPID);

            if(hProcess) {
                PrintNTCurDirAndEnv(InfoFile, hProcess, Opt.ExportEnvironment);
                CloseHandle(hProcess);
            }

            if (hProcess)
            {
                Lock l(pPerfThread);
                ProcessPerfData& pd = *pPerfThread->GetProcessData(pdata->dwPID, CurItem.NumberOfLinks);
                if(pd.dwGDIObjects)
                {
                    fprintf(InfoFile,_T("\n%s %u\n"),PrintTitle(MGDIObjects), pd.dwGDIObjects);
                }
                if(pd.dwUSERObjects)
                {
                    fprintf(InfoFile,_T("%s %u\n"),PrintTitle(MUSERObjects), pd.dwUSERObjects);
                }
            }
        }// NT && !*HostName

        if(Opt.ExportPerformance && pPerfThread)
            DumpNTCounters(InfoFile, *pPerfThread, pdata->dwPID, CurItem.NumberOfLinks);
        if(!*HostName && pdata->hwnd)
        {
            TCHAR Title[MAX_PATH]; *Title=0;
            GetWindowText(pdata->hwnd, Title, ARRAYSIZE(Title));
#ifndef UNICODE
            CharToOem(Title,Title);
#endif
            fprintf(InfoFile,_T("\n%s %s\n"),PrintTitle(MTitleWindow), Title);
            fprintf(InfoFile,_T("%-22s %p\n"),_T("HWND:"),pdata->hwnd);
            LONG Style=0,ExtStyle=0;
            if (pdata->hwnd!=NULL)
            {
                Style=GetWindowLong(pdata->hwnd,GWL_STYLE);
                ExtStyle=GetWindowLong(pdata->hwnd,GWL_EXSTYLE);
            }

            static const int Styles[]={
                WS_POPUP,WS_CHILD,WS_MINIMIZE,WS_VISIBLE,WS_DISABLED,
                WS_CLIPSIBLINGS,WS_CLIPCHILDREN,WS_MAXIMIZE,WS_BORDER,WS_DLGFRAME,
                WS_VSCROLL,WS_HSCROLL,WS_SYSMENU,WS_THICKFRAME,WS_MINIMIZEBOX,
                WS_MAXIMIZEBOX
            };

            static TCHAR const * const StrStyles[]={
                _T("WS_POPUP"),_T("WS_CHILD"),_T("WS_MINIMIZE"),_T("WS_VISIBLE"),
                _T("WS_DISABLED"),_T("WS_CLIPSIBLINGS"),_T("WS_CLIPCHILDREN"),
                _T("WS_MAXIMIZE"),_T("WS_BORDER"),_T("WS_DLGFRAME"),_T("WS_VSCROLL"),
                _T("WS_HSCROLL"),_T("WS_SYSMENU"),_T("WS_THICKFRAME"),
                _T("WS_MINIMIZEBOX"),_T("WS_MAXIMIZEBOX")
            };

            static const int ExtStyles[]={
                WS_EX_DLGMODALFRAME,WS_EX_NOPARENTNOTIFY,WS_EX_TOPMOST,
                WS_EX_ACCEPTFILES,WS_EX_TRANSPARENT,WS_EX_MDICHILD,
                WS_EX_TOOLWINDOW,WS_EX_WINDOWEDGE,WS_EX_CLIENTEDGE,WS_EX_CONTEXTHELP,
                WS_EX_RIGHT,WS_EX_RTLREADING,WS_EX_LEFTSCROLLBAR,WS_EX_CONTROLPARENT,
                WS_EX_STATICEDGE,WS_EX_APPWINDOW,
                0x00080000, 0x00100000L, 0x00400000L, 0x08000000L
                /*WS_EX_LAYERED,WS_EX_NOINHERITLAYOUT,
                  WS_EX_LAYOUTRTL,WS_EX_NOACTIVATE*/
            };

            static TCHAR const * const StrExtStyles[]={
                _T("WS_EX_DLGMODALFRAME"),_T("WS_EX_NOPARENTNOTIFY"),_T("WS_EX_TOPMOST"),
                _T("WS_EX_ACCEPTFILES"),_T("WS_EX_TRANSPARENT"),_T("WS_EX_MDICHILD"),
                _T("WS_EX_TOOLWINDOW"),_T("WS_EX_WINDOWEDGE"),_T("WS_EX_CLIENTEDGE"),
                _T("WS_EX_CONTEXTHELP"),_T("WS_EX_RIGHT"),_T("WS_EX_RTLREADING"),
                _T("WS_EX_LEFTSCROLLBAR"),_T("WS_EX_CONTROLPARENT"),
                _T("WS_EX_STATICEDGE"),_T("WS_EX_APPWINDOW"),_T("WS_EX_LAYERED"),
                _T("WS_EX_NOINHERITLAYOUT"),_T("WS_EX_LAYOUTRTL"),_T("WS_EX_NOACTIVATE")
            };

            TCHAR StyleStr[1024], ExtStyleStr[1024];
            *StyleStr = *ExtStyleStr=0;
            size_t i;
            for (i=0; i<ARRAYSIZE(Styles); i++)
                if (Style & Styles[i])
                {
                    lstrcat(StyleStr, _T(" "));
                    lstrcat(StyleStr, StrStyles[i]);
                }
            for (i=0; i<ARRAYSIZE(ExtStyles); i++)
                if (Style & ExtStyles[i])
                {
                    lstrcat(ExtStyleStr, _T(" "));
                    lstrcat(ExtStyleStr, StrExtStyles[i]);
                }

            fprintf(InfoFile,_T("%-22s %08X %s\n"),PrintTitle(MTitleStyle),Style,StyleStr);
            fprintf(InfoFile,_T("%-22s %08X %s\n"),PrintTitle(MTitleExtStyle),ExtStyle,ExtStyleStr);
        }

        if(!*HostName && Opt.ExportModuleInfo && pdata->dwPID!=8) {
            fprintf(InfoFile,_T("\n%s\n%s%s\n"),  GetMsg(MTitleModules), GetMsg(MColBaseSize),
                Opt.ExportModuleVersion ? GetMsg(MColPathVerDesc) : GetMsg(MColPathVerDescNotShown) );
            if(!NT)
                PrintModules95(InfoFile, pdata->dwPID, Opt);
            else
                PrintModulesNT(InfoFile, pdata->dwPID, Opt);
        }
        if(NT && !*HostName && Opt.ExportHandles && pdata->dwPID /*&& pdata->dwPID!=8*/)
            PrintHandleInfo(pdata->dwPID, InfoFile, (Opt.ExportHandles&2)!=0, pPerfThread);
        CloseHandle(InfoFile);
    }
    return 1;
}


int Plist::DeleteFiles(PluginPanelItem *PanelItem,int ItemsNumber,
                       int OpMode)
{
    if (ItemsNumber==0)
        return FALSE;

//    if(Info.AdvControl(Info.ModuleNumber, ACTL_GETPOLICIES, 0) & FFPOL_KILLTASK)
//        ;

    if(*HostName && !Opt.EnableWMI)
    {
        //cannot kill remote process
        const TCHAR *MsgItems[]={GetMsg(MCannotDeleteProc),GetMsg(MCannotKillRemote),GetMsg(MOk)};
        Message(FMSG_WARNING,NULL,MsgItems,ARRAYSIZE(MsgItems));
        return FALSE;
    }

    const TCHAR *MsgItems[]={GetMsg(MDeleteTitle),GetMsg(MDeleteProcesses),
        GetMsg(MDeleteDelete),GetMsg(MCancel)};
    TCHAR Msg[512];

    if (ItemsNumber==1)
    {
#ifdef UNICODE
#define cFileName lpwszFileName
#endif
        FSF.sprintf(Msg,GetMsg(MDeleteProcess),PanelItem[0].FindData.cFileName);
        MsgItems[1]=Msg;
    }
    if (Message(0,NULL,MsgItems,ARRAYSIZE(MsgItems),2)!=0)
        return FALSE;
    if (ItemsNumber>1)
    {
        TCHAR Msg[512];
        FSF.sprintf(Msg,GetMsg(MDeleteNumberOfProcesses),ItemsNumber);
        MsgItems[1]=Msg;
        if (Message(FMSG_WARNING,NULL,MsgItems,ARRAYSIZE(MsgItems),2)!=0)
            return FALSE;
    }
    for (int I=0;I<ItemsNumber;I++)
    {
        PluginPanelItem& CurItem = PanelItem[I];
        ProcessData *pdata=(ProcessData *)CurItem.UserData;
        BOOL Success;
        int MsgId = 0;

        if(*HostName) { // try WMI
            if(!ConnectWMI()) {
                WmiError();
                Success = FALSE;
                break;
            }
            Success = FALSE;
            switch(pWMI->TerminateProcess(pdata->dwPID)) {
                case -1:
                    WmiError();
                    continue;
                case 0: Success = TRUE; break;
                case 2: MsgId = MTAccessDenied; break;
                case 3: MsgId = MTInsufficientPrivilege; break;
                default: MsgId = MTUnknownFailure; break;
            }
        }
        else if (NT)
            Success = KillProcessNT(pdata->dwPID,pdata->hwnd);
        else
            Success = KillProcess(pdata->dwPID);
        if (!Success)
        {
            TCHAR Msg[512];
            FSF.sprintf(Msg,GetMsg(MCannotDelete),CurItem.FindData.cFileName);
#undef cFileName
            const TCHAR *MsgItems[]={GetMsg(MDeleteTitle),Msg, 0, GetMsg(MOk)};
            int nItems = ARRAYSIZE(MsgItems);
            if(MsgId)
                MsgItems[2] = GetMsg(MsgId);
            else {
                MsgItems[2] = MsgItems[3];
                nItems--;
            }
            Message(FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,nItems);
            return FALSE;
        }
    }
    if(pPerfThread)
        pPerfThread->SmartReread();

    return TRUE;
}


int Plist::ProcessEvent(int Event,void *Param)
{
    if(Event==FE_IDLE && (pPerfThread && pPerfThread->Updated() /*|| !pPerfThread&&GetTickCount()-LastUpdateTime>1000*/))
        Reread();
    if(Event==FE_CLOSE) {
        PanelInfo pi;
#ifndef UNICODE
        Info.Control(this,FCTL_GETPANELSHORTINFO, &pi);
#else
        Info.Control(this,FCTL_GETPANELINFO,0,(LONG_PTR)&pi);
#endif
        SetRegKey(0,_T("StartPanelMode"), pi.ViewMode);
        SetRegKey(0,_T("SortMode"), pi.SortMode==SM_CTIME ? SortMode : pi.SortMode);
    }
    if(Event==FE_CHANGEVIEWMODE) {
        if(/*pPerfThread || */_tcschr((TCHAR*)Param,_T('Z')) || _tcschr((TCHAR*)Param,_T('C')))
            Reread();
    }
    return FALSE;
}

void Plist::Reread()
{
#ifndef UNICODE
    Info.Control(this,FCTL_UPDATEPANEL, (void*)1);
    Info.Control(this,FCTL_REDRAWPANEL, NULL);
#else
    Info.Control(this,FCTL_UPDATEPANEL,1,NULL);
    Info.Control(this,FCTL_REDRAWPANEL,0,NULL);
#endif
    PanelInfo PInfo;
#ifndef UNICODE
    Info.Control(this,FCTL_GETANOTHERPANELSHORTINFO,&PInfo);
#else
    Info.Control(PANEL_PASSIVE, FCTL_GETPANELINFO,0,(LONG_PTR)&PInfo);
#endif
    if (PInfo.PanelType==PTYPE_QVIEWPANEL)
    {
#ifndef UNICODE
        Info.Control(this,FCTL_UPDATEANOTHERPANEL,(void *)1);
        Info.Control(this,FCTL_REDRAWANOTHERPANEL,NULL);
#else
        Info.Control(PANEL_PASSIVE, FCTL_UPDATEPANEL,1,NULL);
        Info.Control(PANEL_PASSIVE, FCTL_REDRAWPANEL,0,NULL);
#endif
    }
}

void Plist::PutToCmdLine(TCHAR* tmp)
{
    unsigned l = lstrlen(tmp);
    TCHAR* tmp1 = 0;
    if(_tcscspn(tmp,_T(" &^"))!=l) {
        tmp1 = new TCHAR[l+3];
        memcpy(tmp1+1,tmp,l*sizeof(TCHAR));
        tmp1[0] = tmp1[l+1] = _T('\"');
        tmp1[l+2] = 0;
        tmp = tmp1;
    }
#ifndef UNICODE
    Info.Control(this,FCTL_INSERTCMDLINE, tmp);
    Info.Control(this,FCTL_INSERTCMDLINE, (void *)_T(" "));
#else
    Info.Control(this,FCTL_INSERTCMDLINE,0,(LONG_PTR)tmp);
    Info.Control(this,FCTL_INSERTCMDLINE,0,(LONG_PTR)_T(" "));
#endif
    delete tmp1;
}

bool Plist::Connect(LPCTSTR pMachine, LPCTSTR pUser, LPCTSTR pPasw)
{
    TCHAR Machine[ARRAYSIZE(HostName)];
    lstrcpyn(Machine, pMachine, ARRAYSIZE(Machine));

    // Add "\\" if missing
    if(!NORM_M_PREFIX(Machine)) {
      //Convert "//" to "\\"
      if(!REV_M_PREFIX(Machine))
        memmove(Machine+2, Machine, (lstrlen(Machine)+1)*sizeof(TCHAR));
#ifndef UNICODE
        *(LPWORD)Machine = 0x5c5c;
#else
        *(LPDWORD)Machine= 0x5c005c;
#endif
    }
    //Try to connect...
    LPCTSTR ConnectItems[] = {_T(""),GetMsg(MConnect)};
    HANDLE hScreen = Info.SaveScreen(0,0,-1,-1);
    Message(0,0,ConnectItems,2,0);
    if(pUser && *pUser) {
        static NETRESOURCE nr =
            { RESOURCE_GLOBALNET, RESOURCETYPE_DISK, RESOURCEDISPLAYTYPE_SERVER,
              RESOURCEUSAGE_CONTAINER, NULL, NULL, (TCHAR *)_T(""), NULL};
        nr.lpRemoteName = Machine;
        DWORD dwErr = WNetAddConnection2(&nr,pPasw,pUser,0);
        if(dwErr==ERROR_SESSION_CREDENTIAL_CONFLICT) {
            dwErr = WNetCancelConnection2(Machine, 0, 0);
            dwErr = WNetAddConnection2(&nr,pPasw,pUser,0);
        }
        if(dwErr!=NO_ERROR) {
            SetLastError(dwErr);
            WinError();
            return false;
        }
    }
    PerfThread* pNewPerfThread = new PerfThread(*this, Machine, pUser?pUser:0, pUser?pPasw:0);
    if( !pNewPerfThread->IsOK() )
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

int Plist::ProcessKey(int Key,unsigned int ControlState)
{
    if (ControlState==PKF_CONTROL && Key==_T('R'))
    {
        if(pPerfThread)
            pPerfThread->SmartReread();
        return FALSE;
    }
    if (ControlState==0 && Key==VK_RETURN)
    {
        //check for the command line; if it's not empty, don't process Enter
#ifndef UNICODE
        TCHAR CmdLine[1024];
        Info.Control(this,FCTL_GETCMDLINE, CmdLine);
        if(*CmdLine)
#else
        if(Info.Control(this,FCTL_GETCMDLINE,0,NULL)>1)
#endif
            return FALSE;

        PanelInfo PInfo;
#ifndef UNICODE
        Info.Control(this,FCTL_GETPANELINFO,&PInfo);
#else
        Info.Control(this,FCTL_GETPANELINFO,0,(LONG_PTR)&PInfo);
#endif
        if (PInfo.CurrentItem < PInfo.ItemsNumber)
        {
#ifndef UNICODE
            PluginPanelItem& CurItem = PInfo.PanelItems[PInfo.CurrentItem];
#else
            PluginPanelItem* _CurItem=(PluginPanelItem*)new char[Info.Control(this,FCTL_GETPANELITEM,PInfo.CurrentItem,0)];
            PluginPanelItem& CurItem=*_CurItem;
            Info.Control(this,FCTL_GETPANELITEM,PInfo.CurrentItem,(LONG_PTR)&CurItem);
#endif
            if (!CurItem.UserData)
            {
#ifdef UNICODE
                delete [] (char*)_CurItem;
#endif
                return FALSE;
            }
            HWND hWnd = ((ProcessData *)CurItem.UserData)->hwnd;
#ifdef UNICODE
            delete [] (char*)_CurItem;
#endif
            if (hWnd!=NULL && (IsWindowVisible(hWnd) ||
                (IsIconic(hWnd) && (GetWindowLong(hWnd,GWL_STYLE) & WS_DISABLED)==0))
                )
            {
#ifndef SPI_GETFOREGROUNDLOCKTIMEOUT
#define SPI_GETFOREGROUNDLOCKTIMEOUT        0x2000
#define SPI_SETFOREGROUNDLOCKTIMEOUT        0x2001
#endif

                // Allow SetForegroundWindow on Win98+.
                DWORD dwMs;
                // Remember the current value.
                BOOL bSPI = SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &dwMs, 0);
                if(bSPI) // Reset foreground lock timeout
                    bSPI = SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, 0, 0);
                SetForegroundWindow(hWnd);
                if(bSPI) // Restore the old value
                    SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)(SIZE_T)dwMs, 0);

                WINDOWPLACEMENT wp;
                wp.length = sizeof(wp);
                if (!GetWindowPlacement(hWnd,&wp) || wp.showCmd!=SW_SHOWMAXIMIZED)
                    ShowWindowAsync(hWnd,SW_RESTORE);
            }
        }
        return TRUE;
    }
    else if (ControlState==PKF_SHIFT && Key==VK_F3)
    {
        PanelInfo pi;
#ifndef UNICODE
        Info.Control(this,FCTL_GETPANELINFO,&pi);
        if(pi.CurrentItem >= pi.ItemsNumber || !lstrcmp(pi.PanelItems[pi.CurrentItem].FindData.cFileName,".."))
#else
        Info.Control(this,FCTL_GETPANELINFO,0,(LONG_PTR)&pi);
        PluginPanelItem* PPI=(PluginPanelItem*)new char[Info.Control(this,FCTL_GETPANELITEM,pi.CurrentItem,0)];
        Info.Control(this,FCTL_GETPANELITEM,pi.CurrentItem,(LONG_PTR)PPI);
        bool Exit=pi.CurrentItem >= pi.ItemsNumber || !lstrcmp(PPI->FindData.lpwszFileName,L"..");
        delete [] (char*)PPI;
        if(Exit)
#endif
        {
          return TRUE;
        }
        InitDialogItem InitItems[]={ {DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(TCHAR *)MViewWithOptions}, };
        FarDialogItem DialogItems[NVIEWITEMS + 1];
        InitDialogItems(InitItems,DialogItems,ARRAYSIZE(InitItems));
        _Opt LocalOpt = Opt;
        MakeViewOptions(DialogItems+1, LocalOpt, 2);
#ifndef UNICODE
        int ExitCode = Info.Dialog(Info.ModuleNumber,-1,-1,76,NVIEWITEMS+3,_T("Config"),
                                   DialogItems,ARRAYSIZE(DialogItems));
#define _REF  DialogItems
#else
        HANDLE hDlg = Info.DialogInit(Info.ModuleNumber,-1,-1,76,NVIEWITEMS+3,_T("Config"),
                                DialogItems,ARRAYSIZE(DialogItems),0,0,NULL,0);
        if(hDlg == INVALID_HANDLE_VALUE)
        {
          return TRUE;
        }
        int ExitCode = Info.DialogRun(hDlg);
#define _REF  hDlg
#endif
        if(ExitCode!=-1)
          GetViewOptions(_REF, 1, LocalOpt);
#undef _REF
#ifdef UNICODE
        Info.DialogFree(hDlg);
#endif
        if(ExitCode==-1)
        {
          return TRUE;
        }
        TCHAR FileName[MAX_PATH];
#ifndef UNICODE
        FSF.MkTemp(FileName, _T("prc"));
#else
        FSF.MkTemp(FileName, ARRAYSIZE(FileName), _T("prc"));
#endif

        WCONST TCHAR *lpFileName=FileName;
#ifndef UNICODE
        if(GetFiles(pi.PanelItems + pi.CurrentItem, 1, 0, WADDR lpFileName, OPM_VIEW|0x10000, LocalOpt))
#else
        PPI=(PluginPanelItem*)new char[Info.Control(this,FCTL_GETPANELITEM,pi.CurrentItem,0)];
        Info.Control(this,FCTL_GETPANELITEM,pi.CurrentItem,(LONG_PTR)PPI);
        if(GetFiles(PPI, 1, 0, WADDR lpFileName, OPM_VIEW|0x10000, LocalOpt))
#endif
        {
          //TODO: viewer crashed on exit!
#ifndef UNICODE
          Info.Viewer (FileName,pi.PanelItems[pi.CurrentItem].FindData.cFileName, 0,0,-1,-1, VF_NONMODAL|VF_DELETEONCLOSE);
#else
          Info.Viewer (FileName,PPI->FindData.lpwszFileName, 0,0,-1,-1, VF_NONMODAL|VF_DELETEONCLOSE,CP_AUTODETECT);
#endif
        }
#ifdef UNICODE
        delete [] (char*)PPI;
#endif
        return TRUE;
    }
    else if (ControlState==0 && Key==VK_F6)
    {
        InitDialogItem Items[] = {
            { DI_DOUBLEBOX, 3,1, 44,9, 0,0,0,0, GetMsg(MSelectComputer)},
            { DI_TEXT, 5,2, 0,0, 0,0,0,0, GetMsg(MComputer)},
            { DI_EDIT, 5,3, 42,0, 1,(DWORD_PTR)_T("ProcessList.Computer"),DIF_HISTORY,1,HostName},
            { DI_TEXT, 5,4, 0,0, 0,0,0,0, GetMsg(MEmptyForLocal)},
            { DI_TEXT, 0,5, 0,0, 0,0,DIF_SEPARATOR,0, _T("")},
            { DI_TEXT, 5,6, 0,0, 0,0,0,0, GetMsg(MUsername)},
            { DI_TEXT,25,6, 0,0, 0,0,0,0, GetMsg(MPaswd)},
            { DI_EDIT, 5,7,22,0, 0,(DWORD_PTR)_T("ProcessList.Username"),DIF_HISTORY,0,_T("")},
            { DI_PSWEDIT,26,7,42,0, 0,0, 0, 0, _T("")},
            { DI_TEXT, 5,8, 0,0, 0,0,0,0, GetMsg(MEmptyForCurrent)},
        };
        FarDialogItem FarItems[ARRAYSIZE(Items)];
        InitDialogItems(Items, FarItems, ARRAYSIZE(Items));

        //Loop until successful connect or user cancel in dialog
        for(bool stop = false; ; )
        {
          const TCHAR *compname;
#ifndef UNICODE
          if(Info.Dialog(Info.ModuleNumber, -1,-1, 48, 11, _T("Contents"),
                         FarItems, ARRAYSIZE(Items)) == -1) break;
#define _REF  FarItems
#else
          HANDLE hDlg=Info.DialogInit(Info.ModuleNumber, -1,-1, 48, 11, _T("Contents"),
                                      FarItems, ARRAYSIZE(Items),0,0,NULL,0);
          if(hDlg==INVALID_HANDLE_VALUE) break;
#define _REF  hDlg
          if(Info.DialogRun(hDlg)==-1)
            stop = true;
          else
#endif
          if(*(compname=GetPtr(_REF,2))==0 || !lstrcmp(compname, _T("\\\\")))
          {
              //go to local computer
              delete pPerfThread;
              DisconnectWMI();
              pPerfThread = NT ? new PerfThread(*this/*, GetPtr(_REF,7), GetPtr(_REF,8)*/) : 0;
              *HostName = 0;
              stop = true;
          } else if(Connect(compname, GetPtr(_REF,7), GetPtr(_REF,8)))
              stop = true;
#ifdef UNICODE
          Info.DialogFree(hDlg);
#endif
          if(stop) break;
#undef _REF
        }
        Reread();
        return TRUE;
    }
    else if (ControlState==PKF_SHIFT && Key==VK_F6) {
        // go to local host
        delete pPerfThread;
        DisconnectWMI();
        pPerfThread = NT ? new PerfThread(*this) : 0;
        *HostName = 0;
        Reread();
        return TRUE;
    }
#if 0
    else if (ControlState==PKF_ALT && Key==VK_F6) {
        if(!Opt.EnableWMI)
            return TRUE;
        const TCHAR *MsgItems[]={/*GetMsg(MAttachDebugger)*/_T("Attach Debugger"),/*GetMsg(MConfirmAttachDebugger)*/_T("Do you want to attach debugger to this process?"),GetMsg(MYes),GetMsg(MNo)};
        if(Message(0,NULL,MsgItems,ARRAYSIZE(MsgItems),2)!=0)
            return TRUE;
        PanelInfo pi;
        Control(FCTL_GETPANELINFO,&pi);
        PluginPanelItem& item = pi.PanelItems[pi.CurrentItem];
        if(!lstrcmp(item.FindData.cFileName, _T("..")))
            return TRUE;
        int i;
        if(ConnectWMI() && pWMI && item.UserData)
            switch(i=pWMI->AttachDebuggerToProcess(((ProcessData*)item.UserData)->dwPID)) {
                case -1: WmiError(); break;
                case 0: break;
                case 2: SetLastError(i); WinError(); break;
                default:
                    TCHAR buf[80];
                    FSF.sprintf(buf,_T("Return code: %d"), i);
                    const TCHAR *MsgItems[]={/*GetMsg(MAttachDebugger)*/_T("Attach Debugger"),buf,GetMsg(MOk)};
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
    else if(ControlState==PKF_SHIFT && (Key==VK_F1||Key==VK_F2) && (!*HostName||Opt.EnableWMI)) {
        //lower/raise priority class
        PanelInfo PInfo;
#ifndef UNICODE
        Info.Control(this,FCTL_GETPANELINFO,&PInfo);
#else
        Info.Control(this,FCTL_GETPANELINFO,0,(LONG_PTR)&PInfo);
#endif
        if(PInfo.SelectedItemsNumber>1)
        {
            const TCHAR *MsgItems[]={GetMsg(MChangePriority),GetMsg(MConfirmChangePriority),GetMsg(MYes),GetMsg(MNo)};
            if(Message(0,NULL,MsgItems,ARRAYSIZE(MsgItems),2)!=0)
            {
                return TRUE;
            }
        }

        if(*HostName && Opt.EnableWMI && !ConnectWMI())
        {
            WmiError();
            return TRUE;
        }

        bool bNewPri = W2K || *HostName;
        static const USHORT PrClasses[] =
            { IDLE_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS,
              HIGH_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS};
        static const USHORT PrClasses2k[] =
            { IDLE_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS,
              NORMAL_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS,
              HIGH_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS};
        const int N = bNewPri ? ARRAYSIZE(PrClasses2k) : ARRAYSIZE(PrClasses);

        DebugToken token;
        for(int i=0; i<PInfo.SelectedItemsNumber; i++)
        {
#ifndef UNICODE
            PluginPanelItem& Item = PInfo.SelectedItems[i];
#else
            PluginPanelItem* _Item=(PluginPanelItem*)new char[Info.Control(this,FCTL_GETSELECTEDPANELITEM,i,0)];
            PluginPanelItem& Item=*_Item;
            Info.Control(this,FCTL_GETSELECTEDPANELITEM,i,(LONG_PTR)&Item);
#endif
            SetLastError(0);
            if(((ProcessData*)Item.UserData)->dwPID) {

                if(!*HostName) {
                    HANDLE hProcess=OpenProcessForced(&token, PROCESS_QUERY_INFORMATION|PROCESS_SET_INFORMATION,((ProcessData*)Item.UserData)->dwPID);
                    if(hProcess) {
                        DWORD dwPriorityClass = GetPriorityClass(hProcess);
                        if(dwPriorityClass==0)
                            WinError();
                        else {
                            for(int i=0; i<N; i++)
                                if( (bNewPri && dwPriorityClass==PrClasses2k[i]) ||
                                    (!bNewPri && dwPriorityClass==PrClasses[i])) {
                                        bool bChange = false;
                                        if(Key==VK_F1 && i>0) {
                                            i--; bChange = true;
                                        } else if(Key==VK_F2 && i<N-1) {
                                            i++; bChange = true;
                                        }
                                        if(bChange && !SetPriorityClass(hProcess,
                                            bNewPri ? PrClasses2k[i] : PrClasses[i]) )
                                            WinError();
                                        //                  else
                                        //                      Item.Flags &= ~PPIF_SELECTED;
                                        break;
                                    }
                        }
                        CloseHandle(hProcess);
                    }
                    else
                        WinError();

                } else if(pWMI) { //*HostName
                    DWORD dwPriorityClass = pWMI->GetProcessPriority(((ProcessData*)Item.UserData)->dwPID);
                    if(!dwPriorityClass) {
                        WmiError();
                        continue;
                    }
                    static const BYTE Pr2K[ARRAYSIZE(PrClasses2k)] = {4,6,8,10,13,24};
                    for(int i=0; i<N; i++)
                        if( dwPriorityClass==Pr2K[i] ) {
                            bool bChange = false;
                            if(Key==VK_F1 && i>0) {
                                i--; bChange = true;
                            } else if(Key==VK_F2 && i<N-1) {
                                i++; bChange = true;
                            }
                            if(bChange && pWMI->SetProcessPriority(((ProcessData*)Item.UserData)->dwPID,
                                PrClasses2k[i])!=0 )
                                WmiError();
                            break;
                        }
                }
            } // if dwPID
#ifdef UNICODE
            delete [] (char*)_Item;
#endif
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
        if(pPerfThread)
            pPerfThread->SmartReread();
        Reread();
        return TRUE;
        /*  } else if (ControlState==(PKF_ALT|PKF_SHIFT) && Key==VK_F9) {
        Config();
        return TRUE;*/
    } else if(ControlState==PKF_CONTROL && Key==_T('F'))
    {
        PanelInfo pi;
#ifndef UNICODE
        Info.Control(this,FCTL_GETPANELINFO,&pi);
#else
        Info.Control(this,FCTL_GETPANELINFO,0,(LONG_PTR)&pi);
#endif
        if (pi.CurrentItem < pi.ItemsNumber)
        {
#ifndef UNICODE
            ProcessData* pData = (ProcessData *)pi.PanelItems[pi.CurrentItem].UserData;
#else
            PluginPanelItem* PPI=(PluginPanelItem*)new char[Info.Control(this,FCTL_GETPANELITEM,pi.CurrentItem,0)];
            Info.Control(this,FCTL_GETPANELITEM,pi.CurrentItem,(LONG_PTR)PPI);
            ProcessData* pData = (ProcessData *)PPI->UserData;
#endif
            if(pData)
                PutToCmdLine(pData->FullPath);
#ifdef UNICODE
            delete [] (char*)PPI;
#endif
        }
        return TRUE;
    } else if(ControlState==PKF_CONTROL && Key==VK_F12) {

        struct {int id, mode;} StaticItems[] = {
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
#ifndef UNICODE
#define LASTBYTE(_array) ((_array)[sizeof(_array) - 1])
#endif

        int nMoreData = pPerfThread ? ARRAYSIZE(Counters) + 1 : 0;

        PanelInfo pi;
#ifndef UNICODE
        Info.Control(this,FCTL_GETPANELINFO, &pi);
#else
        Info.Control(this,FCTL_GETPANELINFO,0,(LONG_PTR)&pi);
#endif
        TCHAR cIndicator = pi.Flags&PFLAGS_REVERSESORTORDER ? _T('-') : _T('+');

        Array<FarMenuItem> Items(NSTATICITEMS + nMoreData*2);
#ifdef UNICODE
        struct {
            BYTE  m;
            BYTE  a;
        }Flags[ARRAYSIZE(Counters)*2 + NSTATICITEMS + 1] = { {0} };
#endif
        DWORD i;
        for(i=0; i<NSTATICITEMS; i++)
            if(StaticItems[i].id==0)
                Items[i].Separator = 1;
            else {
#ifndef UNICODE
                lstrcpyn(Items[i].Text, GetMsg(StaticItems[i].id), sizeof(Items[0].Text));
                LASTBYTE(Items[i].Text) = StaticItems[i].mode;
#else
                Items[i].Text = GetMsg(StaticItems[i].id);
                Flags[i].m = StaticItems[i].mode;
#endif
                int sm = pi.SortMode;
                if(sm==SM_CTIME)
                    sm = SortMode;
                if(sm==StaticItems[i].mode)
                    Items[i].Checked = cIndicator;
            }
        int nItems = NSTATICITEMS;
        if(pPerfThread) {
            Items[nItems++].Separator = 1;
            const PerfLib* pl = pPerfThread->GetPerfLib();
            for(i=0; i<ARRAYSIZE(Counters); i++)
                if(pl->dwCounterTitles[i]) {
#ifndef UNICODE
                    lstrcpyn(Items[nItems].Text, GetMsg(Counters[i].idName), ARRAYSIZE(Items[0].Text));
                    LASTBYTE(Items[nItems].Text) = (char)(SM_PERFCOUNTER+i);
#else
                    Items[nItems].Text = GetMsg(Counters[i].idName);
                    Flags[nItems].m = (BYTE)(SM_PERFCOUNTER+i);
#endif
                    if(SM_PERFCOUNTER+i==SortMode)
                        Items[nItems].Checked = cIndicator;
                    nItems++;
                    if(CANBE_PERSEC(i)) {
#ifdef UNICODE
                        wchar_t tmpStr[512];
#define PUT tmpStr
#else
#define PUT Items[nItems].Text
#endif
                        if(i<3)
                            FSF.sprintf(PUT, _T("%% %s"), GetMsg(Counters[i].idName));
                        else
                            FSF.sprintf(PUT, _T("%s %s"), GetMsg(Counters[i].idName), GetMsg(MperSec));
#undef PUT
#ifndef UNICODE
                        LASTBYTE(Items[nItems].Text) = (char)((SM_PERFCOUNTER+i) | SM_PERSEC);
#else
                        Items[nItems].Text = wcsdup(tmpStr);
                        Flags[nItems].m = (BYTE)((SM_PERFCOUNTER+i) | SM_PERSEC);
                        Flags[nItems].a = 1;
#endif
                        if(((SM_PERFCOUNTER+i) | SM_PERSEC) == SortMode)
                            Items[nItems].Checked = cIndicator;
                        nItems++;
                    }
                }
        }
        // Show sort menu
        int rc= Menu(FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE, GetMsg(MSortBy), 0, 0, 0, Items, nItems);
        if(rc != -1) {
            unsigned mode;
#ifndef UNICODE
            mode = (BYTE)LASTBYTE(Items[rc].Text);
#else
            mode = Flags[rc].m;
#endif

            SortMode = mode;
            if(mode >= SM_CUSTOM)
                mode = SM_CTIME;
#ifndef UNICODE
            Info.Control(this,FCTL_SETSORTMODE, &mode);
#else
            Info.Control(this,FCTL_SETSORTMODE,mode,NULL);
#endif
            /*
            else if(rc==NSTATICITEMS-2)
            Control(FCTL_SETSORTORDER, (void*)&items[rc].mode);
            else if(rc==NSTATICITEMS-1)
            Control(FCTL_SETSORTMODE, (void*)&items[rc].mode);
            */
        }
#ifdef UNICODE
        while(--nItems > (int)NSTATICITEMS)
            if(Flags[nItems].a) free((wchar_t*)Items[nItems].Text);
#endif
        return TRUE;
    }
    return FALSE;
}

TCHAR *Plist::PrintTitle(int MsgId)
{
    static TCHAR FullStr[256];
    TCHAR Str[256];
    FSF.sprintf(Str,_T("%s:"),GetMsg(MsgId));
    FSF.sprintf(FullStr,_T("%-22s"),Str);
    return FullStr;
}

void Plist::FileTimeToText(FILETIME *CurFileTime,FILETIME *SrcTime,TCHAR *TimeText)
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
    if(FileTimeToSystemTime(&Uptime,&st)) {
        int Days=st.wDay-1;
        for (int I=1;I<st.wMonth;I++)
        {
            static const int MonthDays[12]={31,29,31,30,31,30,31,31,30,31,30,31};
            Days+=MonthDays[I-1];
        }
        if (Days>0)
            FSF.sprintf(TimeText,_T("%d %02d:%02d:%02d"),Days,st.wHour,st.wMinute,st.wSecond);
        else
            FSF.sprintf(TimeText,_T("%02d:%02d:%02d"),st.wHour,st.wMinute,st.wSecond);
    } else // failed
        lstrcpy(TimeText, _T("???"));
}

bool Plist::GetVersionInfo(TCHAR* pFullPath, LPBYTE &pBuffer, TCHAR* &pVersion, TCHAR* &pDesc)
{
#ifndef UNICODE
    static const char SFI[] = "StringFileInfo";
#endif
    static const wchar_t WSFI[] = L"StringFileInfo";

  if(!memcmp(pFullPath,_T("\\??\\"),4*sizeof(TCHAR)))
    pFullPath+=4;
    DWORD size =
#ifndef UNICODE
                 pFullPath[1] ? GetFileVersionInfoSize(pFullPath, &size) :
#endif
                          GetFileVersionInfoSizeW((wchar_t*)pFullPath, &size);
    if(!size) return false;
    pBuffer = new BYTE[size];
#ifndef UNICODE
    if(pFullPath[1])
        GetFileVersionInfo(pFullPath, 0, size, pBuffer);
    else
#endif
        GetFileVersionInfoW((wchar_t*)pFullPath, 0, size, pBuffer);

    //Find StringFileInfo
    DWORD ofs;
    for(ofs = NT ? 92 : 70; ofs < size; ofs += *(WORD*)(pBuffer+ofs) )
        if(
#ifndef UNICODE
            (!NT && !FSF.LStricmp((PCH)pBuffer+ofs+6, SFI)) || NT &&
#endif
                          !lstrcmpiW((wchar_t*)(pBuffer+ofs+6), WSFI))
            break;
    if(ofs >= size) {
        delete pBuffer;
        return false;
    }
    TCHAR *langcode;
#ifndef UNICODE
    char  lcode[10];
#endif
    if(NT) {
#ifndef UNICODE
        WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(pBuffer+ofs+42), -1, lcode, sizeof(lcode), 0,0);
        langcode = lcode;
#else
        langcode = (TCHAR*)(pBuffer + ofs + 42);

#endif
    }
    else
        langcode = (TCHAR*)(pBuffer + ofs + 26);

    TCHAR blockname[48];
    unsigned dsize;

#ifdef UNICODE
#define SFI WSFI
#endif
    FSF.sprintf(blockname, _T("\\%s\\%s\\FileVersion"), SFI, langcode);
    if(!VerQueryValue(pBuffer, blockname, (void**)&pVersion, &dsize))
        pVersion = 0;

    FSF.sprintf(blockname, _T("\\%s\\%s\\FileDescription"), SFI, langcode);
#undef SFI
    if(!VerQueryValue(pBuffer, blockname, (void**)&pDesc, &dsize))
        pDesc = 0;
    return true;
}

void Plist::PrintVersionInfo(HANDLE InfoFile, TCHAR* pFullPath)
{
    TCHAR   *pVersion, *pDesc;
    LPBYTE  pBuf;
    if(!GetVersionInfo(pFullPath, pBuf, pVersion, pDesc))
        return;
    if(pVersion) {
#ifndef UNICODE
        CharToOem(pVersion,pVersion);
#endif
        fprintf(InfoFile,_T("%s %s\n"),PrintTitle(MTitleFileVersion), pVersion);
    }
    if(pDesc) {
#ifndef UNICODE
        CharToOem(pDesc,pDesc);
#endif
        fprintf(InfoFile,_T("%s %s\n"),PrintTitle(MTitleFileDesc), pDesc);
    }

    delete pBuf;
}

bool Plist::ConnectWMI()
{
    if(!Opt.EnableWMI || dwPluginThread != GetCurrentThreadId())
        return false;
    if(!pWMI)
        pWMI = new WMIConnection();
    if(*pWMI)
        return true;
    return pWMI->Connect(HostName, pPerfThread->UserName, pPerfThread->Password);
}

void Plist::DisconnectWMI()
{
    if(pWMI) { delete pWMI; pWMI = 0; }
}

void Plist::PrintOwnerInfo(HANDLE InfoFile, DWORD dwPid)
{
    TCHAR User[MAX_USERNAME_LENGTH];
    TCHAR UserSid[MAX_USERNAME_LENGTH];
    TCHAR Domain[MAX_USERNAME_LENGTH];

    if(!Opt.EnableWMI || !pPerfThread->IsWMIConnected() || !ConnectWMI())
        return;
    if(!*pWMI) // exists, but counld not connect
        return;

    pWMI->GetProcessOwner(dwPid, User, Domain);
    pWMI->GetProcessUserSid(dwPid, UserSid);

    if(*User || *Domain || *UserSid) {
        fprintf(InfoFile,_T("%s "), PrintTitle(MTitleUsername));
        if(*Domain)
            fprintf(InfoFile, _T("%s\\"), Domain);
        if(*User)
            fprintf(InfoFile, _T("%s"), User);
        if(*UserSid)
            fprintf(InfoFile,_T(" (%s)"), UserSid);
        fputc(_T('\n'),InfoFile);
    }

    int nSession = pWMI->GetProcessSessionId(dwPid);
    if(nSession!=-1)
        fprintf(InfoFile,_T("%s %d\n"), PrintTitle(MTitleSessionId), nSession);
}

int Plist::Compare(const PluginPanelItem *Item1, const PluginPanelItem *Item2,
                   unsigned int Mode)
{
    if(Mode!=SM_CTIME || SortMode<SM_CUSTOM )
        return -2;
    int diff;
    switch(SortMode) {
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

        if(data1==0) return data2 ? 1 : 0;
        if(data2==0) return -1;

        bool bPerSec = false;
        unsigned smode = SortMode;
        if(smode >= SM_PERSEC) {
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
    if(diff==0)
        diff = (DWORD)Item1->UserData - (DWORD)Item2->UserData; // unsorted
    return diff<0 ? -1 : diff==0 ? 0 : 1;
}

bool Plist::bInit;
PanelMode Plist::PanelModesLocal[], Plist::PanelModesRemote[];
TCHAR Plist::PanelModeBuffer[];
TCHAR Plist::ProcPanelModesLocal[NPANELMODES][MAX_MODE_STR], Plist::ProcPanelModesRemote[NPANELMODES][MAX_MODE_STR];
/*
bool Plist::PostUpdate()
{
PanelInfo pi;
if(!Control(FCTL_GETPANELINFO, &pi))
return false;

DWORD dwCtrlR = KEY_CTRL | _T('R');
KeySequence ks = { 0, 1, &dwCtrlR };
(*Info.AdvControl)(Info.ModuleNumber, ACTL_POSTKEYSEQUENCE, &ks);
Control(FCTL_REDRAWPANEL, NULL);
return true;
}
*/
void Plist::WmiError()
{
    if(pWMI) {
        HRESULT hr = pWMI->GetLastHResult();
        SetLastError(hr);
        WinError(hr<(HRESULT)0x80040000 ? 0 : (TCHAR *)_T("wbemcomn"));
    }
}
