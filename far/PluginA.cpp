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
#include "farexcpt.hpp"
#include "DList.hpp"
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

namespace wrapper
{

#define EXP_GETGLOBALINFO       ""
#define EXP_SETSTARTUPINFO      "SetStartupInfo"
#define EXP_OPEN                "OpenPlugin"
#define EXP_CLOSEPANEL          "ClosePlugin"
#define EXP_GETPLUGININFO       "GetPluginInfo"
#define EXP_GETOPENPANELINFO    "GetOpenPluginInfo"
#define EXP_GETFINDDATA         "GetFindData"
#define EXP_FREEFINDDATA        "FreeFindData"
#define EXP_GETVIRTUALFINDDATA  "GetVirtualFindData"
#define EXP_FREEVIRTUALFINDDATA "FreeVirtualFindData"
#define EXP_SETDIRECTORY        "SetDirectory"
#define EXP_GETFILES            "GetFiles"
#define EXP_PUTFILES            "PutFiles"
#define EXP_DELETEFILES         "DeleteFiles"
#define EXP_MAKEDIRECTORY       "MakeDirectory"
#define EXP_PROCESSHOSTFILE     "ProcessHostFile"
#define EXP_SETFINDLIST         "SetFindList"
#define EXP_CONFIGURE           "Configure"
#define EXP_EXITFAR             "ExitFAR"
#define EXP_PROCESSPANELINPUT   "ProcessKey"
#define EXP_PROCESSPANELEVENT   "ProcessEvent"
#define EXP_PROCESSEDITOREVENT  "ProcessEditorEvent"
#define EXP_COMPARE             "Compare"
#define EXP_PROCESSEDITORINPUT  "ProcessEditorInput"
#define EXP_PROCESSVIEWEREVENT  "ProcessViewerEvent"
#define EXP_PROCESSDIALOGEVENT  "ProcessDialogEvent"
#define EXP_PROCESSSYNCHROEVENT ""
#if defined(MANTIS_0000466)
#define EXP_PROCESSMACRO        ""
#endif
#if defined(MANTIS_0001687)
#define EXP_PROCESSCONSOLEINPUT ""
#endif
#define EXP_ANALYSE             ""
#define EXP_GETCUSTOMDATA       ""
#define EXP_FREECUSTOMDATA      ""
#define EXP_CLOSEANALYSE        ""

#define EXP_OPENFILEPLUGIN      "OpenFilePlugin"
#define EXP_GETMINFARVERSION    "GetMinFarVersion"


static const char* _ExportsNamesA[i_LAST] =
{
	EXP_GETGLOBALINFO,
	EXP_SETSTARTUPINFO,
	EXP_OPEN,
	EXP_CLOSEPANEL,
	EXP_GETPLUGININFO,
	EXP_GETOPENPANELINFO,
	EXP_GETFINDDATA,
	EXP_FREEFINDDATA,
	EXP_GETVIRTUALFINDDATA,
	EXP_FREEVIRTUALFINDDATA,
	EXP_SETDIRECTORY,
	EXP_GETFILES,
	EXP_PUTFILES,
	EXP_DELETEFILES,
	EXP_MAKEDIRECTORY,
	EXP_PROCESSHOSTFILE,
	EXP_SETFINDLIST,
	EXP_CONFIGURE,
	EXP_EXITFAR,
	EXP_PROCESSPANELINPUT,
	EXP_PROCESSPANELEVENT,
	EXP_PROCESSEDITOREVENT,
	EXP_COMPARE,
	EXP_PROCESSEDITORINPUT,
	EXP_PROCESSVIEWEREVENT,
	EXP_PROCESSDIALOGEVENT,
	EXP_PROCESSSYNCHROEVENT,
#if defined(MANTIS_0000466)
	EXP_PROCESSMACRO,
#endif
#if defined(MANTIS_0001687)
	EXP_PROCESSCONSOLEINPUT,
#endif
	EXP_ANALYSE,
	EXP_GETCUSTOMDATA,
	EXP_FREECUSTOMDATA,
	EXP_CLOSEANALYSE,

	EXP_OPENFILEPLUGIN,
	EXP_GETMINFARVERSION,
};


static const wchar_t* _ExportsNamesW[i_LAST] =
{
	W(EXP_GETGLOBALINFO),
	W(EXP_SETSTARTUPINFO),
	W(EXP_OPEN),
	W(EXP_CLOSEPANEL),
	W(EXP_GETPLUGININFO),
	W(EXP_GETOPENPANELINFO),
	W(EXP_GETFINDDATA),
	W(EXP_FREEFINDDATA),
	W(EXP_GETVIRTUALFINDDATA),
	W(EXP_FREEVIRTUALFINDDATA),
	W(EXP_SETDIRECTORY),
	W(EXP_GETFILES),
	W(EXP_PUTFILES),
	W(EXP_DELETEFILES),
	W(EXP_MAKEDIRECTORY),
	W(EXP_PROCESSHOSTFILE),
	W(EXP_SETFINDLIST),
	W(EXP_CONFIGURE),
	W(EXP_EXITFAR),
	W(EXP_PROCESSPANELINPUT),
	W(EXP_PROCESSPANELEVENT),
	W(EXP_PROCESSEDITOREVENT),
	W(EXP_COMPARE),
	W(EXP_PROCESSEDITORINPUT),
	W(EXP_PROCESSVIEWEREVENT),
	W(EXP_PROCESSDIALOGEVENT),
	W(EXP_PROCESSSYNCHROEVENT),
#if defined(MANTIS_0000466)
	W(EXP_PROCESSMACRO),
#endif
#if defined(MANTIS_0001687)
	W(EXP_PROCESSCONSOLEINPUT),
#endif
	W(EXP_ANALYSE),
	W(EXP_GETCUSTOMDATA),
	W(EXP_FREECUSTOMDATA),
	W(EXP_CLOSEANALYSE),

	W(EXP_OPENFILEPLUGIN),
	W(EXP_GETMINFARVERSION),
};

typedef void   (WINAPI *iClosePanelPrototype)          (HANDLE hPlugin);
typedef int    (WINAPI *iComparePrototype)             (HANDLE hPlugin,const oldfar::PluginPanelItem *Item1,const oldfar::PluginPanelItem *Item2,unsigned int Mode);
typedef int    (WINAPI *iConfigurePrototype)           (int ItemNumber);
typedef int    (WINAPI *iDeleteFilesPrototype)         (HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef void   (WINAPI *iExitFARPrototype)             ();
typedef void   (WINAPI *iFreeFindDataPrototype)        (HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber);
typedef void   (WINAPI *iFreeVirtualFindDataPrototype) (HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber);
typedef int    (WINAPI *iGetFilesPrototype)            (HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
typedef int    (WINAPI *iGetFindDataPrototype)         (HANDLE hPlugin,oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
typedef int    (WINAPI *iGetMinFarVersionPrototype)    ();
typedef void   (WINAPI *iGetOpenPanelInfoPrototype)    (HANDLE hPlugin,oldfar::OpenPanelInfo *Info);
typedef void   (WINAPI *iGetPluginInfoPrototype)       (oldfar::PluginInfo *Info);
typedef int    (WINAPI *iGetVirtualFindDataPrototype)  (HANDLE hPlugin,oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber,const char *Path);
typedef int    (WINAPI *iMakeDirectoryPrototype)       (HANDLE hPlugin,char *Name,int OpMode);
typedef HANDLE (WINAPI *iOpenFilePluginPrototype)      (char *Name,const unsigned char *Data,int DataSize);
typedef HANDLE (WINAPI *iOpenPrototype)                (int OpenFrom,intptr_t Item);
typedef int    (WINAPI *iProcessEditorEventPrototype)  (int Event,void *Param);
typedef int    (WINAPI *iProcessEditorInputPrototype)  (const INPUT_RECORD *Rec);
typedef int    (WINAPI *iProcessPanelEventPrototype)   (HANDLE hPlugin,int Event,void *Param);
typedef int    (WINAPI *iProcessHostFilePrototype)     (HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef int    (WINAPI *iProcessPanelInputPrototype)   (HANDLE hPlugin,int Key,unsigned int ControlState);
typedef int    (WINAPI *iPutFilesPrototype)            (HANDLE hPlugin,oldfar::PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
typedef int    (WINAPI *iSetDirectoryPrototype)        (HANDLE hPlugin,const char *Dir,int OpMode);
typedef int    (WINAPI *iSetFindListPrototype)         (HANDLE hPlugin,const oldfar::PluginPanelItem *PanelItem,int ItemsNumber);
typedef void   (WINAPI *iSetStartupInfoPrototype)      (const oldfar::PluginStartupInfo *Info);
typedef int    (WINAPI *iProcessViewerEventPrototype)  (int Event,void *Param);
typedef int    (WINAPI *iProcessDialogEventPrototype)  (int Event,void *Param);

#define UnicodeToOEM(src,dst,lendst)    WideCharToMultiByte(CP_OEMCP,0,(src),-1,(dst),(int)(lendst),nullptr,nullptr)
#define OEMToUnicode(src,dst,lendst)    MultiByteToWideChar(CP_OEMCP,0,(src),-1,(dst),(int)(lendst))

void ConvertKeyBarTitlesA(const oldfar::KeyBarTitles *kbtA, KeyBarTitles *kbtW, bool FullStruct=true);

inline int IsSpaceA(int x) { return x==' '  || x=='\t';  }
inline int IsEolA(int x)   { return x=='\r' || x=='\n'; }
inline int IsSlashA(int x) { return x=='\\' || x=='/';  }

static unsigned char LowerToUpper[256];
static unsigned char UpperToLower[256];
static unsigned char IsUpperOrLower[256];

void LocalUpperInit()
{
	for (size_t I=0; I<ARRAYSIZE(LowerToUpper); I++)
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
}

struct QsortexHelper
{
	int (__cdecl *fcmp)(const void *, const void *,void *userparam);
	void* user;
};

int WINAPI qsortCmp(const void *one, const void *two,void *user)
{
	return reinterpret_cast<int(__cdecl*)(const void*,const void*)>(user)(one,two);
}

int WINAPI qsortexCmp(const void *one, const void *two,void *user)
{
	QsortexHelper* helper=static_cast<QsortexHelper*>(user);
	return helper->fcmp(one,two,helper->user);
}

void WINAPI qsort(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *))
{
	NativeFSF.qsort(base,nelem,width,qsortCmp,reinterpret_cast<void*>(fcmp));
}

void WINAPI qsortex(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *userparam),void *userparam)
{
	QsortexHelper helper={fcmp,userparam};
	NativeFSF.qsort(base,nelem,width,qsortexCmp,&helper);
}

void* WINAPI bsearch(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *))
{
	return NativeFSF.bsearch(key,base,nelem,width,qsortCmp,reinterpret_cast<void*>(fcmp));
}

int WINAPI LocalIslower(unsigned Ch)
{
	return(Ch<256 && IsUpperOrLower[Ch]==1);
}

int WINAPI LocalIsupper(unsigned Ch)
{
	return(Ch<256 && IsUpperOrLower[Ch]==2);
}

int WINAPI LocalIsalpha(unsigned Ch)
{
	if (Ch>=256)
		return FALSE;

	char CvtCh=Ch;
	OemToCharBuffA(&CvtCh,&CvtCh,1);
	return(IsCharAlphaA(CvtCh));
}

int WINAPI LocalIsalphanum(unsigned Ch)
{
	if (Ch>=256)
		return FALSE;

	char CvtCh=Ch;
	OemToCharBuffA(&CvtCh,&CvtCh,1);
	return(IsCharAlphaNumericA(CvtCh));
}

unsigned WINAPI LocalUpper(unsigned LowerChar)
{
	return(LowerChar < 256 ? LowerToUpper[LowerChar]:LowerChar);
}

void WINAPI LocalUpperBuf(char *Buf,int Length)
{
	for (int I=0; I<Length; I++)
		Buf[I]=LocalUpper(Buf[I]);
}

unsigned WINAPI LocalLower(unsigned UpperChar)
{
	return(UpperChar < 256 ? UpperToLower[UpperChar]:UpperChar);
}

void WINAPI LocalLowerBuf(char *Buf,int Length)
{
	for (int I=0; I<Length; I++)
		Buf[I]=LocalLower(Buf[I]);
}

void WINAPI LocalStrupr(char *s1)
{
	while (*s1)
	{
		*s1=LowerToUpper[(unsigned)*s1];
		s1++;
	}
}

void WINAPI LocalStrlwr(char *s1)
{
	while (*s1)
	{
		*s1=UpperToLower[(unsigned)*s1];
		s1++;
	}
}

const char * __cdecl LocalStrstri(const char *str1, const char *str2)
{
	const char *cp = str1;
	const char *s1, *s2;

	if (!*str2)
		return str1;

	while (*cp)
	{
		s1 = cp;
		s2 = str2;

		while (*s1 && *s2 && !(LocalLower(*s1) - LocalLower(*s2)))
		{
			s1++;
			s2++;
		}

		if (!*s2)
			return cp;

		cp++;
	}

	return nullptr;
}

const char * __cdecl LocalRevStrstri(const char *str1, const char *str2)
{
	size_t len1 = strlen(str1);
	size_t len2 = strlen(str2);

	if (len2 > len1)
		return nullptr;

	if (!*str2)
		return &str1[len1];

	const char *cp = &str1[len1 - len2];
	const char *s1, *s2;

	while (cp >= str1)
	{
		s1 = cp;
		s2 = str2;

		while (*s1 && *s2 && !(LocalLower(*s1) - LocalLower(*s2)))
		{
			s1++;
			s2++;
		}

		if (!*s2)
			return cp;

		cp--;
	}

	return nullptr;
}

int __cdecl LocalStricmp(const char *s1,const char *s2)
{
	while (1)
	{
		if (UpperToLower[(unsigned)*s1] != UpperToLower[(unsigned)*s2])
			return (UpperToLower[(unsigned)*s1] < UpperToLower[(unsigned)*s2]) ? -1 : 1;

		if (!*(s1++))
			break;

		s2++;
	}

	return 0;
}

int __cdecl LocalStrnicmp(const char *s1,const char *s2,int n)
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

int WINAPI LStricmp(const char *s1,const char *s2)
{
	return LocalStricmp(s1,s2);
}

int WINAPI LStrnicmp(const char *s1,const char *s2,int n)
{
	return LocalStrnicmp(s1,s2,n);
}

const char *FirstSlashA(const char *String)
{
	do
	{
		if (IsSlashA(*String))
			return String;
	}
	while (*String++);

	return nullptr;
}

bool FirstSlashA(const char *String,size_t &pos)
{
	bool Ret=false;
	const char *Ptr=FirstSlashA(String);

	if (Ptr)
	{
		pos=Ptr-String;
		Ret=true;
	}

	return Ret;
}

const char *LastSlashA(const char *String)
{
	const char *Start = String;

	while (*String++)
		;

	while (--String!=Start && !IsSlashA(*String))
		;

	return IsSlashA(*String)?String:nullptr;
}

bool LastSlashA(const char *String,size_t &pos)
{
	bool Ret=false;
	const char *Ptr=LastSlashA(String);

	if (Ptr)
	{
		pos=Ptr-String;
		Ret=true;
	}

	return Ret;
}

void AnsiToUnicodeBin(const char *lpszAnsiString, wchar_t *lpwszUnicodeString, int nLength, UINT CodePage=CP_OEMCP)
{
	if (lpszAnsiString && lpwszUnicodeString && nLength)
	{
		wmemset(lpwszUnicodeString, 0, nLength);
		MultiByteToWideChar(CodePage,0,lpszAnsiString,nLength,lpwszUnicodeString,nLength);
	}
}

wchar_t *AnsiToUnicodeBin(const char *lpszAnsiString, int nLength, UINT CodePage=CP_OEMCP)
{
	wchar_t *lpResult = (wchar_t*)xf_malloc(nLength*sizeof(wchar_t));
	AnsiToUnicodeBin(lpszAnsiString,lpResult,nLength,CodePage);
	return lpResult;
}

wchar_t *AnsiToUnicode(const char *lpszAnsiString, UINT CodePage=CP_OEMCP)
{
	if (!lpszAnsiString)
		return nullptr;

	return AnsiToUnicodeBin(lpszAnsiString,(int)strlen(lpszAnsiString)+1,CodePage);
}

char *UnicodeToAnsiBin(const wchar_t *lpwszUnicodeString, int nLength, UINT CodePage=CP_OEMCP)
{
	/* $ 06.01.2008 TS
		! Увеличил размер выделяемой под строку памяти на 1 байт для нормальной
			работы старых плагинов, которые не знали, что надо смотреть на длину,
			а не на завершающий ноль (например в EditorGetString.StringText).
	*/
	if (!lpwszUnicodeString || (nLength < 0))
		return nullptr;

	char *lpResult = (char*)xf_malloc(nLength+1);
	memset(lpResult, 0, nLength+1);

	if (nLength)
	{
		WideCharToMultiByte(
		    CodePage,
		    0,
		    lpwszUnicodeString,
		    nLength,
		    lpResult,
		    nLength,
		    nullptr,
		    nullptr
		);
	}

	return lpResult;
}

char *UnicodeToAnsi(const wchar_t *lpwszUnicodeString, UINT CodePage=CP_OEMCP)
{
	if (!lpwszUnicodeString)
		return nullptr;

	return UnicodeToAnsiBin(lpwszUnicodeString,StrLength(lpwszUnicodeString)+1,CodePage);
}

wchar_t **ArrayAnsiToUnicode(char ** lpaszAnsiString, size_t iCount)
{
	wchar_t** lpaResult = nullptr;

	if (lpaszAnsiString)
	{
		lpaResult = (wchar_t**) xf_malloc((iCount+1)*sizeof(wchar_t*));

		if (lpaResult)
		{
			for (size_t i=0; i<iCount; i++)
			{
				lpaResult[i]=(lpaszAnsiString[i])?AnsiToUnicode(lpaszAnsiString[i]):nullptr;
			}

			lpaResult[iCount] = (wchar_t*)(intptr_t) 1; //Array end mark
		}
	}

	return lpaResult;
}

void FreeArrayUnicode(wchar_t ** lpawszUnicodeString)
{
	if (lpawszUnicodeString)
	{
		for (int i=0; (intptr_t)lpawszUnicodeString[i] != 1; i++) //Until end mark
		{
			if (lpawszUnicodeString[i]) xf_free(lpawszUnicodeString[i]);
		}

		xf_free(lpawszUnicodeString);
	}
}

DWORD OldKeyToKey(DWORD dOldKey)
{
	if (dOldKey&0x100)
	{
		dOldKey=(dOldKey^0x100)|EXTENDED_KEY_BASE;
	}
	else if (dOldKey&0x200)
	{
		dOldKey=(dOldKey^0x200)|INTERNAL_KEY_BASE;
	}
	else
	{
		DWORD CleanKey=dOldKey&~KEY_CTRLMASK;

		if (CleanKey>0x80 && CleanKey<0x100)
		{
			char OemChar=static_cast<char>(CleanKey);
			wchar_t WideChar=0;
			MultiByteToWideChar(CP_OEMCP,0,&OemChar,1,&WideChar,1);
			dOldKey=(dOldKey^CleanKey)|WideChar;
		}
	}

	return dOldKey;
}

DWORD KeyToOldKey(DWORD dKey)
{
	if (dKey&EXTENDED_KEY_BASE)
	{
		dKey=(dKey^EXTENDED_KEY_BASE)|0x100;
	}
	else if (dKey&INTERNAL_KEY_BASE)
	{
		dKey=(dKey^INTERNAL_KEY_BASE)|0x200;
	}
	else
	{
		DWORD CleanKey=dKey&~KEY_CTRLMASK;

		if (CleanKey>0x80 && CleanKey<0x10000)
		{
			wchar_t WideChar=static_cast<wchar_t>(CleanKey);
			char OemChar=0;
			WideCharToMultiByte(CP_OEMCP,0,&WideChar,1,&OemChar,1,0,nullptr);
			dKey=(dKey^CleanKey)|OemChar;
		}
	}

	return dKey;
}


void ConvertInfoPanelLinesA(const oldfar::InfoPanelLine *iplA, InfoPanelLine **piplW, size_t iCount)
{
	if (iplA && piplW && (iCount>0))
	{
		InfoPanelLine *iplW = (InfoPanelLine *) xf_malloc(iCount*sizeof(InfoPanelLine));

		if (iplW)
		{
			for (size_t i=0; i<iCount; i++)
			{
				iplW[i].Text=AnsiToUnicodeBin(iplA[i].Text,80); //BUGBUG
				iplW[i].Data=AnsiToUnicodeBin(iplA[i].Data,80); //BUGBUG
				iplW[i].Separator=iplA[i].Separator;
			}
		}

		*piplW = iplW;
	}
}

void FreeUnicodeInfoPanelLines(InfoPanelLine *iplW,size_t InfoLinesNumber)
{
	for (size_t i=0; i<InfoLinesNumber; i++)
	{
		if (iplW[i].Text)
			xf_free((void*)iplW[i].Text);

		if (iplW[i].Data)
			xf_free((void*)iplW[i].Data);
	}

	if (iplW)
		xf_free((void*)iplW);
}

void ConvertPanelModesA(const oldfar::PanelMode *pnmA, PanelMode **ppnmW, size_t iCount)
{
	if (pnmA && ppnmW && (iCount>0))
	{
		PanelMode *pnmW = new PanelMode[iCount]();
		if (pnmW)
		{
			for (size_t i=0; i<iCount; i++)
			{
				size_t iColumnCount = 0;

				if (pnmA[i].ColumnTypes)
				{
					char *lpTypes = xf_strdup(pnmA[i].ColumnTypes);
					const char *lpToken = strtok(lpTypes, ",");

					while (lpToken && *lpToken)
					{
						iColumnCount++;
						lpToken = strtok(nullptr, ",");
					}

					xf_free(lpTypes);
				}

				pnmW[i].StructSize = sizeof(PanelMode);
				pnmW[i].ColumnTypes = (pnmA[i].ColumnTypes)?AnsiToUnicode(pnmA[i].ColumnTypes):nullptr;
				pnmW[i].ColumnWidths = (pnmA[i].ColumnWidths)?AnsiToUnicode(pnmA[i].ColumnWidths):nullptr;
				pnmW[i].ColumnTitles = (pnmA[i].ColumnTitles && (iColumnCount>0))?ArrayAnsiToUnicode(pnmA[i].ColumnTitles,iColumnCount):nullptr;
				pnmW[i].StatusColumnTypes = (pnmA[i].StatusColumnTypes)?AnsiToUnicode(pnmA[i].StatusColumnTypes):nullptr;
				pnmW[i].StatusColumnWidths = (pnmA[i].StatusColumnWidths)?AnsiToUnicode(pnmA[i].StatusColumnWidths):nullptr;
				pnmW[i].Flags = 0;
				if (pnmA[i].FullScreen) pnmW[i].Flags |= PMFLAGS_FULLSCREEN;
				if (pnmA[i].DetailedStatus) pnmW[i].Flags |= PMFLAGS_DETAILEDSTATUS;
				if (pnmA[i].AlignExtensions) pnmW[i].Flags |= PMFLAGS_ALIGNEXTENSIONS;
				if (pnmA[i].CaseConversion) pnmW[i].Flags |= PMFLAGS_CASECONVERSION;
			}
		}

		*ppnmW = pnmW;
	}
}

void FreeUnicodePanelModes(PanelMode *pnmW, size_t iCount)
{
	if (pnmW)
	{
		for (size_t i=0; i<iCount; i++)
		{
			if (pnmW[i].ColumnTypes) xf_free((void*)pnmW[i].ColumnTypes);

			if (pnmW[i].ColumnWidths) xf_free((void*)pnmW[i].ColumnWidths);

			if (pnmW[i].ColumnTitles)	FreeArrayUnicode((wchar_t**)pnmW[i].ColumnTitles);

			if (pnmW[i].StatusColumnTypes) xf_free((void*)pnmW[i].StatusColumnTypes);

			if (pnmW[i].StatusColumnWidths) xf_free((void*)pnmW[i].StatusColumnWidths);
		}

		delete[] pnmW;
	}
}


static void ProcLabels(const char *Title,KeyBarLabel *Label, int& j, WORD Key, DWORD Shift)
{
	if (Title)
	{
		Label->Text=AnsiToUnicode(Title);
		Label->LongText=nullptr;
		Label->Key.VirtualKeyCode=Key;
		Label->Key.ControlKeyState=Shift;
		j++;
	}
}

void ConvertKeyBarTitlesA(const oldfar::KeyBarTitles *kbtA, KeyBarTitles *kbtW, bool FullStruct)
{
	if (kbtA && kbtW)
	{
		int i;
		kbtW->CountLabels=0;
		kbtW->Labels=nullptr;

		for (i=0; i<12; i++)
		{
			if (kbtA->Titles[i])
				kbtW->CountLabels++;
			if (kbtA->CtrlTitles[i])
				kbtW->CountLabels++;
			if (kbtA->AltTitles[i])
				kbtW->CountLabels++;
			if (kbtA->ShiftTitles[i])
				kbtW->CountLabels++;
			if (FullStruct)
			{
				if (kbtA->CtrlShiftTitles[i])
					kbtW->CountLabels++;
				if (kbtA->AltShiftTitles[i])
					kbtW->CountLabels++;
				if (kbtA->CtrlAltTitles[i])
					kbtW->CountLabels++;
			}
		}

		if (kbtW->CountLabels)
		{
			kbtW->Labels=new KeyBarLabel[kbtW->CountLabels];
			if (kbtW->Labels)
			{
				int j;
				for (j=i=0; i<12; i++)
				{
					if (kbtA->Titles[i])
						ProcLabels(kbtA->Titles[i],kbtW->Labels+j,j,VK_F1+i,0);
					if (kbtA->CtrlTitles[i])
						ProcLabels(kbtA->CtrlTitles[i],kbtW->Labels+j,j,VK_F1+i,LEFT_CTRL_PRESSED);
					if (kbtA->AltTitles[i])
						ProcLabels(kbtA->AltTitles[i],kbtW->Labels+j,j,VK_F1+i,LEFT_ALT_PRESSED);
					if (kbtA->ShiftTitles[i])
						ProcLabels(kbtA->ShiftTitles[i],kbtW->Labels+j,j,VK_F1+i,SHIFT_PRESSED);

					if (FullStruct)
					{
						if (kbtA->CtrlShiftTitles[i])
							ProcLabels(kbtA->CtrlShiftTitles[i],kbtW->Labels+j,j,VK_F1+i,LEFT_CTRL_PRESSED|SHIFT_PRESSED);
						if (kbtA->AltShiftTitles[i])
							ProcLabels(kbtA->AltShiftTitles[i],kbtW->Labels+j,j,VK_F1+i,LEFT_ALT_PRESSED|SHIFT_PRESSED);
						if (kbtA->CtrlAltTitles[i])
							ProcLabels(kbtA->CtrlAltTitles[i],kbtW->Labels+j,j,VK_F1+i,LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED);
					}
				}

			}
			else
				kbtW->CountLabels=0;
		}
	}
}

void FreeUnicodeKeyBarTitles(KeyBarTitles *kbtW)
{
	if (kbtW && kbtW->CountLabels && kbtW->Labels)
	{
		for (size_t i=0; i < kbtW->CountLabels; i++)
		{
			if (kbtW->Labels[i].Text)
				xf_free((void*)kbtW->Labels[i].Text);
		}
		delete[] kbtW->Labels;
		kbtW->Labels=nullptr;
		kbtW->CountLabels=0;
	}
}

static void WINAPI FreeUserData(void* UserData,HANDLE hPlugin,unsigned __int64 Flags)
{
	xf_free(UserData);
}

void ConvertPanelItemA(const oldfar::PluginPanelItem *PanelItemA, PluginPanelItem **PanelItemW, size_t ItemsNumber)
{
	*PanelItemW = (PluginPanelItem *)xf_malloc(ItemsNumber*sizeof(PluginPanelItem));
	memset(*PanelItemW,0,ItemsNumber*sizeof(PluginPanelItem));

	for (size_t i=0; i<ItemsNumber; i++)
	{
		(*PanelItemW)[i].Flags = 0;
		if(PanelItemA[i].Flags&oldfar::PPIF_PROCESSDESCR)
			(*PanelItemW)[i].Flags|=PPIF_PROCESSDESCR;
		if(PanelItemA[i].Flags&oldfar::PPIF_SELECTED)
			(*PanelItemW)[i].Flags|=PPIF_SELECTED;

		(*PanelItemW)[i].NumberOfLinks = PanelItemA[i].NumberOfLinks;

		if (PanelItemA[i].Description)
			(*PanelItemW)[i].Description = AnsiToUnicode(PanelItemA[i].Description);

		if (PanelItemA[i].Owner)
			(*PanelItemW)[i].Owner = AnsiToUnicode(PanelItemA[i].Owner);

		if (PanelItemA[i].CustomColumnNumber)
		{
			(*PanelItemW)[i].CustomColumnNumber = PanelItemA[i].CustomColumnNumber;
			(*PanelItemW)[i].CustomColumnData = ArrayAnsiToUnicode(PanelItemA[i].CustomColumnData,PanelItemA[i].CustomColumnNumber);
		}

		if(PanelItemA[i].Flags&oldfar::PPIF_USERDATA)
		{
			void* UserData = (void*)PanelItemA[i].UserData;
			DWORD Size=*(DWORD *)UserData;
			(*PanelItemW)[i].UserData.UserData = xf_malloc(Size);
			memcpy((*PanelItemW)[i].UserData.UserData,UserData,Size);
			(*PanelItemW)[i].UserData.Callback = FreeUserData;
		}
		else
		{
			(*PanelItemW)[i].UserData.UserData = (void*)PanelItemA[i].UserData;
			(*PanelItemW)[i].UserData.Callback = nullptr;
		}
		(*PanelItemW)[i].CRC32 = PanelItemA[i].CRC32;
		(*PanelItemW)[i].FileAttributes = PanelItemA[i].FindData.dwFileAttributes;
		(*PanelItemW)[i].CreationTime = PanelItemA[i].FindData.ftCreationTime;
		(*PanelItemW)[i].LastAccessTime = PanelItemA[i].FindData.ftLastAccessTime;
		(*PanelItemW)[i].LastWriteTime = PanelItemA[i].FindData.ftLastWriteTime;
		(*PanelItemW)[i].FileSize = (unsigned __int64)PanelItemA[i].FindData.nFileSizeLow + (((unsigned __int64)PanelItemA[i].FindData.nFileSizeHigh)<<32);
		(*PanelItemW)[i].AllocationSize = (unsigned __int64)PanelItemA[i].PackSize + (((unsigned __int64)PanelItemA[i].PackSizeHigh)<<32);
		(*PanelItemW)[i].FileName = AnsiToUnicode(PanelItemA[i].FindData.cFileName);
		(*PanelItemW)[i].AlternateFileName = AnsiToUnicode(PanelItemA[i].FindData.cAlternateFileName);
	}
}

void ConvertPanelItemToAnsi(const PluginPanelItem &PanelItem, oldfar::PluginPanelItem &PanelItemA)
{
	PanelItemA.Flags = 0;

	if(PanelItem.Flags&PPIF_PROCESSDESCR)
		PanelItemA.Flags|=oldfar::PPIF_PROCESSDESCR;

	if(PanelItem.Flags&PPIF_SELECTED)
		PanelItemA.Flags|=oldfar::PPIF_SELECTED;

	if(PanelItem.UserData.Callback==FreeUserData)
		PanelItemA.Flags|=oldfar::PPIF_USERDATA;

	PanelItemA.NumberOfLinks=PanelItem.NumberOfLinks;

	if (PanelItem.Description)
		PanelItemA.Description=UnicodeToAnsi(PanelItem.Description);

	if (PanelItem.Owner)
		PanelItemA.Owner=UnicodeToAnsi(PanelItem.Owner);

	if (PanelItem.CustomColumnNumber)
	{
		PanelItemA.CustomColumnNumber=static_cast<int>(PanelItem.CustomColumnNumber);
		PanelItemA.CustomColumnData=(char **)xf_malloc(PanelItem.CustomColumnNumber*sizeof(char *));

		for (size_t j=0; j<PanelItem.CustomColumnNumber; j++)
			PanelItemA.CustomColumnData[j] = UnicodeToAnsi(PanelItem.CustomColumnData[j]);
	}

	if (PanelItem.UserData.UserData&&PanelItem.UserData.Callback==FreeUserData)
	{
		DWORD Size=*(DWORD *)PanelItem.UserData.UserData;
		PanelItemA.UserData=(intptr_t)xf_malloc(Size);
		memcpy((void *)PanelItemA.UserData,PanelItem.UserData.UserData,Size);
	}
	else
		PanelItemA.UserData = (intptr_t)PanelItem.UserData.UserData;

	PanelItemA.CRC32 = PanelItem.CRC32;
	PanelItemA.FindData.dwFileAttributes = PanelItem.FileAttributes;
	PanelItemA.FindData.ftCreationTime = PanelItem.CreationTime;
	PanelItemA.FindData.ftLastAccessTime = PanelItem.LastAccessTime;
	PanelItemA.FindData.ftLastWriteTime = PanelItem.LastWriteTime;
	PanelItemA.FindData.nFileSizeLow = (DWORD)PanelItem.FileSize;
	PanelItemA.FindData.nFileSizeHigh = (DWORD)(PanelItem.FileSize>>32);
	PanelItemA.PackSize = (DWORD)PanelItem.AllocationSize;
	PanelItemA.PackSizeHigh = (DWORD)(PanelItem.AllocationSize>>32);
	UnicodeToOEM(PanelItem.FileName,PanelItemA.FindData.cFileName,sizeof(PanelItemA.FindData.cFileName));
	UnicodeToOEM(PanelItem.AlternateFileName,PanelItemA.FindData.cAlternateFileName,sizeof(PanelItemA.FindData.cAlternateFileName));
}

void ConvertPanelItemsArrayToAnsi(const PluginPanelItem *PanelItemW, oldfar::PluginPanelItem *&PanelItemA, size_t ItemsNumber)
{
	PanelItemA = (oldfar::PluginPanelItem *)xf_malloc(ItemsNumber*sizeof(oldfar::PluginPanelItem));
	memset(PanelItemA,0,ItemsNumber*sizeof(oldfar::PluginPanelItem));

	for (size_t i=0; i<ItemsNumber; i++)
	{
		ConvertPanelItemToAnsi(PanelItemW[i],PanelItemA[i]);
	}
}

void FreeUnicodePanelItem(PluginPanelItem *PanelItem, size_t ItemsNumber)
{
	for (size_t i=0; i<ItemsNumber; i++)
	{
		if (PanelItem[i].Description)
			xf_free((void*)PanelItem[i].Description);

		if (PanelItem[i].Owner)
			xf_free((void*)PanelItem[i].Owner);

		if (PanelItem[i].CustomColumnNumber)
		{
			for (size_t j=0; j<PanelItem[i].CustomColumnNumber; j++)
				xf_free((void*)PanelItem[i].CustomColumnData[j]);

			xf_free((void*)PanelItem[i].CustomColumnData);
		}

		FreePluginPanelItem(&PanelItem[i]);
	}

	xf_free(PanelItem);
}

void FreePanelItemA(oldfar::PluginPanelItem *PanelItem, size_t ItemsNumber, bool bFreeArray=true)
{
	for (size_t i=0; i<ItemsNumber; i++)
	{
		if (PanelItem[i].Description)
			xf_free(PanelItem[i].Description);

		if (PanelItem[i].Owner)
			xf_free(PanelItem[i].Owner);

		if (PanelItem[i].CustomColumnNumber)
		{
			for (int j=0; j<PanelItem[i].CustomColumnNumber; j++)
				xf_free(PanelItem[i].CustomColumnData[j]);

			xf_free(PanelItem[i].CustomColumnData);
		}

		if (PanelItem[i].UserData&&PanelItem[i].Flags&oldfar::PPIF_USERDATA)
		{
			xf_free((PVOID)PanelItem[i].UserData);
		}
	}

	if (bFreeArray)
		xf_free(PanelItem);
}

char* WINAPI RemoveTrailingSpacesA(char *Str)
{
	if (!Str)
		return nullptr;

	if (*Str == '\0')
		return Str;

	char *ChPtr;
	size_t I;

	for (ChPtr=Str+(I=strlen(Str))-1; I > 0; I--, ChPtr--)
	{
		if (IsSpaceA(*ChPtr) || IsEolA(*ChPtr))
			*ChPtr=0;
		else
			break;
	}

	return Str;
}

char *WINAPI FarItoaA(int value, char *string, int radix)
{
	if (string)
		return _itoa(value,string,radix);

	return nullptr;
}

char *WINAPI FarItoa64A(__int64 value, char *string, int radix)
{
	if (string)
		return _i64toa(value, string, radix);

	return nullptr;
}

int WINAPI FarAtoiA(const char *s)
{
	if (s)
		return atoi(s);

	return 0;
}

__int64 WINAPI FarAtoi64A(const char *s)
{
	return s?_atoi64(s):0;
}

char* WINAPI PointToNameA(char *Path)
{
	if (!Path)
		return nullptr;

	char *NamePtr=Path;

	while (*Path)
	{
		if (IsSlashA(*Path) || (*Path==':' && Path==NamePtr+1))
			NamePtr=Path+1;

		Path++;
	}

	return(NamePtr);
}

void WINAPI UnquoteA(char *Str)
{
	if (!Str)
		return;

	char *Dst=Str;

	while (*Str)
	{
		if (*Str!='\"')
			*Dst++=*Str;

		Str++;
	}

	*Dst=0;
}

char* WINAPI RemoveLeadingSpacesA(char *Str)
{
	char *ChPtr;

	if (!(ChPtr=Str))
		return nullptr;

	for (; IsSpaceA(*ChPtr); ChPtr++)
		;

	if (ChPtr!=Str)
		memmove(Str,ChPtr,strlen(ChPtr)+1);

	return Str;
}

char* WINAPI RemoveExternalSpacesA(char *Str)
{
	return RemoveTrailingSpacesA(RemoveLeadingSpacesA(Str));
}

char* WINAPI TruncStrA(char *Str,int MaxLength)
{
	if (Str)
	{
		int Length;

		if (MaxLength<0)
			MaxLength=0;

		if ((Length=(int)strlen(Str))>MaxLength)
		{
			if (MaxLength>3)
			{
				char *MovePos = Str+Length-MaxLength+3;
				memmove(Str+3,MovePos,strlen(MovePos)+1);
				memcpy(Str,"...",3);
			}

			Str[MaxLength]=0;
		}
	}

	return(Str);
}

char* WINAPI TruncPathStrA(char *Str, int MaxLength)
{
	if (Str)
	{
		int nLength = (int)strlen(Str);

		if (nLength > MaxLength)
		{
			char *lpStart = nullptr;

			if (*Str && (Str[1] == ':') && IsSlash(Str[2]))
				lpStart = Str+3;
			else
			{
				if ((Str[0] == '\\') && (Str[1] == '\\'))
				{
					if ((lpStart = const_cast<char*>(FirstSlashA(Str+2))) )
						if ((lpStart = const_cast<char*>(FirstSlashA(lpStart+1))))
							lpStart++;
				}
			}

			if (!lpStart || (lpStart-Str > MaxLength-5))
				return TruncStrA(Str, MaxLength);

			char *lpInPos = lpStart+3+(nLength-MaxLength);
			memmove(lpStart+3, lpInPos, strlen(lpInPos)+1);
			memcpy(lpStart, "...", 3);
		}
	}

	return Str;
}

char *InsertQuoteA(char *Str)
{
	size_t l = strlen(Str);

	if (*Str != '"')
	{
		memmove(Str+1,Str,++l);
		*Str='"';
	}

	if (Str[l-1] != '"')
	{
		Str[l++] = '\"';
		Str[l] = 0;
	}

	return Str;
}

char* WINAPI QuoteSpaceOnlyA(char *Str)
{
	if (Str && strchr(Str,' '))
		InsertQuoteA(Str);

	return(Str);
}

BOOL AddEndSlashA(char *Path,char TypeSlash)
{
	BOOL Result=FALSE;

	if (Path)
	{
		/* $ 06.12.2000 IS
		  ! Теперь функция работает с обоими видами слешей, также происходит
		    изменение уже существующего конечного слеша на такой, который
		    встречается чаще.
		*/
		char *end;
		int Slash=0, BackSlash=0;

		if (!TypeSlash)
		{
			end=Path;

			while (*end)
			{
				Slash+=(*end=='\\');
				BackSlash+=(*end=='/');
				end++;
			}
		}
		else
		{
			end=Path+strlen(Path);

			if (TypeSlash == '\\')
				Slash=1;
			else
				BackSlash=1;
		}

		int Length=(int)(end-Path);
		char c=(Slash<BackSlash)?'/':'\\';
		Result=TRUE;

		if (!Length)
		{
			*end=c;
			end[1]=0;
		}
		else
		{
			end--;

			if (!IsSlashA(*end))
			{
				end[1]=c;
				end[2]=0;
			}
			else
				*end=c;
		}

		/* IS $ */
	}

	return Result;
}

BOOL WINAPI AddEndSlashA(char *Path)
{
	return AddEndSlashA(Path,0);
}

void WINAPI GetPathRootA(const char *Path, char *Root)
{
	string strPath(Path);
	wchar_t Buffer[MAX_PATH];
	NativeFSF.GetPathRoot(strPath, Buffer, ARRAYSIZE(Buffer));
	UnicodeToOEM(Buffer, Root, ARRAYSIZE(Buffer));
}

int WINAPI CopyToClipboardA(const char *Data)
{
	wchar_t *p = Data?AnsiToUnicode(Data):nullptr;
	int ret = NativeFSF.CopyToClipboard(p);

	if (p) xf_free(p);

	return ret;
}

char* WINAPI PasteFromClipboardA()
{
	wchar_t *p = NativeFSF.PasteFromClipboard();

	if (p)
		return UnicodeToAnsi(p);

	return nullptr;
}

void WINAPI DeleteBufferA(void *Buffer)
{
	if (Buffer) xf_free(Buffer);
}

int WINAPI ProcessNameA(const char *Param1,char *Param2,DWORD Flags)
{
	string strP1(Param1), strP2(Param2);
	int size = (int)(strP1.GetLength()+strP2.GetLength()+oldfar::NM)+1; //а хрен ещё как угадать скока там этот Param2 для PN_GENERATENAME
	wchar_t *p=(wchar_t *)xf_malloc(size*sizeof(wchar_t));
	wcscpy(p,strP2);
	int newFlags = 0;

	if (Flags&oldfar::PN_SKIPPATH)
	{
		newFlags|=PN_SKIPPATH;
		Flags &= ~oldfar::PN_SKIPPATH;
	}

	if (Flags == oldfar::PN_CMPNAME)
	{
		newFlags|=PN_CMPNAME;
	}
	else if (Flags == oldfar::PN_CMPNAMELIST)
	{
		newFlags|=PN_CMPNAMELIST;
	}
	else if (Flags&oldfar::PN_GENERATENAME)
	{
		newFlags|=PN_GENERATENAME|(Flags&0xFF);
	}

	int ret = static_cast<int>(NativeFSF.ProcessName(strP1,p,size,newFlags));

	if (newFlags&PN_GENERATENAME)
		UnicodeToOEM(p,Param2,size);

	xf_free(p);
	return ret;
}

int WINAPI KeyNameToKeyA(const char *Name)
{
	string strN(Name);
	return KeyToOldKey(KeyNameToKey(strN));
}

BOOL WINAPI FarKeyToNameA(int Key,char *KeyText,int Size)
{
	string strKT;
	if (KeyToText(OldKeyToKey(Key),strKT))
	{
		UnicodeToOEM(strKT, KeyText,Size>0?Size+1:32);
		return TRUE;
	}
	return FALSE;
}

int WINAPI InputRecordToKeyA(const INPUT_RECORD *r)
{
	return KeyToOldKey(InputRecordToKey(r));
}

char* WINAPI FarMkTempA(char *Dest, const char *Prefix)
{
	string strP(Prefix);
	wchar_t D[oldfar::NM] = {};
	NativeFSF.MkTemp(D,ARRAYSIZE(D),strP);
	UnicodeToOEM(D,Dest,sizeof(D));
	return Dest;
}

int WINAPI FarMkLinkA(const char *Src,const char *Dest, DWORD OldFlags)
{
	string strS(Src), strD(Dest);
	LINK_TYPE Type = LINK_HARDLINK;

	switch (OldFlags&0xf)
	{
		case oldfar::FLINK_HARDLINK:    Type = LINK_HARDLINK; break;
		case oldfar::FLINK_JUNCTION:    Type = LINK_JUNCTION; break;
		case oldfar::FLINK_VOLMOUNT:    Type = LINK_VOLMOUNT; break;
		case oldfar::FLINK_SYMLINKFILE: Type = LINK_SYMLINKFILE; break;
		case oldfar::FLINK_SYMLINKDIR:  Type = LINK_SYMLINKDIR; break;
	}

	MKLINK_FLAGS Flags = MLF_NONE;
	if (OldFlags&oldfar::FLINK_SHOWERRMSG)       Flags|=MLF_SHOWERRMSG;

	if (OldFlags&oldfar::FLINK_DONOTUPDATEPANEL) Flags|=MLF_DONOTUPDATEPANEL;

	return NativeFSF.MkLink(strS, strD, Type, Flags);
}

int WINAPI GetNumberOfLinksA(const char *Name)
{
	string n(Name);
	return static_cast<int>(NativeFSF.GetNumberOfLinks(n));
}

int WINAPI ConvertNameToRealA(const char *Src,char *Dest,int DestSize)
{
	string strSrc(Src),strDest;
	ConvertNameToReal(strSrc,strDest);

	if (!Dest)
		return (int)strDest.GetLength();
	else
		strDest.GetCharString(Dest,DestSize);

	return Min((int)strDest.GetLength(),DestSize);
}

int WINAPI FarGetReparsePointInfoA(const char *Src,char *Dest,int DestSize)
{
	int Result = 0;
	if (Src && *Src)
	{
		string strSrc(Src);
		wchar_t Buffer[MAX_PATH];
		Result = static_cast<int>(NativeFSF.GetReparsePointInfo(strSrc, Buffer, ARRAYSIZE(Buffer)));
		if (DestSize && Dest)
		{
			if(Result > MAX_PATH)
			{
				wchar_t* Tmp = new wchar_t[DestSize];
				NativeFSF.GetReparsePointInfo(strSrc, Tmp, DestSize);
				UnicodeToOEM(Tmp, Dest, DestSize);
				delete[] Tmp;
			}
			else
			{
				UnicodeToOEM(Buffer, Dest, DestSize);
			}
		}
	}
	return Result;
}

struct FAR_SEARCH_A_CALLBACK_PARAM
{
	oldfar::FRSUSERFUNC Func;
	void *Param;
};

static int WINAPI FarRecursiveSearchA_Callback(const PluginPanelItem *FData,const wchar_t *FullName,void *param)
{
	FAR_SEARCH_A_CALLBACK_PARAM* pCallbackParam = static_cast<FAR_SEARCH_A_CALLBACK_PARAM*>(param);
	WIN32_FIND_DATAA FindData={};
	FindData.dwFileAttributes = FData->FileAttributes;
	FindData.ftCreationTime = FData->CreationTime;
	FindData.ftLastAccessTime = FData->LastAccessTime;
	FindData.ftLastWriteTime = FData->LastWriteTime;
	FindData.nFileSizeLow = (DWORD)FData->FileSize;
	FindData.nFileSizeHigh = (DWORD)(FData->FileSize>>32);
	UnicodeToOEM(FData->FileName,FindData.cFileName,sizeof(FindData.cFileName));
	UnicodeToOEM(FData->AlternateFileName,FindData.cAlternateFileName,sizeof(FindData.cAlternateFileName));
	char FullNameA[oldfar::NM];
	UnicodeToOEM(FullName,FullNameA,sizeof(FullNameA));
	return pCallbackParam->Func(&FindData,FullNameA,pCallbackParam->Param);
}

void WINAPI FarRecursiveSearchA(const char *InitDir,const char *Mask,oldfar::FRSUSERFUNC Func,DWORD Flags,void *Param)
{
	string strInitDir(InitDir);
	string strMask(Mask);
	FAR_SEARCH_A_CALLBACK_PARAM CallbackParam;
	CallbackParam.Func = Func;
	CallbackParam.Param = Param;
	int newFlags = 0;

	if (Flags&oldfar::FRS_RETUPDIR) newFlags|=FRS_RETUPDIR;

	if (Flags&oldfar::FRS_RECUR) newFlags|=FRS_RECUR;

	if (Flags&oldfar::FRS_SCANSYMLINK) newFlags|=FRS_SCANSYMLINK;

	NativeFSF.FarRecursiveSearch(static_cast<const wchar_t *>(strInitDir),static_cast<const wchar_t *>(strMask),FarRecursiveSearchA_Callback,newFlags,static_cast<void *>(&CallbackParam));
}

DWORD WINAPI ExpandEnvironmentStrA(const char *src, char *dest, size_t size)
{
	string strS(src), strD;
	apiExpandEnvironmentStrings(strS,strD);
	DWORD len = (DWORD)Min(strD.GetLength(),size-1);
	strD.GetCharString(dest,len+1);
	return len;
}

int WINAPI FarViewerA(const char *FileName,const char *Title,int X1,int Y1,int X2,int Y2,DWORD Flags)
{
	string strFN(FileName), strT(Title);
	return NativeInfo.Viewer(strFN,strT,X1,Y1,X2,Y2,Flags,CP_DEFAULT);
}

int WINAPI FarEditorA(const char *FileName,const char *Title,int X1,int Y1,int X2,int Y2,DWORD Flags,int StartLine,int StartChar)
{
	string strFN(FileName), strT(Title);
	return NativeInfo.Editor(strFN,strT,X1,Y1,X2,Y2,Flags,StartLine,StartChar,CP_DEFAULT);
}

int WINAPI FarCmpNameA(const char *pattern,const char *str,int skippath)
{
	return ProcessNameA(pattern, const_cast<char*>(str), oldfar::PN_CMPNAME|(skippath?oldfar::PN_SKIPPATH:0));
}

void WINAPI FarTextA(int X,int Y,int ConColor,const char *Str)
{
	FarColor Color;
	Colors::ConsoleColorToFarColor(ConColor, Color);
	if (!Str) return NativeInfo.Text(X,Y,&Color,nullptr);

	string strS(Str);
	return NativeInfo.Text(X,Y,&Color,strS);
}

BOOL WINAPI FarShowHelpA(const char *ModuleName,const char *HelpTopic,DWORD Flags)
{
	string strMN(ModuleName), strHT(HelpTopic);
	return NativeInfo.ShowHelp(strMN,(HelpTopic?strHT.CPtr():nullptr),Flags);
}

int WINAPI FarInputBoxA(const char *Title,const char *Prompt,const char *HistoryName,const char *SrcText,char *DestText,int DestLength,const char *HelpTopic,DWORD Flags)
{
	string strT(Title), strP(Prompt), strHN(HistoryName), strST(SrcText), strD, strHT(HelpTopic);
	wchar_t *D = strD.GetBuffer(DestLength);
	DWORD NewFlags=0;

	if (Flags&oldfar::FIB_ENABLEEMPTY)
		NewFlags|=FIB_ENABLEEMPTY;
	if (Flags&oldfar::FIB_PASSWORD)
		NewFlags|=FIB_PASSWORD;
	if (Flags&oldfar::FIB_EXPANDENV)
		NewFlags|=FIB_EXPANDENV;
	if (Flags&oldfar::FIB_NOUSELASTHISTORY)
		NewFlags|=FIB_NOUSELASTHISTORY;
	if (Flags&oldfar::FIB_BUTTONS)
		NewFlags|=FIB_BUTTONS;
	if (Flags&oldfar::FIB_NOAMPERSAND)
		NewFlags|=FIB_NOAMPERSAND;

	int ret = NativeInfo.InputBox(&FarGuid,&FarGuid,(Title?strT.CPtr():nullptr),(Prompt?strP.CPtr():nullptr),(HistoryName?strHN.CPtr():nullptr),(SrcText?strST.CPtr():nullptr),D,DestLength,(HelpTopic?strHT.CPtr():nullptr),NewFlags);
	strD.ReleaseBuffer();

	if (ret && DestText)
		strD.GetCharString(DestText,DestLength+1);

	return ret;
}

#define GetPluginGuid(n) &reinterpret_cast<Plugin*>(n)->GetGUID()
int WINAPI FarMessageFnA(intptr_t PluginNumber,DWORD Flags,const char *HelpTopic,const char * const *Items,int ItemsNumber,int ButtonsNumber)
{
	string strHT(HelpTopic);
	wchar_t **p;
	int c=0;

	Flags&=~oldfar::FMSG_DOWN;

	if (Flags&oldfar::FMSG_ALLINONE)
	{
		p = (wchar_t **)AnsiToUnicode((const char *)Items);
	}
	else
	{
		c = ItemsNumber;
		p = (wchar_t **)xf_malloc(c*sizeof(wchar_t*));

		for (int i=0; i<c; i++)
			p[i] = AnsiToUnicode(Items[i]);
	}

	FARMESSAGEFLAGS NewFlags = FMSG_NONE;
	if (Flags&oldfar::FMSG_WARNING)
		NewFlags|=FMSG_WARNING;
	if (Flags&oldfar::FMSG_ERRORTYPE)
		NewFlags|=FMSG_ERRORTYPE;
	if (Flags&oldfar::FMSG_KEEPBACKGROUND)
		NewFlags|=FMSG_KEEPBACKGROUND;
	if (Flags&oldfar::FMSG_LEFTALIGN)
		NewFlags|=FMSG_LEFTALIGN;
	if (Flags&oldfar::FMSG_ALLINONE)
		NewFlags|=FMSG_ALLINONE;

	switch(Flags&0x000f0000)
	{
	case oldfar::FMSG_MB_OK:
		NewFlags|=FMSG_MB_OK;
		break;
	case oldfar::FMSG_MB_OKCANCEL:
		NewFlags|=FMSG_MB_OKCANCEL;
		break;
	case oldfar::FMSG_MB_ABORTRETRYIGNORE:
		NewFlags|=FMSG_MB_ABORTRETRYIGNORE;
		break;
	case oldfar::FMSG_MB_YESNO:
		NewFlags|=FMSG_MB_YESNO;
		break;
	case oldfar::FMSG_MB_YESNOCANCEL:
		NewFlags|=FMSG_MB_YESNOCANCEL;
		break;
	case oldfar::FMSG_MB_RETRYCANCEL:
		NewFlags|=FMSG_MB_RETRYCANCEL;
		break;
	}

	int ret = NativeInfo.Message(GetPluginGuid(PluginNumber),&FarGuid,NewFlags,(HelpTopic?strHT.CPtr():nullptr),p,ItemsNumber,ButtonsNumber);

	for (int i=0; i<c; i++)
		xf_free(p[i]);

	xf_free(p);
	return ret;
}

const char * WINAPI FarGetMsgFnA(intptr_t PluginHandle,int MsgId)
{
	//BUGBUG, надо проверять, что PluginHandle - плагин
	PluginA *pPlugin = (PluginA*)PluginHandle;
	string strPath = pPlugin->GetModuleName();
	CutToSlash(strPath);

	if (pPlugin->InitLang(strPath))
		return pPlugin->GetMsgA(static_cast<LNGID>(MsgId));

	return "";
}

int WINAPI FarMenuFnA(intptr_t PluginNumber,int X,int Y,int MaxHeight,DWORD Flags,const char *Title,const char *Bottom,const char *HelpTopic,const int *BreakKeys,int *BreakCode,const oldfar::FarMenuItem *Item,int ItemsNumber)
{
	string strT(Title), strB(Bottom), strHT(HelpTopic);
	const wchar_t *wszT  = Title?strT.CPtr():nullptr;
	const wchar_t *wszB  = Bottom?strB.CPtr():nullptr;
	const wchar_t *wszHT = HelpTopic?strHT.CPtr():nullptr;

	DWORD NewFlags=0;

	if (Flags&oldfar::FMENU_SHOWAMPERSAND)
		NewFlags|=FMENU_SHOWAMPERSAND;
	if (Flags&oldfar::FMENU_WRAPMODE)
		NewFlags|=FMENU_WRAPMODE;
	if (Flags&oldfar::FMENU_AUTOHIGHLIGHT)
		NewFlags|=FMENU_AUTOHIGHLIGHT;
	if (Flags&oldfar::FMENU_REVERSEAUTOHIGHLIGHT)
		NewFlags|=FMENU_REVERSEAUTOHIGHLIGHT;
	if (Flags&oldfar::FMENU_CHANGECONSOLETITLE)
		NewFlags|=FMENU_CHANGECONSOLETITLE;

	if (!Item) ItemsNumber=0;

	FarMenuItem *mi = (FarMenuItem *)xf_malloc(ItemsNumber*sizeof(*mi));

	if (Flags&oldfar::FMENU_USEEXT)
	{
		oldfar::FarMenuItemEx *p = (oldfar::FarMenuItemEx *)Item;

		for (int i=0; i<ItemsNumber; i++)
		{
			mi[i].Flags = 0;
			if (p[i].Flags&oldfar::MIF_SELECTED)
				mi[i].Flags|=MIF_SELECTED;
			if (p[i].Flags&oldfar::MIF_CHECKED)
				mi[i].Flags|=MIF_CHECKED;
			if (p[i].Flags&oldfar::MIF_SEPARATOR)
				mi[i].Flags|=MIF_SEPARATOR;
			if (p[i].Flags&oldfar::MIF_DISABLE)
				mi[i].Flags|=MIF_DISABLE;
			if (p[i].Flags&oldfar::MIF_GRAYED)
				mi[i].Flags|=MIF_GRAYED;
			if (p[i].Flags&oldfar::MIF_HIDDEN)
				mi[i].Flags|=MIF_HIDDEN;
			mi[i].Text = AnsiToUnicode(p[i].Flags&oldfar::MIF_USETEXTPTR?p[i].TextPtr:p[i].Text);
			INPUT_RECORD input={0};
			KeyToInputRecord(OldKeyToKey(p[i].AccelKey),&input);
			mi[i].AccelKey.VirtualKeyCode = input.Event.KeyEvent.dwControlKeyState;
			mi[i].AccelKey.ControlKeyState = input.Event.KeyEvent.dwControlKeyState;
			mi[i].Reserved = p[i].Reserved;
			mi[i].UserData = p[i].UserData;
		}
	}
	else
	{
		for (int i=0; i<ItemsNumber; i++)
		{
			mi[i].Flags = 0;

			if (Item[i].Selected)
				mi[i].Flags|=MIF_SELECTED;

			if (Item[i].Checked)
			{
				mi[i].Flags|=MIF_CHECKED;

				if (Item[i].Checked>1)
					AnsiToUnicodeBin((const char*)&Item[i].Checked,(wchar_t*)&mi[i].Flags,1);
			}

			if (Item[i].Separator)
			{
				mi[i].Flags|=MIF_SEPARATOR;
				mi[i].Text = nullptr;
			}
			else
			{
				mi[i].Text = AnsiToUnicode(Item[i].Text);
			}

			mi[i].AccelKey.VirtualKeyCode = 0;
			mi[i].AccelKey.ControlKeyState = 0;
			mi[i].Reserved = 0;
			mi[i].UserData = 0;
		}
	}

	FarKey* NewBreakKeys=nullptr;
	if (BreakKeys)
	{
		int BreakKeysCount=0;
		while(BreakKeys[BreakKeysCount++]) ;
		if (BreakKeysCount)
		{
			NewBreakKeys=(FarKey*)xf_malloc(BreakKeysCount*sizeof(FarKey));
			for(int ii=0;ii<BreakKeysCount;++ii)
			{
				NewBreakKeys[ii].VirtualKeyCode=BreakKeys[ii]&0xffff;
				NewBreakKeys[ii].ControlKeyState=0;
				DWORD Flags=BreakKeys[ii]>>16;
				if(Flags&oldfar::PKF_CONTROL) NewBreakKeys[ii].ControlKeyState|=LEFT_CTRL_PRESSED;
				if(Flags&oldfar::PKF_ALT) NewBreakKeys[ii].ControlKeyState|=LEFT_ALT_PRESSED;
				if(Flags&oldfar::PKF_SHIFT) NewBreakKeys[ii].ControlKeyState|=SHIFT_PRESSED;
			}
		}
	}

	int ret = NativeInfo.Menu(GetPluginGuid(PluginNumber),&FarGuid,X,Y,MaxHeight,NewFlags,wszT,wszB,wszHT,NewBreakKeys,BreakCode,mi,ItemsNumber);

	for (int i=0; i<ItemsNumber; i++)
		if (mi[i].Text) xf_free((wchar_t *)mi[i].Text);

	if (mi) xf_free(mi);

	if (NewBreakKeys) xf_free(NewBreakKeys);

	return ret;
}

typedef struct DialogData
{
	oldfar::FARWINDOWPROC DlgProc;
	HANDLE hDlg;
	oldfar::FarDialogItem *diA;
	FarDialogItem *di;
	FarList *l;
} *PDialogData;

DList<PDialogData>DialogList;

oldfar::FarDialogItem* OneDialogItem=nullptr;

PDialogData FindCurrentDialogData(HANDLE hDlg)
{
	PDialogData Result=nullptr;
	for(PDialogData* i=DialogList.Last();i;i=DialogList.Prev(i))
	{
		if((*i)->hDlg==hDlg)
		{
			Result=*i;
			break;
		}
	}
	return Result;
}

oldfar::FarDialogItem* CurrentDialogItemA(HANDLE hDlg,int ItemNumber)
{
	PDialogData Data=FindCurrentDialogData(hDlg);
	return Data?&Data->diA[ItemNumber]:nullptr;
}

FarDialogItem* CurrentDialogItem(HANDLE hDlg,int ItemNumber)
{
	PDialogData Data=FindCurrentDialogData(hDlg);
	return Data?&Data->di[ItemNumber]:nullptr;
}

FarList* CurrentList(HANDLE hDlg,int ItemNumber)
{
	PDialogData Data=FindCurrentDialogData(hDlg);
	return Data?&Data->l[ItemNumber]:nullptr;
}

TStack<FarDialogEvent>OriginalEvents;

class StackHandler
{
public:
	StackHandler(FarDialogEvent& e){OriginalEvents.Push(e);}
	~StackHandler(){FarDialogEvent e; OriginalEvents.Pop(e);}
};

intptr_t WINAPI FarDefDlgProcA(HANDLE hDlg, int Msg, int Param1, void* Param2)
{
	FarDialogEvent* TopEvent = OriginalEvents.Peek();
	intptr_t Result = NativeInfo.DefDlgProc(TopEvent->hDlg, TopEvent->Msg, TopEvent->Param1, TopEvent->Param2);
	switch(Msg)
	{
	case DN_CTLCOLORDIALOG:
	case DN_CTLCOLORDLGITEM:
		Result = reinterpret_cast<intptr_t>(Param2);
		break;
	}
	return Result;
}

intptr_t WINAPI CurrentDlgProc(HANDLE hDlg, int Msg, int Param1, void* Param2)
{
	PDialogData Data=FindCurrentDialogData(hDlg);
	return (Data && Data->DlgProc)? Data->DlgProc(Data->hDlg,Msg,Param1,Param2) : FarDefDlgProcA(hDlg, Msg, Param1, Param2);
}

void UnicodeListItemToAnsi(FarListItem* li, oldfar::FarListItem* liA)
{
	UnicodeToOEM(li->Text, liA->Text, sizeof(liA->Text)-1);
	liA->Flags=0;

	if (li->Flags&LIF_SELECTED)       liA->Flags|=oldfar::LIF_SELECTED;

	if (li->Flags&LIF_CHECKED)        liA->Flags|=oldfar::LIF_CHECKED;

	if (li->Flags&LIF_SEPARATOR)      liA->Flags|=oldfar::LIF_SEPARATOR;

	if (li->Flags&LIF_DISABLE)        liA->Flags|=oldfar::LIF_DISABLE;

	if (li->Flags&LIF_GRAYED)         liA->Flags|=oldfar::LIF_GRAYED;

	if (li->Flags&LIF_HIDDEN)         liA->Flags|=oldfar::LIF_HIDDEN;

	if (li->Flags&LIF_DELETEUSERDATA) liA->Flags|=oldfar::LIF_DELETEUSERDATA;
}

size_t GetAnsiVBufSize(oldfar::FarDialogItem &diA)
{
	return (diA.X2-diA.X1+1)*(diA.Y2-diA.Y1+1);
}

PCHAR_INFO GetAnsiVBufPtr(FAR_CHAR_INFO* VBuf, size_t Size)
{
	PCHAR_INFO VBufA=nullptr;
	if (VBuf)
	{
		VBufA=*reinterpret_cast<PCHAR_INFO*>(&VBuf[Size]);
	}
	return VBufA;
}

void SetAnsiVBufPtr(FAR_CHAR_INFO* VBuf, PCHAR_INFO VBufA, size_t Size)
{
	if (VBuf)
	{
		*reinterpret_cast<PCHAR_INFO*>(&VBuf[Size])=VBufA;
	}
}

void AnsiVBufToUnicode(PCHAR_INFO VBufA, FAR_CHAR_INFO* VBuf, size_t Size,bool NoCvt)
{
	if (VBuf && VBufA)
	{
		for (size_t i=0; i<Size; i++)
		{
			if (NoCvt)
			{
				VBuf[i].Char=VBufA[i].Char.UnicodeChar;
			}
			else
			{
				AnsiToUnicodeBin(&VBufA[i].Char.AsciiChar,&VBuf[i].Char,1);
			}
			Colors::ConsoleColorToFarColor(VBufA[i].Attributes, VBuf[i].Attributes);
		}
	}
}

FAR_CHAR_INFO* AnsiVBufToUnicode(oldfar::FarDialogItem &diA)
{
	FAR_CHAR_INFO* VBuf = nullptr;

	if (diA.VBuf)
	{
		size_t Size = GetAnsiVBufSize(diA);
		// +sizeof(PCHAR_INFO) потому что там храним поинтер на анси vbuf.
		VBuf = static_cast<FAR_CHAR_INFO*>(xf_malloc(Size*sizeof(FAR_CHAR_INFO)+sizeof(PCHAR_INFO)));

		if (VBuf)
		{
			AnsiVBufToUnicode(diA.VBuf, VBuf, Size,(diA.Flags&oldfar::DIF_NOTCVTUSERCONTROL)==oldfar::DIF_NOTCVTUSERCONTROL);
			SetAnsiVBufPtr(VBuf, diA.VBuf, Size);
		}
	}

	return VBuf;
}

void AnsiListItemToUnicode(oldfar::FarListItem* liA, FarListItem* li)
{
	wchar_t* ListItemText=(wchar_t*)xf_malloc(ARRAYSIZE(liA->Text)*sizeof(wchar_t));
	OEMToUnicode(liA->Text, ListItemText, sizeof(liA->Text)-1);
	li->Text=ListItemText;
	li->Flags=0;

	if (liA->Flags&oldfar::LIF_SELECTED)       li->Flags|=LIF_SELECTED;

	if (liA->Flags&oldfar::LIF_CHECKED)        li->Flags|=LIF_CHECKED;

	if (liA->Flags&oldfar::LIF_SEPARATOR)      li->Flags|=LIF_SEPARATOR;

	if (liA->Flags&oldfar::LIF_DISABLE)        li->Flags|=LIF_DISABLE;

	if (liA->Flags&oldfar::LIF_GRAYED)         li->Flags|=LIF_GRAYED;

	if (liA->Flags&oldfar::LIF_HIDDEN)         li->Flags|=LIF_HIDDEN;

	if (liA->Flags&oldfar::LIF_DELETEUSERDATA) li->Flags|=LIF_DELETEUSERDATA;
}

void AnsiDialogItemToUnicodeSafe(oldfar::FarDialogItem &diA, FarDialogItem &di)
{
	switch (diA.Type)
	{
		case oldfar::DI_TEXT:
			di.Type=DI_TEXT;
			break;
		case oldfar::DI_VTEXT:
			di.Type=DI_VTEXT;
			break;
		case oldfar::DI_SINGLEBOX:
			di.Type=DI_SINGLEBOX;
			break;
		case oldfar::DI_DOUBLEBOX:
			di.Type=DI_DOUBLEBOX;
			break;
		case oldfar::DI_EDIT:
			di.Type=DI_EDIT;
			break;
		case oldfar::DI_PSWEDIT:
			di.Type=DI_PSWEDIT;
			break;
		case oldfar::DI_FIXEDIT:
			di.Type=DI_FIXEDIT;
			break;
		case oldfar::DI_BUTTON:
			di.Type=DI_BUTTON;
			di.Selected=diA.Selected;
			break;
		case oldfar::DI_CHECKBOX:
			di.Type=DI_CHECKBOX;
			di.Selected=diA.Selected;
			break;
		case oldfar::DI_RADIOBUTTON:
			di.Type=DI_RADIOBUTTON;
			di.Selected=diA.Selected;
			break;
		case oldfar::DI_COMBOBOX:
			di.Type=DI_COMBOBOX;
			break;
		case oldfar::DI_LISTBOX:
			di.Type=DI_LISTBOX;
			break;
		case oldfar::DI_MEMOEDIT:
			di.Type=DI_MEMOEDIT;
			break;
		case oldfar::DI_USERCONTROL:
			di.Type=DI_USERCONTROL;
			break;
	}

	di.X1=diA.X1;
	di.Y1=diA.Y1;
	di.X2=diA.X2;
	di.Y2=diA.Y2;
	di.Flags=0;

	if (diA.Focus)
	{
		di.Flags|=DIF_FOCUS;
	}

	// emulate old listbox behaviour
	if (diA.Type == oldfar::DI_LISTBOX)
	{
		di.Flags|=DIF_LISTTRACKMOUSE;
	}

	if (diA.Flags)
	{
		if (diA.Flags&oldfar::DIF_BOXCOLOR)
			di.Flags|=DIF_BOXCOLOR;

		if (diA.Flags&oldfar::DIF_GROUP)
			di.Flags|=DIF_GROUP;

		if (diA.Flags&oldfar::DIF_LEFTTEXT)
			di.Flags|=DIF_LEFTTEXT;

		if (diA.Flags&oldfar::DIF_MOVESELECT)
			di.Flags|=DIF_MOVESELECT;

		if (diA.Flags&oldfar::DIF_SHOWAMPERSAND)
			di.Flags|=DIF_SHOWAMPERSAND;

		if (diA.Flags&oldfar::DIF_CENTERGROUP)
			di.Flags|=DIF_CENTERGROUP;

		if (diA.Flags&oldfar::DIF_NOBRACKETS)
			di.Flags|=DIF_NOBRACKETS;

		if (diA.Flags&oldfar::DIF_MANUALADDHISTORY)
			di.Flags|=DIF_MANUALADDHISTORY;

		if (diA.Flags&oldfar::DIF_SEPARATOR)
			di.Flags|=DIF_SEPARATOR;

		if (diA.Flags&oldfar::DIF_SEPARATOR2)
			di.Flags|=DIF_SEPARATOR2;

		if (diA.Flags&oldfar::DIF_EDITOR)
			di.Flags|=DIF_EDITOR;

		if (diA.Flags&oldfar::DIF_LISTNOAMPERSAND)
			di.Flags|=DIF_LISTNOAMPERSAND;

		if (diA.Flags&oldfar::DIF_LISTNOBOX)
			di.Flags|=DIF_LISTNOBOX;

		if (diA.Flags&oldfar::DIF_HISTORY)
			di.Flags|=DIF_HISTORY;

		if (diA.Flags&oldfar::DIF_BTNNOCLOSE)
			di.Flags|=DIF_BTNNOCLOSE;

		if (diA.Flags&oldfar::DIF_CENTERTEXT)
			di.Flags|=DIF_CENTERTEXT;

		if (diA.Flags&oldfar::DIF_SEPARATORUSER)
			di.Flags|=DIF_SEPARATORUSER;

		if (diA.Flags&oldfar::DIF_EDITEXPAND)
			di.Flags|=DIF_EDITEXPAND;

		if (diA.Flags&oldfar::DIF_DROPDOWNLIST)
			di.Flags|=DIF_DROPDOWNLIST;

		if (diA.Flags&oldfar::DIF_USELASTHISTORY)
			di.Flags|=DIF_USELASTHISTORY;

		if (diA.Flags&oldfar::DIF_MASKEDIT)
			di.Flags|=DIF_MASKEDIT;

		if (diA.Flags&oldfar::DIF_SELECTONENTRY)
			di.Flags|=DIF_SELECTONENTRY;

		if (diA.Flags&oldfar::DIF_3STATE)
			di.Flags|=DIF_3STATE;

		if (diA.Flags&oldfar::DIF_EDITPATH)
			di.Flags|=DIF_EDITPATH;

		if (diA.Flags&oldfar::DIF_LISTWRAPMODE)
			di.Flags|=DIF_LISTWRAPMODE;

		if (diA.Flags&oldfar::DIF_LISTAUTOHIGHLIGHT)
			di.Flags|=DIF_LISTAUTOHIGHLIGHT;

		if (diA.Flags&oldfar::DIF_AUTOMATION)
			di.Flags|=DIF_AUTOMATION;

		if (diA.Flags&oldfar::DIF_HIDDEN)
			di.Flags|=DIF_HIDDEN;

		if (diA.Flags&oldfar::DIF_READONLY)
			di.Flags|=DIF_READONLY;

		if (diA.Flags&oldfar::DIF_NOFOCUS)
			di.Flags|=DIF_NOFOCUS;

		if (diA.Flags&oldfar::DIF_DISABLE)
			di.Flags|=DIF_DISABLE;
	}

	if (diA.DefaultButton) di.Flags|=DIF_DEFAULTBUTTON;
}

void AnsiDialogItemToUnicode(oldfar::FarDialogItem &diA, FarDialogItem &di,FarList &l)
{
	ClearStruct(di);
	AnsiDialogItemToUnicodeSafe(diA,di);

	switch (di.Type)
	{
		case DI_LISTBOX:
		case DI_COMBOBOX:
		{
			if (diA.ListItems && IsPtr(diA.ListItems))
			{
				l.Items = (FarListItem *)xf_malloc(diA.ListItems->ItemsNumber*sizeof(FarListItem));
				l.ItemsNumber = diA.ListItems->ItemsNumber;
				di.ListItems=&l;

				for (size_t j=0; j<di.ListItems->ItemsNumber; j++)
					AnsiListItemToUnicode(&diA.ListItems->Items[j],&l.Items[j]);
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
				di.History=AnsiToUnicode(diA.History);
			else if (diA.Flags&oldfar::DIF_MASKEDIT && diA.Mask)
				di.Mask=AnsiToUnicode(diA.Mask);

			break;
		}
		default:
			break;
	}

	if (diA.Type==oldfar::DI_USERCONTROL)
	{
		di.Data = (wchar_t*)xf_malloc(sizeof(diA.Data));

		if (di.Data) memcpy((char*)di.Data,diA.Data,sizeof(diA.Data));

		di.MaxLength = 0;
	}
	else if ((diA.Type==oldfar::DI_EDIT || diA.Type==oldfar::DI_COMBOBOX) && diA.Flags&oldfar::DIF_VAREDIT)
		di.Data = AnsiToUnicode(diA.Ptr.PtrData);
	else
		di.Data = AnsiToUnicode(diA.Data);

	//BUGBUG тут надо придумать как сделать лучше: maxlen=513 например и также подумать что делать для DIF_VAREDIT
	//di->MaxLen = 0;
}

void FreeUnicodeDialogItem(FarDialogItem &di)
{
	switch (di.Type)
	{
		case DI_EDIT:
		case DI_FIXEDIT:

			if ((di.Flags&DIF_HISTORY) && di.History)
				xf_free((void *)di.History);
			else if ((di.Flags&DIF_MASKEDIT) && di.Mask)
				xf_free((void *)di.Mask);

			break;
		case DI_LISTBOX:
		case DI_COMBOBOX:

			if (di.ListItems)
			{
				if (di.ListItems->Items)
				{
					for (size_t i=0; i<di.ListItems->ItemsNumber; i++)
					{
						if (di.ListItems->Items[i].Text)
							xf_free((void *)di.ListItems->Items[i].Text);
					}

					xf_free(di.ListItems->Items);
					di.ListItems->Items=nullptr;
				}
			}

			break;
		case DI_USERCONTROL:

			if (di.VBuf)
				xf_free(di.VBuf);

			break;
		default:
			break;
	}

	if (di.Data)
		xf_free((void *)di.Data);
}

void FreeAnsiDialogItem(oldfar::FarDialogItem &diA)
{
	if ((diA.Type==oldfar::DI_EDIT || diA.Type==oldfar::DI_FIXEDIT) && (diA.Flags&oldfar::DIF_HISTORY ||diA.Flags&oldfar::DIF_MASKEDIT) && diA.History)
	{
		xf_free((void*)diA.History);
		diA.History = nullptr;
	}

	if ((diA.Type==oldfar::DI_EDIT || diA.Type==oldfar::DI_COMBOBOX) && diA.Flags&oldfar::DIF_VAREDIT && diA.Ptr.PtrData)
	{
		xf_free(diA.Ptr.PtrData);
		diA.Ptr.PtrData = nullptr;
	}
}

void UnicodeDialogItemToAnsiSafe(FarDialogItem &di,oldfar::FarDialogItem &diA)
{
	switch (di.Type)
	{
		case DI_TEXT:
			diA.Type=oldfar::DI_TEXT;
			break;
		case DI_VTEXT:
			diA.Type=oldfar::DI_VTEXT;
			break;
		case DI_SINGLEBOX:
			diA.Type=oldfar::DI_SINGLEBOX;
			break;
		case DI_DOUBLEBOX:
			diA.Type=oldfar::DI_DOUBLEBOX;
			break;
		case DI_EDIT:
			diA.Type=oldfar::DI_EDIT;
			break;
		case DI_PSWEDIT:
			diA.Type=oldfar::DI_PSWEDIT;
			break;
		case DI_FIXEDIT:
			diA.Type=oldfar::DI_FIXEDIT;
			break;
		case DI_BUTTON:
			diA.Type=oldfar::DI_BUTTON;
			diA.Selected=di.Selected;
			break;
		case DI_CHECKBOX:
			diA.Type=oldfar::DI_CHECKBOX;
			diA.Selected=di.Selected;
			break;
		case DI_RADIOBUTTON:
			diA.Type=oldfar::DI_RADIOBUTTON;
			diA.Selected=di.Selected;
			break;
		case DI_COMBOBOX:
			diA.Type=oldfar::DI_COMBOBOX;
			break;
		case DI_LISTBOX:
			diA.Type=oldfar::DI_LISTBOX;
			break;
		case DI_MEMOEDIT:
			diA.Type=oldfar::DI_MEMOEDIT;
			break;
		case DI_USERCONTROL:
			diA.Type=oldfar::DI_USERCONTROL;
			break;
	}

	diA.X1=di.X1;
	diA.Y1=di.Y1;
	diA.X2=di.X2;
	diA.Y2=di.Y2;
	diA.Focus=(di.Flags&DIF_FOCUS)?true:false;

	if (diA.Flags&oldfar::DIF_SETCOLOR)
	{
		diA.Flags=oldfar::DIF_SETCOLOR|(diA.Flags&oldfar::DIF_COLORMASK);
	}
	else
	{
		diA.Flags=0;
	}

	if (di.Flags)
	{
		if (di.Flags&DIF_BOXCOLOR)
			diA.Flags|=oldfar::DIF_BOXCOLOR;

		if (di.Flags&DIF_GROUP)
			diA.Flags|=oldfar::DIF_GROUP;

		if (di.Flags&DIF_LEFTTEXT)
			diA.Flags|=oldfar::DIF_LEFTTEXT;

		if (di.Flags&DIF_MOVESELECT)
			diA.Flags|=oldfar::DIF_MOVESELECT;

		if (di.Flags&DIF_SHOWAMPERSAND)
			diA.Flags|=oldfar::DIF_SHOWAMPERSAND;

		if (di.Flags&DIF_CENTERGROUP)
			diA.Flags|=oldfar::DIF_CENTERGROUP;

		if (di.Flags&DIF_NOBRACKETS)
			diA.Flags|=oldfar::DIF_NOBRACKETS;

		if (di.Flags&DIF_MANUALADDHISTORY)
			diA.Flags|=oldfar::DIF_MANUALADDHISTORY;

		if (di.Flags&DIF_SEPARATOR)
			diA.Flags|=oldfar::DIF_SEPARATOR;

		if (di.Flags&DIF_SEPARATOR2)
			diA.Flags|=oldfar::DIF_SEPARATOR2;

		if (di.Flags&DIF_EDITOR)
			diA.Flags|=oldfar::DIF_EDITOR;

		if (di.Flags&DIF_LISTNOAMPERSAND)
			diA.Flags|=oldfar::DIF_LISTNOAMPERSAND;

		if (di.Flags&DIF_LISTNOBOX)
			diA.Flags|=oldfar::DIF_LISTNOBOX;

		if (di.Flags&DIF_HISTORY)
			diA.Flags|=oldfar::DIF_HISTORY;

		if (di.Flags&DIF_BTNNOCLOSE)
			diA.Flags|=oldfar::DIF_BTNNOCLOSE;

		if (di.Flags&DIF_CENTERTEXT)
			diA.Flags|=oldfar::DIF_CENTERTEXT;

		if (di.Flags&DIF_SEPARATORUSER)
			diA.Flags|=oldfar::DIF_SEPARATORUSER;

		if (di.Flags&DIF_EDITEXPAND)
			diA.Flags|=oldfar::DIF_EDITEXPAND;

		if (di.Flags&DIF_DROPDOWNLIST)
			diA.Flags|=oldfar::DIF_DROPDOWNLIST;

		if (di.Flags&DIF_USELASTHISTORY)
			diA.Flags|=oldfar::DIF_USELASTHISTORY;

		if (di.Flags&DIF_MASKEDIT)
			diA.Flags|=oldfar::DIF_MASKEDIT;

		if (di.Flags&DIF_SELECTONENTRY)
			diA.Flags|=oldfar::DIF_SELECTONENTRY;

		if (di.Flags&DIF_3STATE)
			diA.Flags|=oldfar::DIF_3STATE;

		if (di.Flags&DIF_EDITPATH)
			diA.Flags|=oldfar::DIF_EDITPATH;

		if (di.Flags&DIF_LISTWRAPMODE)
			diA.Flags|=oldfar::DIF_LISTWRAPMODE;

		if (di.Flags&DIF_LISTAUTOHIGHLIGHT)
			diA.Flags|=oldfar::DIF_LISTAUTOHIGHLIGHT;

		if (di.Flags&DIF_AUTOMATION)
			diA.Flags|=oldfar::DIF_AUTOMATION;

		if (di.Flags&DIF_HIDDEN)
			diA.Flags|=oldfar::DIF_HIDDEN;

		if (di.Flags&DIF_READONLY)
			diA.Flags|=oldfar::DIF_READONLY;

		if (di.Flags&DIF_NOFOCUS)
			diA.Flags|=oldfar::DIF_NOFOCUS;

		if (di.Flags&DIF_DISABLE)
			diA.Flags|=oldfar::DIF_DISABLE;
	}

	diA.DefaultButton=(di.Flags&DIF_DEFAULTBUTTON)?true:false;
}

oldfar::FarDialogItem* UnicodeDialogItemToAnsi(FarDialogItem &di,HANDLE hDlg,int ItemNumber)
{
	oldfar::FarDialogItem *diA=CurrentDialogItemA(hDlg,ItemNumber);

	if (!diA)
	{
		if (OneDialogItem)
			xf_free(OneDialogItem);

		OneDialogItem=(oldfar::FarDialogItem*)xf_malloc(sizeof(oldfar::FarDialogItem));
		ClearStruct(*OneDialogItem);
		diA=OneDialogItem;
	}

	FreeAnsiDialogItem(*diA);
	UnicodeDialogItemToAnsiSafe(di,*diA);

	switch (diA->Type)
	{
		case oldfar::DI_USERCONTROL:
			diA->VBuf=GetAnsiVBufPtr(di.VBuf, GetAnsiVBufSize(*diA));
			break;
		case oldfar::DI_EDIT:
		case oldfar::DI_FIXEDIT:
		{
			if (di.Flags&DIF_HISTORY)
				diA->History=UnicodeToAnsi(di.History);
			else if (di.Flags&DIF_MASKEDIT)
				diA->Mask=UnicodeToAnsi(di.Mask);
		}
		break;
		case oldfar::DI_COMBOBOX:
		case oldfar::DI_LISTBOX:
			diA->ListPos=static_cast<int>(NativeInfo.SendDlgMessage(hDlg,DM_LISTGETCURPOS,ItemNumber,0));
			break;
	}

	if (diA->Type==oldfar::DI_USERCONTROL)
	{
		if (di.Data) memcpy(diA->Data,(char*)di.Data,sizeof(diA->Data));
	}
	else if ((diA->Type==oldfar::DI_EDIT || diA->Type==oldfar::DI_COMBOBOX) && diA->Flags&oldfar::DIF_VAREDIT)
	{
		diA->Ptr.PtrLength=StrLength(di.Data);
		diA->Ptr.PtrData=(char*)xf_malloc(diA->Ptr.PtrLength+1);
		UnicodeToOEM(di.Data,diA->Ptr.PtrData,diA->Ptr.PtrLength+1);
	}
	else
		UnicodeToOEM(di.Data,diA->Data,sizeof(diA->Data));

	return diA;
}

intptr_t WINAPI DlgProcA(HANDLE hDlg, int NewMsg, int Param1, void* Param2)
{
	FarDialogEvent e = {hDlg, NewMsg, Param1, Param2};
	StackHandler sh(e);

	static wchar_t* HelpTopic = nullptr;
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
				FarColor* Color = static_cast<FarColor*>(Param2);
				Colors::ConsoleColorToFarColor(static_cast<int>(CurrentDlgProc(hDlg, oldfar::DN_CTLCOLORDIALOG, Param1, ToPtr(Colors::FarColorToConsoleColor(*Color)))),*Color);
			}
			break;

		case DN_DRAWDIALOG:
			Msg=oldfar::DN_DRAWDIALOG;
			break;

		case DN_CTLCOLORDLGITEM:
			{
				FarDialogItemColors* lc = reinterpret_cast<FarDialogItemColors*>(Param2);

				oldfar::FarDialogItem* diA = CurrentDialogItemA(hDlg, Param1);

				// first, emulate DIF_SETCOLOR
				if(diA->Flags&oldfar::DIF_SETCOLOR)
				{
					BYTE Colors = diA->Flags&oldfar::DIF_COLORMASK;
					Colors::ConsoleColorToFarColor(Colors, lc->Colors[0]);
				}

				DWORD Result = static_cast<DWORD>(CurrentDlgProc(hDlg, oldfar::DN_CTLCOLORDLGITEM, Param1, ToPtr(MAKELONG(
					MAKEWORD(Colors::FarColorToConsoleColor(lc->Colors[0]), Colors::FarColorToConsoleColor(lc->Colors[1])),
					MAKEWORD(Colors::FarColorToConsoleColor(lc->Colors[2]), Colors::FarColorToConsoleColor(lc->Colors[3]))))));
				if(lc->ColorsCount > 0)
					Colors::ConsoleColorToFarColor(LOBYTE(LOWORD(Result)),lc->Colors[0]);
				if(lc->ColorsCount > 1)
					Colors::ConsoleColorToFarColor(HIBYTE(LOWORD(Result)),lc->Colors[1]);
				if(lc->ColorsCount > 2)
					Colors::ConsoleColorToFarColor(LOBYTE(HIWORD(Result)),lc->Colors[2]);
				if(lc->ColorsCount > 3)
					Colors::ConsoleColorToFarColor(HIBYTE(HIWORD(Result)),lc->Colors[3]);
			}
			break;

		case DN_CTLCOLORDLGLIST:
			{
				FarDialogItemColors* lc = reinterpret_cast<FarDialogItemColors*>(Param2);
				oldfar::FarListColors lcA={};
				lcA.ColorCount = static_cast<int>(lc->ColorsCount);
				LPBYTE Colors = new BYTE[lcA.ColorCount];
				lcA.Colors = Colors;
				for(size_t i = 0; i < lc->ColorsCount; ++i)
				{
					lcA.Colors[i] = static_cast<BYTE>(Colors::FarColorToConsoleColor(lc->Colors[i]));
				}
				intptr_t Result = CurrentDlgProc(hDlg, oldfar::DN_CTLCOLORDLGLIST, Param1, &lcA);
				if(Result)
				{
					lc->ColorsCount = lcA.ColorCount;
					for(size_t i = 0; i < lc->ColorsCount; ++i)
					{
						Colors::ConsoleColorToFarColor(lcA.Colors[i], lc->Colors[i]);
					}
				}
				delete[] Colors;
				return Result != 0;
			}
			break;

		case DN_DRAWDLGITEM:
		{
			Msg=oldfar::DN_DRAWDLGITEM;
			FarDialogItem *di = (FarDialogItem *)Param2;
			oldfar::FarDialogItem *FarDiA=UnicodeDialogItemToAnsi(*di,hDlg,Param1);
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
				if (HelpTopic) xf_free(HelpTopic);

				HelpTopic = AnsiToUnicode((const char *)ret);
				ret = (intptr_t)HelpTopic;
			}

			xf_free(HelpTopicA);
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
		case DM_KILLSAVESCREEN: Msg=oldfar::DM_KILLSAVESCREEN; break;
		case DM_ALLKEYMODE:     Msg=oldfar::DM_ALLKEYMODE; break;
		case DN_ACTIVATEAPP:    Msg=oldfar::DN_ACTIVATEAPP; break;
		case DN_INPUT:
			{
				INPUT_RECORD* record=(INPUT_RECORD *)Param2;
				if (record->EventType==MOUSE_EVENT)
				{
					Msg=oldfar::DN_MOUSEEVENT;
					Param1=0;
					Param2=&record->Event.MouseEvent;
					break;
				}
			}
			return NativeInfo.DefDlgProc(hDlg, NewMsg, Param1, Param2);
		case DN_CONTROLINPUT:
			{
				INPUT_RECORD* record=(INPUT_RECORD *)Param2;
				if (record->EventType==MOUSE_EVENT)
				{
					Msg=oldfar::DN_MOUSECLICK;
					Param2=&record->Event.MouseEvent;
					break;
				}
				else if (record->EventType==KEY_EVENT)
				{
					Msg=oldfar::DN_KEY;
					Param2=ToPtr(KeyToOldKey((DWORD)InputRecordToKey((const INPUT_RECORD *)Param2)));
					break;
				}
			}
			return NativeInfo.DefDlgProc(hDlg, NewMsg, Param1, Param2);
		default:
			break;
	}

	return CurrentDlgProc(hDlg, Msg, Param1, Param2);
}

intptr_t WINAPI FarSendDlgMessageA(HANDLE hDlg, int OldMsg, int Param1, void* Param2)
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
			size_t item_size=NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, Param1, 0);

