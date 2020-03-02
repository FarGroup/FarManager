
/*
  CUSTOM.CPP

  Second-level plugin module for FAR Manager and MultiArc plugin

  Copyright (c) 1996 Eugene Roshal
  Copyrigth (c) 2000 FAR group
*/

#include <windows.h>
#include <string.h>
#include <dos.h>
#include <plugin.hpp>
#include "fmt.hpp"

#define PCRE_STATIC
#include "pcre++.h"
using namespace PCRE;

#undef isspace
#define isspace(c) ((c)==' ' || (c)=='\t')
#undef isdigit
#define isdigit(c) ((c) >= '0' && (c) <= '9')
#undef isxdigit
#define isxdigit(c) (((c)>='0' && (c)<='9') || ((c)>='a' && (c)<='f') || ((c)>='A' && (c)<='F'))

///////////////////////////////////////////////////////////////////////////////
// Forward declarations

BOOL WINAPI OpenArchivePipe(const char *Name, int *Type);
int GetString(char *Str, int MaxSize);
int HexCharToNum(int HexChar);
int GetSectionName(int Num, char *Name, int MaxSize);

DWORD GetIniString(LPCTSTR lpAppName,LPCTSTR lpKeyName,LPCTSTR lpDefault,LPTSTR lpReturnedString,DWORD nSize);
UINT GetIniInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,INT lpDefault);

void FillFormat(const char *TypeName);
void MakeFiletime(SYSTEMTIME st, SYSTEMTIME syst, LPFILETIME pft);
int StringToInt(const char *str);
__int64 StringToInt64(const char *str);
int StringToIntHex(const char *str);
void ParseListingItemRegExp(Match match,
    PluginPanelItem *Item, ArcItemInfo *Info,
    SYSTEMTIME &stModification, SYSTEMTIME &stCreation, SYSTEMTIME &stAccess);
void ParseListingItemPlain(const char *CurFormat, const char *CurStr,
    PluginPanelItem *Item, ArcItemInfo *Info,
    SYSTEMTIME &stModification, SYSTEMTIME &stCreation, SYSTEMTIME &stAccess);

///////////////////////////////////////////////////////////////////////////////
// Constants

enum { PROF_STR_LEN = 256 };


///////////////////////////////////////////////////////////////////////////////
// Class StringList

class StringList
{
public:
    StringList()
        :   pNext(0)
    {
        str[0] = '\0';
    }

    ~StringList()
    {
        delete pNext;
    }

    StringList *Add()
    {
        delete pNext;
        pNext = new StringList;
        return pNext;
    }

    StringList *Next()
    {
        return pNext;
    }

    char *Str()
    {
        return str;
    }

    void Empty()
    {
        delete pNext;
        pNext = 0;
        str[0] = '\0';
    }

    void *operator new(size_t nSize)
    {
        return HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, nSize);
    }

    void operator delete(void *p)
    {
        HeapFree(GetProcessHeap(), 0, p);
    }

protected:
    char str[PROF_STR_LEN];
    StringList *pNext;
};


///////////////////////////////////////////////////////////////////////////////
// Class MetaReplacer

class MetaReplacer
{
    char *m_command;
    char m_fileName[NM], m_shortName[NM];

    class Meta
    {
    public:
        enum Type
        {
            invalidType = -1,
            archiveName,
            shortArchiveName
        };

        enum Flags
        {
            quoteWithSpaces = 1,
            quoteAll = 2,
            useForwardSlashes = 4,
            useNameOnly = 8,
            usePathOnly = 16,
            useANSI = 32
        };

    private:
        bool m_isValid;
        Type m_type;
        unsigned int m_flags;
        int m_length;

    public:
        Meta(const char *start)
            :   m_isValid(false), m_type(invalidType), m_flags(0), m_length(0)
        {
            if((start[0] != '%') || (start[1] != '%'))
                return;

            char typeChars[] = { 'A', 'a' };
            char flagChars[] = { 'Q', 'q', 'S', 'W', 'P', 'A' };

            for(size_t i = 0; i < sizeof(typeChars); ++i)
                if(start[2] == typeChars[i])
                    m_type = (Type) i;

            if(m_type == invalidType)
                return;

            const char *p = start + 3;

            for(; *p; ++p)
            {
                bool isFlagChar = false;
                for(size_t i = 0; i < sizeof(flagChars); ++i)
                    if(*p == flagChars[i])
                    {
                        m_flags |= 1 << i;
                        isFlagChar = true;
                    }

                if(!isFlagChar)
                    break;
            }

            m_length = (int)(p - start);

            m_isValid = true;

        }

