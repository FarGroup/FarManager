/*
$Id: Pclass.cpp,v 1.2 2003/04/18 11:05:35 yutsis Exp $

$Log: Pclass.cpp,v $
Revision 1.2  2003/04/18 11:05:35  yutsis
Plist::ConnectWMI() works only in the plugin's main thread.

*/

#include "proclist.hpp"
#include <farkeys.hpp>
#include "perfthread.hpp"
#include "proclng.hpp"

class StrTok {
    char* tok;
    char* ptr;
    char* buf;
public:
    StrTok(char* str, char* token) : tok (token) {
        buf = new char[strlen(str)+1];
        strcpy(buf, str);
        ptr = strtok(buf,token);
    }
    operator char* () { return ptr; }
    void operator ++ () { ptr = strtok(NULL, tok); }
    operator bool() { return ptr!=NULL; }
    ~StrTok() { delete buf; }
};

Plist::Plist()
{
    pWMI = 0;
    LastUpdateTime=0;
    bInit = false; // force initialize when opening the panel
    *HostName = 0;
    pPerfThread = 0;
    dwPluginThread = GetCurrentThreadId();
    SortMode = GetRegKey(0,"SortMode", SM_UNSORTED); //SM_CUSTOM;
    StartPanelMode = GetRegKey(0,"StartPanelMode", 1)+'0';

    InitializePanelModes();

    if(NT)
        pPerfThread = new PerfThread(*this);
}

void Plist::InitializePanelModes()
{
    static char StatusWidth9x[] = "0,8,0,5";
    static char StatusWidthNT[] = "0,8,0,5";
    static char StatusCols[] = "N,S,D,T";

    // Default panel modes. Overridable from registry.
    // These modes are translated into PanelModesXX
    static struct { char *Cols, *Width; }
    DefaultModesNT[NPANELMODES] = {
        /*0*/ {"N,X15T,X16T,X17T,X18S", "12,0,0,0,4"}, // I/O
        /*1*/ {"N,XI,XP,X0S,X6", "0,4,2,3,9"}, // General info
        /*2*/ {"N,N","0,0"},// Names only
        /*3*/ {"N,XI,XC,D,T","0,4,4,0,0"},    // Startup: PID/Date/Time
        /*4*/ {"N,XI,X4,X6","0,4,9,9"}, // Memory (basic)
        /*5*/ {"N,XI,X4,X6,X10,X12,X0,X1,X2","12,4,0,0,0,0,8,8,8"},     // Extended Memory/Time
        /*6*/ {"N,ZD","12,0"}, // Descriptions
        /*7*/ {"N,XP,X0S,X1S,X2S,X11S,X14S,X18S","0,2,3,2,2,3,4,3"}, // Dynamic Performance
        /*8*/ {"N,XI,O","0,5,15"}, // Owners (not implemented)
        /*9*/ {"N,XI,XT,X3,XG,XU","0,4,3,4,4,4"} // Resources
    },
        DefaultModes9x[NPANELMODES] = {
            /*0*/ {"N,N","0,0"},// Names only
            /*1*/ {"N,XP,XB,XI", "0,2,2,8"},// General info
            /*2*/ {"N,N","0,0"},// Names only
            /*3*/ {"N,XP,XI,XC,XT","0,2,8,8,3"},
            /*4*/ {"N,S","0,8"},
            /*5*/ {"N,N","0,0"},// Names only
            /*6*/ {"N,ZP","12,0"},
            /*7*/ {"N,ZD","12,0"},
            /*8*/ {"N,ZW","12,0"},
            /*9*/ {"N,XI,XT","0,8,3"}
        },
            DefaultModesRemoteNT[NPANELMODES] = {
                /*0*/ {"N,X15T,X16T,X17T,X18S", "12,0,0,0,4"}, // I/O
                /*1*/ {"N,XI,XP,X0S,X6", "0,4,2,3,9"}, // General info
                /*2*/ {"N,N","0,0"},// Names only
                /*3*/ {"N,XI,XC,D,T","0,4,4,0,0"},    // Startup: PID/Date/Time
                /*4*/ {"N,XI,X4,X6","0,4,9,9"}, // Memory (basic)
                /*5*/ {"N,XI,X4,X6,X10,X12,X0,X1,X2","12,4,0,0,0,0,8,8,8"},     // Extended Memory/Time
                /*6*/ {"N,ZD","12,0"}, // Descriptions
                /*7*/ {"N,XP,X0S,X1S,X2S,X11S,X14S,X18S","0,2,3,2,2,3,4,3"}, // Dynamic Performance
                /*8*/ {"N,XI,O","0,5,15"}, // Owners (not implemented)
                /*9*/ {"N,XI,XT,X3","0,4,3,4"}
            };
#define MAXCOLS MAX_CUSTOM_COLS+4

        char name[20];
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

            wsprintf(name, "Mode%d", iMode);

            GetRegKey(name, "ColumnsLocal", ProcPanelModesLocal[iMode], NT ? DefaultModesNT[iMode].Cols : DefaultModes9x[iMode].Cols, sizeof ProcPanelModesLocal[iMode]);
            GetRegKey(name, "ColumnsRemote", ProcPanelModesRemote[iMode], DefaultModesRemoteNT[iMode].Cols, sizeof ProcPanelModesRemote[iMode]);
            GetRegKey(name, "WidthsLocal", PanelModesLocal[iMode].ColumnWidths, NT ? DefaultModesNT[iMode].Width : DefaultModes9x[iMode].Width, MAX_MODE_STR-1);
            GetRegKey(name, "WidthsRemote", PanelModesRemote[iMode].ColumnWidths, DefaultModesRemoteNT[iMode].Width, MAX_MODE_STR-1);
            GetRegKey(name, "FullScreenLocal", PanelModesLocal[iMode].FullScreen, iMode==5 && NT ? 1 : 0);
            GetRegKey(name, "FullScreenRemote", PanelModesRemote[iMode].FullScreen, iMode==5 ? 1 : 0);

            //Status line is the same for all modes currently and cannot be changed.
            TranslateMode(StatusCols, PanelModesLocal[iMode].StatusColumnTypes);
            TranslateMode(StatusCols, PanelModesRemote[iMode].StatusColumnTypes);
            strcpy(PanelModesLocal[iMode].StatusColumnWidths, NT ? StatusWidthNT : StatusWidth9x);
            strcpy(PanelModesRemote[iMode].StatusColumnWidths, StatusWidthNT);
        }
}

Plist::~Plist()
{
    delete pPerfThread;
    DisconnectWMI();
}