			if (item_size)
			{
				FarGetDialogItem gdi = {sizeof(FarGetDialogItem), item_size, (FarDialogItem *)xf_malloc(item_size)};

				if (gdi.Item)
				{
					NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, Param1, &gdi);
					oldfar::FarDialogItem *FarDiA=UnicodeDialogItemToAnsi(*gdi.Item,hDlg,Param1);
					xf_free(gdi.Item);
					*reinterpret_cast<oldfar::FarDialogItem*>(Param2)=*FarDiA;
					return TRUE;
				}
			}

			return FALSE;
		}
		case oldfar::DM_GETDLGRECT: Msg = DM_GETDLGRECT; break;
		case oldfar::DM_GETTEXT:
		{
			if (!Param2)
				return NativeInfo.SendDlgMessage(hDlg, DM_GETTEXT, Param1, 0);

			oldfar::FarDialogItemData* didA = (oldfar::FarDialogItemData*)Param2;
			if (!didA->PtrLength) //вот такой хреновый API!!!
				didA->PtrLength = static_cast<int>(NativeInfo.SendDlgMessage(hDlg, DM_GETTEXT, Param1, 0));
			wchar_t* text = (wchar_t*) xf_malloc((didA->PtrLength+1)*sizeof(wchar_t));
			//BUGBUG: если didA->PtrLength=0, то вернётся с учётом '\0', в Энц написано, что без, хз как правильно.
			FarDialogItemData did = {sizeof(FarDialogItemData), (size_t)didA->PtrLength, text};
			intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_GETTEXT, Param1, &did);
			didA->PtrLength = (unsigned)did.PtrLength;
			UnicodeToOEM(text,didA->PtrData,didA->PtrLength+1);
			xf_free(text);
			return ret;
		}
		case oldfar::DM_GETTEXTLENGTH: Msg = DM_GETTEXT; break;

		case oldfar::DM_KEY:
		{
			if (!Param1 || !Param2) return FALSE;

			int Count = (int)Param1;
			DWORD* KeysA = (DWORD*)Param2;
			INPUT_RECORD* KeysW = (INPUT_RECORD*)xf_malloc(Count*sizeof(INPUT_RECORD));

			for (int i=0; i<Count; i++)
			{
				KeyToInputRecord(OldKeyToKey(KeysA[i]),KeysW+i);
			}
			intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_KEY, Param1, KeysW);
			xf_free(KeysW);
			return ret;
		}
		case oldfar::DM_MOVEDIALOG: Msg = DM_MOVEDIALOG; break;
		case oldfar::DM_SETDLGDATA: Msg = DM_SETDLGDATA; break;
		case oldfar::DM_SETDLGITEM:
		{
			if (!Param2)
				return FALSE;

			FarDialogItem *di=CurrentDialogItem(hDlg,Param1);

			if (di->Type==DI_LISTBOX || di->Type==DI_COMBOBOX)
				di->ListItems=CurrentList(hDlg,Param1);

			FreeUnicodeDialogItem(*di);
			oldfar::FarDialogItem *diA = (oldfar::FarDialogItem *)Param2;
			AnsiDialogItemToUnicode(*diA,*di,*di->ListItems);

			// save color info
			if(diA->Flags&oldfar::DIF_SETCOLOR)
			{
				oldfar::FarDialogItem *diA_Copy=CurrentDialogItemA(hDlg,Param1);
				diA_Copy->Flags = diA->Flags;
			}

			return NativeInfo.SendDlgMessage(hDlg, DM_SETDLGITEM, Param1, di);
		}
		case oldfar::DM_SETFOCUS: Msg = DM_SETFOCUS; break;
		case oldfar::DM_REDRAW:   Msg = DM_REDRAW; break;
		case oldfar::DM_SETTEXT:
		{
			if (!Param2)return 0;

			oldfar::FarDialogItemData* didA = (oldfar::FarDialogItemData*)Param2;

			if (!didA->PtrData) return 0;

			wchar_t* text = AnsiToUnicode(didA->PtrData);
			//BUGBUG - PtrLength ни на что не влияет.
			FarDialogItemData di = {sizeof(FarDialogItemData),(size_t)didA->PtrLength,text};
			intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_SETTEXT, Param1, &di);
			xf_free(text);
			return ret;
		}
		case oldfar::DM_SETMAXTEXTLENGTH: Msg = DM_SETMAXTEXTLENGTH; break;
		case oldfar::DM_SHOWDIALOG:       Msg = DM_SHOWDIALOG; break;
		case oldfar::DM_GETFOCUS:         Msg = DM_GETFOCUS; break;
		case oldfar::DM_GETCURSORPOS:     Msg = DM_GETCURSORPOS; break;
		case oldfar::DM_SETCURSORPOS:     Msg = DM_SETCURSORPOS; break;
		case oldfar::DM_GETTEXTPTR:
		{
			intptr_t length = NativeInfo.SendDlgMessage(hDlg, DM_GETTEXT, Param1, 0);

			if (!Param2) return length;

			wchar_t* text = (wchar_t *) xf_malloc((length +1)* sizeof(wchar_t));
			FarDialogItemData item = {sizeof(FarDialogItemData), length, text};
			length = NativeInfo.SendDlgMessage(hDlg, DM_GETTEXT, Param1, &item);
			UnicodeToOEM(text, (char *)Param2, length+1);
			xf_free(text);
			return length;
		}
		case oldfar::DM_SETTEXTPTR:
		{
			if (!Param2) return FALSE;

			wchar_t* text = AnsiToUnicode((char*)Param2);
			intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, Param1, text);
			xf_free(text);
			return ret;
		}
		case oldfar::DM_SHOWITEM: Msg = DM_SHOWITEM; break;
		case oldfar::DM_ADDHISTORY:
		{
			if (!Param2) return FALSE;

			wchar_t* history = AnsiToUnicode((char*)Param2);
			intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_ADDHISTORY, Param1, history);
			xf_free(history);
			return ret;
		}
		case oldfar::DM_GETCHECK:
		{
			intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_GETCHECK, Param1, 0);
			intptr_t state = 0;

			if (ret == oldfar::BSTATE_UNCHECKED) state=BSTATE_UNCHECKED;
			else if (ret == oldfar::BSTATE_CHECKED)   state=BSTATE_CHECKED;
			else if (ret == oldfar::BSTATE_3STATE)    state=BSTATE_3STATE;

			return state;
		}
		case oldfar::DM_SETCHECK:
		{
			FARCHECKEDSTATE State = BSTATE_UNCHECKED;
			switch(static_cast<oldfar::FARCHECKEDSTATE>(reinterpret_cast<intptr_t>(Param2)))
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

			oldfar::FarListGetItem* lgiA = (oldfar::FarListGetItem*)Param2;
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
				oldfar::FarListPos *lpA = (oldfar::FarListPos *)Param2;
				lpA->SelectPos=lp.SelectPos;
				lpA->TopPos=lp.TopPos;
				return ret;
			}
			else return NativeInfo.SendDlgMessage(hDlg, DM_LISTGETCURPOS, Param1, 0);

		case oldfar::DM_LISTSETCURPOS:
		{
			if (!Param2) return FALSE;

			oldfar::FarListPos *lpA = (oldfar::FarListPos *)Param2;
			FarListPos lp = {sizeof(FarListPos),lpA->SelectPos,lpA->TopPos};
			return NativeInfo.SendDlgMessage(hDlg, DM_LISTSETCURPOS, Param1, &lp);
		}
		case oldfar::DM_LISTDELETE:
		{
			oldfar::FarListDelete *ldA = (oldfar::FarListDelete *)Param2;
			FarListDelete ld={sizeof(FarListDelete)};

			if (Param2)
			{
				ld.Count = ldA->Count;
				ld.StartIndex = ldA->StartIndex;
			}

			return NativeInfo.SendDlgMessage(hDlg, DM_LISTDELETE, Param1, Param2?&ld:0);
		}
		case oldfar::DM_LISTADD:
		{
			FarList newlist = {};

			if (Param2)
			{
				oldfar::FarList *oldlist = (oldfar::FarList*) Param2;
				newlist.ItemsNumber = oldlist->ItemsNumber;

				if (newlist.ItemsNumber)
				{
					newlist.Items = (FarListItem*)xf_malloc(newlist.ItemsNumber*sizeof(FarListItem));

					if (newlist.Items)
					{
						for (size_t i=0; i<newlist.ItemsNumber; i++)
							AnsiListItemToUnicode(&oldlist->Items[i], &newlist.Items[i]);
					}
				}
			}

			intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTADD, Param1, Param2?&newlist:0);

			if (newlist.Items)
			{
				for (size_t i=0; i<newlist.ItemsNumber; i++)
					if (newlist.Items[i].Text) xf_free((void*)newlist.Items[i].Text);

				xf_free(newlist.Items);
			}

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

			if (newstr) xf_free(newstr);

			return ret;
		}
		case oldfar::DM_LISTUPDATE:
		{
			FarListUpdate newui = {sizeof(FarListUpdate)};

			if (Param2)
			{
				oldfar::FarListUpdate *oldui = (oldfar::FarListUpdate*) Param2;
				newui.Index=oldui->Index;
				AnsiListItemToUnicode(&oldui->Item, &newui.Item);
			}

			intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTUPDATE, Param1, Param2?&newui:0);

			if (newui.Item.Text) xf_free((void*)newui.Item.Text);

			return ret;
		}
		case oldfar::DM_LISTINSERT:
		{
			FarListInsert newli = {sizeof(FarListInsert)};

			if (Param2)
			{
				oldfar::FarListInsert *oldli = (oldfar::FarListInsert*) Param2;
				newli.Index=oldli->Index;
				AnsiListItemToUnicode(&oldli->Item, &newli.Item);
			}

			intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTINSERT, Param1, Param2?&newli:0);

			if (newli.Item.Text) xf_free((void*)newli.Item.Text);

			return ret;
		}
		case oldfar::DM_LISTFINDSTRING:
		{
			FarListFind newlf = {sizeof(FarListFind)};

			if (Param2)
			{
				oldfar::FarListFind *oldlf = (oldfar::FarListFind*) Param2;
				newlf.StartIndex=oldlf->StartIndex;
				newlf.Pattern = (oldlf->Pattern)?AnsiToUnicode(oldlf->Pattern):nullptr;

				if (oldlf->Flags&oldfar::LIFIND_EXACTMATCH) newlf.Flags|=LIFIND_EXACTMATCH;
			}

			intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTFINDSTRING, Param1, Param2?&newlf:0);

			if (newlf.Pattern) xf_free((void*)newlf.Pattern);

			return ret;
		}
		case oldfar::DM_LISTINFO:
		{
			if (!Param2) return FALSE;

			FarListInfo li={sizeof(FarListInfo)};
			intptr_t Result=NativeInfo.SendDlgMessage(hDlg, DM_LISTINFO, Param1, &li);
			if (Result)
			{
				oldfar::FarListInfo *liA = (oldfar::FarListInfo *)Param2;
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
				oldfar::FarListItemData *oldlid = (oldfar::FarListItemData*) Param2;
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

			intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTSETDATA, Param1, Param2?&newlid:0);
			return ret;
		}
		case oldfar::DM_LISTSETTITLES:
		{
			if (!Param2) return FALSE;

			oldfar::FarListTitles *ltA = (oldfar::FarListTitles *)Param2;
			FarListTitles lt = {sizeof(FarListTitles),0,ltA->Title?AnsiToUnicode(ltA->Title):nullptr,0,ltA->Bottom?AnsiToUnicode(ltA->Bottom):nullptr};
			intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTSETTITLES, Param1, &lt);

			if (lt.Bottom) xf_free((wchar_t *)lt.Bottom);

			if (lt.Title) xf_free((wchar_t *)lt.Title);

			return ret;
		}
		case oldfar::DM_LISTGETTITLES:
		{
			if (Param2)
			{
				oldfar::FarListTitles *OldListTitle=(oldfar::FarListTitles *)Param2;
				FarListTitles ListTitle={sizeof(FarListTitles)};

				if (OldListTitle->Title)
				{
					ListTitle.TitleSize=OldListTitle->TitleLen+1;
					ListTitle.Title=(wchar_t *)xf_malloc(sizeof(wchar_t)*ListTitle.TitleSize);
				}

				if (OldListTitle->Bottom)
				{
					ListTitle.BottomSize=OldListTitle->BottomLen+1;
					ListTitle.Bottom=(wchar_t *)xf_malloc(sizeof(wchar_t)*ListTitle.BottomSize);
				}

				intptr_t Ret=NativeInfo.SendDlgMessage(hDlg,DM_LISTGETTITLES,Param1,&ListTitle);

				if (Ret)
				{
					UnicodeToOEM(ListTitle.Title,OldListTitle->Title,OldListTitle->TitleLen);
					UnicodeToOEM(ListTitle.Bottom,OldListTitle->Bottom,OldListTitle->BottomLen);
				}

				if (ListTitle.Title)
					xf_free((wchar_t *)ListTitle.Title);

				if (ListTitle.Bottom)
					xf_free((wchar_t *)ListTitle.Bottom);

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
				return NativeInfo.SendDlgMessage(hDlg, DM_SETHISTORY, Param1, 0);
			else
			{
				FarDialogItem *di=CurrentDialogItem(hDlg,Param1);
				xf_free((void*)di->History);
				di->History = AnsiToUnicode((const char *)Param2);
				return NativeInfo.SendDlgMessage(hDlg, DM_SETHISTORY, Param1, const_cast<wchar_t*>(di->History));
			}

		case oldfar::DM_GETITEMPOSITION:     Msg = DM_GETITEMPOSITION; break;
		case oldfar::DM_SETMOUSEEVENTNOTIFY: Msg = DM_SETMOUSEEVENTNOTIFY; break;
		case oldfar::DM_EDITUNCHANGEDFLAG:   Msg = DM_EDITUNCHANGEDFLAG; break;
		case oldfar::DM_GETITEMDATA:         Msg = DM_GETITEMDATA; break;
		case oldfar::DM_SETITEMDATA:         Msg = DM_SETITEMDATA; break;
		case oldfar::DM_LISTSET:
		{
			FarList newlist = {};

			if (Param2)
			{
				oldfar::FarList *oldlist = (oldfar::FarList*) Param2;
				newlist.ItemsNumber = oldlist->ItemsNumber;

				if (newlist.ItemsNumber)
				{
					newlist.Items = (FarListItem*)xf_malloc(newlist.ItemsNumber*sizeof(FarListItem));

					if (newlist.Items)
					{
						for (size_t i=0; i<newlist.ItemsNumber; i++)
							AnsiListItemToUnicode(&oldlist->Items[i], &newlist.Items[i]);
					}
				}
			}

			intptr_t ret = NativeInfo.SendDlgMessage(hDlg, DM_LISTSET, Param1, Param2?&newlist:0);

			if (newlist.Items)
			{
				for (size_t i=0; i<newlist.ItemsNumber; i++)
					if (newlist.Items[i].Text) xf_free((void*)newlist.Items[i].Text);

				xf_free(newlist.Items);
			}

			return ret;
		}
		case oldfar::DM_LISTSETMOUSEREACTION:
		{
			oldfar::FARLISTMOUSEREACTIONTYPE OldType = static_cast<oldfar::FARLISTMOUSEREACTIONTYPE>(reinterpret_cast<intptr_t>(Param2));
			FarDialogItem DlgItem = {};
			NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEMSHORT, Param1, &DlgItem);
			FARDIALOGITEMFLAGS OldFlags = DlgItem.Flags;
			DlgItem.Flags&=~(DIF_LISTTRACKMOUSE|DIF_LISTTRACKMOUSEINFOCUS);
			switch (OldType)
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
			oldfar::EditorSelect *esA = (oldfar::EditorSelect *)Param2;
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

			oldfar::EditorSelect *esA = (oldfar::EditorSelect *)Param2;
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

int WINAPI FarDialogExA(intptr_t PluginNumber,int X1,int Y1,int X2,int Y2,const char *HelpTopic,oldfar::FarDialogItem *Item,int ItemsNumber,DWORD Reserved,DWORD Flags,oldfar::FARWINDOWPROC DlgProc,void* Param)
{
	string strHT(HelpTopic);

	if (!Item || !ItemsNumber)
		return -1;

	oldfar::FarDialogItem* diA=new oldfar::FarDialogItem[ItemsNumber]();

	// to save DIF_SETCOLOR state
	for(int i = 0; i < ItemsNumber; ++i)
	{
		diA[i].Flags = Item[i].Flags;
	}

	FarDialogItem* di = new FarDialogItem[ItemsNumber]();
	FarList* l = new FarList[ItemsNumber]();

	for (int i=0; i<ItemsNumber; i++)
	{
		AnsiDialogItemToUnicode(Item[i],di[i],l[i]);
	}

	DWORD DlgFlags = 0;

	if (Flags&oldfar::FDLG_WARNING)      DlgFlags|=FDLG_WARNING;

	if (Flags&oldfar::FDLG_SMALLDIALOG)  DlgFlags|=FDLG_SMALLDIALOG;

	if (Flags&oldfar::FDLG_NODRAWSHADOW) DlgFlags|=FDLG_NODRAWSHADOW;

	if (Flags&oldfar::FDLG_NODRAWPANEL)  DlgFlags|=FDLG_NODRAWPANEL;

	if (Flags&oldfar::FDLG_NONMODAL)     DlgFlags|=FDLG_NONMODAL;

	int ret = -1;
	HANDLE hDlg = NativeInfo.DialogInit(GetPluginGuid(PluginNumber), &FarGuid, X1, Y1, X2, Y2, (HelpTopic?strHT.CPtr():nullptr), (FarDialogItem *)di, ItemsNumber, 0, DlgFlags, DlgProcA, Param);
	PDialogData NewDialogData=new DialogData;
	NewDialogData->DlgProc=DlgProc;
	NewDialogData->hDlg=hDlg;
	NewDialogData->diA=diA;
	NewDialogData->di=di;
	NewDialogData->l=l;
	DialogList.Push(&NewDialogData);

	if (hDlg != INVALID_HANDLE_VALUE)
	{
		ret = NativeInfo.DialogRun(hDlg);

		for (int i=0; i<ItemsNumber; i++)
		{
			size_t Size = NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, i, 0);
			FarGetDialogItem gdi = {sizeof(FarGetDialogItem), Size, static_cast<FarDialogItem*>(xf_malloc(Size))};

			if (gdi.Item)
			{
				NativeInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, i, &gdi);
				UnicodeDialogItemToAnsiSafe(*gdi.Item,Item[i]);
				const wchar_t *res = gdi.Item->Data;

				if (!res) res = L"";

				if ((di[i].Type==DI_EDIT || di[i].Type==DI_COMBOBOX) && Item[i].Flags&oldfar::DIF_VAREDIT)
					UnicodeToOEM(res, Item[i].Ptr.PtrData, Item[i].Ptr.PtrLength+1);
				else
					UnicodeToOEM(res, Item[i].Data, sizeof(Item[i].Data));

				if (gdi.Item->Type==DI_USERCONTROL)
				{
					di[i].VBuf=gdi.Item->VBuf;
					Item[i].VBuf=GetAnsiVBufPtr(gdi.Item->VBuf,GetAnsiVBufSize(Item[i]));
				}

				if (gdi.Item->Type==DI_COMBOBOX || gdi.Item->Type==DI_LISTBOX)
				{
					Item[i].ListPos = static_cast<int>(NativeInfo.SendDlgMessage(hDlg,DM_LISTGETCURPOS,i,0));
				}

				xf_free(gdi.Item);
			}

			FreeAnsiDialogItem(diA[i]);
		}

		NativeInfo.DialogFree(hDlg);

		for (int i=0; i<ItemsNumber; i++)
		{
			if (di[i].Type==DI_LISTBOX || di[i].Type==DI_COMBOBOX)
				di[i].ListItems=CurrentList(hDlg,i);

			FreeUnicodeDialogItem(di[i]);
		}
	}

	delete *DialogList.Last();
	DialogList.Delete(DialogList.Last());

	delete[] diA;
	delete[] di;
	delete[] l;

	return ret;
}