        bool isValidMeta() const
        {
            return m_isValid;
        }

        Type getType() const
        {
            return m_type;
        }

        unsigned int getFlags() const
        {
            return m_flags;
        }

        int getLength() const
        {
            return m_length;
        }
    };

    static void convertNameToShort(const char *Src, char *Dest)
    {
        char ShortName[NM], AnsiName[NM];
        int AnsiApis = AreFileApisANSI();

        if(!AnsiApis)
            SetFileApisToANSI();

        OemToChar(Src, AnsiName);

        if(GetShortPathName(AnsiName, ShortName, sizeof(ShortName)) && *ShortName)
            CharToOem(ShortName, Dest);
        else
            lstrcpy(Dest, Src);

        if(!AnsiApis)
            SetFileApisToOEM();
    }

  public:
    MetaReplacer(const char *command, const char *arcName)
        :   m_command(strdup(command))
    {
        lstrcpyn(m_fileName, arcName, sizeof(m_fileName));
        convertNameToShort(m_fileName, m_shortName);
    }

    virtual ~MetaReplacer()
    {
        free(m_command);
    }

    void replaceTo(char *buffer)
    {
        char *dest = buffer;

        *dest = 0;

        bool bReplacedSomething = false;

        for(const char *command = m_command; *command;)
        {
            Meta m(command);

            if(!m.isValidMeta())
            {
                *dest++ = *command++;
                *dest = 0;
                continue;
            }

            command += m.getLength();
            bReplacedSomething = true;

            char *var = (m.getType() == Meta::archiveName) ? m_fileName : m_shortName;
            char *lastSlash = strrchr(var, '\\');

            if(m.getFlags() & Meta::useNameOnly)
            {
                var = lastSlash ? lastSlash + 1 : var;
            }
            else if(m.getFlags() & Meta::usePathOnly)
            {
                if(lastSlash)
                    *lastSlash = 0;
            }

            bool bQuote = (m.getFlags() & Meta::quoteAll)
                || ((m.getFlags() & Meta::quoteWithSpaces) && strchr(var, ' '));

            if(bQuote)
                lstrcat(dest, "\"");

            lstrcat(dest, var);

            if(bQuote)
                lstrcat(dest, "\"");

            if(m.getFlags() & Meta::useForwardSlashes)
            {
                for(; *dest; ++dest)
                    if(*dest == '\\')
                        *dest = '/';
            }


            if(lastSlash)
                *lastSlash = '\\';

            while(*dest)
                dest++;

        }

        if(!bReplacedSomething) // there were no meta-symbols, should use old-style method
        {
            lstrcat(buffer, " ");
            lstrcat(buffer, m_shortName);
        }

    }
};


///////////////////////////////////////////////////////////////////////////////
// Variables

int     CurType;
char    *OutData;
DWORD   OutDataPos, OutDataSize;

char    FormatFileName[NM],UserFormatFileName[NM];


char    StartText[PROF_STR_LEN], EndText[PROF_STR_LEN];

StringList *Format = 0;
StringList *IgnoreStrings = 0;

int     IgnoreErrors;
int     ArcChapters;

const char Str_TypeName[] = "TypeName";


///////////////////////////////////////////////////////////////////////////////
// Library function pointers

FARSTDLOCALSTRICMP  LStricmp;
FARSTDLOCALSTRNICMP LStrnicmp;
FARSTDSPRINTF       SPrintf;
FARSTDMKTEMP        MkTemp;
FARSTDLOCALUPPER    LUpper;


///////////////////////////////////////////////////////////////////////////////
// Exported functions

void WINAPI _export SetFarInfo(const PluginStartupInfo *Info)
{
    LStricmp = Info->FSF->LStricmp;
    LStrnicmp = Info->FSF->LStrnicmp;
    SPrintf = Info->FSF->sprintf;
    MkTemp = Info->FSF->MkTemp;
    LUpper = Info->FSF->LUpper;
}

DWORD WINAPI _export LoadFormatModule(const char *ModuleName)
{
    lstrcpy(FormatFileName, ModuleName);
    lstrcpy(strrchr(FormatFileName, '\\') + 1, "custom.ini");
    lstrcpy(UserFormatFileName,FormatFileName);
    lstrcpy(strrchr(UserFormatFileName,'.'),"_user.ini");
    return (0);
}

