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

#include "plugin.hpp"

class FileFilter;

const wchar_t *GetShellAction(const wchar_t *FileName,DWORD& ImageSubsystem,DWORD& Error);

string &FormatNumber(const wchar_t *Src, string &strDest, int NumDigits=0);
string &InsertCommas(unsigned __int64 li, string &strDest);

void DeleteDirTree(const wchar_t *Dir);

void Log(char *fmt,...);

string &RemoveChar(string &strStr,wchar_t Target,BOOL Dup=TRUE);

wchar_t *InsertString(wchar_t *Str,int Pos,const wchar_t *InsStr,int InsSize=0);

int ReplaceStrings(string &strStr,const wchar_t *FindStr,const wchar_t *ReplStr,int Count=-1,BOOL IgnoreCase=FALSE);

#define RemoveHighlights(Str) RemoveChar(Str,L'&')

BOOL IsCaseMixed(const string &strStr);
BOOL IsCaseLower(const string &strStr);

int DeleteFileWithFolder(const wchar_t *FileName);

int ToPercent(unsigned long N1,unsigned long N2);
int ToPercent64(unsigned __int64 N1,unsigned __int64 N2);
int CmpName(const wchar_t *pattern,const wchar_t *str,int skippath=TRUE);
// обработать им€ файла: сравнить с маской, масками, сгенерировать по маске
int WINAPI ProcessName(const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags);

wchar_t* WINAPI QuoteSpace(wchar_t *Str);
string &QuoteSpace(string &strStr);


wchar_t* WINAPI InsertQuote(wchar_t *Str);
string& InsertQuote(string& strStr);

int ProcessGlobalFileTypes(const wchar_t *Name,int AlwaysWaitFinish);
int ProcessLocalFileTypes(const wchar_t *Name,const wchar_t *ShortName,int Mode,int AlwaysWaitFinish);
void ProcessExternal(const wchar_t *Command,const wchar_t *Name,const wchar_t *ShortName,int AlwaysWaitFinish);

int SubstFileName(string &strStr, const wchar_t *Name, const wchar_t *ShortName,
                  string *strListName=NULL,
                  string *strAnotherListName = NULL,
                  string *strShortListName=NULL,
                  string *strAnotherShortListName=NULL,
                  int IgnoreInput=FALSE,const wchar_t *CmdLineDir=NULL);
BOOL ExtractIfExistCommand(string &strCommandText);
void EditFileTypes();

int  IsLocalDrive(const wchar_t *Path);

int HiStrlen(const wchar_t *Str);
int HiFindRealPos(const wchar_t *Str, int Pos, BOOL ShowAmp);

bool GetShellType(const wchar_t *Ext, string &strType,ASSOCIATIONTYPE aType=AT_FILEEXTENSION);

bool CutToSlash(string &strStr, bool bInclude = false);
string &CutToNameUNC(string &strPath);
string &CutToFolderNameIfFolder(string &strPath);
const wchar_t *PointToNameUNC(const wchar_t *lpwszPath);

int GetShortcutFolder(int Key,string *pDestFolder, string *pPluginModule=NULL,
                      string *pPluginFile=NULL,string *pPluginData=NULL);
int SaveFolderShortcut(int Key,string *pSrcFolder,string *pPluginModule=NULL,
                       string *pPluginFile=NULL,string *pPluginData=NULL);
int GetShortcutFolderSize(int Key);
void ShowFolderShortcut();
void ShowFilter();

string& CenterStr(const wchar_t *Src, string &strDest,int Length);

const wchar_t *GetCommaWord(const wchar_t *Src,string &strWord,wchar_t Separator=L',');

void Transform(string &strBuffer,const wchar_t *ConvStr,wchar_t TransformType);

void GetFileDateAndTime(const wchar_t *Src,unsigned *Dst,int Separator);
void StrToDateTime(const wchar_t *CDate, const wchar_t *CTime, FILETIME &ft, int DateFormat, int DateSeparator, int TimeSeparator, bool bRelative=false);

bool CheckFileSizeStringFormat(const wchar_t *FileSizeStr);
unsigned __int64 ConvertFileSizeString(const wchar_t *FileSizeStr);

void ConvertDate(const FILETIME &ft,string &strDateText, string &strTimeText,int TimeLength,
        int Brief=FALSE,int TextMonth=FALSE,int FullYear=FALSE,int DynInit=FALSE);