int WINAPI FarDialogFnA(intptr_t PluginNumber,int X1,int Y1,int X2,int Y2,const char *HelpTopic,oldfar::FarDialogItem *Item,int ItemsNumber)
{
	return FarDialogExA(PluginNumber, X1, Y1, X2, Y2, HelpTopic, Item, ItemsNumber, 0, 0, 0, 0);
}

void ConvertUnicodePanelInfoToAnsi(PanelInfo* PIW, oldfar::PanelInfo* PIA)
{
	PIA->PanelType = 0;

	switch (PIW->PanelType)
	{
		case PTYPE_FILEPANEL:  PIA->PanelType = oldfar::PTYPE_FILEPANEL;  break;
		case PTYPE_TREEPANEL:  PIA->PanelType = oldfar::PTYPE_TREEPANEL;  break;
		case PTYPE_QVIEWPANEL: PIA->PanelType = oldfar::PTYPE_QVIEWPANEL; break;
		case PTYPE_INFOPANEL:  PIA->PanelType = oldfar::PTYPE_INFOPANEL;  break;
	}

	PIA->Plugin = (PIW->Flags&PFLAGS_PLUGIN)?1:0;
	PIA->PanelRect.left   = PIW->PanelRect.left;
	PIA->PanelRect.top    = PIW->PanelRect.top;
	PIA->PanelRect.right  = PIW->PanelRect.right;
	PIA->PanelRect.bottom = PIW->PanelRect.bottom;
	PIA->ItemsNumber = static_cast<int>(PIW->ItemsNumber);
	PIA->SelectedItemsNumber = static_cast<int>(PIW->SelectedItemsNumber);
	PIA->PanelItems = nullptr;
	PIA->SelectedItems = nullptr;
	PIA->CurrentItem = static_cast<int>(PIW->CurrentItem);
	PIA->TopPanelItem = static_cast<int>(PIW->TopPanelItem);
	PIA->Visible = (PIW->Flags&PFLAGS_VISIBLE)?1:0;;
	PIA->Focus = (PIW->Flags&PFLAGS_FOCUS)?1:0;;
	PIA->ViewMode = PIW->ViewMode;
	PIA->ShortNames = (PIW->Flags&PFLAGS_ALTERNATIVENAMES)?1:0;;
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

	if (PIW->Flags&PFLAGS_SHOWHIDDEN)       PIA->Flags|=oldfar::PFLAGS_SHOWHIDDEN;

	if (PIW->Flags&PFLAGS_HIGHLIGHT)        PIA->Flags|=oldfar::PFLAGS_HIGHLIGHT;

	if (PIW->Flags&PFLAGS_REVERSESORTORDER) PIA->Flags|=oldfar::PFLAGS_REVERSESORTORDER;

	if (PIW->Flags&PFLAGS_USESORTGROUPS)    PIA->Flags|=oldfar::PFLAGS_USESORTGROUPS;

	if (PIW->Flags&PFLAGS_SELECTEDFIRST)    PIA->Flags|=oldfar::PFLAGS_SELECTEDFIRST;

	if (PIW->Flags&PFLAGS_REALNAMES)        PIA->Flags|=oldfar::PFLAGS_REALNAMES;

	if (PIW->Flags&PFLAGS_NUMERICSORT)      PIA->Flags|=oldfar::PFLAGS_NUMERICSORT;

	if (PIW->Flags&PFLAGS_PANELLEFT)        PIA->Flags|=oldfar::PFLAGS_PANELLEFT;

	PIA->Reserved = PIW->Reserved;
}