BOOL WINAPI _export IsArchive(const char *Name, const unsigned char *Data, int DataSize)
{
    char *Dot = strrchr((char *) Name, '.');

    for(int I = 0;; I++)
    {
        char TypeName[NM], Name[NM], Ext[NM], ID[512];
        int IDPos;

        if(!GetSectionName(I, TypeName, sizeof(TypeName)))
            break;

        GetIniString(TypeName, Str_TypeName, TypeName, Name, sizeof(Name));

        if(*Name == 0)
            break;

        GetIniString(TypeName, "ID", "", ID, sizeof(ID));
        IDPos = GetIniInt(TypeName, "IDPos", -1);

        if(*ID)
        {
            unsigned char IDData[256], *CurID = (unsigned char *) &ID[0];
            int IDLength = 0;

            while(1)
            {
                while(isspace(*CurID))
                    CurID++;
                if(*CurID == 0)
                    break;
                IDData[IDLength++] = HexCharToNum(CurID[0]) * 16 + HexCharToNum(CurID[1]);
                while(*CurID && !isspace(*CurID))
                    CurID++;
            }

            int Found = FALSE;

            if(IDPos >= 0)
                Found = (IDPos <= DataSize - IDLength) && (memcmp(Data + IDPos, IDData, IDLength) == 0);
            else
            {
                for(int I = 0; I <= DataSize - IDLength; I++)
                    if(memcmp(Data + I, IDData, IDLength) == 0)
                    {
                        Found = TRUE;
                        break;
                    }
            }
            if(Found)
            {
                if(GetIniInt(TypeName, "IDOnly", 0))
                {
                    CurType = I;
                    return (TRUE);
                }
            }
            else
                continue;
        }

        GetIniString(TypeName, "Extension", "", Ext, sizeof(Ext));

        if(Dot != NULL && *Ext != 0 && LStricmp(Dot + 1, Ext) == 0)
        {
            CurType = I;
            return (TRUE);
        }
    }
    return (FALSE);
}

DWORD WINAPI _export GetSFXPos(void)
{
    return 0;
}


BOOL WINAPI _export OpenArchive(const char *Name, int *Type)
{
    char TypeName[NM], Command[512];

    if(!GetSectionName(CurType, TypeName, sizeof(TypeName)))
        return (FALSE);

    GetIniString(TypeName, "List", "", Command, sizeof(Command));

    if(*Command == 0)
        return (FALSE);

    IgnoreErrors = GetIniInt(TypeName, "IgnoreErrors", 0);
    *Type = CurType;

    ArcChapters = -1;

    MetaReplacer meta(Command, Name);

    meta.replaceTo(Command);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    HANDLE StdInput = GetStdHandle(STD_INPUT_HANDLE);

    char TempName[NM];

    if(MkTemp(TempName, "FAR") == NULL)
        return (FALSE);

    HANDLE OutHandle = CreateFile(TempName, GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);

    if(OutHandle == INVALID_HANDLE_VALUE)
        return (FALSE);

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = StdInput;
    si.hStdOutput = OutHandle;
    si.hStdError = /*GetStdHandle(STD_ERROR_HANDLE) */ OutHandle;
    DWORD ConsoleMode;

    GetConsoleMode(StdInput, &ConsoleMode);
    SetConsoleMode(StdInput, ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT |
                   ENABLE_ECHO_INPUT | ENABLE_MOUSE_INPUT);
    char SaveTitle[512];

    GetConsoleTitle(SaveTitle, sizeof(SaveTitle));
    SetConsoleTitle(Command);

    char ExpandedCmd[512];

    ExpandEnvironmentStrings(Command, ExpandedCmd, sizeof(ExpandedCmd));

    DWORD ExitCode = CreateProcess(NULL, ExpandedCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

    if(ExitCode)
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &ExitCode);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        ExitCode = (ExitCode < GetIniInt(TypeName, "Errorlevel", 1000));
    }

    if(ExitCode)
    {
        OutData = NULL;
        DWORD FileSize = GetFileSize(OutHandle, NULL);

        if(FileSize != 0xFFFFFFFF)
        {
            SetFilePointer(OutHandle, 0, NULL, FILE_BEGIN);
            OutData = (char *) GlobalAlloc(GMEM_FIXED, FileSize);
            ReadFile(OutHandle, OutData, FileSize, &OutDataSize, NULL);
            OutDataPos = 0;
        }
        if(OutData == NULL)
            ExitCode = 0;
    }

    SetConsoleTitle(SaveTitle);
    SetConsoleMode(StdInput, ConsoleMode);
    CloseHandle(OutHandle);

    FillFormat(TypeName);

    if(ExitCode && OutDataSize == 0)
    {
        GlobalFree((HGLOBAL) OutData);
        return (OpenArchivePipe(Name, Type));
    }

    return (ExitCode);
}