void Plist::SavePanelModes()
{
    char name[20];
    for(int iMode=0; iMode<NPANELMODES; iMode++) {
        wsprintf(name, "Mode%d", iMode);
        SetRegKey(name, "ColumnsLocal", ProcPanelModesLocal[iMode]);
        SetRegKey(name, "ColumnsRemote", ProcPanelModesRemote[iMode]);
        SetRegKey(name, "WidthsLocal", PanelModesLocal[iMode].ColumnWidths);
        SetRegKey(name, "WidthsRemote", PanelModesRemote[iMode].ColumnWidths);
        SetRegKey(name, "FullScreenLocal", PanelModesLocal[iMode].FullScreen);
        SetRegKey(name, "FullScreenRemote", PanelModesRemote[iMode].FullScreen);
    }
}

bool Plist::TranslateMode(char* src, char* dest)
{
    if(!dest) return true;
    if(!src) { *dest=0; return true; }
    int iCustomMode = '0';
    bool bWasDesc = false;
    while(*src) {
        switch(*src&~0x20) {
case 'Z':
    switch(*++src&~0x20) {
case 'P':case 'W':case 'D':case 'C':
    break;
default:
    return false;
    }
    if(bWasDesc)
        return false;
    *dest++ = 'Z';
    bWasDesc = true;
    //*dest++ = iCustomMode++;
    if(*src && *src!=',')
        return false;
    break;
case 'X':
    switch(*++src&~0x20) {
case 'P':case 'I':case 'C'://case 'W':
case 'T':case 'B':case 'G':case 'U':
    src++;
    break;
default:
    char* endptr;
    if(*src<'0' || *src>'9')
        return false;
    strtol(src,&endptr,10);
    if(endptr==src) return false;
    src = endptr;
    }
    *dest++ = 'C';
    *dest++ = iCustomMode++;
    /*if(*src && *src!=',')
    return false;*/
    while(*src && *src!=',') src++;
    break;
default:
    while(*src && *src!=',')
        *dest++ = *src++;
        }
        if(*src==',')
            *dest++ = *src++;
    }
    *dest = 0;
    return true;
}

void Plist::GeneratePanelModes()
{
    for(int iMode=0; iMode<NPANELMODES; iMode++) {
        TranslateMode(ProcPanelModesLocal[iMode], PanelModesLocal[iMode].ColumnTypes);
        TranslateMode(ProcPanelModesRemote[iMode], PanelModesRemote[iMode].ColumnTypes);
        /*TranslateMode(ProcPanelStModesNT[iMode], PanelModesNT[iMode].StatusColumnTypes);
        if(!NT) TranslateMode(ProcPanelStModes9x[iMode], PanelModes9x[iMode].StatusColumnTypes);*/
    }
}

int DescIDs[] = { MColFullPathname, MColumnTitle,
MTitleFileDesc, MCommandLine };

#define CANBE_PERSEC(n) ((n)<3 || (n)==11 || (n)>=14)