void FreeAnsiPanelInfo(oldfar::PanelInfo* PIA)
{
	if (PIA->PanelItems)
		FreePanelItemA(PIA->PanelItems,PIA->ItemsNumber);

	if (PIA->SelectedItems)
		FreePanelItemA(PIA->SelectedItems,PIA->SelectedItemsNumber);

	ClearStruct(*PIA);
}

int WINAPI FarPanelControlA(HANDLE hPlugin,int Command,void *Param)
{
	static oldfar::PanelInfo PanelInfoA={},AnotherPanelInfoA={};
	static int Reenter=0;

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

			if (ParamW) xf_free(ParamW);

			return ret;
		}
		case oldfar::FCTL_GETANOTHERPANELINFO:
		case oldfar::FCTL_GETPANELINFO:
		{
			if (!Param)
				return FALSE;

			bool Passive=Command==oldfar::FCTL_GETANOTHERPANELINFO;

			if (Reenter)
			{
				//Попытка борьбы с рекурсией (вызов GET*PANELINFO из GetOpenPanelInfo).
				//Так как у нас всё статик то должно сработать нормально в 99% случаев
				*(oldfar::PanelInfo*)Param=Passive?AnotherPanelInfoA:PanelInfoA;
				return TRUE;
			}

			Reenter++;

			if (Passive)
				hPlugin=PANEL_PASSIVE;

			oldfar::PanelInfo* OldPI=Passive?&AnotherPanelInfoA:&PanelInfoA;
			PanelInfo PI = {sizeof(PanelInfo)};
			int ret = static_cast<int>(NativeInfo.PanelControl(hPlugin,FCTL_GETPANELINFO,0,&PI));
			FreeAnsiPanelInfo(OldPI);

			if (ret)
			{
				ConvertUnicodePanelInfoToAnsi(&PI,OldPI);

				if (PI.ItemsNumber)
				{
					OldPI->PanelItems = (oldfar::PluginPanelItem *)xf_malloc(PI.ItemsNumber*sizeof(oldfar::PluginPanelItem));

					if (OldPI->PanelItems)
					{
						memset(OldPI->PanelItems,0,PI.ItemsNumber*sizeof(oldfar::PluginPanelItem));
						PluginPanelItem* PPI=nullptr; int PPISize=0;

						for (int i=0; i<static_cast<int>(PI.ItemsNumber); i++)
						{
							int NewPPISize=static_cast<int>(NativeInfo.PanelControl(hPlugin,FCTL_GETPANELITEM,i,0));

							if (NewPPISize>PPISize)
							{
								PluginPanelItem* NewPPI=(PluginPanelItem*)xf_realloc(PPI,NewPPISize);

								if (NewPPI)
								{
									PPI=NewPPI;
									PPISize=NewPPISize;
								}
								else
									break;
							}
							FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), (size_t)PPISize, PPI};
							NativeInfo.PanelControl(hPlugin,FCTL_GETPANELITEM, i, &gpi);
							if(PPI)
							{
								ConvertPanelItemToAnsi(*PPI,OldPI->PanelItems[i]);
							}
						}

						if (PPI)
							xf_free(PPI);
					}
				}

				if (PI.SelectedItemsNumber)
				{
					OldPI->SelectedItems = (oldfar::PluginPanelItem *)xf_malloc(PI.SelectedItemsNumber*sizeof(oldfar::PluginPanelItem));

					if (OldPI->SelectedItems)
					{
						memset(OldPI->SelectedItems,0,PI.SelectedItemsNumber*sizeof(oldfar::PluginPanelItem));
						PluginPanelItem* PPI=nullptr; int PPISize=0;

						for (int i=0; i<static_cast<int>(PI.SelectedItemsNumber); i++)
						{
							int NewPPISize=static_cast<int>(NativeInfo.PanelControl(hPlugin,FCTL_GETSELECTEDPANELITEM,i,0));

							if (NewPPISize>PPISize)
							{
								PluginPanelItem* NewPPI=(PluginPanelItem*)xf_realloc(PPI,NewPPISize);

								if (NewPPI)
								{
									PPI=NewPPI;
									PPISize=NewPPISize;
								}
								else
									break;
							}
							FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), (size_t)PPISize, PPI};
							NativeInfo.PanelControl(hPlugin,FCTL_GETSELECTEDPANELITEM, i, &gpi);
							if(PPI)
							{
								ConvertPanelItemToAnsi(*PPI,OldPI->SelectedItems[i]);
							}
						}

						if (PPI)
							xf_free(PPI);
					}
				}

				size_t dirSize=NativeInfo.PanelControl(hPlugin,FCTL_GETPANELDIRECTORY,0,0);
				if(dirSize)
				{
					FarPanelDirectory* dirInfo=(FarPanelDirectory*)new char[dirSize];
					dirInfo->StructSize=sizeof(FarPanelDirectory);
					NativeInfo.PanelControl(hPlugin,FCTL_GETPANELDIRECTORY,static_cast<int>(dirSize),dirInfo);
					UnicodeToOEM(dirInfo->Name,OldPI->CurDir,sizeof(OldPI->CurDir));
					delete[](char*)dirInfo;
				}
				wchar_t ColumnTypes[sizeof(OldPI->ColumnTypes)];
				NativeInfo.PanelControl(hPlugin,FCTL_GETCOLUMNTYPES,sizeof(OldPI->ColumnTypes),ColumnTypes);
				UnicodeToOEM(ColumnTypes,OldPI->ColumnTypes,sizeof(OldPI->ColumnTypes));
				wchar_t ColumnWidths[sizeof(OldPI->ColumnWidths)];
				NativeInfo.PanelControl(hPlugin,FCTL_GETCOLUMNWIDTHS,sizeof(OldPI->ColumnWidths),ColumnWidths);
				UnicodeToOEM(ColumnWidths,OldPI->ColumnWidths,sizeof(OldPI->ColumnWidths));
				*(oldfar::PanelInfo*)Param=*OldPI;
			}
			else
			{
				ClearStruct(*(oldfar::PanelInfo*)Param);
			}

			Reenter--;
			return ret;
		}
		case oldfar::FCTL_GETANOTHERPANELSHORTINFO:
		case oldfar::FCTL_GETPANELSHORTINFO:
		{
			if (!Param)
				return FALSE;

			oldfar::PanelInfo *OldPI=(oldfar::PanelInfo*)Param;
			ClearStruct(*OldPI);

			if (Command==oldfar::FCTL_GETANOTHERPANELSHORTINFO)
				hPlugin=PANEL_PASSIVE;

			PanelInfo PI = {sizeof(PanelInfo)};
			int ret = static_cast<int>(NativeInfo.PanelControl(hPlugin,FCTL_GETPANELINFO,0,&PI));

			if (ret)
			{
				ConvertUnicodePanelInfoToAnsi(&PI,OldPI);
				size_t dirSize=NativeInfo.PanelControl(hPlugin,FCTL_GETPANELDIRECTORY,0,0);
				if(dirSize)
				{
					FarPanelDirectory* dirInfo=(FarPanelDirectory*)new char[dirSize];
					dirInfo->StructSize=sizeof(FarPanelDirectory);
					NativeInfo.PanelControl(hPlugin,FCTL_GETPANELDIRECTORY,static_cast<int>(dirSize),dirInfo);
					UnicodeToOEM(dirInfo->Name,OldPI->CurDir,sizeof(OldPI->CurDir));
					delete[](char*)dirInfo;
				}
				wchar_t ColumnTypes[sizeof(OldPI->ColumnTypes)];
				NativeInfo.PanelControl(hPlugin,FCTL_GETCOLUMNTYPES,sizeof(OldPI->ColumnTypes),ColumnTypes);
				UnicodeToOEM(ColumnTypes,OldPI->ColumnTypes,sizeof(OldPI->ColumnTypes));
				wchar_t ColumnWidths[sizeof(OldPI->ColumnWidths)];
				NativeInfo.PanelControl(hPlugin,FCTL_GETCOLUMNWIDTHS,sizeof(OldPI->ColumnWidths),ColumnWidths);
				UnicodeToOEM(ColumnWidths,OldPI->ColumnWidths,sizeof(OldPI->ColumnWidths));
			}

			return ret;
		}
		case oldfar::FCTL_SETANOTHERSELECTION:
			hPlugin=PANEL_PASSIVE;
		case oldfar::FCTL_SETSELECTION:
		{
			if (!Param)
				return FALSE;

			oldfar::PanelInfo *OldPI=(oldfar::PanelInfo*)Param;
			NativeInfo.PanelControl(hPlugin,FCTL_BEGINSELECTION,0,0);

			for (int i=0; i<OldPI->ItemsNumber; i++)
			{
				NativeInfo.PanelControl(hPlugin,FCTL_SETSELECTION,i,ToPtr(OldPI->PanelItems[i].Flags & oldfar::PPIF_SELECTED));
			}

			NativeInfo.PanelControl(hPlugin,FCTL_ENDSELECTION,0,0);
			return TRUE;
		}
		case oldfar::FCTL_REDRAWANOTHERPANEL:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_REDRAWPANEL:
		{
			if (!Param)
				return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_REDRAWPANEL,0,0));

			oldfar::PanelRedrawInfo* priA = (oldfar::PanelRedrawInfo*)Param;
			PanelRedrawInfo pri = {sizeof(PanelRedrawInfo), (size_t)priA->CurrentItem,(size_t)priA->TopPanelItem};
			return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_REDRAWPANEL,0,&pri));
		}
		case oldfar::FCTL_SETANOTHERNUMERICSORT:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_SETNUMERICSORT:
			return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETNUMERICSORT,(Param&&(*(int*)Param))?1:0,0));
		case oldfar::FCTL_SETANOTHERPANELDIR:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_SETPANELDIR:
		{
			if (!Param)
				return FALSE;

			wchar_t* Dir = AnsiToUnicode((char*)Param);
			FarPanelDirectory dirInfo={sizeof(FarPanelDirectory),Dir,NULL,FarGuid,NULL};
			int ret = static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETPANELDIRECTORY,0,&dirInfo));
			xf_free(Dir);
			return ret;
		}
		case oldfar::FCTL_SETANOTHERSORTMODE:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_SETSORTMODE:

			if (!Param)
				return FALSE;

			return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETSORTMODE,*(int*)Param,0));
		case oldfar::FCTL_SETANOTHERSORTORDER:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_SETSORTORDER:
			return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETSORTORDER,(Param&&(*(int*)Param))?TRUE:FALSE,0));
		case oldfar::FCTL_SETANOTHERVIEWMODE:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_SETVIEWMODE:
			return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETVIEWMODE,(Param?*(int *)Param:0),0));
		case oldfar::FCTL_UPDATEANOTHERPANEL:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_UPDATEPANEL:
			return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_UPDATEPANEL,Param?1:0,0));
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
				oldfar::CmdLineSelect* clsA = (oldfar::CmdLineSelect*)Param;
				clsA->SelStart = cls.SelStart;
				clsA->SelEnd = cls.SelEnd;
			}

			return ret;
		}
		case oldfar::FCTL_INSERTCMDLINE:
		{
			if (!Param)
				return FALSE;

			wchar_t* s = AnsiToUnicode((const char*)Param);
			int ret = static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_INSERTCMDLINE,0,s));
			xf_free(s);
			return ret;
		}
		case oldfar::FCTL_SETCMDLINE:
		{
			if (!Param)
				return FALSE;

			wchar_t* s = AnsiToUnicode((const char*)Param);
			int ret = static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETCMDLINE,0,s));
			xf_free(s);
			return ret;
		}
		case oldfar::FCTL_SETCMDLINEPOS:

			if (!Param)
				return FALSE;

			return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETCMDLINEPOS,*(int*)Param,0));
		case oldfar::FCTL_SETCMDLINESELECTION:
		{
			if (!Param)
				return FALSE;

			oldfar::CmdLineSelect* clsA = (oldfar::CmdLineSelect*)Param;
			CmdLineSelect cls = {sizeof(CmdLineSelect),clsA->SelStart,clsA->SelEnd};
			return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETCMDLINESELECTION,0,&cls));
		}
		case oldfar::FCTL_GETUSERSCREEN:
			return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_GETUSERSCREEN,0,0));
		case oldfar::FCTL_SETUSERSCREEN:
			return static_cast<int>(NativeInfo.PanelControl(hPlugin, FCTL_SETUSERSCREEN,0,0));
	}

	return FALSE;
}