int WINAPI _export GetArcItem(PluginPanelItem *Item, ArcItemInfo *Info)
{
    char Str[512];
    StringList *CurFormatNode = Format;
    SYSTEMTIME stModification, stCreation, stAccess, syst;

    memset(&stModification, 0, sizeof(stModification));
    memset(&stCreation, 0, sizeof(stCreation));
    memset(&stAccess, 0, sizeof(stAccess));
    GetSystemTime(&syst);

    while(GetString(Str, sizeof(Str)))
    {
        RegExp re;

        if(*StartText)
        {
            if(re.compile(StartText))
            {
                if(re.match(Str))
                    *StartText = 0;
            }
            else
            {
                if((*StartText == '^' && strncmp(Str, StartText + 1, lstrlen(StartText + 1)) == 0) ||
                   (*StartText != '^' && strstr(Str, StartText) != NULL))
                {
                    *StartText = 0;
                }
            }
            continue;
        }

        if(*EndText)
        {
            if(re.compile(EndText))
            {
                if(re.match(Str))
                    break;
            }
            else if(*EndText == '^')
            {
                if(strncmp(Str, EndText + 1, lstrlen(EndText + 1)) == 0)
                    break;
            }
            else if(strstr(Str, EndText) != NULL)
                break;

        }

        bool bFoundIgnoreString = false;
        for(StringList * CurIgnoreString = IgnoreStrings; CurIgnoreString->Next(); CurIgnoreString = CurIgnoreString->Next())
        {
            if(re.compile(CurIgnoreString->Str()))
            {
                if(re.match(Str))
                    bFoundIgnoreString = true;
            }
            else if(*CurIgnoreString->Str() == '^')
            {
                if(strncmp(Str, CurIgnoreString->Str() + 1, lstrlen(CurIgnoreString->Str() + 1)) == 0)
                    bFoundIgnoreString = true;
            }
            else if(strstr(Str, CurIgnoreString->Str()) != NULL)
                bFoundIgnoreString = true;
        }

        if(bFoundIgnoreString)
            continue;

        if(re.compile(CurFormatNode->Str()))
        {
            if(Match match = re.match(Str))
                ParseListingItemRegExp(match, Item, Info, stModification, stCreation, stAccess);
        }
        else
            ParseListingItemPlain(CurFormatNode->Str(), Str, Item, Info, stModification, stCreation, stAccess);

        CurFormatNode = CurFormatNode->Next();
        if(!CurFormatNode || !CurFormatNode->Next())
        {
            MakeFiletime(stModification, syst, &Item->FindData.ftLastWriteTime);
            MakeFiletime(stCreation, syst, &Item->FindData.ftCreationTime);
            MakeFiletime(stAccess, syst, &Item->FindData.ftLastAccessTime);

            for(int I = lstrlen(Item->FindData.cFileName) - 1; I >= 0; I--)
            {
                int Ch = Item->FindData.cFileName[I];

                if(Ch == ' ' || Ch == '\t')
                    Item->FindData.cFileName[I] = 0;
                else
                    break;
            }
            return (GETARC_SUCCESS);
        }
    }

    return (GETARC_EOF);
}


BOOL WINAPI _export CloseArchive(ArcInfo * Info)
{
    if(IgnoreErrors)
        Info->Flags |= AF_IGNOREERRORS;

    if(ArcChapters < 0)
        ArcChapters = 0;
    Info->Chapters = ArcChapters;

    GlobalFree((HGLOBAL) OutData);

    delete Format;
    delete IgnoreStrings;
    Format = 0;
    IgnoreStrings = 0;

    return (TRUE);
}


BOOL WINAPI _export GetFormatName(int Type, char *FormatName, char *DefaultExt)
{
    char TypeName[NM];

    if(!GetSectionName(Type, TypeName, sizeof(TypeName)))
        return (FALSE);

    GetIniString(TypeName, Str_TypeName, TypeName, FormatName, 64);
    GetIniString(TypeName, "Extension", "", DefaultExt, NM);

    return (*FormatName != 0);
}


BOOL WINAPI _export GetDefaultCommands(int Type, int Command, char *Dest)
{
    char TypeName[NM], FormatName[NM];

    if(!GetSectionName(Type, TypeName, sizeof(TypeName)))
        return (FALSE);

    GetIniString(TypeName, Str_TypeName, TypeName, FormatName, 64);

    if(*FormatName == 0)
        return (FALSE);

    static const char *CmdNames[] = { "Extract", "ExtractWithoutPath", "Test", "Delete",
        "Comment", "CommentFiles", "SFX", "Lock", "Protect", "Recover",
        "Add", "Move", "AddRecurse", "MoveRecurse", "AllFilesMask"
    };

    if(Command < (int)(ARRAYSIZE(CmdNames)))
    {
        GetIniString(TypeName, CmdNames[Command], "", Dest, 512);
        return (TRUE);
    }

    return (FALSE);
}