void ConvertRelativeDate(const FILETIME &ft,string &strDaysText,string &strTimeText);

void ShellOptions(int LastCommand,MOUSE_EVENT_RECORD *MouseEvent);

int CheckFolder(const wchar_t *Name);
int CheckShortcutFolder(string *pTestPath,int IsHostFile, BOOL Silent=FALSE);

int Execute(const wchar_t *CmdStr,int AlwaysWaitFinish,int SeparateWindow=FALSE,int DirectRun=FALSE,int FolderRun=FALSE);

class Panel;
void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir=FALSE);
int  CheckUpdateAnotherPanel(Panel *SrcPanel,const wchar_t *SelName);

int GetDirInfo(const wchar_t *Title,const wchar_t *DirName,unsigned long &DirCount,
               unsigned long &FileCount,unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize,unsigned __int64 &RealSize,
               unsigned long &ClusterSize,clock_t MsgWaitTime,
               FileFilter *Filter,
               DWORD Flags=GETDIRINFO_SCANSYMLINKDEF);
int GetPluginDirInfo(HANDLE hPlugin,const wchar_t *DirName,unsigned long &DirCount,
               unsigned long &FileCount,unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize);

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

void CreatePath(string &strPath);

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

//int ConvertWildcards(const char *Src,char *Dest, int SelectedFolderNameLength);
int ConvertWildcards(const wchar_t *SrcName,string &strDest, int SelectedFolderNameLength);

const wchar_t* WINAPI PrepareOSIfExist(const wchar_t *CmdLine);
bool IsBatchExtType(const wchar_t *ExtPtr);

void __PrepareKMGTbStr(void);
string& __stdcall FileSizeToStr(string &strDestStr, unsigned __int64 Size, int Width=-1, int ViewFlags=COLUMN_COMMAS);


DWORD WINAPI FarGetLogicalDrives(void);

string &Add_PATHEXT(string &strDest);

string& WINAPI FarFormatText(const wchar_t *SrcText, int Width, string &strDestText, const wchar_t* Break, DWORD Flags);


int PathMayBeAbsolute(const wchar_t *Src);

string& PrepareDiskPath(string &strPath, BOOL CheckFullPath=TRUE);

//   WordDiv  - набор разделителей слова в кодировке OEM
// возвращает указатель на начало слова
const wchar_t * const CalcWordFromString(const wchar_t *Str,int CurPos,int *Start,int *End,const wchar_t *WordDiv);

bool IsDriveTypeRemote(UINT DriveType);

bool PathPrefix(const wchar_t *Path);
BOOL IsNetworkPath(const wchar_t *Path);
BOOL IsLocalPath(const wchar_t *Path);
BOOL IsLocalRootPath(const wchar_t *Path);
BOOL IsLocalPrefixPath(const wchar_t *Path);
BOOL IsLocalVolumePath(const wchar_t *Path);
BOOL IsLocalVolumeRootPath(const wchar_t *Path);

BOOL ProcessOSAliases(string &strStr);

int PartCmdLine(const wchar_t *CmdStr, string &strNewCmdStr, string &strNewCmdPar);

int _MakePath1(DWORD Key,string &strPathName, const wchar_t *Param2,int ShortNameAsIs=TRUE);

string &CurPath2ComputerName(const wchar_t *CurDir, string &strComputerName);
int CheckDisksProps(const wchar_t *SrcPath,const wchar_t *DestPath,int CheckedType);

bool GetFileFormat (FILE *file, UINT &nCodePage, bool *pSignatureFound = NULL, bool bUseHeuristics = true);

string& HiText2Str(string& strDest, const wchar_t *Str);

__int64 FileTimeDifference(const FILETIME *a, const FILETIME* b);
unsigned __int64 FileTimeToUI64(const FILETIME *ft);

wchar_t *ReadString (FILE *file, wchar_t *lpwszDest, int nDestLength, int nCodePage);

const wchar_t *FirstSlash(const wchar_t *String);
bool FirstSlash(const wchar_t *String,size_t &pos);
const wchar_t *LastSlash(const wchar_t *String);
bool LastSlash(const wchar_t *String,size_t &pos);

#endif  // __FARFUNC_HPP__