int WINAPI FarGetDirListA(const char *Dir,oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber)
{
	if (!Dir || !*Dir || !pPanelItem || !pItemsNumber)
		return FALSE;

	*pPanelItem=nullptr;
	*pItemsNumber=0;
	string strDir(Dir);
	DeleteEndSlash(strDir, true);

	PluginPanelItem *pItems;
	size_t ItemsNumber;
	int ret=NativeInfo.GetDirList(strDir, &pItems, &ItemsNumber);

	size_t PathOffset = ExtractFilePath(strDir).GetLength() + 1;

	if (ret && ItemsNumber)
	{
		//+sizeof(int) чтоб хранить ItemsNumber ибо в FarFreeDirListA как то надо знать
		*pPanelItem=(oldfar::PluginPanelItem *)xf_malloc(ItemsNumber*sizeof(oldfar::PluginPanelItem)+sizeof(int));

		if (*pPanelItem)
		{
			*pItemsNumber = static_cast<int>(ItemsNumber);
			**((int **)pPanelItem) = static_cast<int>(ItemsNumber);
			(*((int **)pPanelItem))++;
			memset(*pPanelItem,0,ItemsNumber*sizeof(oldfar::PluginPanelItem));

			for (size_t i=0; i<ItemsNumber; i++)
			{
				(*pPanelItem)[i].FindData.dwFileAttributes = pItems[i].FileAttributes;
				(*pPanelItem)[i].FindData.ftCreationTime = pItems[i].CreationTime;
				(*pPanelItem)[i].FindData.ftLastAccessTime = pItems[i].LastAccessTime;
				(*pPanelItem)[i].FindData.ftLastWriteTime = pItems[i].LastWriteTime;
				(*pPanelItem)[i].FindData.nFileSizeLow = (DWORD)pItems[i].FileSize;
				(*pPanelItem)[i].FindData.nFileSizeHigh = (DWORD)(pItems[i].FileSize>>32);
				UnicodeToOEM(pItems[i].FileName+PathOffset,(*pPanelItem)[i].FindData.cFileName,MAX_PATH);
				UnicodeToOEM(pItems[i].AlternateFileName,(*pPanelItem)[i].FindData.cAlternateFileName,14);
			}
		}
		else
		{
			ret = FALSE;
		}

		NativeInfo.FreeDirList(pItems,ItemsNumber);
	}

	return ret;
}