///////////////////////////////////////////////////////////////////////////////
// Utility functions

int HexCharToNum(int HexChar)
{
    HexChar = LUpper(HexChar);
    if(HexChar >= '0' && HexChar <= '9')
        return (HexChar - '0');
    else if(HexChar >= 'A' && HexChar <= 'F')
        return (HexChar - 'A' + 10);
    return (0);
}



int GetSectionName(int Num, char *Name, int MaxSize)
{
    char Buf[8192];
    char *Section = Buf;

    GetPrivateProfileSectionNames(Buf, sizeof(Buf), FormatFileName);
    char tmp[3];
    while(*Section)
    {
        if(*Section != ';' && !GetPrivateProfileSection(Section,tmp,sizeof(tmp),UserFormatFileName))
        {
            if(!Num)
            {
                lstrcpyn(Name, Section, MaxSize);
                return TRUE;
            }
            Num--;
        }
        Section += lstrlen(Section) + 1;
    }

    Section = Buf;
    GetPrivateProfileSectionNames(Buf,sizeof(Buf),UserFormatFileName);
    while(*Section)
    {
        if(*Section != ';')
        {
            if(!Num)
            {
                lstrcpyn(Name, Section, MaxSize);
                return TRUE;
            }
            Num--;
        }
        Section += lstrlen(Section) + 1;
    }
    return FALSE;
}

DWORD GetIniString(LPCTSTR lpAppName,LPCTSTR lpKeyName,LPCTSTR lpDefault,LPTSTR lpReturnedString,DWORD nSize)
{
	GetPrivateProfileString(lpAppName,lpKeyName,lpDefault,lpReturnedString,nSize,FormatFileName);
	return GetPrivateProfileString(lpAppName,lpKeyName,lpReturnedString,lpReturnedString,nSize,UserFormatFileName);
}

UINT GetIniInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,INT lpDefault)
{
	return GetPrivateProfileInt(lpAppName,lpKeyName,GetPrivateProfileInt(lpAppName,lpKeyName,lpDefault,FormatFileName),UserFormatFileName);
}

void FillFormat(const char *TypeName)
{
    GetIniString(TypeName, "Start", "", StartText, sizeof(StartText));
    GetIniString(TypeName, "End", "", EndText, sizeof(EndText));

    int FormatNumber = 0;

    delete Format;
    Format = new StringList;
    for(StringList * CurFormat = Format;; CurFormat = CurFormat->Add())
    {
        char FormatName[100];

        SPrintf(FormatName, "Format%d", FormatNumber++);
        GetIniString(TypeName, FormatName, "", CurFormat->Str(), PROF_STR_LEN);
        if(*CurFormat->Str() == 0)
            break;
    }

    int Number = 0;

    delete IgnoreStrings;
    IgnoreStrings = new StringList;
    for(StringList * CurIgnoreString = IgnoreStrings;; CurIgnoreString = CurIgnoreString->Add())
    {
        char Name[100];

        SPrintf(Name, "IgnoreString%d", Number++);
        GetIniString(TypeName, Name, "", CurIgnoreString->Str(), PROF_STR_LEN);
        if(*CurIgnoreString->Str() == 0)
            break;
    }
}

int GetString(char *Str, int MaxSize)
{
    if(OutDataPos >= OutDataSize)
        return (FALSE);

    int StartPos = OutDataPos;

    while(OutDataPos < OutDataSize)
    {
        int Ch = OutData[OutDataPos];

        if(Ch == '\r' || Ch == '\n')
            break;
        OutDataPos++;
    }

    int Length = OutDataPos - StartPos;
    int DestLength = Length >= MaxSize ? MaxSize - 1 : Length;

    lstrcpyn(Str, OutData + StartPos, DestLength + 1);

    while(OutDataPos < OutDataSize)
    {
        int Ch = OutData[OutDataPos];

        if(Ch != '\r' && Ch != '\n')
            break;
        OutDataPos++;
    }

    return (TRUE);
}