void GenerateTitles(char ProcPanelModes[][MAX_MODE_STR], PanelMode* PanelModes, char* Titles[][MAXCOLS])
{
    char buf[80];
    for(int i=0; i<NPANELMODES; i++) {
        if(*ProcPanelModes[i]) {
            strncpy(buf, ProcPanelModes[i], sizeof buf-1);
            buf[sizeof buf-1]=0;
            int ii=0;
            for(StrTok tok(buf,","); tok; ++tok) {
                int id=0;
                switch(*tok&~0x20) {
case 'N': id = MColumnModule; break;
case 'Z':
    switch(tok[1]&~0x20) {
case 'P': id = MTitleFullPath; break;
case 'W': id = MColumnTitle; break;
case 'D': id = MTitleFileDesc; break;
case 'C': id = MCommandLine; break;
    }
    break;
case 'X':
    switch(tok[1]&~0x20) {
case 'P': id = MColumnPriority; break;
case 'I': id = MTitlePID; break;
case 'C': id = MColumnParentPID; break;
    //case 'W': id = ; break;
case 'T': id = MTitleThreads; break;
case 'B': id = MColumnBits; break;
case 'G': id = MColumnGDI; break;
case 'U': id = MColumnUSER; break;
default:
    int n = atoi(&tok[1]);
    if(n>=0 && n<NCOUNTERS) {
        id = Counters[n].idCol;
        if(strpbrk(&tok[1], "Ss") && CANBE_PERSEC(n))
            id++;
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
    static char* TitlesLocal[NPANELMODES][MAXCOLS];
    static char* TitlesRemote[NPANELMODES][MAXCOLS];

    static char* OldMsg0;

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
    Info->StructSize = sizeof *Info;
    Info->Flags = OPIF_ADDDOTS|OPIF_SHOWNAMESONLY;

    Info->CurDir = "";

    static char Title[100];
    if(*HostName)
        wsprintf(Title,"%s: %s ", HostName, GetMsg(MPlistPanel));
    else
        wsprintf(Title," %s ",GetMsg(MPlistPanel));
    Info->PanelTitle=Title;

    Info->PanelModesArray = PanelModes(Info->PanelModesNumber);

    Info->StartPanelMode = StartPanelMode;
    Info->StartSortMode = SortMode >= SM_CUSTOM ? SM_CTIME : SortMode; //SM_UNSORTED;

    static KeyBarTitles keybartitles = {
        { 0, 0, 0, 0, 0, "", "", }, { 0, }, { 0, 0, 0, 0, 0, "",},
        { "", "", "", 0, "", "", "", }
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
            IsIconic(hWnd) && (GetWindowLong(hWnd,GWL_STYLE) & WS_DISABLED)==0 ;
        if(!((EnumWndData*)lParam)->hWnd || bVisible)
            ((EnumWndData*)lParam)->hWnd = hWnd;
        return !bVisible;
    }
    return TRUE;
}

class ui64Table {
    static unsigned _int64 Table[21];
public:
    ui64Table()
    {
        unsigned _int64 n = 1;
        for(int i=0; i<sizeof Table/sizeof *Table; i++,n*=10)
            Table[i] = n;
    }
    static unsigned _int64 tenpow(unsigned n) {
        if(n>=sizeof Table/sizeof *Table)
            n = sizeof Table/sizeof *Table - 1;
        return Table[n];
    }
} _ui64Table;
unsigned _int64 ui64Table::Table[];

void ui64toa_width(unsigned _int64 value, char* buf, unsigned width, bool bThousands)
{
    if(width < 1)
        return;

    char* pSuffix = "";
    unsigned uDivider = bThousands ? 1000 : 1024;
    if(width<=20) {
        if(value >= ui64Table::tenpow(width)) {
            value /= uDivider;
            pSuffix = "K";
        }
        if(value >= ui64Table::tenpow(width)) {
            value /= uDivider;
            pSuffix = "M";
        }
    }
    _ui64toa(value, buf, 10);
    strcat(buf,pSuffix);
}

int Plist::GetFindData(PluginPanelItem*& pPanelItem,int &ItemsNumber,int OpMode)
{
    Lock l(pPerfThread);
    int RetCode = pPerfThread ? GetListNT(pPanelItem,ItemsNumber,*pPerfThread) :
    GetList95(pPanelItem,ItemsNumber);
    if(!RetCode) return FALSE;

    PanelInfo pi;
    Control(FCTL_GETPANELINFO, &pi);

    char (* ProcPanelModes)[MAX_MODE_STR] = *HostName ? ProcPanelModesRemote : ProcPanelModesLocal;
    int cDescMode = 0;
    if(!*HostName) {
        char* p = strchr(ProcPanelModes[pi.ViewMode], 'Z' );
        if(p)
            cDescMode = p[1];
    }
    for(int i = 0; i < ItemsNumber; i++)
    {
        PluginPanelItem &CurItem = pPanelItem[i];
        ProcessData & pdata = *((ProcessData *)CurItem.UserData);

        // Make descriptions
        char Title[NM]; *Title=0;
        char* pDesc="", *pBuf=0;
        EnumWndData ewdata = { pdata.dwPID, 0 };
        EnumWindows((WNDENUMPROC)EnumWndProc, (LPARAM)&ewdata);
        pdata.hwnd = ewdata.hWnd;

        if(cDescMode) {
            switch(cDescMode&~0x20) {
            case 'P':
                if(*pdata.FullPath)
                    pDesc = pdata.FullPath;
                break;
            case 'W':
                if(ewdata.hWnd)
                    GetWindowText(ewdata.hWnd, Title, sizeof Title);
                pDesc = Title;
                break;
            case 'D':
                char *pVersion;
                if(!Plist::GetVersionInfo(pdata.FullPath, pBuf, pVersion, pDesc))
                    pDesc = "";
                break;
            case 'C':
                if(NT)
                    pDesc = ((ProcessDataNT *)CurItem.UserData)->CommandLine;
                break;
            default:
                cDescMode = 0;
            }
            if(cDescMode)
            {
                CurItem.Description = new char[strlen(pDesc)+1];
                strcpy(CurItem.Description, pDesc);
                CharToOem(CurItem.Description, CurItem.Description);
            }
            delete pBuf;
        }
        ProcessPerfData* pd = 0;
        if(pPerfThread)
            pd = pPerfThread->GetProcessData(pdata.dwPID, CurItem.NumberOfLinks);
        int DataOffset = sizeof(char*) * MAX_CUSTOM_COLS;
        int Widths[MAX_CUSTOM_COLS]; memset(Widths, 0, sizeof Widths);

        unsigned uCustomColSize = 0;

        int nCols=0;
        for(StrTok tokn(pi.ColumnWidths, ", "); (bool)tokn && nCols<MAX_CUSTOM_COLS; ++tokn) {
            uCustomColSize += (Widths[nCols++] = atoi(tokn)) + 1;
        }

        if(nCols) {
            CurItem.CustomColumnData = (char**)new char[DataOffset + uCustomColSize];
            char* pData = ((char*)CurItem.CustomColumnData)+DataOffset; // Start offset of column data aftet ptrs

            int nCustomCols;
            nCustomCols = nCols = 0;
            for(StrTok tok(ProcPanelModes[pi.ViewMode], ", "); tok; ++tok, ++nCols) {
                if((*tok&~0x20)=='X') { // Custom column
                    bool bCol = true;
                    DWORD dwData = 0;
                    int nBase = 10;
                    int iCounter = -1;
                    int nColWidth = Widths[nCols];
                    if(nColWidth==0)
                        continue;
                    bool bPerSec = false, bThousands = false;
                    char c = tok[1];
                    switch(c&~0x20) {
                        case 'P': dwData = pdata.dwPrBase; break;
                        case 'I': dwData = pdata.dwPID;
                            if(!pPerfThread) nBase = 16;
                            break;
                        case 'C': dwData = pdata.dwParentPID;
                            if(!pPerfThread) nBase = 16;
                            break;
                            //case 'W': dwData = hwnd; nBase = 16; break;
                        case 'T': dwData = CurItem.NumberOfLinks; break;
                        case 'B': dwData = pdata.uAppType; break;
                        case 'G': if(pd) dwData = pd->dwGDIObjects; break;
                        case 'U': if(pd) dwData = pd->dwUSERObjects; break;
                        default:
                            if(c<'0' || c>'9')
                                bCol = false;
                            else {
                                iCounter = atoi(&tok[1]);
                                if(strpbrk(&tok[1], "Ss") && CANBE_PERSEC(iCounter))
                                    bPerSec = true;
                                if(strpbrk(&tok[1], "Tt"))
                                    bThousands = true;
                            }
                    }
                    if(!bCol)
                        continue;

                    CurItem.CustomColumnData[nCustomCols] = pData;

                    int nBufSize = max( nColWidth+1, 16); // to provide space for itoa
                    Array<char> buf(nBufSize);
                    if(c>='A') // Not a performance counter
                        itoa(dwData, buf, nBase);
                    else if(pd && iCounter>=0) {    // Format performance counters
                        if(iCounter<3 && !bPerSec)  // first 3 are date/time
                            strncpy(buf, PrintTime(pd->qwCounters[iCounter], false), nBufSize-1);
                        else
                            ui64toa_width(bPerSec ? pd->qwResults[iCounter] : pd->qwCounters[iCounter],
                            buf, nColWidth, bThousands);
                    }
                    else
                        *buf=0;
                    int nVisibleDigits = strlen(buf);
                    if(nVisibleDigits > nColWidth) nVisibleDigits = nColWidth;
                    memset(pData,' ',nColWidth - nVisibleDigits);
                    pData += nColWidth-nVisibleDigits;
                    strncpy(pData,buf,nVisibleDigits);
                    pData[nVisibleDigits] = 0;
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
    }
    delete PanelItem;
}

int Plist::GetFiles(PluginPanelItem *PanelItem,int ItemsNumber,
                    int Move,char *DestPath,int OpMode, _Opt& Opt)
{
    if (ItemsNumber==0)
        return 0;

    for (int I=0;I<ItemsNumber;I++)
    {
        PluginPanelItem &CurItem = PanelItem[I];
        ProcessData *pdata=(ProcessData *)CurItem.UserData;
        ProcessDataNT PData;
        if(!pdata) {
            if(!pPerfThread) {
                memset(&PData, 0, sizeof PData);
                PData.dwPID = strtoul(CurItem.FindData.cAlternateFileName, 0, 16);
                if(GetPData95(PData))
                    pdata = &PData;
            }
            else {
                PData.dwPID = atoi(CurItem.FindData.cAlternateFileName);
                ProcessPerfData* ppd = pPerfThread->GetProcessData(PData.dwPID, CurItem.NumberOfLinks);
                if(ppd && GetPDataNT(PData, *ppd))
                    pdata = &PData;
            }
            if(!pdata)
                return 0;
        }
        // may be 0 if called from FindFile
        char FileName[MAX_PATH]; FileName[sizeof FileName-1] = 0;
        strncpy(FileName, DestPath, sizeof FileName-1);
        if(!(OpMode&0x10000)) {
            FSF.AddEndSlash(FileName);
            strncat(FileName,CurItem.FindData.cFileName, sizeof FileName-1);
            strncat(FileName,".txt", sizeof FileName-1);
        }
        // Replace "invalid" chars by underscores
        char* pname = strrchr(FileName, '\\');
        if(!pname) pname = FileName; else pname++;
        static char invalid_chars[] = ":*?\\/\"<>;|";
        for(; *pname; pname++) {
            if(strchr(invalid_chars, *pname) || *pname<' ')
                *pname = '_';
        }

        FILE *InfoFile=fopen(FileName,"wt");
        if (InfoFile==NULL)
            return 0;
        char AppType[100];
        if (!pPerfThread && pdata->uAppType)
            wsprintf(AppType,", %d%s",pdata->uAppType,GetMsg(MBits));
        else
            *AppType=0;

        char ModuleName[MAX_PATH];
        //if(Opt.AnsiOutput)
        //      OemToChar(CurItem.FindData.cFileName, ModuleName);
        //else
        strcpy(ModuleName, CurItem.FindData.cFileName);
        fprintf(InfoFile,"%s %s%s\n",PrintTitle(MTitleModule),ModuleName,AppType);
        if (pdata && pdata->FullPath && *pdata->FullPath) {
            fprintf(InfoFile,"%s %s\n",PrintTitle(MTitleFullPath),(char*)OemString(pdata->FullPath));
            PrintVersionInfo(InfoFile, pdata->FullPath);
        }
        fprintf(InfoFile, pPerfThread ? "%s %d\n":"%s %08X\n",PrintTitle(MTitlePID),pdata->dwPID);
        fprintf(InfoFile, "%s ", PrintTitle(MTitleParentPID));
        if(pPerfThread) {
            Lock l(pPerfThread);
            ProcessPerfData* pParentData = pPerfThread->GetProcessData(pdata->dwParentPID, 0);
            char* pName = pdata->dwParentPID && pParentData ? pParentData->ProcessName : 0;
            fprintf(InfoFile, pName ? "%d  (%s)\n" : "%d\n",pdata->dwParentPID, pName);
        }
        else
            fprintf(InfoFile, "%08X\n",pdata->dwParentPID);

        fprintf(InfoFile,"%s %d\n",PrintTitle(MTitlePriority),pdata->dwPrBase);
        fprintf(InfoFile,"%s %d\n",PrintTitle(MTitleThreads),CurItem.NumberOfLinks);

        if(!pPerfThread) {
            fprintf(InfoFile,"%s %8d\n",PrintTitle(MTitleModuleSize),CurItem.PackSize);
            fprintf(InfoFile,"%s %8d\n",PrintTitle(MTitleHeapSize),GetHeapSize(pdata->dwPID));
        }
        else
            PrintOwnerInfo(InfoFile, pdata->dwPID);

        // Time information

        if(*(ULONGLONG*)&CurItem.FindData.ftCreationTime)
        {
            FILETIME CurFileTime;
            GetSystemTimeAsFileTime(&CurFileTime);

            SYSTEMTIME Current,Compare;
            GetLocalTime(&Current);
            FileTimeToSystemTime(&CurItem.FindData.ftCreationTime,&Compare);
            SystemTimeToTzSpecificLocalTime(NULL,&Compare,&Compare);

            char DateText[MAX_DATETIME],TimeText[MAX_DATETIME];
            ConvertDate(CurItem.FindData.ftCreationTime,DateText,TimeText);

            if (Current.wYear!=Compare.wYear || Current.wMonth!=Compare.wMonth ||
                Current.wDay!=Compare.wDay)
                fprintf(InfoFile,"\n%s %s %s\n",PrintTitle(MTitleStarted),DateText,TimeText);
            else
                fprintf(InfoFile,"\n%s %s\n",PrintTitle(MTitleStarted),TimeText);

            //fprintf(InfoFile,"%s %s\n",PrintTitle(MTitleUptime),PrintNTUptime((void*)CurItem.UserData));
            FileTimeToText  (&CurFileTime,&CurItem.FindData.ftCreationTime,TimeText);
            fprintf(InfoFile,"%s %s\n",PrintTitle(MTitleUptime),TimeText);
        }

        HANDLE hProcess = 0;

        if (NT && !*HostName) // local only
        {
            if (*((ProcessDataNT*)pdata)->CommandLine)
                fprintf(InfoFile, "\n%s:\n%s\n", GetMsg(MCommandLine), (char*)OemString(((ProcessDataNT*)pdata)->CommandLine));

            SetLastError(0);
            hProcess = OpenProcessForced(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|READ_CONTROL,pdata->dwPID);

            if(hProcess) {
                PrintNTCurDirAndEnv(InfoFile, hProcess, Opt.ExportEnvironment);
                CloseHandle(hProcess);
            }

            if (hProcess)
            {
                Lock l(pPerfThread);
                ProcessPerfData& pd = *pPerfThread->GetProcessData(pdata->dwPID, CurItem.NumberOfLinks);
                if(pd.dwGDIObjects)
                    fprintf(InfoFile,"\n%s %d\n",PrintTitle(MGDIObjects), pd.dwGDIObjects);
                if(pd.dwUSERObjects)
                    fprintf(InfoFile,"%s %d\n",PrintTitle(MUSERObjects), pd.dwUSERObjects);
            }
            ChangePrivileges(FALSE,FALSE);
        }// NT && !*HostName

        if(Opt.ExportPerformance && pPerfThread)
            DumpNTCounters(InfoFile, *pPerfThread, pdata->dwPID, CurItem.NumberOfLinks);
        if(!*HostName && pdata->hwnd)
        {
            char Title[NM]; *Title=0;
            GetWindowText(pdata->hwnd, Title, sizeof Title);
            //if(!Opt.AnsiOutput)
            CharToOem(Title,Title);
            fprintf(InfoFile,"\n%s %s\n",PrintTitle(MTitleWindow), Title);
            fprintf(InfoFile,"%-22s %08X\n","HWND:",pdata->hwnd);
            LONG Style=0,ExtStyle=0;
            if (pdata->hwnd!=NULL)
            {
                Style=GetWindowLong(pdata->hwnd,GWL_STYLE);
                ExtStyle=GetWindowLong(pdata->hwnd,GWL_EXSTYLE);
            }

            static int Styles[]={
                WS_POPUP,WS_CHILD,WS_MINIMIZE,WS_VISIBLE,WS_DISABLED,
                    WS_CLIPSIBLINGS,WS_CLIPCHILDREN,WS_MAXIMIZE,WS_BORDER,WS_DLGFRAME,
                    WS_VSCROLL,WS_HSCROLL,WS_SYSMENU,WS_THICKFRAME,WS_MINIMIZEBOX,
                    WS_MAXIMIZEBOX
            };

            static char *StrStyles[]={
                "WS_POPUP","WS_CHILD","WS_MINIMIZE","WS_VISIBLE","WS_DISABLED",
                    "WS_CLIPSIBLINGS","WS_CLIPCHILDREN","WS_MAXIMIZE","WS_BORDER","WS_DLGFRAME",
                    "WS_VSCROLL","WS_HSCROLL","WS_SYSMENU","WS_THICKFRAME","WS_MINIMIZEBOX",
                    "WS_MAXIMIZEBOX"
            };

            static int ExtStyles[]={
                WS_EX_DLGMODALFRAME,WS_EX_NOPARENTNOTIFY,WS_EX_TOPMOST,
                    WS_EX_ACCEPTFILES,WS_EX_TRANSPARENT,WS_EX_MDICHILD,
                    WS_EX_TOOLWINDOW,WS_EX_WINDOWEDGE,WS_EX_CLIENTEDGE,WS_EX_CONTEXTHELP,
                    WS_EX_RIGHT,WS_EX_RTLREADING,WS_EX_LEFTSCROLLBAR,WS_EX_CONTROLPARENT,
                    WS_EX_STATICEDGE,WS_EX_APPWINDOW,
                    0x00080000, 0x00100000L, 0x00400000L, 0x08000000L
                    /*WS_EX_LAYERED,WS_EX_NOINHERITLAYOUT,
                    WS_EX_LAYOUTRTL,WS_EX_NOACTIVATE*/
            };

            static char *StrExtStyles[]={
                "WS_EX_DLGMODALFRAME","WS_EX_NOPARENTNOTIFY","WS_EX_TOPMOST",
                    "WS_EX_ACCEPTFILES","WS_EX_TRANSPARENT","WS_EX_MDICHILD",
                    "WS_EX_TOOLWINDOW","WS_EX_WINDOWEDGE","WS_EX_CLIENTEDGE","WS_EX_CONTEXTHELP",
                    "WS_EX_RIGHT","WS_EX_RTLREADING","WS_EX_LEFTSCROLLBAR","WS_EX_CONTROLPARENT",
                    "WS_EX_STATICEDGE","WS_EX_APPWINDOW","WS_EX_LAYERED","WS_EX_NOINHERITLAYOUT",
                    "WS_EX_LAYOUTRTL","WS_EX_NOACTIVATE"
            };

            char StyleStr[1024], ExtStyleStr[1024];
            *StyleStr = *ExtStyleStr=0;
            int i;
            for (i=0; i<sizeof Styles/sizeof *Styles; i++)
                if (Style & Styles[i])
                {
                    strncat(StyleStr, " ", sizeof StyleStr - 1);
                    strncat(StyleStr, StrStyles[i], sizeof StyleStr - 1);
                }
                for (i=0; i<sizeof ExtStyles/sizeof *ExtStyles; i++)
                    if (Style & ExtStyles[i])
                    {
                        strncat(ExtStyleStr, " ", sizeof ExtStyleStr-1);
                        strncat(ExtStyleStr, StrExtStyles[i], sizeof ExtStyleStr-1);
                    }

                    fprintf(InfoFile,"%-22s %08X %s\n",PrintTitle(MTitleStyle),Style,StyleStr);
                    fprintf(InfoFile,"%-22s %08X %s\n",PrintTitle(MTitleExtStyle),ExtStyle,ExtStyleStr);
        }

        if(!*HostName && Opt.ExportModuleInfo && pdata->dwPID!=8) {
            fprintf(InfoFile,"\n%s\n%s%s\n",  GetMsg(MTitleModules), GetMsg(MColBaseSize),
                Opt.ExportModuleVersion ? GetMsg(MColPathVerDesc) : GetMsg(MColPathVerDescNotShown) );
            if(!NT)
                PrintModules95(InfoFile, pdata->dwPID, Opt);
            else
                PrintModulesNT(InfoFile, pdata->dwPID, Opt);
        }
        if(NT && !*HostName && Opt.ExportHandles && pdata->dwPID /*&& pdata->dwPID!=8*/)
            PrintHandleInfo(pdata->dwPID, InfoFile, (Opt.ExportHandles&2)!=0, pPerfThread);
        fclose(InfoFile);
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
        const char *MsgItems[]={GetMsg(MCannotDeleteProc),GetMsg(MCannotKillRemote),GetMsg(MOk)};
        Message(FMSG_WARNING,NULL,MsgItems,sizeof MsgItems/sizeof *MsgItems);
        return FALSE;
    }

    const char *MsgItems[]={GetMsg(MDeleteTitle),GetMsg(MDeleteProcesses),
        GetMsg(MDeleteDelete),GetMsg(MCancel)};
    char Msg[512];

    if (ItemsNumber==1)
    {
        wsprintf(Msg,GetMsg(MDeleteProcess),PanelItem[0].FindData.cFileName);
        MsgItems[1]=Msg;
    }
    if (Message(0,NULL,MsgItems,sizeof MsgItems/sizeof *MsgItems,2)!=0)
        return FALSE;
    if (ItemsNumber>1)
    {
        char Msg[512];
        wsprintf(Msg,GetMsg(MDeleteNumberOfProcesses),ItemsNumber);
        MsgItems[1]=Msg;
        if (Message(FMSG_WARNING,NULL,MsgItems,sizeof MsgItems/sizeof *MsgItems,2)!=0)
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
            char Msg[512];
            wsprintf(Msg,GetMsg(MCannotDelete),CurItem.FindData.cFileName);
            const char *MsgItems[]={GetMsg(MDeleteTitle),Msg, 0, GetMsg(MOk)};
            int nItems = sizeof MsgItems/sizeof *MsgItems;
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
        Control(FCTL_GETPANELINFO, &pi);
        SetRegKey(0,"StartPanelMode", pi.ViewMode);
        SetRegKey(0,"SortMode", pi.SortMode==SM_CTIME ? SortMode : pi.SortMode);
    }
    if(Event==FE_CHANGEVIEWMODE) {
        if(/*pPerfThread || */strchr((char*)Param,'Z') || strchr((char*)Param,'C'))
            Reread();
    }
    return FALSE;
}

void Plist::Reread()
{
    Control(FCTL_UPDATEPANEL, (void*)1);
    Control(FCTL_REDRAWPANEL, NULL);

    PanelInfo PInfo;
    Control(FCTL_GETANOTHERPANELINFO,&PInfo);
    if (PInfo.PanelType==PTYPE_QVIEWPANEL)
    {
        Control(FCTL_UPDATEANOTHERPANEL,(void *)1);
        Control(FCTL_REDRAWANOTHERPANEL,NULL);
    }
}

void Plist::PutToCmdLine(char* tmp)
{
    unsigned l = strlen(tmp);
    char* tmp1 = 0;
    if(strcspn(tmp," &^")!=l) {
        tmp1 = new char[l+3];
        memcpy(tmp1+1,tmp,l);
        tmp1[0] = tmp1[l+1] = '\"';
        tmp1[l+2] = '\0';
        tmp = tmp1;
    }
    Info.Control(this,FCTL_INSERTCMDLINE, tmp);
    Info.Control(this,FCTL_INSERTCMDLINE, " ");
    delete tmp1;
}

bool Plist::Connect(LPCSTR pMachine, LPCSTR pUser, LPCSTR pPasw)
{
    char Machine[sizeof HostName];
    strncpy(Machine, pMachine, sizeof Machine-1);
    Machine[sizeof Machine-1] = 0;

    //Convert "//" to "\\"
    if(*(short*)Machine == 0x2f2f)
        *(short*)Machine = 0x5c5c;
    // Add "\\" if missing
    if(*(short*)Machine != 0x5c5c)
    {
        memmove(Machine+2, Machine, strlen(Machine)+1);
        *(short*)Machine = 0x5c5c;
    }
    /*
    //Extract the username if any
    char* p;
    if(p=strchr(Machine, '@')) {
    char* user = strtok(Machine+2,"@");
    char* host = strtok(user,":");
    memmove(p, host, strlen(host));
    }
    */

    //Try to connect...
    LPCSTR ConnectItems[] = {"",GetMsg(MConnect)};
    HANDLE hScreen = Info.SaveScreen(0,0,-1,-1);
    Message(0,0,ConnectItems,2,0);
    if(pUser && *pUser) {
        static NETRESOURCE nr = { RESOURCE_GLOBALNET, RESOURCETYPE_DISK, RESOURCEDISPLAYTYPE_SERVER,
            RESOURCEUSAGE_CONTAINER, NULL, NULL, "", NULL};
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
        WinError(0,TRUE);
        Info.RestoreScreen(hScreen);
        delete pNewPerfThread;
    }
    else
    {
        Info.RestoreScreen(hScreen);
        delete pPerfThread;
        DisconnectWMI();
        pPerfThread = pNewPerfThread;
        strcpy(HostName, Machine);
        return true;
    }
    return false;
}

int Plist::ProcessKey(int Key,unsigned int ControlState)
{
    if (ControlState==PKF_CONTROL && Key=='R')
    {
        if(pPerfThread)
            pPerfThread->SmartReread();
        return FALSE;
    }
    if (ControlState==0 && Key==VK_RETURN)
    {
        //check for the command line; if it's not empty, don't process Enter
        char CmdLine[1024];

        Control(FCTL_GETCMDLINE, CmdLine);
        if(*CmdLine)
            return FALSE;

        PanelInfo PInfo;
        Control(FCTL_GETPANELINFO,&PInfo);
        if (PInfo.CurrentItem < PInfo.ItemsNumber)
        {
            PluginPanelItem& CurItem = PInfo.PanelItems[PInfo.CurrentItem];
            if (!CurItem.UserData)
                return FALSE;
            HWND hWnd = ((ProcessData *)CurItem.UserData)->hwnd;
            if (hWnd!=NULL && (IsWindowVisible(hWnd) ||
                IsIconic(hWnd) && (GetWindowLong(hWnd,GWL_STYLE) & WS_DISABLED)==0)
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
                    SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)dwMs, 0);

                WINDOWPLACEMENT wp;
                wp.length = sizeof wp;
                if (!GetWindowPlacement(hWnd,&wp) || wp.showCmd!=SW_SHOWMAXIMIZED)
                    ShowWindowAsync(hWnd,SW_RESTORE);
            }
        }
        return TRUE;
    }
    else if (ControlState==PKF_SHIFT && Key==VK_F3)
    {
        PanelInfo pi;
        Control(FCTL_GETPANELINFO,&pi);
        if (pi.CurrentItem >= pi.ItemsNumber ||
            !strcmp(pi.PanelItems[pi.CurrentItem].FindData.cFileName, ".."))
            return TRUE;

        InitDialogItem InitItems[]={ DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(char *)MViewWithOptions, };
        FarDialogItem DialogItems[NVIEWITEMS + 1];
        InitDialogItems(InitItems,DialogItems,sizeof InitItems/sizeof *InitItems);
        _Opt LocalOpt = Opt;
        MakeViewOptions(DialogItems+1, LocalOpt, 2);
        int ExitCode = Info.Dialog(Info.ModuleNumber,-1,-1,76,NVIEWITEMS+3,"Config",DialogItems,sizeof DialogItems/sizeof *DialogItems);
        if(ExitCode==-1)
            return TRUE;
        GetViewOptions(DialogItems+1, LocalOpt);

        char FileName[MAX_PATH];
        FSF.MkTemp(FileName, "prc");

        if(!GetFiles(pi.PanelItems + pi.CurrentItem, 1, 0, FileName, OPM_VIEW|0x10000, LocalOpt))
            return TRUE;
        Info.Viewer (FileName, pi.PanelItems[pi.CurrentItem].FindData.cFileName, 0,0,-1,-1, VF_NONMODAL|VF_DELETEONCLOSE);
        return TRUE;
    }
    else if (ControlState==0 && Key==VK_F6)
    {
        InitDialogItem Items[] = {
            { DI_DOUBLEBOX, 3,1, 44,9, 0,0,0,0, GetMsg(MSelectComputer)},
            { DI_TEXT, 5,2, 0,0, 0,0,0,0, GetMsg(MComputer)},
            { DI_EDIT, 5,3, 42,0, 1,(int)"ProcessList.Computer",DIF_HISTORY,1,HostName},
            { DI_TEXT, 5,4, 0,0, 0,0,0,0, GetMsg(MEmptyForLocal)},
            { DI_TEXT, 0,5, 0,0, 0,0,DIF_SEPARATOR,0, ""},
            { DI_TEXT, 5,6, 0,0, 0,0,0,0, GetMsg(MUsername)},
            { DI_TEXT,25,6, 0,0, 0,0,0,0, GetMsg(MPaswd)},
            { DI_EDIT, 5,7,22,0, 0,(int)"ProcessList.Username",DIF_HISTORY,0,""},
            { DI_PSWEDIT,26,7,42,0, 0,0, 0, 0, ""},
            { DI_TEXT, 5,8, 0,0, 0,0,0,0, GetMsg(MEmptyForCurrent)},
        };
        FarDialogItem FarItems[sizeof Items / sizeof *Items];
        InitDialogItems(Items, FarItems, sizeof Items / sizeof *Items);

        //Loop until successful connect or user cancel in dialog
        while(Info.Dialog(Info.ModuleNumber, -1,-1, 48, 11, "Contents",
            FarItems, sizeof Items / sizeof *Items)!=-1)
        {
            if(*FarItems[2].Data==0 || !strcmp(FarItems[2].Data,"\\\\"))
            {
                //go to local computer
                delete pPerfThread;
                DisconnectWMI();
                pPerfThread = NT ? new PerfThread(*this/*, FarItems[7].Data, FarItems[8].Data*/) : 0;
                *HostName = 0;
                break;
            }
            if(Connect(FarItems[2].Data, FarItems[7].Data, FarItems[8].Data))
                break;
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
        const char *MsgItems[]={/*GetMsg(MAttachDebugger)*/"Attach Debugger",/*GetMsg(MConfirmAttachDebugger)*/"Do you want to attach debugger to this process?",GetMsg(MYes),GetMsg(MNo)};
        if(Message(0,NULL,MsgItems,sizeof MsgItems/sizeof *MsgItems,2)!=0)
            return TRUE;
        PanelInfo pi;
        Control(FCTL_GETPANELINFO,&pi);
        PluginPanelItem& item = pi.PanelItems[pi.CurrentItem];
        if(!strcmp(item.FindData.cFileName, ".."))
            return TRUE;
        int i;
        if(ConnectWMI() && pWMI && item.UserData)
            switch(i=pWMI->AttachDebuggerToProcess(((ProcessData*)item.UserData)->dwPID)) {
                case -1: WmiError(); break;
                case 0: break;
                case 2: SetLastError(i); WinError(); break;
                default:
                    char buf[80];
                    sprintf(buf,"Return code: %d", i);
                    const char *MsgItems[]={/*GetMsg(MAttachDebugger)*/"Attach Debugger",buf,GetMsg(MOk)};
                    Message(FMSG_WARNING,0,MsgItems,sizeof MsgItems/sizeof *MsgItems);
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
        Control(FCTL_GETPANELINFO,&PInfo);

        if(PInfo.SelectedItemsNumber>1)
        {
            const char *MsgItems[]={GetMsg(MChangePriority),GetMsg(MConfirmChangePriority),GetMsg(MYes),GetMsg(MNo)};
            if(Message(0,NULL,MsgItems,sizeof MsgItems/sizeof *MsgItems,2)!=0)
                return TRUE;
        }

        if(*HostName && Opt.EnableWMI && !ConnectWMI())
        {
            WmiError();
            return TRUE;
        }

        bool bNewPri = W2K || *HostName;
        static USHORT PrClasses[] = { IDLE_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS,
            HIGH_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS};
        static USHORT PrClasses2k[] = { IDLE_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS,
            NORMAL_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS, HIGH_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS};
        const int N = bNewPri ? sizeof PrClasses2k/sizeof *PrClasses2k : sizeof PrClasses/sizeof *PrClasses;

        for(int i=0; i<PInfo.SelectedItemsNumber; i++)
        {

            PluginPanelItem& Item = PInfo.SelectedItems[i];
            SetLastError(0);
            if(((ProcessData*)Item.UserData)->dwPID) {

                if(!*HostName) {
                    HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_SET_INFORMATION,FALSE,((ProcessData*)Item.UserData)->dwPID);
                    if(GetLastError()==ERROR_ACCESS_DENIED && ChangePrivileges(TRUE,FALSE))
                        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_SET_INFORMATION, FALSE, ((ProcessData*)Item.UserData)->dwPID);
                    if(hProcess) {
                        DWORD dwPriorityClass = GetPriorityClass(hProcess);
                        if(dwPriorityClass==0)
                            WinError();
                        else {
                            for(int i=0; i<N; i++)
                                if( bNewPri && dwPriorityClass==PrClasses2k[i] ||
                                    !bNewPri && dwPriorityClass==PrClasses[i]) {
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
                    static BYTE Pr2K[sizeof PrClasses2k/sizeof *PrClasses2k] = {4,6,8,10,13,24};
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
        }
        ChangePrivileges(FALSE,FALSE);
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
    } else if(ControlState==PKF_CONTROL && Key=='F')
    {
        PanelInfo pi;
        Control(FCTL_GETPANELINFO,&pi);
        if (pi.CurrentItem < pi.ItemsNumber)
        {
            ProcessData* pData = (ProcessData *)pi.PanelItems[pi.CurrentItem].UserData;
            if(pData)
                PutToCmdLine(pData->FullPath);
        }
        return TRUE;
    } else if(ControlState==PKF_CONTROL && Key==VK_F12) {

        struct {int id, mode;} StaticItems[] = {
            MSortByName,SM_NAME, MSortByExt,SM_EXT,  MSortByTime,SM_MTIME,
                MSortBySize,SM_SIZE, MSortByUnsorted,SM_UNSORTED,
                MSortByDescriptions,SM_DESCR,
                MSortByOwner,SM_OWNER,
                //      MPageFileBytes,SM_COMPRESSEDSIZE,
                MTitlePID,SM_PID,
                MTitleParentPID,SM_PARENTPID,
                MTitleThreads,SM_NUMLINKS,
                MTitlePriority,SM_PRIOR,
                //      0,-1,
                //      MUseSortGroups,0, MShowSelectedFirst,-1
        };

#define NSTATICITEMS (sizeof StaticItems/sizeof *StaticItems)
#define LASTBYTE(_array) ((_array)[sizeof(_array) - 1])

        int nMoreData = pPerfThread ? sizeof Counters/sizeof *Counters + 1 : 0;

        PanelInfo pi;
        Control(FCTL_GETPANELINFO, &pi);
        char cIndicator = pi.Flags&PFLAGS_REVERSESORTORDER ? '-' : '+';

        Array<FarMenuItem> Items(NSTATICITEMS + nMoreData*2);
        int i;
        for(i=0; i<NSTATICITEMS; i++)
            if(StaticItems[i].id==0)
                Items[i].Separator = 1;
            else {
                strncpy(Items[i].Text, GetMsg(StaticItems[i].id), sizeof Items[0].Text);
                LASTBYTE(Items[i].Text) = StaticItems[i].mode;
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
                for(i=0; i<sizeof Counters/sizeof *Counters; i++)
                    if(pl->dwCounterTitles[i]) {
                        strncpy(Items[nItems].Text, GetMsg(Counters[i].idName), sizeof Items[0].Text);
                        LASTBYTE(Items[nItems].Text) = SM_PERFCOUNTER+i;
                        if(SM_PERFCOUNTER+i==(int)SortMode)
                            Items[nItems].Checked = cIndicator;
                        nItems++;
                        if(CANBE_PERSEC(i)) {
                            if(i<3)
                                wsprintf(Items[nItems].Text, "%% %s", GetMsg(Counters[i].idName));
                            else
                                wsprintf(Items[nItems].Text, "%s %s", GetMsg(Counters[i].idName), GetMsg(MperSec));
                            LASTBYTE(Items[nItems].Text) = (SM_PERFCOUNTER+i) | SM_PERSEC;
                            if(((SM_PERFCOUNTER+i) | SM_PERSEC) == (int)SortMode)
                                Items[nItems].Checked = cIndicator;
                            nItems++;
                        }
                    }
            }
            // Show sort menu
            int rc= Menu(FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE, GetMsg(MSortBy), 0, 0, 0, Items, nItems);
            if(rc==-1) return TRUE;

            unsigned mode = (BYTE)LASTBYTE(Items[rc].Text);
            SortMode = mode;
            if(mode >= SM_CUSTOM)
                mode = SM_CTIME;
            Control(FCTL_SETSORTMODE, &mode);
            /*
            else if(rc==NSTATICITEMS-2)
            Control(FCTL_SETSORTORDER, (void*)&items[rc].mode);
            else if(rc==NSTATICITEMS-1)
            Control(FCTL_SETSORTMODE, (void*)&items[rc].mode);
            */
            return TRUE;
    }
    return FALSE;
}

char *Plist::PrintTitle(int MsgId)
{
    static char FullStr[256];
    char Str[256];
    wsprintf(Str,"%s:",GetMsg(MsgId));
    wsprintf(FullStr,"%-22s",Str);
    return FullStr;
}

void Plist::FileTimeToText(FILETIME *CurFileTime,FILETIME *SrcTime,char *TimeText)
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
            static int MonthDays[12]={31,29,31,30,31,30,31,31,30,31,30,31};
            Days+=MonthDays[I-1];
        }
        if (Days>0)
            wsprintf(TimeText,"%d %02d:%02d:%02d",Days,st.wHour,st.wMinute,st.wSecond);
        else
            wsprintf(TimeText,"%02d:%02d:%02d",st.wHour,st.wMinute,st.wSecond);
    } else // failed
        strcpy(TimeText, "???");
}

bool Plist::GetVersionInfo(char* pFullPath, char* &pBuffer, char* &pVersion, char* &pDesc)
{
    static char SFI[] = "StringFileInfo";
    static wchar_t WSFI[] = L"StringFileInfo";

    if(*(DWORD*)pFullPath==0x5C3F3F5C) /* \??\ */
        pFullPath += 4;
    if(*(LONGLONG*)pFullPath==0x005C003F003F005C) /* \??\ */
        pFullPath += 8;
    DWORD size = pFullPath[1] ? GetFileVersionInfoSize(pFullPath, &size) :
    GetFileVersionInfoSizeW((wchar_t*)pFullPath, &size);
    if(!size) return false;
    pBuffer = new char[size];
    if(pFullPath[1])
        GetFileVersionInfo(pFullPath, 0, size, pBuffer);
    else
        GetFileVersionInfoW((wchar_t*)pFullPath, 0, size, pBuffer);

    //Find StringFileInfo
    DWORD ofs;
    for(ofs = NT ? 92 : 70; ofs < size; ofs += *(WORD*)(pBuffer+ofs) )
        if( !NT && !stricmp(pBuffer+ofs+6, SFI) || NT && !wcsicmp((wchar_t*)(pBuffer+ofs+6), WSFI))
            break;
    if(ofs >= size) {
        delete pBuffer;
        return false;
    }
    char* langcode, lcode[10];
    if(NT) {
        WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(pBuffer+ofs+42), -1, lcode, sizeof lcode, 0,0);
        langcode = lcode;
    }
    else
        langcode = pBuffer + ofs + 26;

    char blockname[48];
    unsigned dsize;

    wsprintf(blockname, "\\%s\\%s\\FileVersion", SFI, langcode);
    if(!VerQueryValue(pBuffer, blockname, (void**)&pVersion, &dsize))
        pVersion = 0;

    wsprintf(blockname, "\\%s\\%s\\FileDescription", SFI, langcode);
    if(!VerQueryValue(pBuffer, blockname, (void**)&pDesc, &dsize))
        pDesc = 0;
    return true;
}

void Plist::PrintVersionInfo(FILE* InfoFile, char* pFullPath)
{
    char *pBuf, *pVersion, *pDesc;
    if(!GetVersionInfo(pFullPath, pBuf, pVersion, pDesc))
        return;
    if(pVersion) {
        //if(!Opt.AnsiOutput)
        CharToOem(pVersion,pVersion);
        fprintf(InfoFile,"%s %s\n",PrintTitle(MTitleFileVersion), pVersion);
    }
    if(pDesc) {
        //if(!Opt.AnsiOutput)
        CharToOem(pDesc,pDesc);
        fprintf(InfoFile,"%s %s\n",PrintTitle(MTitleFileDesc), pDesc);
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

void Plist::PrintOwnerInfo(FILE* InfoFile, DWORD dwPid)
{
    char User[MAX_USERNAME_LENGTH];
    char UserSid[MAX_USERNAME_LENGTH];
    char Domain[MAX_USERNAME_LENGTH];

    if(!Opt.EnableWMI || !pPerfThread->IsWMIConnected() || !ConnectWMI())
        return;
    if(!*pWMI) // exists, but counld not connect
        return;

    pWMI->GetProcessOwner(dwPid, User, Domain);
    pWMI->GetProcessUserSid(dwPid, UserSid);

    if(*User || *Domain || *UserSid) {
        fprintf(InfoFile,"%s ", PrintTitle(MTitleUsername));
        if(*Domain)
            fprintf(InfoFile, "%s\\", Domain);
        if(*User)
            fprintf(InfoFile, User);
        if(*UserSid)
            fprintf(InfoFile," (%s)", UserSid);
        fputc('\n',InfoFile);
    }

    int nSession = pWMI->GetProcessSessionId(dwPid);
    if(nSession!=-1)
        fprintf(InfoFile,"%s %d\n", PrintTitle(MTitleSessionId), nSession);
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
        LONGLONG diff = bPerSec ? data2->qwResults[i] - data1->qwResults[i] :
        data2->qwCounters[i] - data1->qwCounters[i];
        return diff<0 ? -1 : diff==0 ? 0 : 1;
    }
    }
    if(diff==0)
        diff = (DWORD)Item1->UserData - (DWORD)Item2->UserData; // unsorted
    return diff<0 ? -1 : diff==0 ? 0 : 1;
}

bool Plist::bInit;
PanelMode Plist::PanelModesLocal[], Plist::PanelModesRemote[];
char Plist::PanelModeBuffer[];
char Plist::ProcPanelModesLocal[NPANELMODES][MAX_MODE_STR], Plist::ProcPanelModesRemote[NPANELMODES][MAX_MODE_STR];
/*
bool Plist::PostUpdate()
{
PanelInfo pi;
if(!Control(FCTL_GETPANELINFO, &pi))
return false;

DWORD dwCtrlR = KEY_CTRL | 'R';
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
        WinError(hr<0x80040000 ? 0 : "wbemcomn");
    }
}