int WINAPI FarGetPluginDirListA(intptr_t PluginNumber,HANDLE hPlugin,const char *Dir,oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber)
{
	if (!Dir || !*Dir || !pPanelItem || !pItemsNumber)
		return FALSE;

	*pPanelItem=nullptr;
	*pItemsNumber=0;
	string strDir(Dir);

	PluginPanelItem *pPanelItemW;
	size_t ItemsNumber;
	int ret=NativeInfo.GetPluginDirList(GetPluginGuid(PluginNumber), hPlugin, strDir, &pPanelItemW, &ItemsNumber);

	if (ret && ItemsNumber)
	{
		//+sizeof(int) чтоб хранить ItemsNumber ибо в FarFreeDirListA как то надо знать
		*pPanelItem=(oldfar::PluginPanelItem *)xf_malloc(ItemsNumber*sizeof(oldfar::PluginPanelItem)+sizeof(int));

		if (*pPanelItem)
		{
			*pItemsNumber = static_cast<int>(ItemsNumber);
			**((int **)pPanelItem) = static_cast<int>(ItemsNumber);
			(*((int **)pPanelItem))++;
			memset(*pPanelItem,0,ItemsNumber*sizeof(oldfar::PluginPanelItem));

			for (size_t i=0; i<ItemsNumber; i++)
			{
				ConvertPanelItemToAnsi(pPanelItemW[i],(*pPanelItem)[i]);
			}
		}
		else
		{
			ret = FALSE;
		}

		NativeInfo.FreePluginDirList(hPlugin, pPanelItemW, ItemsNumber);
	}

	return ret;
}

void WINAPI FarFreeDirListA(const oldfar::PluginPanelItem *PanelItem)
{
	if (!PanelItem)
		return;

	//Тут хранится ItemsNumber полученный в FarGetDirListA или FarGetPluginDirListA
	int *base = ((int *)PanelItem) - 1;
	FreePanelItemA((oldfar::PluginPanelItem *)PanelItem, *base, false);
	xf_free(base);
}

static __int64 GetSetting(FARSETTINGS_SUBFOLDERS Root,const wchar_t* Name)
{
	__int64 result=0;
	FarSettingsCreate settings={sizeof(FarSettingsCreate),FarGuid,INVALID_HANDLE_VALUE};
	HANDLE Settings=NativeInfo.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings)?settings.Handle:0;
	if(Settings)
	{
		FarSettingsItem item={Root,Name,FST_UNKNOWN,{0}};
		if(NativeInfo.SettingsControl(Settings,SCTL_GET,0,&item)&&FST_QWORD==item.Type)
		{
			result=item.Number;
		}
		NativeInfo.SettingsControl(Settings,SCTL_FREE,0,0);
	}
	return result;
}

intptr_t WINAPI FarAdvControlA(intptr_t ModuleNumber,oldfar::ADVANCED_CONTROL_COMMANDS Command, void *Param)
{
#ifndef FAR_LUA
	static char *ErrMsg1 = nullptr;
	static char *ErrMsg2 = nullptr;
	static char *ErrMsg3 = nullptr;
#endif

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
			return IsConsoleFullscreen()?TRUE:FALSE;

		case oldfar::ACTL_GETSYSWORDDIV:
		{
			intptr_t Length = 0;
			FarSettingsCreate settings={sizeof(FarSettingsCreate),FarGuid,INVALID_HANDLE_VALUE};
			HANDLE Settings=NativeInfo.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings)?settings.Handle:0;
			if(Settings)
			{
				FarSettingsItem item={FSSF_EDITOR,L"WordDiv",FST_UNKNOWN,{0}};
				if(NativeInfo.SettingsControl(Settings,SCTL_GET,0,&item)&&FST_STRING==item.Type)
				{
					Length=Min(oldfar::NM,StrLength(item.String)+1);
					if(Param) UnicodeToOEM(item.String,(char*)Param,oldfar::NM);
				}
				NativeInfo.SettingsControl(Settings,SCTL_FREE,0,0);
			}
			return Length;
		}
		case oldfar::ACTL_WAITKEY:
			{
				INPUT_RECORD input={0};
				KeyToInputRecord(OldKeyToKey(static_cast<int>(reinterpret_cast<intptr_t>(Param))),&input);
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_WAITKEY, 0, &input);
			}
			break;

		case oldfar::ACTL_GETCOLOR:
			{
				FarColor Color;
				int ColorIndex = static_cast<int>(reinterpret_cast<intptr_t>(Param));

				// there was a reserved position after COL_VIEWERARROWS in Far 1.x.
				if(ColorIndex > COL_VIEWERARROWS-COL_FIRSTPALETTECOLOR)
				{
					ColorIndex--;
				}
				return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETCOLOR, ColorIndex, &Color)? Colors::FarColorToConsoleColor(Color) :-1;
			}
			break;

		case oldfar::ACTL_GETARRAYCOLOR:
			{
				size_t PaletteSize = NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETARRAYCOLOR, 0, nullptr);
				if(Param)
				{
					FarColor* Color = new FarColor[PaletteSize];
					NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETARRAYCOLOR, 0, Color);
					LPBYTE OldColors = static_cast<LPBYTE>(Param);
					for(size_t i = 0; i < PaletteSize; ++i)
					{
						OldColors[i] = static_cast<BYTE>(Colors::FarColorToConsoleColor(Color[i]));
					}
					delete[] Color;
				}
				return PaletteSize;
			}
			break;
		case oldfar::ACTL_EJECTMEDIA:
		{

			ActlEjectMedia eject={sizeof(ActlEjectMedia)};
			if(Param)
			{
				oldfar::ActlEjectMedia* ejectA=(oldfar::ActlEjectMedia*)Param;
				eject.Letter=ejectA->Letter;
				eject.Flags=ejectA->Flags;
			}
			return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_EJECTMEDIA, 0, &eject);
		}
		case oldfar::ACTL_KEYMACRO:
		{
#ifdef FAR_LUA
			return FALSE;
#else
			if (!Param) return FALSE;

			oldfar::ActlKeyMacro *kmA=(oldfar::ActlKeyMacro *)Param;
			FAR_MACRO_CONTROL_COMMANDS Command = MCTL_LOADALL;
			int Param1=0;
			bool Process=true;

			MacroSendMacroText mtW = {};
			mtW.StructSize = sizeof(MacroSendMacroText);

			switch (kmA->Command)
			{
				case oldfar::MCMD_LOADALL:
					Command=MCTL_LOADALL;
					break;
				case oldfar::MCMD_SAVEALL:
					Command=MCTL_SAVEALL;
					break;
				case oldfar::MCMD_GETSTATE:
					Command=MCTL_GETSTATE;
					break;
				case oldfar::MCMD_POSTMACROSTRING:
					Command=MCTL_SENDSTRING;
					Param1=MSSC_POST;
					mtW.SequenceText=AnsiToUnicode(kmA->PlainText.SequenceText);

					if (kmA->PlainText.Flags&oldfar::KSFLAGS_DISABLEOUTPUT) mtW.Flags|=KMFLAGS_DISABLEOUTPUT;

					if (kmA->PlainText.Flags&oldfar::KSFLAGS_NOSENDKEYSTOPLUGINS) mtW.Flags|=KMFLAGS_NOSENDKEYSTOPLUGINS;

					break;

				case oldfar::MCMD_CHECKMACRO:
					Command=MCTL_SENDSTRING;
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
				res = NativeInfo.MacroControl(0,Command,Param1,&mtW);

				if (Command == MCTL_SENDSTRING)
				{
					switch (Param1)
					{
						case MSSC_CHECK:
						{
							if (ErrMsg1) xf_free(ErrMsg1);

							if (ErrMsg2) xf_free(ErrMsg2);

							if (ErrMsg3) xf_free(ErrMsg3);

							string ErrMessage[3];

							CtrlObject->Macro.GetMacroParseError(&ErrMessage[0],&ErrMessage[1],&ErrMessage[2],nullptr);

							kmA->MacroResult.ErrMsg1 = ErrMsg1 = UnicodeToAnsi(ErrMessage[0]);
							kmA->MacroResult.ErrMsg2 = ErrMsg2 = UnicodeToAnsi(ErrMessage[1]);
							kmA->MacroResult.ErrMsg3 = ErrMsg3 = UnicodeToAnsi(ErrMessage[2]);

							if (mtW.SequenceText)
								xf_free((void*)mtW.SequenceText);

							break;
						}

						case MSSC_POST:

							if (mtW.SequenceText)
								xf_free((void*)mtW.SequenceText);

							break;
					}
				}
			}

			return res;
#endif
		}
		case oldfar::ACTL_POSTKEYSEQUENCE:
		{
#ifdef FAR_LUA
			return FALSE;
#else
			if (!Param)
				return FALSE;

			oldfar::KeySequence *ksA = (oldfar::KeySequence*)Param;

			if (!ksA->Count || !ksA->Sequence)
				return FALSE;

			MacroRecord MRec={};

			if (ksA->Flags&oldfar::KSFLAGS_DISABLEOUTPUT)
				MRec.Flags|=MFLAGS_DISABLEOUTPUT;

			if (ksA->Flags&oldfar::KSFLAGS_NOSENDKEYSTOPLUGINS)
				MRec.Flags|=MFLAGS_NOSENDKEYSTOPLUGINS;

			MRec.BufferSize=ksA->Count;

			DWORD* Sequence = (DWORD*)xf_malloc(MRec.BufferSize*sizeof(DWORD));
			for (int i=0; i<MRec.BufferSize; i++)
			{
				Sequence[i]=OldKeyToKey(ksA->Sequence[i]);
			}

			if (MRec.BufferSize == 1)
				MRec.Buffer=(DWORD *)(intptr_t)Sequence[0];
			else
				MRec.Buffer=Sequence;

			intptr_t ret = CtrlObject->Macro.PostNewMacro(&MRec,TRUE,TRUE);
			xf_free(Sequence);

			return ret;
#endif
		}
		case oldfar::ACTL_GETSHORTWINDOWINFO:
		case oldfar::ACTL_GETWINDOWINFO:
		{
			if (!Param)
				return FALSE;

			oldfar::WindowInfo *wiA = (oldfar::WindowInfo *)Param;
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

				wiA->Modified = (wi.Flags&WIF_MODIFIED)?true:false;
				wiA->Current = (wi.Flags&WIF_CURRENT)?true:false;

				if (Command==oldfar::ACTL_GETWINDOWINFO)
				{
					if (wi.TypeNameSize)
					{
						wi.TypeName=new wchar_t[wi.TypeNameSize];
					}

					if (wi.NameSize)
					{
						wi.Name=new wchar_t[wi.NameSize];
					}

					if (wi.TypeName && wi.Name)
					{
						NativeInfo.AdvControl(GetPluginGuid(ModuleNumber),ACTL_GETWINDOWINFO, 0, &wi);
						UnicodeToOEM(wi.TypeName,wiA->TypeName,sizeof(wiA->TypeName));
						UnicodeToOEM(wi.Name,wiA->Name,sizeof(wiA->Name));
					}

					if (wi.TypeName)
					{
						delete[] wi.TypeName;
					}

					if (wi.Name)
					{
						delete[] wi.Name;
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
			return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETWINDOWCOUNT, 0, 0);
		case oldfar::ACTL_SETCURRENTWINDOW:
			return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_SETCURRENTWINDOW, static_cast<int>(reinterpret_cast<intptr_t>(Param)), nullptr);
		case oldfar::ACTL_COMMIT:
			return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_COMMIT, 0, 0);
		case oldfar::ACTL_GETFARHWND:
			return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_GETFARHWND, 0, 0);
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
			if (!Param) return FALSE;

			oldfar::FarSetColors *scA = (oldfar::FarSetColors *)Param;
			FarSetColors sc = {sizeof(FarSetColors), 0, (size_t)scA->StartIndex, (size_t)scA->ColorCount, new FarColor[scA->ColorCount]};
			for(size_t i = 0; i < sc.ColorsCount; ++i)
			{
				Colors::ConsoleColorToFarColor(scA->Colors[i], sc.Colors[i]);
			}

			if (scA->Flags&oldfar::FCLR_REDRAW) sc.Flags|=FSETCLR_REDRAW;
			intptr_t Result = NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_SETARRAYCOLOR, 0, &sc);
			delete[] sc.Colors;
			return Result;
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
			return NativeInfo.AdvControl(GetPluginGuid(ModuleNumber), ACTL_REDRAWALL, 0, 0);
	}

	return FALSE;
}

UINT GetEditorCodePageA()
{
	EditorInfo info={sizeof(EditorInfo)};
	NativeInfo.EditorControl(-1,ECTL_GETINFO,0,&info);
	UINT CodePage=info.CodePage;
	CPINFO cpi;

	if (!GetCPInfo(CodePage, &cpi) || cpi.MaxCharSize>1)
		CodePage=GetACP();

	return CodePage;
}

int GetEditorCodePageFavA()
{
	UINT CodePage=GetEditorCodePageA();
	int result=-((int)CodePage+2);

	if (GetOEMCP()==CodePage)
	{
		result=0;
	}
	else if (GetACP()==CodePage)
	{
		result=1;
	}
	else
	{
		DWORD selectType, Index = 0, FavIndex = 2;
		string sTableName;
		while (GeneralCfg->EnumValues(FavoriteCodePagesKey,Index++,sTableName,&selectType))
		{
			if (!(selectType&CPST_FAVORITE))
				continue;

			if (static_cast<UINT>(_wtoi(sTableName)) == CodePage)
			{
				result=FavIndex;
				break;
			}

			FavIndex++;
		}
	}

	return result;
}

void MultiByteRecode(UINT nCPin, UINT nCPout, char *szBuffer, int nLength)
{
	if (szBuffer && nLength > 0)
	{
		wchar_t *wszTempTable = (wchar_t *) xf_malloc(nLength * sizeof(wchar_t));

		if (wszTempTable)
		{
			MultiByteToWideChar(nCPin, 0, szBuffer, nLength, wszTempTable, nLength);
			WideCharToMultiByte(nCPout, 0, wszTempTable, nLength, szBuffer, nLength, nullptr, nullptr);
			xf_free(wszTempTable);
		}
	}
};

UINT ConvertCharTableToCodePage(int Command)
{
	string strTableName;
	UINT nCP = 0;

	if (Command<0)
	{
		nCP=-(Command+2);
	}
	else
	{
		switch (Command)
		{
			case 0 /* OEM */: 	nCP = GetOEMCP();	break;
			case 1 /* ANSI */:	nCP = GetACP(); 	break;
			default:
			{
				DWORD selectType,Index=0;
				int FavIndex=2;

				for (;;)
				{
					if (!GeneralCfg->EnumValues(FavoriteCodePagesKey,Index++,strTableName,&selectType)) return CP_DEFAULT;

					if (!(selectType&CPST_FAVORITE)) continue;

					if (FavIndex==Command)
					{
						nCP=_wtoi(strTableName);
						break;
					}

					FavIndex++;
				}
			}
		}
	}

	return nCP;
}