void MakeFiletime(SYSTEMTIME st, SYSTEMTIME syst, LPFILETIME pft)
{
    if(st.wDay == 0)
        st.wDay = syst.wDay;
    if(st.wMonth == 0)
        st.wMonth = syst.wMonth;
    if(st.wYear == 0)
        st.wYear = syst.wYear;
    else
    {
        if(st.wYear < 50)
            st.wYear += 2000;
        else if(st.wYear < 100)
            st.wYear += 1900;
    }

    FILETIME ft;

    if(SystemTimeToFileTime(&st, &ft))
    {
        LocalFileTimeToFileTime(&ft, pft);
    }
}


BOOL WINAPI OpenArchivePipe(const char *Name, int *Type)
{
    char TypeName[NM], Command[512];

    if(!GetSectionName(CurType, TypeName, sizeof(TypeName)))
        return (FALSE);
    GetIniString(TypeName, "List", "", Command, sizeof(Command));
    if(*Command == 0)
        return (FALSE);
    *Type = CurType;


    MetaReplacer meta(Command, Name);

    meta.replaceTo(Command);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    HANDLE hChildStdoutRd, hChildStdoutWr;
    HANDLE StdInput = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE StdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE StdError = GetStdHandle(STD_ERROR_HANDLE);
    SECURITY_ATTRIBUTES saAttr;

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if(!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 32768))
        return (FALSE);
    SetStdHandle(STD_OUTPUT_HANDLE, hChildStdoutWr);
    SetStdHandle(STD_ERROR_HANDLE, hChildStdoutWr);

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

    DWORD ConsoleMode;

    GetConsoleMode(StdInput, &ConsoleMode);
    SetConsoleMode(StdInput, ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT |
                   ENABLE_ECHO_INPUT | ENABLE_MOUSE_INPUT);

    char SaveTitle[512];

    GetConsoleTitle(SaveTitle, sizeof(SaveTitle));
    SetConsoleTitle(Command);

    char ExpandedCmd[512];

    ExpandEnvironmentStrings(Command, ExpandedCmd, sizeof(ExpandedCmd));

    DWORD ExitCode = CreateProcess(NULL, ExpandedCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

    SetStdHandle(STD_OUTPUT_HANDLE, StdOutput);
    SetStdHandle(STD_ERROR_HANDLE, StdError);
    CloseHandle(hChildStdoutWr);

    if(ExitCode)
    {
        const int ReadSize = 32768;

        OutDataSize = OutDataPos = 0;
        OutData = (char *) GlobalAlloc(GMEM_FIXED, 0);

        while(1)
        {
            DWORD Read;

            if((OutData =
                (char *) GlobalReAlloc(OutData, OutDataSize + ReadSize, GMEM_MOVEABLE)) == NULL)
                return (FALSE);
            if(!ReadFile(hChildStdoutRd, OutData + OutDataSize, ReadSize, &Read, NULL))
                break;
            OutDataSize += Read;
        }

        CloseHandle(hChildStdoutRd);
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &ExitCode);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        ExitCode = (ExitCode < GetIniInt(TypeName, "Errorlevel", 1000));

        if(!ExitCode)
            GlobalFree((HGLOBAL) OutData);
    }

    SetConsoleTitle(SaveTitle);
    SetConsoleMode(StdInput, ConsoleMode);

    FillFormat(TypeName);

    return (ExitCode);
}

int StringToInt(const char *str)
{
    int i = 0;
    for(const char *p = str; p && *p; ++p)
        if(isdigit(*p))
            i = i * 10 + (*p - '0');
    return i;
}

__int64 StringToInt64(const char *str)
{
    __int64 i = 0;
    for(const char *p = str; p && *p; ++p)
        if(isdigit(*p))
            i = i * 10 + (*p - '0');
    return i;
}

int StringToIntHex(const char *str)
{
    int i = 0;
    for(const char *p = str; p && *p; ++p)
        if(isxdigit(*p))
        {
            char dig_sub = (*p >= 'a' ? 'a' : (*p >= 'A' ? 'A' : '0'));
            i = i * 16 + (*p - dig_sub);
        }
    return i;
}

