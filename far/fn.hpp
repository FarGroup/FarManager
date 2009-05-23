#ifndef __FARFUNC_HPP__
#define __FARFUNC_HPP__
/*
fn.hpp

ќписани€ функций
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

enum {
  COLUMN_MARK           = 0x80000000,
  COLUMN_NAMEONLY       = 0x40000000,
  COLUMN_RIGHTALIGN     = 0x20000000,
  COLUMN_FORMATTED      = 0x10000000,
  COLUMN_COMMAS         = 0x08000000,
  COLUMN_THOUSAND       = 0x04000000,
  COLUMN_BRIEF          = 0x02000000,
  COLUMN_MONTH          = 0x01000000,
  COLUMN_FLOATSIZE      = 0x00800000,
  COLUMN_ECONOMIC       = 0x00400000,
  COLUMN_MINSIZEINDEX   = 0x00200000,
  COLUMN_SHOWBYTESINDEX = 0x00100000,
  COLUMN_FULLOWNER      = 0x00080000,

  //MINSIZEINDEX может быть только 0, 1, 2 или 3 (K,M,G,T)
  COLUMN_MINSIZEINDEX_MASK = 0x00000003,
};

enum {
  // DRIVE_UNKNOWN            = 0,
  // DRIVE_NO_ROOT_DIR        = 1,
  // DRIVE_REMOVABLE          = 2,
  // DRIVE_FIXED              = 3,
  // DRIVE_REMOTE             = 4,
  // DRIVE_CDROM              = 5,
  // DRIVE_RAMDISK            = 6,

  DRIVE_SUBSTITUTE            =15,
  DRIVE_REMOTE_NOT_CONNECTED  =16,
  DRIVE_CD_RW                 =18,
  DRIVE_CD_RWDVD              =19,
  DRIVE_DVD_ROM               =20,
  DRIVE_DVD_RW                =21,
  DRIVE_DVD_RAM               =22,
  DRIVE_USBDRIVE              =40,
  DRIVE_NOT_INIT              =255,
};

enum {
    XC_QUIT                = (unsigned long) -777,
    XC_OPEN_ERROR          = 0,
    XC_MODIFIED            = 1,
    XC_NOT_MODIFIED        = 2,
    XC_LOADING_INTERRUPTED = 3,
    XC_EXISTS              = 4,
};

enum CHECKFOLDERCONST{ // for CheckFolder()
  CHKFLD_ERROR     = -2,
  CHKFLD_NOTACCESS = -1,
  CHKFLD_EMPTY     =  0,
  CHKFLD_NOTEMPTY  =  1,
  CHKFLD_NOTFOUND  =  2,
};

enum CHECKEDPROPS_TYPE{
  CHECKEDPROPS_ISSAMEDISK,
  CHECKEDPROPS_ISDST_ENCRYPTION,
};

string &FormatNumber(const wchar_t *Src, string &strDest, int NumDigits=0);
string &InsertCommas(unsigned __int64 li, string &strDest);

string &RemoveChar(string &strStr,wchar_t Target,BOOL Dup=TRUE);
wchar_t *InsertString(wchar_t *Str,int Pos,const wchar_t *InsStr,int InsSize=0);
int ReplaceStrings(string &strStr,const wchar_t *FindStr,const wchar_t *ReplStr,int Count=-1,BOOL IgnoreCase=FALSE);
BOOL IsCaseMixed(const string &strStr);
BOOL IsCaseLower(const string &strStr);

int ToPercent(unsigned long N1,unsigned long N2);
int ToPercent64(unsigned __int64 N1,unsigned __int64 N2);

const wchar_t *GetCommaWord(const wchar_t *Src,string &strWord,wchar_t Separator=L',');
int CmpName(const wchar_t *pattern,const wchar_t *str,int skippath=TRUE);
// обработать им€ файла: сравнить с маской, масками, сгенерировать по маске
int WINAPI ProcessName(const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags);

wchar_t* WINAPI QuoteSpace(wchar_t *Str);
string &QuoteSpace(string &strStr);
wchar_t* WINAPI InsertQuote(wchar_t *Str);
string& InsertQuote(string& strStr);
bool CutToSlash(string &strStr, bool bInclude = false);
string &CutToNameUNC(string &strPath);
string &CutToFolderNameIfFolder(string &strPath);
const wchar_t *PointToNameUNC(const wchar_t *lpwszPath);

string& CenterStr(const wchar_t *Src, string &strDest,int Length);

void Transform(string &strBuffer,const wchar_t *ConvStr,wchar_t TransformType);

bool CheckFileSizeStringFormat(const wchar_t *FileSizeStr);
unsigned __int64 ConvertFileSizeString(const wchar_t *FileSizeStr);

int CheckFolder(const wchar_t *Name);
int CheckShortcutFolder(string *pTestPath,int IsHostFile, BOOL Silent=FALSE);

class Panel;
void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir=FALSE);
int  CheckUpdateAnotherPanel(Panel *SrcPanel,const wchar_t *SelName);

BOOL UnExpandEnvString(const char *Path, const char *EnvVar, char* Dest, int DestSize);
BOOL PathUnExpandEnvStr(const char *Path, char* Dest, int DestSize);

void WINAPI Unquote(string &strStr);
void WINAPI Unquote(wchar_t *Str);

void UnquoteExternal(string &strStr);

wchar_t* WINAPI RemoveLeadingSpaces(wchar_t *Str);
string& WINAPI RemoveLeadingSpaces(string &strStr);

wchar_t *WINAPI RemoveTrailingSpaces(wchar_t *Str);
string& WINAPI RemoveTrailingSpaces(string &strStr);

wchar_t* WINAPI RemoveExternalSpaces(wchar_t *Str);
string & WINAPI RemoveExternalSpaces(string &strStr);
string & WINAPI RemoveUnprintableCharacters(string &strStr);

wchar_t* __stdcall TruncStr(wchar_t *Str,int MaxLength);
string& __stdcall TruncStr(string &strStr,int MaxLength);

wchar_t* WINAPI TruncStrFromEnd(wchar_t *Str,int MaxLength);
string& __stdcall TruncStrFromEnd(string &strStr, int MaxLength);

wchar_t* __stdcall TruncPathStr(wchar_t *Str, int MaxLength);
string& __stdcall TruncPathStr(string &strStr, int MaxLength);

wchar_t* WINAPI QuoteSpaceOnly(wchar_t *Str);
string& WINAPI QuoteSpaceOnly(string &strStr);
BOOL IsWordDiv(const wchar_t *WordDiv, wchar_t Chr);

const wchar_t* __stdcall PointToName(const wchar_t *lpwszPath);
const wchar_t* __stdcall PointToFolderNameIfFolder(const wchar_t *lpwszPath);
const wchar_t* PointToExt(const wchar_t *lpwszPath);

BOOL  TestParentFolderName(const wchar_t *Name);
BOOL TestCurrentFolderName(const wchar_t *Name);

BOOL  AddEndSlash(string &strPath, wchar_t TypeSlash);
BOOL  AddEndSlash(string &strPath);
BOOL  AddEndSlash(wchar_t *Path, wchar_t TypeSlash);
BOOL  WINAPI AddEndSlash(wchar_t *Path);
BOOL  WINAPI DeleteEndSlash(string &strPath,bool allendslash=false);
string& ReplaceSlashToBSlash(string& strStr);

#ifdef __cplusplus
extern "C" {
#endif

typedef int  (WINAPI *FRSUSERFUNCW)(const FAR_FIND_DATA *FData,const wchar_t *FullName,void *param);
void WINAPI FarRecursiveSearch(const wchar_t *initdir,const wchar_t *mask,FRSUSERFUNCW func,DWORD flags,void *param);

wchar_t* __stdcall FarMkTemp(wchar_t *Dest, DWORD size, const wchar_t *Prefix);
string& FarMkTempEx(string &strDest, const wchar_t *Prefix=NULL, BOOL WithPath=TRUE);

/* $ 15.02.2002 IS
   ”становка нужного диска и каталога и установление соответствующей переменной
   окружени€. ¬ случае успеха возвращаетс€ не ноль.
   ≈сли ChangeDir==FALSE, то не мен€ем текущий  диск, а только устанавливаем
   переменные окружени€.
*/
BOOL FarChDir(const wchar_t *NewDir,BOOL ChangeDir=TRUE);