int WINAPI FarEditorControlA(oldfar::EDITOR_CONTROL_COMMANDS OldCommand,void* Param)
{
	static char *gt=nullptr;
	static char *geol=nullptr;
	static char *fn=nullptr;
	EDITOR_CONTROL_COMMANDS Command=ECTL_GETSTRING;
	switch (OldCommand)
	{
		case oldfar::ECTL_ADDCOLOR:
			if(Param)
			{
				oldfar::EditorColor* ecA = static_cast<oldfar::EditorColor*>(Param);
				EditorColor ec={sizeof(ec)};
				ec.StringNumber = ecA->StringNumber;
				ec.StartPos = ecA->StartPos;
				ec.EndPos = ecA->EndPos;
				Colors::ConsoleColorToFarColor(ecA->Color,ec.Color);
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
					ecA->Color = Colors::FarColorToConsoleColor(ec.Color);
					if(ec.Color.Flags&ECF_TABMARKFIRST) ecA->Color|=oldfar::ECF_TAB1;
				}
				return Result;
			}
			return FALSE;

		case oldfar::ECTL_GETSTRING:
		{
			EditorGetString egs={sizeof(EditorGetString)};
			oldfar::EditorGetString *oegs=(oldfar::EditorGetString *)Param;

			if (!oegs) return FALSE;

			egs.StringNumber=oegs->StringNumber;
			int ret=static_cast<int>(NativeInfo.EditorControl(-1,ECTL_GETSTRING,0,&egs));

			if (ret)
			{
				oegs->StringNumber=egs.StringNumber;
				oegs->StringLength=egs.StringLength;
				oegs->SelStart=egs.SelStart;
				oegs->SelEnd=egs.SelEnd;

				if (gt) xf_free(gt);

				if (geol) xf_free(geol);

				UINT CodePage=GetEditorCodePageA();
				gt = UnicodeToAnsiBin(egs.StringText,egs.StringLength,CodePage);
				geol = UnicodeToAnsi(egs.StringEOL,CodePage);
				oegs->StringText=gt;
				oegs->StringEOL=geol;
				return TRUE;
			}

			return FALSE;
		}
		case oldfar::ECTL_INSERTTEXT:
		{
			const char *p=(const char *)Param;

			if (!p) return FALSE;

			string strP(p);
			return static_cast<int>(NativeInfo.EditorControl(-1,ECTL_INSERTTEXT,0,const_cast<wchar_t*>(strP.CPtr())));
		}
		case oldfar::ECTL_GETINFO:
		{
			EditorInfo ei={sizeof(EditorInfo)};
			oldfar::EditorInfo *oei=(oldfar::EditorInfo *)Param;

			if (!oei)
				return FALSE;

			int ret=static_cast<int>(NativeInfo.EditorControl(-1,ECTL_GETINFO,0,&ei));

			if (ret)
			{
				if (fn)
					xf_free(fn);

				ClearStruct(*oei);
				size_t FileNameSize=NativeInfo.EditorControl(-1,ECTL_GETFILENAME,0,0);

				if (FileNameSize)
				{
					LPWSTR FileName=new wchar_t[FileNameSize];
					NativeInfo.EditorControl(-1,ECTL_GETFILENAME,0,FileName);
					fn = UnicodeToAnsi(FileName);
					oei->FileName=fn;
					delete[] FileName;
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
				oei->BookMarkCount=ei.BookMarkCount;
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

			oldfar::EditorConvertText *ect=(oldfar::EditorConvertText*) Param;
			UINT CodePage=GetEditorCodePageA();
			MultiByteRecode(OldCommand==oldfar::ECTL_OEMTOEDITOR ? CP_OEMCP : CodePage, OldCommand==oldfar::ECTL_OEMTOEDITOR ?  CodePage : CP_OEMCP, ect->Text, ect->TextLength);
			return TRUE;
		}
		case oldfar::ECTL_SAVEFILE:
		{
			EditorSaveFile newsf = {sizeof(EditorSaveFile)};

			if (Param)
			{
				oldfar::EditorSaveFile *oldsf = (oldfar::EditorSaveFile*) Param;
				newsf.FileName = AnsiToUnicode(oldsf->FileName);
				newsf.FileEOL=(oldsf->FileEOL)?AnsiToUnicode(oldsf->FileEOL):nullptr;
			}

			int ret = static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SAVEFILE, 0, Param?&newsf:0));

			if (newsf.FileName) xf_free((void*)newsf.FileName);

			if (newsf.FileEOL) xf_free((void*)newsf.FileEOL);

			return ret;
		}
		case oldfar::ECTL_PROCESSINPUT:	//BUGBUG?
		{
			if (Param)
			{
				INPUT_RECORD *pIR = (INPUT_RECORD*) Param;

				switch (pIR->EventType)
				{
					case KEY_EVENT:
					case FARMACRO_KEY_EVENT:
					{
						wchar_t res;
						MultiByteToWideChar(
						    CP_OEMCP,
						    0,
						    &pIR->Event.KeyEvent.uChar.AsciiChar,
						    1,
						    &res,
						    1
						);
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
				INPUT_RECORD *pIR = (INPUT_RECORD*) Param;

				switch (pIR->EventType)
				{
					case KEY_EVENT:
					case FARMACRO_KEY_EVENT:
					{
						char res;
						WideCharToMultiByte(
						    CP_OEMCP,
						    0,
						    &pIR->Event.KeyEvent.uChar.UnicodeChar,
						    1,
						    &res,
						    1,
						    nullptr,
						    nullptr
						);
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
					oldfar::KeyBarTitles* oldkbt = (oldfar::KeyBarTitles*)Param;
					KeyBarTitles newkbt;
					ConvertKeyBarTitlesA(oldkbt, &newkbt);
					int ret = static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SETKEYBAR, 0, &newkbt));
					FreeUnicodeKeyBarTitles(&newkbt);
					return ret;
			}
		}
		case oldfar::ECTL_SETPARAM:
		{
			EditorSetParameter newsp = {sizeof(EditorSetParameter)};

			if (Param)
			{
				oldfar::EditorSetParameter *oldsp= (oldfar::EditorSetParameter*) Param;
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

								if ((UINT)newsp.iParam==CP_DEFAULT) return FALSE;

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
						newsp.wszParam = (oldsp->cParam)?AnsiToUnicode(oldsp->cParam):nullptr;
						int ret = static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SETPARAM, 0, &newsp));

						if (newsp.wszParam) xf_free(newsp.wszParam);

						return ret;
					}
					case oldfar::ESPT_GETWORDDIV:
					{
						if (!oldsp->cParam) return FALSE;

						*oldsp->cParam=0;
						newsp.Type = ESPT_GETWORDDIV;
						newsp.wszParam = nullptr;
						newsp.Size = 0;
						newsp.Size = NativeInfo.EditorControl(-1,ECTL_SETPARAM, 0, &newsp);
						newsp.wszParam = (wchar_t*)xf_malloc(newsp.Size*sizeof(wchar_t));

						if (newsp.wszParam)
						{
							int ret = static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SETPARAM, 0, &newsp));
							char *olddiv = UnicodeToAnsi(newsp.wszParam);

							if (olddiv)
							{
								xstrncpy(oldsp->cParam, olddiv, 0x100);
								xf_free(olddiv);
							}

							xf_free(newsp.wszParam);
							return ret;
						}

						return FALSE;
					}
					default:
						return FALSE;
				}
			}

			return static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SETPARAM, 0, Param?&newsp:0));
		}
		case oldfar::ECTL_SETSTRING:
		{
			EditorSetString newss = {sizeof(EditorSetString)};

			if (Param)
			{
				oldfar::EditorSetString *oldss = (oldfar::EditorSetString*) Param;
				newss.StringNumber=oldss->StringNumber;
				UINT CodePage=GetEditorCodePageA();
				newss.StringText=(oldss->StringText)?AnsiToUnicodeBin(oldss->StringText, oldss->StringLength,CodePage):nullptr;
				newss.StringEOL=(oldss->StringEOL && *oldss->StringEOL)?AnsiToUnicode(oldss->StringEOL,CodePage):nullptr;
				newss.StringLength=oldss->StringLength;
			}

			int ret = static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SETSTRING, 0, Param?&newss:0));

			if (newss.StringText) xf_free((void*)newss.StringText);

			if (newss.StringEOL) xf_free((void*)newss.StringEOL);

			return ret;
		}
		case oldfar::ECTL_SETTITLE:
		{
			wchar_t* newtit = nullptr;

			if (Param)
			{
				newtit=AnsiToUnicode((char*)Param);
			}

			int ret = static_cast<int>(NativeInfo.EditorControl(-1,ECTL_SETTITLE, 0, newtit));

			if (newtit) xf_free(newtit);

			return ret;
		}
		// BUGBUG, convert params
		case oldfar::ECTL_DELETEBLOCK:	Command = ECTL_DELETEBLOCK; break;
		case oldfar::ECTL_DELETECHAR:		Command = ECTL_DELETECHAR; break;
		case oldfar::ECTL_DELETESTRING:	Command = ECTL_DELETESTRING; break;
		case oldfar::ECTL_EXPANDTABS:		Command = ECTL_EXPANDTABS; break;
		case oldfar::ECTL_GETBOOKMARKS:	Command = ECTL_GETBOOKMARKS; break;
		case oldfar::ECTL_INSERTSTRING:	Command = ECTL_INSERTSTRING; break;
		case oldfar::ECTL_QUIT:					Command = ECTL_QUIT; break;
		case oldfar::ECTL_REALTOTAB:		Command = ECTL_REALTOTAB; break;
		case oldfar::ECTL_REDRAW:				Command = ECTL_REDRAW; break;
		case oldfar::ECTL_SELECT:				Command = ECTL_SELECT; break;
		case oldfar::ECTL_SETPOSITION:	Command = ECTL_SETPOSITION; break;
		case oldfar::ECTL_TABTOREAL:		Command = ECTL_TABTOREAL; break;
		case oldfar::ECTL_ADDSTACKBOOKMARK:			Command = ECTL_ADDSESSIONBOOKMARK; break;
		case oldfar::ECTL_PREVSTACKBOOKMARK:		Command = ECTL_PREVSESSIONBOOKMARK; break;
		case oldfar::ECTL_NEXTSTACKBOOKMARK:		Command = ECTL_NEXTSESSIONBOOKMARK; break;
		case oldfar::ECTL_CLEARSTACKBOOKMARKS:	Command = ECTL_CLEARSESSIONBOOKMARKS; break;
		case oldfar::ECTL_DELETESTACKBOOKMARK:	Command = ECTL_DELETESESSIONBOOKMARK; break;
		case oldfar::ECTL_GETSTACKBOOKMARKS:		Command = ECTL_GETSESSIONBOOKMARKS; break;
		default:
			return FALSE;
	}
	return static_cast<int>(NativeInfo.EditorControl(-1, Command, 0, Param));
}

int WINAPI FarViewerControlA(int Command,void* Param)
{
	static char* filename=nullptr;

	switch (Command)
	{
		case oldfar::VCTL_GETINFO:
		{
			if (!Param) return FALSE;

			oldfar::ViewerInfo* viA = (oldfar::ViewerInfo*)Param;

			if (!viA->StructSize) return FALSE;

			ViewerInfo viW = {sizeof(viW)};

			if (NativeInfo.ViewerControl(-1,VCTL_GETINFO, 0, &viW) == FALSE) return FALSE;

			viA->ViewerID = viW.ViewerID;

			if (filename) xf_free(filename);

			filename = UnicodeToAnsi(viW.FileName);
			viA->FileName = filename;
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
			viA->CurMode.Wrap           = viW.CurMode.Wrap;
			viA->CurMode.WordWrap       = viW.CurMode.WordWrap;
			viA->CurMode.Hex            = viW.CurMode.Hex;
			viA->LeftPos = (int)viW.LeftPos;
			viA->Reserved3 = 0;
			break;
		}
		case oldfar::VCTL_QUIT:
			return static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_QUIT,0, 0));
		case oldfar::VCTL_REDRAW:
			return static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_REDRAW,0, 0));
		case oldfar::VCTL_SETKEYBAR:
		{
			switch ((intptr_t)Param)
			{
				case 0:
				case -1:
					return static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_SETKEYBAR,0, Param));
				default:
					oldfar::KeyBarTitles* kbtA = (oldfar::KeyBarTitles*)Param;
					KeyBarTitles kbt;
					ConvertKeyBarTitlesA(kbtA, &kbt);
					int ret=static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_SETKEYBAR,0, &kbt));
					FreeUnicodeKeyBarTitles(&kbt);
					return ret;
			}
		}
		case oldfar::VCTL_SETPOSITION:
		{
			if (!Param) return FALSE;

			oldfar::ViewerSetPosition* vspA = (oldfar::ViewerSetPosition*)Param;
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
			if (!Param) return static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_SELECT,0, 0));

			oldfar::ViewerSelect* vsA = (oldfar::ViewerSelect*)Param;
			ViewerSelect vs = {sizeof(ViewerSelect),vsA->BlockStartPos,vsA->BlockLen};
			return static_cast<int>(NativeInfo.ViewerControl(-1,VCTL_SELECT,0, &vs));
		}
		case oldfar::VCTL_SETMODE:
		{
			if (!Param) return FALSE;

			oldfar::ViewerSetMode* vsmA = (oldfar::ViewerSetMode*)Param;
			ViewerSetMode vsm={sizeof(ViewerSetMode)};

			switch (vsmA->Type)
			{
				case oldfar::VSMT_HEX:      vsm.Type = VSMT_HEX;      break;
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

int WINAPI FarCharTableA(int Command, char *Buffer, int BufferSize)
{
	if (Command != oldfar::FCT_DETECT)
	{
		if (BufferSize != (int) sizeof(oldfar::CharTableSet))
			return -1;

		oldfar::CharTableSet *TableSet=(oldfar::CharTableSet*)Buffer;
		//Preset. Also if Command != FCT_DETECT and failed, buffer must be filled by OEM data.
		strcpy(TableSet->TableName,"<failed>");

		for (unsigned int i = 0; i < 256; ++i)
		{
			TableSet->EncodeTable[i] = TableSet->DecodeTable[i] = i;
			TableSet->UpperTable[i] = LocalUpper(i);
			TableSet->LowerTable[i] = LocalLower(i);
		}

		UINT nCP = ConvertCharTableToCodePage(Command);

		if (nCP==CP_DEFAULT) return -1;

		CPINFOEX cpiex;

		if (!GetCPInfoEx(nCP, 0, &cpiex))
		{
			CPINFO cpi;

			if (!GetCPInfo(nCP, &cpi))
				return -1;

			cpiex.MaxCharSize = cpi.MaxCharSize;
			cpiex.CodePageName[0] = L'\0';
		}

		if (cpiex.MaxCharSize != 1)
			return -1;

		wchar_t *codePageName = FormatCodePageName(nCP, cpiex.CodePageName, sizeof(cpiex.CodePageName)/sizeof(wchar_t));
		FormatString sTableName;
		sTableName<<fmt::MinWidth(5)<<nCP<<BoxSymbols[BS_V1]<<L" "<<codePageName;
		sTableName.GetCharString(TableSet->TableName, sizeof(TableSet->TableName) - 1, CP_OEMCP);
		wchar_t *us=AnsiToUnicodeBin((char*)TableSet->DecodeTable, sizeof(TableSet->DecodeTable), nCP);
		CharLowerBuff(us, sizeof(TableSet->DecodeTable));
		WideCharToMultiByte(nCP, 0, us, sizeof(TableSet->DecodeTable), (char*)TableSet->LowerTable, sizeof(TableSet->DecodeTable), nullptr, nullptr);
		CharUpperBuff(us, sizeof(TableSet->DecodeTable));
		WideCharToMultiByte(nCP, 0, us, sizeof(TableSet->DecodeTable), (char*)TableSet->UpperTable, sizeof(TableSet->DecodeTable), nullptr, nullptr);
		xf_free(us);
		MultiByteRecode(nCP, CP_OEMCP, (char *) TableSet->DecodeTable, sizeof(TableSet->DecodeTable));
		MultiByteRecode(CP_OEMCP, nCP, (char *) TableSet->EncodeTable, sizeof(TableSet->EncodeTable));
		return Command;
	}

	return -1;
}

char* WINAPI XlatA(
    char *Line,                    // исходная строка
    int StartPos,                  // начало переконвертирования
    int EndPos,                    // конец переконвертирования
    const oldfar::CharTableSet *TableSet, // перекодировочная таблица (может быть nullptr)
    DWORD Flags)                   // флаги (см. enum XLATMODE)
{
	string strLine(Line);
	DWORD NewFlags = 0;

	if (Flags&oldfar::XLAT_SWITCHKEYBLAYOUT)
		NewFlags|=XLAT_SWITCHKEYBLAYOUT;

	if (Flags&oldfar::XLAT_SWITCHKEYBBEEP)
		NewFlags|=XLAT_SWITCHKEYBBEEP;

	if (Flags&oldfar::XLAT_USEKEYBLAYOUTNAME)
		NewFlags|=XLAT_USEKEYBLAYOUTNAME;

	if (Flags&oldfar::XLAT_CONVERTALLCMDLINE)
		NewFlags|=XLAT_CONVERTALLCMDLINE;

	NativeFSF.XLat(strLine.GetBuffer(),StartPos,EndPos,NewFlags);
	strLine.ReleaseBuffer();
	strLine.GetCharString(Line,strLine.GetLength()+1);
	return Line;
}

int WINAPI GetFileOwnerA(const char *Computer,const char *Name, char *Owner)
{
	string strComputer(Computer), strName(Name);
	wchar_t wOwner[MAX_PATH];
	int Ret=static_cast<int>(NativeFSF.GetFileOwner(strComputer,strName, wOwner, ARRAYSIZE(wOwner)));
	if (Ret)
	{
		UnicodeToOEM(wOwner, Owner, oldfar::NM);
	}
	return Ret;
}

static void CheckScreenLock()
{
	if (ScrBuf.GetLockCount() > 0 && !CtrlObject->Macro.PeekKey())
	{
//		ScrBuf.SetLockCount(0);
		ScrBuf.Flush();
	}
}

PluginA::PluginA(PluginManager *owner, const wchar_t *lpwszModuleName):
	Plugin(owner,lpwszModuleName),
	RootKey(nullptr),
	pFDPanelItemA(nullptr),
	pVFDPanelItemA(nullptr),
	OEMApiCnt(0)
{
	ExportsNamesW = _ExportsNamesW;
	ExportsNamesA = _ExportsNamesA;
	ClearStruct(PI);
	ClearStruct(OPI);
}

PluginA::~PluginA()
{
	if (RootKey) xf_free(RootKey);

	FreePluginInfo();
	FreeOpenPanelInfo();
}

oldfar::FarStandardFunctions StandardFunctions =
{
	sizeof(StandardFunctions),
	wrapper::FarAtoiA,
	wrapper::FarAtoi64A,
	wrapper::FarItoaA,
	wrapper::FarItoa64A,
	sprintf,
	sscanf,
	wrapper::qsort,
	wrapper::bsearch,
	wrapper::qsortex,
	_snprintf,
	{},
	wrapper::LocalIslower,
	wrapper::LocalIsupper,
	wrapper::LocalIsalpha,
	wrapper::LocalIsalphanum,
	wrapper::LocalUpper,
	wrapper::LocalLower,
	wrapper::LocalUpperBuf,
	wrapper::LocalLowerBuf,
	wrapper::LocalStrupr,
	wrapper::LocalStrlwr,
	wrapper::LStricmp,
	wrapper::LStrnicmp,
	wrapper::UnquoteA,
	wrapper::ExpandEnvironmentStrA,
	wrapper::RemoveLeadingSpacesA,
	wrapper::RemoveTrailingSpacesA,
	wrapper::RemoveExternalSpacesA,
	wrapper::TruncStrA,
	wrapper::TruncPathStrA,
	wrapper::QuoteSpaceOnlyA,
	wrapper::PointToNameA,
	wrapper::GetPathRootA,
	wrapper::AddEndSlashA,
	wrapper::CopyToClipboardA,
	wrapper::PasteFromClipboardA,
	wrapper::FarKeyToNameA,
	wrapper::KeyNameToKeyA,
	wrapper::InputRecordToKeyA,
	wrapper::XlatA,
	wrapper::GetFileOwnerA,
	wrapper::GetNumberOfLinksA,
	wrapper::FarRecursiveSearchA,
	wrapper::FarMkTempA,
	wrapper::DeleteBufferA,
	wrapper::ProcessNameA,
	wrapper::FarMkLinkA,
	wrapper::ConvertNameToRealA,
	wrapper::FarGetReparsePointInfoA,
};

oldfar::PluginStartupInfo StartupInfo =
{
	sizeof(StartupInfo),
	"", // ModuleName, dynamic
	0, // ModuleNumber, dynamic
	nullptr, // RootKey, dynamic
	wrapper::FarMenuFnA,
	wrapper::FarDialogFnA,
	wrapper::FarMessageFnA,
	wrapper::FarGetMsgFnA,
	wrapper::FarPanelControlA,
	nullptr, // copy from NativeInfo
	nullptr, // copy from NativeInfo
	wrapper::FarGetDirListA,
	wrapper::FarGetPluginDirListA,
	wrapper::FarFreeDirListA,
	wrapper::FarViewerA,
	wrapper::FarEditorA,
	wrapper::FarCmpNameA,
	wrapper::FarCharTableA,
	wrapper::FarTextA,
	wrapper::FarEditorControlA,
	nullptr, // FSF, dynamic
	wrapper::FarShowHelpA,
	wrapper::FarAdvControlA,
	wrapper::FarInputBoxA,
	wrapper::FarDialogExA,
	wrapper::FarSendDlgMessageA,
	wrapper::FarDefDlgProcA,
	0,
	wrapper::FarViewerControlA,
};

static void CreatePluginStartupInfoA(PluginA *pPlugin, oldfar::PluginStartupInfo *PSI, oldfar::FarStandardFunctions *FSF)
{
	StartupInfo.SaveScreen = NativeInfo.SaveScreen;
	StartupInfo.RestoreScreen = NativeInfo.RestoreScreen;

	*PSI=StartupInfo;
	*FSF=StandardFunctions;
	PSI->ModuleNumber=(intptr_t)pPlugin;
	PSI->FSF=FSF;
	pPlugin->GetModuleName().GetCharString(PSI->ModuleName,sizeof(PSI->ModuleName));
}

bool PluginA::GetGlobalInfo(GlobalInfo* Info)
{
	Info->StructSize = sizeof(GlobalInfo);
	Info->Title = PointToName(GetModuleName());
	Info->Description = L"Far 1.x plugin";
	Info->Author = L"unknown";
	if(GetGUID()==FarGuid)
	{
		// first load
		UuidCreate(&Info->Guid);
	}
	else
	{
		// use cached
		Info->Guid = GetGUID();
	}
	return true;
}

bool PluginA::SetStartupInfo()
{
	if (Exports[iSetStartupInfo] && !ProcessException)
	{
		oldfar::PluginStartupInfo _info;
		oldfar::FarStandardFunctions _fsf;
		CreatePluginStartupInfoA(this, &_info, &_fsf);
		// скорректирем адреса и плагино-зависимые поля
		strRootKey = Opt.strRegRoot + L"\\Plugins";
		RootKey = UnicodeToAnsi(strRootKey);
		_info.RootKey = RootKey;
		ExecuteStruct es;
		es.id = EXCEPT_SETSTARTUPINFO;
		EXECUTE_FUNCTION(FUNCTION(iSetStartupInfo)(&_info), es);

		if (bPendingRemove)
		{
			return false;
		}
	}

	return true;
}

bool PluginA::CheckMinFarVersion()
{
	if (Exports[iGetMinFarVersion] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_MINFARVERSION;
		es.nDefaultResult = 0;
		EXECUTE_FUNCTION_EX(FUNCTION(iGetMinFarVersion)(), es);

		if (bPendingRemove)
		{
			return false;
		}
	}

	return true;
}

static HANDLE TranslateResult(HANDLE hResult)
{
	if(INVALID_HANDLE_VALUE==hResult) return nullptr;
	if((HANDLE)-2==hResult) return PANEL_STOP;
	return hResult;
}

HANDLE PluginA::Open(int OpenFrom, const GUID& Guid, intptr_t Item)
{
	ChangePriority *ChPriority = new ChangePriority(THREAD_PRIORITY_NORMAL);

	CheckScreenLock(); //??

	{
//		string strCurDir;
//		CtrlObject->CmdLine->GetCurDir(strCurDir);
//		FarChDir(strCurDir);
		g_strDirToSet.Clear();
	}

	HANDLE hResult = nullptr;

	if (Load() && Exports[iOpen] && !ProcessException)
	{
		//CurPluginItem=this; //BUGBUG
		ExecuteStruct es;
		es.id = EXCEPT_OPEN;
		es.hDefaultResult = nullptr;
		es.hResult = nullptr;
		char *ItemA = nullptr;
		oldfar::OpenDlgPluginData DlgData;

		if (Item && OpenFrom == OPEN_COMMANDLINE)
		{
			ItemA = UnicodeToAnsi(((OpenCommandLineInfo *)Item)->CommandLine);
			Item = (intptr_t)ItemA;
		}
		if (Item && OpenFrom == OPEN_SHORTCUT)
		{
			ItemA = UnicodeToAnsi(((OpenShortcutInfo *)Item)->ShortcutData);
			Item = (intptr_t)ItemA;
		}
		if (OpenFrom == OPEN_LEFTDISKMENU || OpenFrom == OPEN_RIGHTDISKMENU || OpenFrom == OPEN_PLUGINSMENU || OpenFrom == OPEN_EDITOR || OpenFrom == OPEN_VIEWER)
		{
			Item=Guid.Data1;
		}
		if (OpenFrom == OPEN_RIGHTDISKMENU)
		{
			OpenFrom = OPEN_LEFTDISKMENU;
		}
		if (OpenFrom == OPEN_FROMMACRO)
		{
			OpenFrom = oldfar::OPEN_FROMMACRO|CtrlObject->Macro.GetMode();
			Item=(intptr_t)UnicodeToAnsi(((OpenMacroInfo*)Item)->Count?((OpenMacroInfo*)Item)->Values[0].String:L"");
		}
		if (OpenFrom == OPEN_DIALOG)
		{
			DlgData.ItemNumber=Guid.Data1;
			DlgData.hDlg=reinterpret_cast<OpenDlgPluginData*>(Item)->hDlg;
			Item=(intptr_t)&DlgData;
		}
		EXECUTE_FUNCTION_EX(FUNCTION(iOpen)(OpenFrom,Item), es);

		if (ItemA) xf_free(ItemA);

		hResult = TranslateResult(es.hResult);
		//CurPluginItem=nullptr; //BUGBUG
		/*    CtrlObject->Macro.SetRedrawEditor(TRUE); //BUGBUG

		    if ( !bUnloaded )
		    {

		      if(OpenFrom == OPEN_EDITOR &&
		         !CtrlObject->Macro.IsExecuting() &&
		         CtrlObject->Plugins->CurEditor &&
		         CtrlObject->Plugins->CurEditor->IsVisible() )
		      {
		        CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
		        CtrlObject->Plugins->CurEditor->Show();
		      }
		      if (hInternal!=INVALID_HANDLE_VALUE)
		      {
		        PluginHandle *hPlugin=new PluginHandle;
		        hPlugin->InternalHandle=es.hResult;
		        hPlugin->PluginNumber=(intptr_t)this;
		        return((HANDLE)hPlugin);
		      }
		      else
		        if ( !g_strDirToSet.IsEmpty() )
		        {
							CtrlObject->Cp()->ActivePanel->SetCurDir(g_strDirToSet,TRUE);
		          CtrlObject->Cp()->ActivePanel->Redraw();
		        }
		    } */
	}

	delete ChPriority;

	return hResult;
}

//////////////////////////////////

HANDLE PluginA::OpenFilePlugin(
    const wchar_t *Name,
    const unsigned char *Data,
    size_t DataSize,
    int OpMode
)
{
	HANDLE hResult = nullptr;

	if (Load() && Exports[iOpenFilePlugin] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_OPENFILEPLUGIN;
		es.hDefaultResult = nullptr;
		char *NameA = nullptr;

		if (Name)
			NameA = UnicodeToAnsi(Name);

		EXECUTE_FUNCTION_EX(FUNCTION(iOpenFilePlugin)(NameA, Data, static_cast<int>(DataSize)), es);

		if (NameA) xf_free(NameA);

		hResult = TranslateResult(es.hResult);
	}

	return hResult;
}


int PluginA::SetFindList(
    HANDLE hPlugin,
    const PluginPanelItem *PanelItem,
    size_t ItemsNumber
)
{
	BOOL bResult = FALSE;

	if (Exports[iSetFindList] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_SETFINDLIST;
		es.bDefaultResult = FALSE;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		EXECUTE_FUNCTION_EX(FUNCTION(iSetFindList)(hPlugin, PanelItemA, static_cast<int>(ItemsNumber)), es);
		FreePanelItemA(PanelItemA,ItemsNumber);
		bResult = es.bResult;
	}

	return bResult;
}

int PluginA::ProcessEditorInput(
    const INPUT_RECORD *D
)
{
	BOOL bResult = FALSE;

	if (Load() && Exports[iProcessEditorInput] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSEDITORINPUT;
		es.bDefaultResult = TRUE; //(TRUE) treat the result as a completed request on exception!
		const INPUT_RECORD *Ptr=D;
		INPUT_RECORD OemRecord;

		if (Ptr->EventType==KEY_EVENT)
		{
			OemRecord=*D;
			CharToOemBuff(&D->Event.KeyEvent.uChar.UnicodeChar,&OemRecord.Event.KeyEvent.uChar.AsciiChar,1);
			Ptr=&OemRecord;
		}

		EXECUTE_FUNCTION_EX(FUNCTION(iProcessEditorInput)(Ptr), es);
		bResult = es.bResult;
	}

	return bResult;
}

int PluginA::ProcessEditorEvent(
    int Event,
    PVOID Param,
    int EditorID
)
{
	if (Load() && Exports[iProcessEditorEvent] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSEDITOREVENT;
		es.nDefaultResult = 0;
		switch(Event)
		{
			case EE_CLOSE:
			case EE_GOTFOCUS:
			case EE_KILLFOCUS:
				Param=&EditorID;
			case EE_READ:
			case EE_SAVE:
			case EE_REDRAW:
				EXECUTE_FUNCTION_EX(FUNCTION(iProcessEditorEvent)(Event, Param), es);
				break;
		}
	}

	return 0; //oops!
}

int PluginA::ProcessViewerEvent(
    int Event,
    void *Param,
    int ViewerID
)
{
	if (Load() && Exports[iProcessViewerEvent] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSVIEWEREVENT;
		es.nDefaultResult = 0;
		switch(Event)
		{
			case VE_CLOSE:
			case VE_GOTFOCUS:
			case VE_KILLFOCUS:
				Param=&ViewerID;
			case VE_READ:
				EXECUTE_FUNCTION_EX(FUNCTION(iProcessViewerEvent)(Event, Param), es);
				break;
		}
	}

	return 0; //oops, again!
}

int PluginA::ProcessDialogEvent(
    int Event,
    FarDialogEvent *Param
)
{
	BOOL bResult = FALSE;

	if (Load() && Exports[iProcessDialogEvent] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSDIALOGEVENT;
		es.bDefaultResult = FALSE;
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessDialogEvent)(Event, Param), es);
		bResult = es.bResult;
	}

	return bResult;
}