void ParseListingItemRegExp(Match match,
    PluginPanelItem *Item, ArcItemInfo *Info,
    SYSTEMTIME &stModification, SYSTEMTIME &stCreation, SYSTEMTIME &stAccess)
{

    if(const char *p = match["name"])
        lstrcpyn(Item->FindData.cFileName, p, sizeof(Item->FindData.cFileName));
    if(const char *p = match["description"])
        lstrcpyn(Info->Description, p, sizeof(Info->Description));

    LARGE_INTEGER SizeFile;
    SizeFile.QuadPart = StringToInt64(match["size"]);
    Item->FindData.nFileSizeLow  = SizeFile.LowPart;
    Item->FindData.nFileSizeHigh = SizeFile.HighPart;
    SizeFile.QuadPart = StringToInt64(match["packedSize"]);
    Item->PackSize               = SizeFile.LowPart;
    Item->PackSizeHigh           = SizeFile.HighPart;

    for(const char *p = match["attr"]; p && *p; ++p)
    {
        switch(LUpper(*p))
        {
            case 'D': Item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;  break;
            case 'H': Item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;     break;
            case 'A': Item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE;    break;
            case 'R': Item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_READONLY;   break;
            case 'S': Item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM;     break;
            case 'C': Item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_COMPRESSED; break;
        }
    }

    stModification.wYear    = StringToInt(match["mYear"]);
    stModification.wDay     = StringToInt(match["mDay"]);
    stModification.wMonth   = StringToInt(match["mMonth"]);

    if(const char *p = match["mMonthA"])
    {
        static const char *Months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };

        for(size_t I = 0; I < sizeof(Months) / sizeof(Months[0]); I++)
            if(LStrnicmp(p, Months[I], 3) == 0)
            {
                stModification.wMonth = (WORD)(I + 1);
                break;
            }
    }

    stModification.wHour    = StringToInt(match["mHour"]);

    if(const char *p = match["mAMPM"])
    {
        switch(LUpper(*p))
        {
        case 'A':
            if(stModification.wHour == 12)
                stModification.wHour -= 12;
            break;
        case 'P':
            if(stModification.wHour < 12)
                stModification.wHour += 12;
            break;
        }
    }

    stModification.wMinute  = StringToInt(match["mMin"]);
    stModification.wSecond  = StringToInt(match["mSec"]);

    stAccess.wDay           = StringToInt(match["aDay"]);
    stAccess.wMonth         = StringToInt(match["aMonth"]);
    stAccess.wYear          = StringToInt(match["aYear"]);
    stAccess.wHour          = StringToInt(match["aHour"]);
    stAccess.wMinute        = StringToInt(match["aMin"]);
    stAccess.wSecond        = StringToInt(match["aSec"]);

    stCreation.wDay         = StringToInt(match["cDay"]);
    stCreation.wMonth       = StringToInt(match["cMonth"]);
    stCreation.wYear        = StringToInt(match["cYear"]);
    stCreation.wHour        = StringToInt(match["cHour"]);
    stCreation.wMinute      = StringToInt(match["cMin"]);
    stCreation.wSecond      = StringToInt(match["cSec"]);

    Item->CRC32             = StringToIntHex(match["CRC"]);

}