#ifdef __cplusplus
};
#endif

int ConvertWildcards(const wchar_t *SrcName,string &strDest, int SelectedFolderNameLength);

void __PrepareKMGTbStr(void);
string& __stdcall FileSizeToStr(string &strDestStr, unsigned __int64 Size, int Width=-1, int ViewFlags=COLUMN_COMMAS);

bool IsDriveTypeRemote(UINT DriveType);
int IsLocalDrive(const wchar_t *Path);
DWORD WINAPI FarGetLogicalDrives(void);

string &Add_PATHEXT(string &strDest);

string& WINAPI FarFormatText(const wchar_t *SrcText, int Width, string &strDestText, const wchar_t* Break, DWORD Flags);

//   WordDiv  - набор разделителей слова в кодировке OEM
// возвращает указатель на начало слова
const wchar_t * const CalcWordFromString(const wchar_t *Str,int CurPos,int *Start,int *End,const wchar_t *WordDiv);

int CheckDisksProps(const wchar_t *SrcPath,const wchar_t *DestPath,int CheckedType);

bool GetFileFormat (FILE *file, UINT &nCodePage, bool *pSignatureFound = NULL, bool bUseHeuristics = true);

wchar_t *ReadString (FILE *file, wchar_t *lpwszDest, int nDestLength, int nCodePage);

const wchar_t *FirstSlash(const wchar_t *String);
bool FirstSlash(const wchar_t *String,size_t &pos);
const wchar_t *LastSlash(const wchar_t *String);
bool LastSlash(const wchar_t *String,size_t &pos);

#endif  // __FARFUNC_HPP__