int PluginA::GetVirtualFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelItem,
    size_t *pItemsNumber,
    const wchar_t *Path
)
{
	BOOL bResult = FALSE;

	if (Exports[iGetVirtualFindData] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETVIRTUALFINDDATA;
		es.bDefaultResult = FALSE;
		pVFDPanelItemA = nullptr;
		size_t Size=StrLength(Path)+1;
		LPSTR PathA=new char[Size];
		UnicodeToOEM(Path,PathA,Size);
		int ItemsNumber = 0;
		EXECUTE_FUNCTION_EX(FUNCTION(iGetVirtualFindData)(hPlugin, &pVFDPanelItemA, &ItemsNumber, PathA), es);
		*pItemsNumber = ItemsNumber;
		bResult = es.bResult;
		delete[] PathA;

		if (bResult && *pItemsNumber)
		{
			ConvertPanelItemA(pVFDPanelItemA, pPanelItem, *pItemsNumber);
		}
	}

	return bResult;
}


void PluginA::FreeVirtualFindData(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber
)
{
	FreeUnicodePanelItem(PanelItem, ItemsNumber);

	if (Exports[iFreeVirtualFindData] && !ProcessException && pVFDPanelItemA)
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEVIRTUALFINDDATA;
		EXECUTE_FUNCTION(FUNCTION(iFreeVirtualFindData)(hPlugin, pVFDPanelItemA, static_cast<int>(ItemsNumber)), es);
		pVFDPanelItemA = nullptr;
	}
}



int PluginA::GetFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    bool Move,
    const wchar_t **DestPath,
    int OpMode
)
{
	int nResult = -1;

	if (Exports[iGetFiles] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETFILES;
		es.nDefaultResult = -1;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		char DestA[oldfar::NM];
		UnicodeToOEM(*DestPath,DestA,sizeof(DestA));
		EXECUTE_FUNCTION_EX(FUNCTION(iGetFiles)(hPlugin, PanelItemA, static_cast<int>(ItemsNumber), Move, DestA, OpMode), es);
		static wchar_t DestW[oldfar::NM];
		OEMToUnicode(DestA,DestW,ARRAYSIZE(DestW));
		*DestPath=DestW;
		FreePanelItemA(PanelItemA,ItemsNumber);
		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginA::PutFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    bool Move,
    int OpMode
)
{
	int nResult = -1;

	if (Exports[iPutFiles] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PUTFILES;
		es.nDefaultResult = -1;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		EXECUTE_FUNCTION_EX(FUNCTION(iPutFiles)(hPlugin, PanelItemA, static_cast<int>(ItemsNumber), Move, OpMode), es);
		FreePanelItemA(PanelItemA,ItemsNumber);
		nResult = (int)es.nResult;
	}

	return nResult;
}

int PluginA::DeleteFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (Exports[iDeleteFiles] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_DELETEFILES;
		es.bDefaultResult = FALSE;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		EXECUTE_FUNCTION_EX(FUNCTION(iDeleteFiles)(hPlugin, PanelItemA, static_cast<int>(ItemsNumber), OpMode), es);
		FreePanelItemA(PanelItemA,ItemsNumber);
		bResult = (int)es.bResult;
	}

	return bResult;
}


int PluginA::MakeDirectory(
    HANDLE hPlugin,
    const wchar_t **Name,
    int OpMode
)
{
	int nResult = -1;

	if (Exports[iMakeDirectory] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_MAKEDIRECTORY;
		es.nDefaultResult = -1;
		char NameA[oldfar::NM];
		UnicodeToOEM(*Name,NameA,sizeof(NameA));
		EXECUTE_FUNCTION_EX(FUNCTION(iMakeDirectory)(hPlugin, NameA, OpMode), es);
		static wchar_t NameW[oldfar::NM];
		OEMToUnicode(NameA,NameW,ARRAYSIZE(NameW));
		*Name=NameW;
		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginA::ProcessHostFile(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (Exports[iProcessHostFile] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSHOSTFILE;
		es.bDefaultResult = FALSE;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessHostFile)(hPlugin, PanelItemA, static_cast<int>(ItemsNumber), OpMode), es);
		FreePanelItemA(PanelItemA,ItemsNumber);
		bResult = es.bResult;
	}

	return bResult;
}


int PluginA::ProcessPanelEvent(
    HANDLE hPlugin,
    int Event,
    PVOID Param
)
{
	BOOL bResult = FALSE;

	if (Exports[iProcessPanelEvent] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSPANELEVENT;
		es.bDefaultResult = FALSE;
		PVOID ParamA = Param;

		if (Param && (Event == FE_COMMAND || Event == FE_CHANGEVIEWMODE))
			ParamA = (PVOID)UnicodeToAnsi((const wchar_t *)Param);

		EXECUTE_FUNCTION_EX(FUNCTION(iProcessPanelEvent)(hPlugin, Event, ParamA), es);

		if (ParamA && (Event == FE_COMMAND || Event == FE_CHANGEVIEWMODE))
			xf_free(ParamA);

		bResult = es.bResult;
	}

	return bResult;
}


int PluginA::Compare(
    HANDLE hPlugin,
    const PluginPanelItem *Item1,
    const PluginPanelItem *Item2,
    DWORD Mode
)
{
	int nResult = -2;

	if (Exports[iCompare] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_COMPARE;
		es.nDefaultResult = -2;
		oldfar::PluginPanelItem *Item1A = nullptr;
		oldfar::PluginPanelItem *Item2A = nullptr;
		ConvertPanelItemsArrayToAnsi(Item1,Item1A,1);
		ConvertPanelItemsArrayToAnsi(Item2,Item2A,1);
		EXECUTE_FUNCTION_EX(FUNCTION(iCompare)(hPlugin, Item1A, Item2A, Mode), es);
		FreePanelItemA(Item1A,1);
		FreePanelItemA(Item2A,1);
		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginA::GetFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelItem,
    size_t *pItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (Exports[iGetFindData] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETFINDDATA;
		es.bDefaultResult = FALSE;
		pFDPanelItemA = nullptr;
		int ItemsNumber = 0;
		EXECUTE_FUNCTION_EX(FUNCTION(iGetFindData)(hPlugin, &pFDPanelItemA, &ItemsNumber, OpMode), es);
		bResult = es.bResult;
		*pItemsNumber = ItemsNumber;

		if (bResult && *pItemsNumber)
		{
			ConvertPanelItemA(pFDPanelItemA, pPanelItem, *pItemsNumber);
		}
	}

	return bResult;
}


void PluginA::FreeFindData(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    size_t ItemsNumber
)
{
	FreeUnicodePanelItem(PanelItem, ItemsNumber);

	if (Exports[iFreeFindData] && !ProcessException && pFDPanelItemA)
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEFINDDATA;
		EXECUTE_FUNCTION(FUNCTION(iFreeFindData)(hPlugin, pFDPanelItemA, static_cast<int>(ItemsNumber)), es);
		pFDPanelItemA = nullptr;
	}
}

int PluginA::ProcessKey(HANDLE hPlugin,const INPUT_RECORD *Rec, bool Pred)
{
	BOOL bResult = FALSE;

	if (Exports[iProcessPanelInput] && !ProcessException)
	{
		int VirtKey;
		int dwControlState;
		//BUGBUG: здесь можно проще.
		TranslateKeyToVK(InputRecordToKey(Rec),VirtKey,dwControlState);

		if (dwControlState&PKF_RALT)
			dwControlState = (dwControlState & (~PKF_RALT)) | PKF_ALT;
		if (dwControlState&PKF_RCONTROL)
			dwControlState = (dwControlState & (~PKF_RCONTROL)) | PKF_CONTROL;

		ExecuteStruct es;
		es.id = EXCEPT_PROCESSPANELINPUT;
		es.bDefaultResult = TRUE; // do not pass this key to far on exception
		EXECUTE_FUNCTION_EX(FUNCTION(iProcessPanelInput)(hPlugin, VirtKey|(Pred?PKF_PREPROCESS:0), dwControlState), es);
		bResult = es.bResult;
	}

	return bResult;
}


void PluginA::ClosePanel(
    HANDLE hPlugin
)
{
	if (Exports[iClosePanel] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_CLOSEPANEL;
		EXECUTE_FUNCTION(FUNCTION(iClosePanel)(hPlugin), es);
	}

	FreeOpenPanelInfo();
	//	m_pManager->m_pCurrentPlugin = (Plugin*)-1;
}


int PluginA::SetDirectory(
    HANDLE hPlugin,
    const wchar_t *Dir,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (Exports[iSetDirectory] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_SETDIRECTORY;
		es.bDefaultResult = FALSE;
		char *DirA = UnicodeToAnsi(Dir);
		EXECUTE_FUNCTION_EX(FUNCTION(iSetDirectory)(hPlugin, DirA, OpMode), es);

		if (DirA) xf_free(DirA);

		bResult = es.bResult;
	}

	return bResult;
}

void PluginA::FreeOpenPanelInfo()
{
	if (OPI.CurDir)
		xf_free((void *)OPI.CurDir);

	if (OPI.HostFile)
		xf_free((void *)OPI.HostFile);

	if (OPI.Format)
		xf_free((void *)OPI.Format);

	if (OPI.PanelTitle)
		xf_free((void *)OPI.PanelTitle);

	if (OPI.InfoLines && OPI.InfoLinesNumber)
	{
		FreeUnicodeInfoPanelLines((InfoPanelLine*)OPI.InfoLines,OPI.InfoLinesNumber);
	}

	if (OPI.DescrFiles)
	{
		FreeArrayUnicode((wchar_t**)OPI.DescrFiles);
	}

	if (OPI.PanelModesArray)
	{
		FreeUnicodePanelModes((PanelMode*)OPI.PanelModesArray, OPI.PanelModesNumber);
	}

	if (OPI.KeyBar)
	{
		FreeUnicodeKeyBarTitles((KeyBarTitles*)OPI.KeyBar);
		xf_free((void *)OPI.KeyBar);
	}

	if (OPI.ShortcutData)
		xf_free((void *)OPI.ShortcutData);

	ClearStruct(OPI);
}

void PluginA::ConvertOpenPanelInfo(oldfar::OpenPanelInfo &Src, OpenPanelInfo *Dest)
{
	FreeOpenPanelInfo();
	OPI.StructSize = sizeof(OPI);
	OPI.Flags = 0;
	if (!(Src.Flags&oldfar::OPIF_USEFILTER)) OPI.Flags|=OPIF_DISABLEFILTER;
	if (!(Src.Flags&oldfar::OPIF_USESORTGROUPS)) OPI.Flags|=OPIF_DISABLESORTGROUPS;
	if (!(Src.Flags&oldfar::OPIF_USEHIGHLIGHTING)) OPI.Flags|=OPIF_DISABLEHIGHLIGHTING;
	if (Src.Flags&oldfar::OPIF_ADDDOTS) OPI.Flags|=OPIF_ADDDOTS;
	if (Src.Flags&oldfar::OPIF_RAWSELECTION) OPI.Flags|=OPIF_RAWSELECTION;
	if (Src.Flags&oldfar::OPIF_REALNAMES) OPI.Flags|=OPIF_REALNAMES;
	if (Src.Flags&oldfar::OPIF_SHOWNAMESONLY) OPI.Flags|=OPIF_SHOWNAMESONLY;
	if (Src.Flags&oldfar::OPIF_SHOWRIGHTALIGNNAMES) OPI.Flags|=OPIF_SHOWRIGHTALIGNNAMES;
	if (Src.Flags&oldfar::OPIF_SHOWPRESERVECASE) OPI.Flags|=OPIF_SHOWPRESERVECASE;
	if (Src.Flags&oldfar::OPIF_COMPAREFATTIME) OPI.Flags|=OPIF_COMPAREFATTIME;
	if (Src.Flags&oldfar::OPIF_EXTERNALGET) OPI.Flags|=OPIF_EXTERNALGET;
	if (Src.Flags&oldfar::OPIF_EXTERNALPUT) OPI.Flags|=OPIF_EXTERNALPUT;
	if (Src.Flags&oldfar::OPIF_EXTERNALDELETE) OPI.Flags|=OPIF_EXTERNALDELETE;
	if (Src.Flags&oldfar::OPIF_EXTERNALMKDIR) OPI.Flags|=OPIF_EXTERNALMKDIR;
	if (Src.Flags&oldfar::OPIF_USEATTRHIGHLIGHTING) OPI.Flags|=OPIF_USEATTRHIGHLIGHTING;

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
		ConvertInfoPanelLinesA(Src.InfoLines, (InfoPanelLine**)&OPI.InfoLines, Src.InfoLinesNumber);
		OPI.InfoLinesNumber = Src.InfoLinesNumber;
	}

	if (Src.DescrFiles && Src.DescrFilesNumber)
	{
		OPI.DescrFiles = ArrayAnsiToUnicode((char**)Src.DescrFiles, Src.DescrFilesNumber);
		OPI.DescrFilesNumber = Src.DescrFilesNumber;
	}

	if (Src.PanelModesArray && Src.PanelModesNumber)
	{
		ConvertPanelModesA(Src.PanelModesArray, (PanelMode**)&OPI.PanelModesArray, Src.PanelModesNumber);
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
		OPI.KeyBar=(KeyBarTitles*) xf_malloc(sizeof(KeyBarTitles));
		ConvertKeyBarTitlesA(Src.KeyBar, (KeyBarTitles*)OPI.KeyBar, Src.StructSize>=(int)sizeof(oldfar::OpenPanelInfo));
	}

	if (Src.ShortcutData)
		OPI.ShortcutData = AnsiToUnicode(Src.ShortcutData);

	*Dest=OPI;
}

void PluginA::GetOpenPanelInfo(
    HANDLE hPlugin,
    OpenPanelInfo *pInfo
)
{
//	m_pManager->m_pCurrentPlugin = this;
	pInfo->StructSize = sizeof(OpenPanelInfo);

	if (Exports[iGetOpenPanelInfo] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETOPENPANELINFO;
		oldfar::OpenPanelInfo InfoA={};
		EXECUTE_FUNCTION(FUNCTION(iGetOpenPanelInfo)(hPlugin, &InfoA), es);
		ConvertOpenPanelInfo(InfoA,pInfo);
	}
}


int PluginA::Configure(const GUID& Guid)
{
	BOOL bResult = FALSE;

	if (Load() && Exports[iConfigure] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_CONFIGURE;
		es.bDefaultResult = FALSE;
		EXECUTE_FUNCTION_EX(FUNCTION(iConfigure)(Guid.Data1), es);
		bResult = es.bResult;
	}

	return bResult;
}

void PluginA::FreePluginInfo()
{
	if (PI.DiskMenu.Count)
	{
		for (size_t i=0; i<PI.DiskMenu.Count; i++)
			xf_free((void *)PI.DiskMenu.Strings[i]);

		xf_free((void *)PI.DiskMenu.Guids);
		xf_free((void *)PI.DiskMenu.Strings);
	}

	if (PI.PluginMenu.Count)
	{
		for (size_t i=0; i<PI.PluginMenu.Count; i++)
			xf_free((void *)PI.PluginMenu.Strings[i]);

		xf_free((void *)PI.PluginMenu.Guids);
		xf_free((void *)PI.PluginMenu.Strings);
	}

	if (PI.PluginConfig.Count)
	{
		for (size_t i=0; i<PI.PluginConfig.Count; i++)
			xf_free((void *)PI.PluginConfig.Strings[i]);

		xf_free((void *)PI.PluginConfig.Guids);
		xf_free((void *)PI.PluginConfig.Strings);
	}

	if (PI.CommandPrefix)
		xf_free((void *)PI.CommandPrefix);

	ClearStruct(PI);
}

void PluginA::ConvertPluginInfo(oldfar::PluginInfo &Src, PluginInfo *Dest)
{
	FreePluginInfo();
	PI.StructSize = sizeof(PI);
	PI.Flags=PF_NONE;
	if(Src.Flags&oldfar::PF_PRELOAD)
		PI.Flags|=PF_PRELOAD;
	if(Src.Flags&oldfar::PF_DISABLEPANELS)
		PI.Flags|=PF_DISABLEPANELS;
	if(Src.Flags&oldfar::PF_EDITOR)
		PI.Flags|=PF_EDITOR;
	if(Src.Flags&oldfar::PF_VIEWER)
		PI.Flags|=PF_VIEWER;
	if(Src.Flags&oldfar::PF_DIALOG)
		PI.Flags|=PF_DIALOG;
	if(Src.Flags&oldfar::PF_FULLCMDLINE)
		PI.Flags|=PF_FULLCMDLINE;

	if (Src.DiskMenuStringsNumber)
	{
		wchar_t **p = (wchar_t **) xf_malloc(Src.DiskMenuStringsNumber*sizeof(wchar_t*));
		GUID* guid=(GUID*) xf_malloc(Src.DiskMenuStringsNumber*sizeof(GUID));
		memset(guid,0,Src.DiskMenuStringsNumber*sizeof(GUID));

		for (int i=0; i<Src.DiskMenuStringsNumber; i++)
		{
			p[i] = AnsiToUnicode(Src.DiskMenuStrings[i]);
			guid[i].Data1=i;
		}

		PI.DiskMenu.Guids = guid;
		PI.DiskMenu.Strings = p;
		PI.DiskMenu.Count = Src.DiskMenuStringsNumber;
	}

	if (Src.PluginMenuStringsNumber)
	{
		wchar_t **p = (wchar_t **) xf_malloc(Src.PluginMenuStringsNumber*sizeof(wchar_t*));
		GUID* guid=(GUID*) xf_malloc(Src.PluginMenuStringsNumber*sizeof(GUID));
		memset(guid,0,Src.PluginMenuStringsNumber*sizeof(GUID));

		for (int i=0; i<Src.PluginMenuStringsNumber; i++)
		{
			p[i] = AnsiToUnicode(Src.PluginMenuStrings[i]);
			guid[i].Data1=i;
		}

		PI.PluginMenu.Guids = guid;
		PI.PluginMenu.Strings = p;
		PI.PluginMenu.Count = Src.PluginMenuStringsNumber;
	}

	if (Src.PluginConfigStringsNumber)
	{
		wchar_t **p = (wchar_t **) xf_malloc(Src.PluginConfigStringsNumber*sizeof(wchar_t*));
		GUID* guid=(GUID*) xf_malloc(Src.PluginConfigStringsNumber*sizeof(GUID));
		memset(guid,0,Src.PluginConfigStringsNumber*sizeof(GUID));

		for (int i=0; i<Src.PluginConfigStringsNumber; i++)
		{
			p[i] = AnsiToUnicode(Src.PluginConfigStrings[i]);
			guid[i].Data1=i;
		}

		PI.PluginConfig.Guids = guid;
		PI.PluginConfig.Strings = p;
		PI.PluginConfig.Count = Src.PluginConfigStringsNumber;
	}

	if (Src.CommandPrefix)
		PI.CommandPrefix = AnsiToUnicode(Src.CommandPrefix);

	*Dest=PI;
}

bool PluginA::GetPluginInfo(PluginInfo *pi)
{
	ClearStruct(*pi);

	if (Exports[iGetPluginInfo] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETPLUGININFO;
		oldfar::PluginInfo InfoA={sizeof(InfoA)};
		EXECUTE_FUNCTION(FUNCTION(iGetPluginInfo)(&InfoA), es);

		if (!bPendingRemove)
		{
			ConvertPluginInfo(InfoA, pi);
			return true;
		}
	}

	return false;
}

void PluginA::ExitFAR(const ExitInfo *Info)
{
	if (Exports[iExitFAR] && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_EXITFAR;
		// ExitInfo ignored for ansi plugins
		EXECUTE_FUNCTION(FUNCTION(iExitFAR)(), es);
	}
}

};

#endif // NO_WRAPPER