void ParseListingItemPlain(const char *CurFormat, const char *CurStr,
    PluginPanelItem *Item, ArcItemInfo *Info,
    SYSTEMTIME &stModification, SYSTEMTIME &stCreation, SYSTEMTIME &stAccess)
{

    enum
    { OP_OUTSIDE, OP_INSIDE, OP_SKIP }
    OptionalPart = OP_OUTSIDE;
    int IsChapter = 0;

    LARGE_INTEGER SizeFile;

    for(; *CurStr && *CurFormat; CurFormat++, CurStr++)
    {
        if(OptionalPart == OP_SKIP)
        {
            if(*CurFormat == ')')
                OptionalPart = OP_OUTSIDE;
            CurStr--;
            continue;
        }
        switch (*CurFormat)
        {
        case '*':
            if(isspace(*CurStr) || !CurStr)
                CurStr--;
            else
                while( /*CurStr[0] && */ CurStr[1] /*&& !isspace(CurStr[0]) */
                      && !isspace(CurStr[1]))
                    CurStr++;
            break;
        case 'n':
            strncat(Item->FindData.cFileName, CurStr, 1);
            break;
        case 'c':
            strncat(Info->Description, CurStr, 1);
            break;
        case '.':
            {
                for(int I = lstrlen(Item->FindData.cFileName); I >= 0; I--)
                    if(isspace(Item->FindData.cFileName[I]))
                        Item->FindData.cFileName[I] = 0;
                if(*Item->FindData.cFileName)
                    lstrcat(Item->FindData.cFileName, ".");
            }
            break;
        case 'z':
            if(isdigit(*CurStr))
            {
                SizeFile.LowPart=Item->FindData.nFileSizeLow;
                SizeFile.HighPart=Item->FindData.nFileSizeHigh;
                SizeFile.QuadPart = SizeFile.QuadPart * 10 + (*CurStr - '0');
                Item->FindData.nFileSizeLow=SizeFile.LowPart;
                Item->FindData.nFileSizeHigh=SizeFile.HighPart;
            }
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'p':
            if(isdigit(*CurStr))
            {
                SizeFile.LowPart=Item->PackSize;
                SizeFile.HighPart=Item->PackSizeHigh;
                SizeFile.QuadPart = SizeFile.QuadPart * 10 + (*CurStr - '0');
                Item->PackSize=SizeFile.LowPart;
                Item->PackSizeHigh=SizeFile.HighPart;
            }
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'a':
            switch (LUpper(*CurStr))
            {
                case 'D': Item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;  break;
                case 'H': Item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;     break;
                case 'A': Item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE;    break;
                case 'R': Item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_READONLY;   break;
                case 'S': Item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM;     break;
                case 'C': Item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_COMPRESSED; break;
            }
            break;
// MODIFICATION DATETIME
        case 'y':
            if(isdigit(*CurStr))
                stModification.wYear = stModification.wYear * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'd':
            if(isdigit(*CurStr))
                stModification.wDay = stModification.wDay * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 't':
            if(isdigit(*CurStr))
                stModification.wMonth = stModification.wMonth * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'T':
            {
                static const char *Months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
                };

                for(size_t I = 0; I < sizeof(Months) / sizeof(Months[0]); I++)
                    if(LStrnicmp(CurStr, Months[I], 3) == 0)
                    {
                        stModification.wMonth = (WORD)(I + 1);
                        while(CurFormat[1] == 'T' && CurStr[1])
                        {
                            CurStr++;
                            CurFormat++;
                        }
                        break;
                    }
            }
            break;
        case 'h':
            if(isdigit(*CurStr))
                stModification.wHour = stModification.wHour * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'H':
            switch (LUpper(*CurStr))
            {
                case 'A':
                    if(stModification.wHour == 12)
                        stModification.wHour -= 12;
                    break;
                case 'P':
                    if(stModification.wHour < 12)
                        stModification.wHour += 12;
                    break;
            }
            break;
        case 'm':
            if(isdigit(*CurStr))
                stModification.wMinute = stModification.wMinute * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 's':
            if(isdigit(*CurStr))
                stModification.wSecond = stModification.wSecond * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
// ACCESS DATETIME
        case 'b':
            if(isdigit(*CurStr))
                stAccess.wDay = stAccess.wDay * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'v':
            if(isdigit(*CurStr))
                stAccess.wMonth = stAccess.wMonth * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'e':
            if(isdigit(*CurStr))
                stAccess.wYear = stAccess.wYear * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'x':
            if(isdigit(*CurStr))
                stAccess.wHour = stAccess.wHour * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'l':
            if(isdigit(*CurStr))
                stAccess.wMinute = stAccess.wMinute * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'k':
            if(isdigit(*CurStr))
                stAccess.wSecond = stAccess.wSecond * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
// CREATION DATETIME
        case 'j':
            if(isdigit(*CurStr))
                stCreation.wDay = stCreation.wDay * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'g':
            if(isdigit(*CurStr))
                stCreation.wMonth = stCreation.wMonth * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'f':
            if(isdigit(*CurStr))
                stCreation.wYear = stCreation.wYear * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'o':
            if(isdigit(*CurStr))
                stCreation.wHour = stCreation.wHour * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'i':
            if(isdigit(*CurStr))
                stCreation.wMinute = stCreation.wMinute * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'u':
            if(isdigit(*CurStr))
                stCreation.wSecond = stCreation.wSecond * 10 + (*CurStr - '0');
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'r':
            if(isxdigit(LUpper(*CurStr)))
            {
                char dig_sub = (*CurStr >= 'a' ? 'a' : (*CurStr >= 'A' ? 'A' : '0'));

                Item->CRC32 = Item->CRC32 * 16 + (*CurStr - dig_sub);
            }
            else if(OP_INSIDE == OptionalPart)
            {
                CurStr--;
                OptionalPart = OP_SKIP;
            }
            break;
        case 'C':
            if(*CurStr == '-')
            {
                IsChapter = 1;
                ArcChapters = 0;
            }
            else if(isdigit(*CurStr))
            {
                if(IsChapter)
                    ArcChapters = ArcChapters * 10 + (*CurStr - '0');
                else
                    Info->Chapter = Info->Chapter * 10 + (*CurStr - '0');
            }
            break;
        case '(':
            OptionalPart = OP_INSIDE;
            CurStr--;
            break;
        case ')':
            OptionalPart = OP_OUTSIDE;
            CurStr--;
            break;
        }
    }
}
