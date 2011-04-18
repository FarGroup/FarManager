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

#include "plugins.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "codepage.hpp"
#include "flink.hpp"
#include "scantree.hpp"
#include "chgprior.hpp"
#include "constitle.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "rdrwdsk.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "udlist.hpp"
#include "farexcpt.hpp"
#include "fileedit.hpp"
#include "RefreshFrameManager.hpp"
#include "plclass.hpp"
#include "PluginA.hpp"
#include "localOEM.hpp"
#include "plugapi.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "interf.hpp"
#include "clipboard.hpp"
#include "xlat.hpp"
#include "fileowner.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "processname.hpp"
#include "mix.hpp"
#include "lasterror.hpp"
#include "colormix.hpp"
#include "configdb.hpp"

static const wchar_t wszReg_OpenPanel[]=L"OpenPlugin"; // !OpenPlugin!
static const wchar_t wszReg_OpenFilePlugin[]=L"OpenFilePlugin";
static const wchar_t wszReg_SetFindList[]=L"SetFindList";
static const wchar_t wszReg_ProcessEditorInput[]=L"ProcessEditorInput";
static const wchar_t wszReg_ProcessEditorEvent[]=L"ProcessEditorEvent";
static const wchar_t wszReg_ProcessViewerEvent[]=L"ProcessViewerEvent";
static const wchar_t wszReg_ProcessDialogEvent[]=L"ProcessDialogEvent";
static const wchar_t wszReg_Configure[]=L"Configure";

static const char NFMP_OpenPanel[]="OpenPlugin"; // !OpenPlugin!
static const char NFMP_OpenFilePlugin[]="OpenFilePlugin";
static const char NFMP_SetFindList[]="SetFindList";
static const char NFMP_ProcessEditorInput[]="ProcessEditorInput";
static const char NFMP_ProcessEditorEvent[]="ProcessEditorEvent";
static const char NFMP_ProcessViewerEvent[]="ProcessViewerEvent";
static const char NFMP_ProcessDialogEvent[]="ProcessDialogEvent";
static const char NFMP_SetStartupInfo[]="SetStartupInfo";
static const char NFMP_ClosePanel[]="ClosePlugin"; // !ClosePlugin!
static const char NFMP_GetPluginInfo[]="GetPluginInfo";
static const char NFMP_GetOpenPanelInfo[]="GetOpenPluginInfo";
static const char NFMP_GetFindData[]="GetFindData";
static const char NFMP_FreeFindData[]="FreeFindData";
static const char NFMP_GetVirtualFindData[]="GetVirtualFindData";
static const char NFMP_FreeVirtualFindData[]="FreeVirtualFindData";
static const char NFMP_SetDirectory[]="SetDirectory";
static const char NFMP_GetFiles[]="GetFiles";
static const char NFMP_PutFiles[]="PutFiles";
static const char NFMP_DeleteFiles[]="DeleteFiles";
static const char NFMP_MakeDirectory[]="MakeDirectory";
static const char NFMP_ProcessHostFile[]="ProcessHostFile";
static const char NFMP_Configure[]="Configure";
static const char NFMP_ExitFAR[]="ExitFAR";
static const char NFMP_ProcessKey[]="ProcessKey";
static const char NFMP_ProcessEvent[]="ProcessEvent";
static const char NFMP_Compare[]="Compare";
static const char NFMP_GetMinFarVersion[]="GetMinFarVersion";

#define UnicodeToOEM(src,dst,lendst)    WideCharToMultiByte(CP_OEMCP,0,(src),-1,(dst),(int)(lendst),nullptr,nullptr)
#define OEMToUnicode(src,dst,lendst)    MultiByteToWideChar(CP_OEMCP,0,(src),-1,(dst),(int)(lendst))

void ConvertKeyBarTitlesA(const oldfar::KeyBarTitles *kbtA, KeyBarTitles *kbtW, bool FullStruct=true);

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

wchar_t **ArrayAnsiToUnicode(char ** lpaszAnsiString, int iCount)
{
	wchar_t** lpaResult = nullptr;

	if (lpaszAnsiString)
	{
		lpaResult = (wchar_t**) xf_malloc((iCount+1)*sizeof(wchar_t*));

		if (lpaResult)
		{
			for (int i=0; i<iCount; i++)
			{
				lpaResult[i]=(lpaszAnsiString[i])?AnsiToUnicode(lpaszAnsiString[i]):nullptr;
			}

			lpaResult[iCount] = (wchar_t*)(INT_PTR) 1; //Array end mark
		}
	}

	return lpaResult;
}

void FreeArrayUnicode(wchar_t ** lpawszUnicodeString)
{
	if (lpawszUnicodeString)
	{
		for (int i=0; (INT_PTR)lpawszUnicodeString[i] != 1; i++) //Until end mark
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


void ConvertInfoPanelLinesA(const oldfar::InfoPanelLine *iplA, InfoPanelLine **piplW, int iCount)
{
	if (iplA && piplW && (iCount>0))
	{
		InfoPanelLine *iplW = (InfoPanelLine *) xf_malloc(iCount*sizeof(InfoPanelLine));

		if (iplW)
		{
			for (int i=0; i<iCount; i++)
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

void ConvertPanelModesA(const oldfar::PanelMode *pnmA, PanelMode **ppnmW, int iCount)
{
	if (pnmA && ppnmW && (iCount>0))
	{
		PanelMode *pnmW = new PanelMode[iCount]();
		if (pnmW)
		{
			for (int i=0; i<iCount; i++)
			{
				int iColumnCount = 0;

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

				pnmW[i].StructSize      = sizeof(PanelMode);
				pnmW[i].ColumnTypes		=	(pnmA[i].ColumnTypes)?AnsiToUnicode(pnmA[i].ColumnTypes):nullptr;
				pnmW[i].ColumnWidths	=	(pnmA[i].ColumnWidths)?AnsiToUnicode(pnmA[i].ColumnWidths):nullptr;
				pnmW[i].ColumnTitles	= (pnmA[i].ColumnTitles && (iColumnCount>0))?ArrayAnsiToUnicode(pnmA[i].ColumnTitles,iColumnCount):nullptr;
				pnmW[i].StatusColumnTypes		=	(pnmA[i].StatusColumnTypes)?AnsiToUnicode(pnmA[i].StatusColumnTypes):nullptr;
				pnmW[i].StatusColumnWidths	=	(pnmA[i].StatusColumnWidths)?AnsiToUnicode(pnmA[i].StatusColumnWidths):nullptr;
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

void ConvertPanelItemA(const oldfar::PluginPanelItem *PanelItemA, PluginPanelItem **PanelItemW, int ItemsNumber)
{
	*PanelItemW = (PluginPanelItem *)xf_malloc(ItemsNumber*sizeof(PluginPanelItem));
	memset(*PanelItemW,0,ItemsNumber*sizeof(PluginPanelItem));

	for (int i=0; i<ItemsNumber; i++)
	{
		(*PanelItemW)[i].Flags = PanelItemA[i].Flags;
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

		(*PanelItemW)[i].UserData = PanelItemA[i].UserData;
		(*PanelItemW)[i].CRC32 = PanelItemA[i].CRC32;
		(*PanelItemW)[i].FileAttributes = PanelItemA[i].FindData.dwFileAttributes;
		(*PanelItemW)[i].CreationTime = PanelItemA[i].FindData.ftCreationTime;
		(*PanelItemW)[i].LastAccessTime = PanelItemA[i].FindData.ftLastAccessTime;
		(*PanelItemW)[i].LastWriteTime = PanelItemA[i].FindData.ftLastWriteTime;
		(*PanelItemW)[i].FileSize = (unsigned __int64)PanelItemA[i].FindData.nFileSizeLow + (((unsigned __int64)PanelItemA[i].FindData.nFileSizeHigh)<<32);
		(*PanelItemW)[i].PackSize = (unsigned __int64)PanelItemA[i].PackSize + (((unsigned __int64)PanelItemA[i].PackSizeHigh)<<32);
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

	if(PanelItem.Flags&PPIF_USERDATA)
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

	if (PanelItem.UserData&&PanelItem.Flags&PPIF_USERDATA)
	{
		DWORD Size=*(DWORD *)PanelItem.UserData;
		PanelItemA.UserData=(DWORD_PTR)xf_malloc(Size);
		memcpy((void *)PanelItemA.UserData,(void *)PanelItem.UserData,Size);
	}
	else
		PanelItemA.UserData = PanelItem.UserData;

	PanelItemA.CRC32 = PanelItem.CRC32;
	PanelItemA.FindData.dwFileAttributes = PanelItem.FileAttributes;
	PanelItemA.FindData.ftCreationTime = PanelItem.CreationTime;
	PanelItemA.FindData.ftLastAccessTime = PanelItem.LastAccessTime;
	PanelItemA.FindData.ftLastWriteTime = PanelItem.LastWriteTime;
	PanelItemA.FindData.nFileSizeLow = (DWORD)PanelItem.FileSize;
	PanelItemA.FindData.nFileSizeHigh = (DWORD)(PanelItem.FileSize>>32);
	PanelItemA.PackSize = (DWORD)PanelItem.PackSize;
	PanelItemA.PackSizeHigh = (DWORD)(PanelItem.PackSize>>32);
	UnicodeToOEM(PanelItem.FileName,PanelItemA.FindData.cFileName,sizeof(PanelItemA.FindData.cFileName));
	UnicodeToOEM(PanelItem.AlternateFileName,PanelItemA.FindData.cAlternateFileName,sizeof(PanelItemA.FindData.cAlternateFileName));
}

void ConvertPanelItemsArrayToAnsi(const PluginPanelItem *PanelItemW, oldfar::PluginPanelItem *&PanelItemA, int ItemsNumber)
{
	PanelItemA = (oldfar::PluginPanelItem *)xf_malloc(ItemsNumber*sizeof(oldfar::PluginPanelItem));
	memset(PanelItemA,0,ItemsNumber*sizeof(oldfar::PluginPanelItem));

	for (int i=0; i<ItemsNumber; i++)
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

void FreePanelItemA(oldfar::PluginPanelItem *PanelItem, int ItemsNumber, bool bFreeArray=true)
{
	for (int i=0; i<ItemsNumber; i++)
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
	string strPath(Path), strRoot;
	GetPathRoot(strPath,strRoot);
	strRoot.GetCharString(Root,strRoot.GetLength()+1);
}

int WINAPI CopyToClipboardA(const char *Data)
{
	wchar_t *p = Data?AnsiToUnicode(Data):nullptr;
	int ret = CopyToClipboard(p);

	if (p) xf_free(p);

	return ret;
}

char* WINAPI PasteFromClipboardA()
{
	wchar_t *p = PasteFromClipboard();

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

	int ret = static_cast<int>(ProcessName(strP1,p,size,newFlags));

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
	int ret=KeyToText(OldKeyToKey(Key),strKT);

	if (ret)
		strKT.GetCharString(KeyText,Size>0?Size+1:32);

	return ret;
}

int WINAPI InputRecordToKeyA(const INPUT_RECORD *r)
{
	return KeyToOldKey(InputRecordToKey(r));
}

char* WINAPI FarMkTempA(char *Dest, const char *Prefix)
{
	string strP(Prefix);
	wchar_t D[oldfar::NM] = {0};
	FarMkTemp(D,ARRAYSIZE(D),strP);
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

	return FarMkLink(strS, strD, Type, Flags);
}

int WINAPI GetNumberOfLinksA(const char *Name)
{
	string n(Name);
	return GetNumberOfLinks(n);
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
	if (Src && *Src)
	{
		string strSrc(Src);
		string strDest;
		DWORD Size=GetReparsePointInfo(strSrc,strDest,nullptr);

		if (DestSize && Dest)
			strDest.GetCharString(Dest,DestSize);

		return Size;
	}

	return 0;
}

struct FAR_SEARCH_A_CALLBACK_PARAM
{
	oldfar::FRSUSERFUNC Func;
	void *Param;
};

static int WINAPI FarRecursiveSearchA_Callback(const PluginPanelItem *FData,const wchar_t *FullName,void *param)
{
	FAR_SEARCH_A_CALLBACK_PARAM* pCallbackParam = static_cast<FAR_SEARCH_A_CALLBACK_PARAM*>(param);
	WIN32_FIND_DATAA FindData={0};
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

	FarRecursiveSearch(static_cast<const wchar_t *>(strInitDir),static_cast<const wchar_t *>(strMask),FarRecursiveSearchA_Callback,newFlags,static_cast<void *>(&CallbackParam));
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
	return FarViewer(strFN,strT,X1,Y1,X2,Y2,Flags,CP_AUTODETECT);
}

int WINAPI FarEditorA(const char *FileName,const char *Title,int X1,int Y1,int X2,int Y2,DWORD Flags,int StartLine,int StartChar)
{
	string strFN(FileName), strT(Title);
	return FarEditor(strFN,strT,X1,Y1,X2,Y2,Flags,StartLine,StartChar,CP_AUTODETECT);
}

int WINAPI FarCmpNameA(const char *pattern,const char *str,int skippath)
{
	return ProcessNameA(pattern, const_cast<char*>(str), oldfar::PN_CMPNAME|(skippath?oldfar::PN_SKIPPATH:0));
}

void WINAPI FarTextA(int X,int Y,int Color,const char *Str)
{
	if (!Str) return FarText(X,Y,Color,nullptr);

	string strS(Str);
	return FarText(X,Y,Color,strS);
}

BOOL WINAPI FarShowHelpA(const char *ModuleName,const char *HelpTopic,DWORD Flags)
{
	string strMN(ModuleName), strHT(HelpTopic);
	return FarShowHelp(strMN,(HelpTopic?strHT.CPtr():nullptr),Flags);
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

	int ret = FarInputBox(-1,(Title?strT.CPtr():nullptr),(Prompt?strP.CPtr():nullptr),(HistoryName?strHN.CPtr():nullptr),(SrcText?strST.CPtr():nullptr),D,DestLength,(HelpTopic?strHT.CPtr():nullptr),NewFlags);
	strD.ReleaseBuffer();

	if (ret && DestText)
		strD.GetCharString(DestText,DestLength+1);

	return ret;
}

int WINAPI FarMessageFnA(INT_PTR PluginNumber,DWORD Flags,const char *HelpTopic,const char * const *Items,int ItemsNumber,int ButtonsNumber)
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

	DWORD NewFlags=0;
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
	if (Flags&oldfar::FMSG_MB_OK)
		NewFlags|=FMSG_MB_OK;
	if (Flags&oldfar::FMSG_MB_OKCANCEL)
		NewFlags|=FMSG_MB_OKCANCEL;
	if (Flags&oldfar::FMSG_MB_ABORTRETRYIGNORE)
		NewFlags|=FMSG_MB_ABORTRETRYIGNORE;
	if (Flags&oldfar::FMSG_MB_YESNO)
		NewFlags|=FMSG_MB_YESNO;
	if (Flags&oldfar::FMSG_MB_YESNOCANCEL)
		NewFlags|=FMSG_MB_YESNOCANCEL;
	if (Flags&oldfar::FMSG_MB_RETRYCANCEL)
		NewFlags|=FMSG_MB_RETRYCANCEL;

	int ret = FarMessageFn(PluginNumber,NewFlags,(HelpTopic?strHT.CPtr():nullptr),p,ItemsNumber,ButtonsNumber);

	for (int i=0; i<c; i++)
		xf_free(p[i]);

	xf_free(p);
	return ret;
}

const char * WINAPI FarGetMsgFnA(INT_PTR PluginHandle,int MsgId)
{
	//BUGBUG, надо проверять, что PluginHandle - плагин
	PluginA *pPlugin = (PluginA*)PluginHandle;
	string strPath = pPlugin->GetModuleName();
	CutToSlash(strPath);

	if (pPlugin->InitLang(strPath))
		return pPlugin->GetMsgA(MsgId);

	return "";
}

int WINAPI FarMenuFnA(INT_PTR PluginNumber,int X,int Y,int MaxHeight,DWORD Flags,const char *Title,const char *Bottom,const char *HelpTopic,const int *BreakKeys,int *BreakCode,const oldfar::FarMenuItem *Item,int ItemsNumber)
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
			mi[i].AccelKey = OldKeyToKey(p[i].AccelKey);
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

			mi[i].AccelKey = 0;
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

	int ret = FarMenuFn(PluginNumber,X,Y,MaxHeight,NewFlags,wszT,wszB,wszHT,NewBreakKeys,BreakCode,mi,ItemsNumber);

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

INT_PTR WINAPI CurrentDlgProc(HANDLE hDlg, int Msg, int Param1, INT_PTR Param2)
{
	INT_PTR Ret=0;
	PDialogData Data=FindCurrentDialogData(hDlg);

	if (Data && Data->DlgProc)
		Ret=Data->DlgProc(Data->hDlg,Msg,Param1,Param2);

	return Ret;
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

PCHAR_INFO GetAnsiVBufPtr(PCHAR_INFO VBuf, size_t Size)
{
	PCHAR_INFO VBufA=nullptr;
	if (VBuf)
	{
		VBufA=*reinterpret_cast<PCHAR_INFO*>(&VBuf[Size]);
	}
	return VBufA;
}

void SetAnsiVBufPtr(PCHAR_INFO VBuf, PCHAR_INFO VBufA, size_t Size)
{
	if (VBuf)
	{
		*reinterpret_cast<PCHAR_INFO*>(&VBuf[Size])=VBufA;
	}
}

void AnsiVBufToUnicode(PCHAR_INFO VBufA, PCHAR_INFO VBuf, size_t Size,bool NoCvt)
{
	if (VBuf && VBufA)
	{
		for (size_t i=0; i<Size; i++)
		{
			if (NoCvt)
			{
				VBuf[i].Char.UnicodeChar=VBufA[i].Char.UnicodeChar;
			}
			else
			{
				AnsiToUnicodeBin(&VBufA[i].Char.AsciiChar,&VBuf[i].Char.UnicodeChar,1);
			}

			VBuf[i].Attributes = VBufA[i].Attributes;
		}
	}
}

PCHAR_INFO AnsiVBufToUnicode(oldfar::FarDialogItem &diA)
{
	PCHAR_INFO VBuf = nullptr;

	if (diA.VBuf)
	{
		size_t Size = GetAnsiVBufSize(diA);
		// +sizeof(PCHAR_INFO) потому что там храним поинтер на анси vbuf.
		VBuf = static_cast<PCHAR_INFO>(xf_malloc(Size*sizeof(CHAR_INFO)+sizeof(PCHAR_INFO)));

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
	if (diA.Focus) di.Flags|=DIF_FOCUS;

	if (diA.Flags)
	{
		if (diA.Flags&oldfar::DIF_SETCOLOR)
			di.Flags|=DIF_SETCOLOR|(diA.Flags&oldfar::DIF_COLORMASK);

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
	memset(&di,0,sizeof(FarDialogItem));
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
	if ((diA.Type==oldfar::DI_EDIT || diA.Type==oldfar::DI_FIXEDIT) &&
	        (diA.Flags&oldfar::DIF_HISTORY ||diA.Flags&oldfar::DIF_MASKEDIT) &&
	        diA.History)
		xf_free((void*)diA.History);

	if ((diA.Type==oldfar::DI_EDIT || diA.Type==oldfar::DI_COMBOBOX) &&
	        diA.Flags&oldfar::DIF_VAREDIT && diA.Ptr.PtrData)
		xf_free(diA.Ptr.PtrData);

	memset(&diA,0,sizeof(oldfar::FarDialogItem));
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
	diA.Flags=0;

	if (di.Flags)
	{
		if (di.Flags&DIF_SETCOLOR)
			diA.Flags|=oldfar::DIF_SETCOLOR|(di.Flags&DIF_COLORMASK);

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
		memset(OneDialogItem,0,sizeof(oldfar::FarDialogItem));
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
			diA->ListPos=static_cast<int>(FarSendDlgMessage(hDlg,DM_LISTGETCURPOS,ItemNumber,0));
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

TStack<FarDialogEvent>OriginalEvents;

class StackHandler
{
public:
	StackHandler(FarDialogEvent& e){OriginalEvents.Push(e);}
	~StackHandler(){FarDialogEvent e; OriginalEvents.Pop(e);}
};

INT_PTR WINAPI DlgProcA(HANDLE hDlg, int NewMsg, int Param1, INT_PTR Param2)
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
		case DN_CTLCOLORDIALOG:  Msg=oldfar::DN_CTLCOLORDIALOG; break;
		case DN_CTLCOLORDLGITEM: Msg=oldfar::DN_CTLCOLORDLGITEM; break;
		case DN_DRAWDIALOG:      Msg=oldfar::DN_DRAWDIALOG; break;

		case DN_CTLCOLORDLGLIST:
			{
				if(Param2)
				{
					FarListColors* lc = reinterpret_cast<FarListColors*>(Param2);
					oldfar::FarListColors lcA={};
					lcA.ColorCount = lc->ColorCount;
					lcA.Colors = lc->Colors;
					INT_PTR Result = CurrentDlgProc(hDlg, oldfar::DN_CTLCOLORDLGLIST, Param1, reinterpret_cast<INT_PTR>(&lcA));
					if(Result)
					{
						lc->ColorCount = lcA.ColorCount;
						lc->Colors = lcA.Colors;
						return TRUE;
					}
				}
				return FALSE;
			}
			break;

		case DN_DRAWDLGITEM:
		{
			Msg=oldfar::DN_DRAWDLGITEM;
			FarDialogItem *di = (FarDialogItem *)Param2;
			oldfar::FarDialogItem *FarDiA=UnicodeDialogItemToAnsi(*di,hDlg,Param1);
			INT_PTR ret = CurrentDlgProc(hDlg, Msg, Param1, (INT_PTR)FarDiA);
			if (ret && (di->Type==DI_USERCONTROL) && (di->VBuf))
			{
				AnsiVBufToUnicode(FarDiA->VBuf, di->VBuf, GetAnsiVBufSize(*FarDiA),(FarDiA->Flags&oldfar::DIF_NOTCVTUSERCONTROL)==oldfar::DIF_NOTCVTUSERCONTROL);
			}

			return ret;
		}
		case DN_EDITCHANGE:
			Msg=oldfar::DN_EDITCHANGE;
			return Param2?FarDefDlgProc(hDlg, NewMsg, Param1, Param2):FALSE;
		case DN_ENTERIDLE: Msg=oldfar::DN_ENTERIDLE; break;
		case DN_GOTFOCUS:  Msg=oldfar::DN_GOTFOCUS; break;
		case DN_HELP:
		{
			char* HelpTopicA = UnicodeToAnsi((const wchar_t *)Param2);
			INT_PTR ret = CurrentDlgProc(hDlg, oldfar::DN_HELP, Param1, (INT_PTR)HelpTopicA);
			if (ret && ret != Param2) // changed
			{
				if (HelpTopic) xf_free(HelpTopic);

				HelpTopic = AnsiToUnicode((const char *)ret);
				ret = (INT_PTR)HelpTopic;
			}

			xf_free(HelpTopicA);
			return ret;
		}
		case DN_HOTKEY:
			Msg=oldfar::DN_HOTKEY;
			Param2=KeyToOldKey((DWORD)InputRecordToKey((const INPUT_RECORD *)Param2));
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
				const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
				if (record->EventType==MOUSE_EVENT)
				{
					Msg=oldfar::DN_MOUSEEVENT;
					Param1=0;
					Param2=(INT_PTR)&record->Event.MouseEvent;
					break;
				}
			}
			return FarDefDlgProc(hDlg, NewMsg, Param1, Param2);
		case DN_CONTROLINPUT:
			{
				const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
				if (record->EventType==MOUSE_EVENT)
				{
					Msg=oldfar::DN_MOUSECLICK;
					Param2=(INT_PTR)&record->Event.MouseEvent;
					break;
				}
				else if (record->EventType==KEY_EVENT)
				{
					Msg=oldfar::DN_KEY;
					Param2=KeyToOldKey((DWORD)InputRecordToKey((const INPUT_RECORD *)Param2));
					break;
				}
			}
			return FarDefDlgProc(hDlg, NewMsg, Param1, Param2);
		default:
			break;
	}

	return CurrentDlgProc(hDlg, Msg, Param1, Param2);
}

LONG_PTR WINAPI FarDefDlgProcA(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	return FarDefDlgProc(OriginalEvents.Peek()->hDlg, OriginalEvents.Peek()->Msg, OriginalEvents.Peek()->Param1, OriginalEvents.Peek()->Param2);
}

LONG_PTR WINAPI FarSendDlgMessageA(HANDLE hDlg, int OldMsg, int Param1, LONG_PTR Param2)
{
	int Msg = DM_FIRST;
	if(OldMsg>oldfar::DM_USER)
	{
		Msg = OldMsg;
	}
	else switch (OldMsg)
	{
		case oldfar::DM_CLOSE:        Msg = DM_CLOSE; break;
		case oldfar::DM_ENABLE:       Msg = DM_ENABLE; break;
		case oldfar::DM_ENABLEREDRAW: Msg = DM_ENABLEREDRAW; break;
		case oldfar::DM_GETDLGDATA:   Msg = DM_GETDLGDATA; break;
		case oldfar::DM_GETDLGITEM:
		{
			size_t item_size=FarSendDlgMessage(hDlg, DM_GETDLGITEM, Param1, 0);

			if (item_size)
			{
				FarDialogItem *di = (FarDialogItem *)xf_malloc(item_size);

				if (di)
				{
					FarSendDlgMessage(hDlg, DM_GETDLGITEM, Param1, (INT_PTR)di);
					oldfar::FarDialogItem *FarDiA=UnicodeDialogItemToAnsi(*di,hDlg,Param1);
					xf_free(di);
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
				return FarSendDlgMessage(hDlg, DM_GETTEXT, Param1, 0);

			oldfar::FarDialogItemData* didA = (oldfar::FarDialogItemData*)Param2;
			if (!didA->PtrLength) //вот такой хреновый API!!!
				didA->PtrLength = static_cast<int>(FarSendDlgMessage(hDlg, DM_GETTEXT, Param1, 0));
			wchar_t* text = (wchar_t*) xf_malloc((didA->PtrLength+1)*sizeof(wchar_t));
			//BUGBUG: если didA->PtrLength=0, то вернётся с учётом '\0', в Энц написано, что без, хз как правильно.
			FarDialogItemData did = {didA->PtrLength, text};
			INT_PTR ret = FarSendDlgMessage(hDlg, DM_GETTEXT, Param1, (INT_PTR)&did);
			didA->PtrLength = (unsigned)did.PtrLength;
			UnicodeToOEM(text,didA->PtrData,didA->PtrLength+1);
			xf_free(text);
			return ret;
		}
		case oldfar::DM_GETTEXTLENGTH: Msg = DM_GETTEXTLENGTH; break;

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
			INT_PTR ret = FarSendDlgMessage(hDlg, DM_KEY, Param1, (INT_PTR)KeysW);
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
			return FarSendDlgMessage(hDlg, DM_SETDLGITEM, Param1, (INT_PTR)di);
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
			FarDialogItemData di = {didA->PtrLength,text};
			INT_PTR ret = FarSendDlgMessage(hDlg, DM_SETTEXT, Param1, (INT_PTR)&di);
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
			INT_PTR length = FarSendDlgMessage(hDlg, DM_GETTEXTPTR, Param1, 0);

			if (!Param2) return length;

			wchar_t* text = (wchar_t *) xf_malloc((length +1)* sizeof(wchar_t));
			length = FarSendDlgMessage(hDlg, DM_GETTEXTPTR, Param1, (INT_PTR)text);
			UnicodeToOEM(text, (char *)Param2, length+1);
			xf_free(text);
			return length;
		}
		case oldfar::DM_SETTEXTPTR:
		{
			if (!Param2) return FALSE;

			wchar_t* text = AnsiToUnicode((char*)Param2);
			INT_PTR ret = FarSendDlgMessage(hDlg, DM_SETTEXTPTR, Param1, (INT_PTR)text);
			xf_free(text);
			return ret;
		}
		case oldfar::DM_SHOWITEM: Msg = DM_SHOWITEM; break;
		case oldfar::DM_ADDHISTORY:
		{
			if (!Param2) return FALSE;

			wchar_t* history = AnsiToUnicode((char*)Param2);
			INT_PTR ret = FarSendDlgMessage(hDlg, DM_ADDHISTORY, Param1, (INT_PTR)history);
			xf_free(history);
			return ret;
		}
		case oldfar::DM_GETCHECK:
		{
			INT_PTR ret = FarSendDlgMessage(hDlg, DM_GETCHECK, Param1, 0);
			INT_PTR state = 0;

			if (ret == oldfar::BSTATE_UNCHECKED) state=BSTATE_UNCHECKED;
			else if (ret == oldfar::BSTATE_CHECKED)   state=BSTATE_CHECKED;
			else if (ret == oldfar::BSTATE_3STATE)    state=BSTATE_3STATE;

			return state;
		}
		case oldfar::DM_SETCHECK:
		{
			INT_PTR state = 0;

			if (Param2 == oldfar::BSTATE_UNCHECKED) state=BSTATE_UNCHECKED;
			else if (Param2 == oldfar::BSTATE_CHECKED)   state=BSTATE_CHECKED;
			else if (Param2 == oldfar::BSTATE_3STATE)    state=BSTATE_3STATE;
			else if (Param2 == oldfar::BSTATE_TOGGLE)    state=BSTATE_TOGGLE;

			return FarSendDlgMessage(hDlg, DM_SETCHECK, Param1, state);
		}
		case oldfar::DM_SET3STATE: Msg = DM_SET3STATE; break;
		case oldfar::DM_LISTSORT:  Msg = DM_LISTSORT; break;
		case oldfar::DM_LISTGETITEM: //BUGBUG, недоделано в фаре
		{
			if (!Param2) return FALSE;

			oldfar::FarListGetItem* lgiA = (oldfar::FarListGetItem*)Param2;
			FarListGetItem lgi = {lgiA->ItemIndex};
			INT_PTR ret = FarSendDlgMessage(hDlg, DM_LISTGETITEM, Param1, (INT_PTR)&lgi);
			UnicodeListItemToAnsi(&lgi.Item, &lgiA->Item);
			return ret;
		}
		case oldfar::DM_LISTGETCURPOS:

			if (Param2)
			{
				FarListPos lp;
				INT_PTR ret=FarSendDlgMessage(hDlg, DM_LISTGETCURPOS, Param1, (INT_PTR)&lp);
				oldfar::FarListPos *lpA = (oldfar::FarListPos *)Param2;
				lpA->SelectPos=lp.SelectPos;
				lpA->TopPos=lp.TopPos;
				return ret;
			}
			else return FarSendDlgMessage(hDlg, DM_LISTGETCURPOS, Param1, 0);

		case oldfar::DM_LISTSETCURPOS:
		{
			if (!Param2) return FALSE;

			oldfar::FarListPos *lpA = (oldfar::FarListPos *)Param2;
			FarListPos lp = {lpA->SelectPos,lpA->TopPos};
			return FarSendDlgMessage(hDlg, DM_LISTSETCURPOS, Param1, (INT_PTR)&lp);
		}
		case oldfar::DM_LISTDELETE:
		{
			oldfar::FarListDelete *ldA = (oldfar::FarListDelete *)Param2;
			FarListDelete ld;

			if (Param2)
			{
				ld.Count = ldA->Count;
				ld.StartIndex = ldA->StartIndex;
			}

			return FarSendDlgMessage(hDlg, DM_LISTDELETE, Param1, Param2?(INT_PTR)&ld:0);
		}
		case oldfar::DM_LISTADD:
		{
			FarList newlist = {0,0};

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

			INT_PTR ret = FarSendDlgMessage(hDlg, DM_LISTADD, Param1, Param2?(INT_PTR)&newlist:0);

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

			INT_PTR ret = FarSendDlgMessage(hDlg, DM_LISTADDSTR, Param1, (INT_PTR)newstr);

			if (newstr) xf_free(newstr);

			return ret;
		}
		case oldfar::DM_LISTUPDATE:
		{
			FarListUpdate newui = {0,0};

			if (Param2)
			{
				oldfar::FarListUpdate *oldui = (oldfar::FarListUpdate*) Param2;
				newui.Index=oldui->Index;
				AnsiListItemToUnicode(&oldui->Item, &newui.Item);
			}

			INT_PTR ret = FarSendDlgMessage(hDlg, DM_LISTUPDATE, Param1, Param2?(INT_PTR)&newui:0);

			if (newui.Item.Text) xf_free((void*)newui.Item.Text);

			return ret;
		}
		case oldfar::DM_LISTINSERT:
		{
			FarListInsert newli = {0,0};

			if (Param2)
			{
				oldfar::FarListInsert *oldli = (oldfar::FarListInsert*) Param2;
				newli.Index=oldli->Index;
				AnsiListItemToUnicode(&oldli->Item, &newli.Item);
			}

			INT_PTR ret = FarSendDlgMessage(hDlg, DM_LISTINSERT, Param1, Param2?(INT_PTR)&newli:0);

			if (newli.Item.Text) xf_free((void*)newli.Item.Text);

			return ret;
		}
		case oldfar::DM_LISTFINDSTRING:
		{
			FarListFind newlf = {0,0,0,0};

			if (Param2)
			{
				oldfar::FarListFind *oldlf = (oldfar::FarListFind*) Param2;
				newlf.StartIndex=oldlf->StartIndex;
				newlf.Pattern = (oldlf->Pattern)?AnsiToUnicode(oldlf->Pattern):nullptr;

				if (oldlf->Flags&oldfar::LIFIND_EXACTMATCH) newlf.Flags|=LIFIND_EXACTMATCH;
			}

			INT_PTR ret = FarSendDlgMessage(hDlg, DM_LISTFINDSTRING, Param1, Param2?(INT_PTR)&newlf:0);

			if (newlf.Pattern) xf_free((void*)newlf.Pattern);

			return ret;
		}
		case oldfar::DM_LISTINFO:
		{
			if (!Param2) return FALSE;

			oldfar::FarListInfo *liA = (oldfar::FarListInfo *)Param2;
			FarListInfo li={0,liA->ItemsNumber,liA->SelectPos,liA->TopPos,liA->MaxHeight,liA->MaxLength};

			if (liA ->Flags&oldfar::LINFO_SHOWNOBOX) li.Flags|=LINFO_SHOWNOBOX;

			if (liA ->Flags&oldfar::LINFO_AUTOHIGHLIGHT) li.Flags|=LINFO_AUTOHIGHLIGHT;

			if (liA ->Flags&oldfar::LINFO_REVERSEHIGHLIGHT) li.Flags|=LINFO_REVERSEHIGHLIGHT;

			if (liA ->Flags&oldfar::LINFO_WRAPMODE) li.Flags|=LINFO_WRAPMODE;

			if (liA ->Flags&oldfar::LINFO_SHOWAMPERSAND) li.Flags|=LINFO_SHOWAMPERSAND;

			return FarSendDlgMessage(hDlg, DM_LISTINFO, Param1, Param2);
		}
		case oldfar::DM_LISTGETDATA:	Msg = DM_LISTGETDATA; break;
		case oldfar::DM_LISTSETDATA:
		{
			FarListItemData newlid = {0,0,0,0};

			if (Param2)
			{
				oldfar::FarListItemData *oldlid = (oldfar::FarListItemData*) Param2;
				newlid.Index=oldlid->Index;
				newlid.DataSize=oldlid->DataSize;
				newlid.Data=oldlid->Data;
			}

			INT_PTR ret = FarSendDlgMessage(hDlg, DM_LISTSETDATA, Param1, Param2?(INT_PTR)&newlid:0);
			return ret;
		}
		case oldfar::DM_LISTSETTITLES:
		{
			if (!Param2) return FALSE;

			oldfar::FarListTitles *ltA = (oldfar::FarListTitles *)Param2;
			FarListTitles lt = {0,ltA->Title?AnsiToUnicode(ltA->Title):nullptr,0,ltA->Bottom?AnsiToUnicode(ltA->Bottom):nullptr};
			Param2 = (INT_PTR)&lt;
			INT_PTR ret = FarSendDlgMessage(hDlg, DM_LISTSETTITLES, Param1, Param2);

			if (lt.Bottom) xf_free((wchar_t *)lt.Bottom);

			if (lt.Title) xf_free((wchar_t *)lt.Title);

			return ret;
		}
		case oldfar::DM_LISTGETTITLES:
		{
			if (Param2)
			{
				oldfar::FarListTitles *OldListTitle=(oldfar::FarListTitles *)Param2;
				FarListTitles ListTitle={0,nullptr,0,nullptr};

				if (OldListTitle->Title)
				{
					ListTitle.TitleLen=OldListTitle->TitleLen;
					ListTitle.Title=(wchar_t *)xf_malloc(sizeof(wchar_t)*ListTitle.TitleLen);
				}

				if (OldListTitle->BottomLen)
				{
					ListTitle.BottomLen=OldListTitle->BottomLen;
					ListTitle.Bottom=(wchar_t *)xf_malloc(sizeof(wchar_t)*ListTitle.BottomLen);
				}

				INT_PTR Ret=FarSendDlgMessage(hDlg,DM_LISTGETTITLES,Param1,(INT_PTR)&ListTitle);

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
				return FarSendDlgMessage(hDlg, DM_SETHISTORY, Param1, 0);
			else
			{
				FarDialogItem *di=CurrentDialogItem(hDlg,Param1);
				xf_free((void*)di->History);
				di->History = AnsiToUnicode((const char *)Param2);
				return FarSendDlgMessage(hDlg, DM_SETHISTORY, Param1, (INT_PTR)di->History);
			}

		case oldfar::DM_GETITEMPOSITION:     Msg = DM_GETITEMPOSITION; break;
		case oldfar::DM_SETMOUSEEVENTNOTIFY: Msg = DM_SETMOUSEEVENTNOTIFY; break;
		case oldfar::DM_EDITUNCHANGEDFLAG:   Msg = DM_EDITUNCHANGEDFLAG; break;
		case oldfar::DM_GETITEMDATA:         Msg = DM_GETITEMDATA; break;
		case oldfar::DM_SETITEMDATA:         Msg = DM_SETITEMDATA; break;
		case oldfar::DM_LISTSET:
		{
			FarList newlist = {0,0};

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

			INT_PTR ret = FarSendDlgMessage(hDlg, DM_LISTSET, Param1, Param2?(INT_PTR)&newlist:0);

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
			INT_PTR type=0;

			if (Param2 == oldfar::LMRT_ONLYFOCUS) type=LMRT_ONLYFOCUS;
			else if (Param2 == oldfar::LMRT_ALWAYS)    type=LMRT_ALWAYS;
			else if (Param2 == oldfar::LMRT_NEVER)     type=LMRT_NEVER;

			return FarSendDlgMessage(hDlg, DM_LISTSETMOUSEREACTION, Param1, type);
		}
		case oldfar::DM_GETCURSORSIZE:   Msg = DM_GETCURSORSIZE; break;
		case oldfar::DM_SETCURSORSIZE:   Msg = DM_SETCURSORSIZE; break;
		case oldfar::DM_LISTGETDATASIZE: Msg = DM_LISTGETDATASIZE; break;
		case oldfar::DM_GETSELECTION:
		{
			if (!Param2) return FALSE;

			EditorSelect es;
			INT_PTR ret=FarSendDlgMessage(hDlg, DM_GETSELECTION, Param1, (INT_PTR)&es);
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
			EditorSelect es;
			es.BlockType      = esA->BlockType;
			es.BlockStartLine = esA->BlockStartLine;
			es.BlockStartPos  = esA->BlockStartPos;
			es.BlockWidth     = esA->BlockWidth;
			es.BlockHeight    = esA->BlockHeight;
			return FarSendDlgMessage(hDlg, DM_SETSELECTION, Param1, (INT_PTR)&es);
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

	return FarSendDlgMessage(hDlg, Msg, Param1, Param2);
}

int WINAPI FarDialogExA(INT_PTR PluginNumber,int X1,int Y1,int X2,int Y2,const char *HelpTopic,oldfar::FarDialogItem *Item,int ItemsNumber,DWORD Reserved,DWORD Flags,oldfar::FARWINDOWPROC DlgProc,LONG_PTR Param)
{
	string strHT(HelpTopic);

	if (!Item || !ItemsNumber)
		return -1;

	oldfar::FarDialogItem* diA=new oldfar::FarDialogItem[ItemsNumber]();
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
	HANDLE hDlg = FarDialogInit(PluginNumber, &FarGuid, X1, Y1, X2, Y2, (HelpTopic?strHT.CPtr():nullptr), (FarDialogItem *)di, ItemsNumber, 0, DlgFlags, DlgProc?DlgProcA:0, Param);
	PDialogData NewDialogData=new DialogData;
	NewDialogData->DlgProc=DlgProc;
	NewDialogData->hDlg=hDlg;
	NewDialogData->diA=diA;
	NewDialogData->di=di;
	NewDialogData->l=l;
	DialogList.Push(&NewDialogData);

	if (hDlg != INVALID_HANDLE_VALUE)
	{
		ret = FarDialogRun(hDlg);

		for (int i=0; i<ItemsNumber; i++)
		{
			FarDialogItem *pdi = (FarDialogItem *)xf_malloc(FarSendDlgMessage(hDlg, DM_GETDLGITEM, i, 0));

			if (pdi)
			{
				FarSendDlgMessage(hDlg, DM_GETDLGITEM, i, (INT_PTR)pdi);
				UnicodeDialogItemToAnsiSafe(*pdi,Item[i]);
				const wchar_t *res = pdi->Data;

				if (!res) res = L"";

				if ((di[i].Type==DI_EDIT || di[i].Type==DI_COMBOBOX) && Item[i].Flags&oldfar::DIF_VAREDIT)
					UnicodeToOEM(res, Item[i].Ptr.PtrData, Item[i].Ptr.PtrLength+1);
				else
					UnicodeToOEM(res, Item[i].Data, sizeof(Item[i].Data));

				if (pdi->Type==DI_USERCONTROL)
				{
					di[i].VBuf=pdi->VBuf;
					Item[i].VBuf=GetAnsiVBufPtr(pdi->VBuf,GetAnsiVBufSize(Item[i]));
				}

				if (pdi->Type==DI_COMBOBOX || pdi->Type==DI_LISTBOX)
				{
					Item[i].ListPos = static_cast<int>(FarSendDlgMessage(hDlg,DM_LISTGETCURPOS,i,0));
				}

				xf_free(pdi);
			}

			FreeAnsiDialogItem(diA[i]);
		}

		FarDialogFree(hDlg);

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

int WINAPI FarDialogFnA(INT_PTR PluginNumber,int X1,int Y1,int X2,int Y2,const char *HelpTopic,oldfar::FarDialogItem *Item,int ItemsNumber)
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
	PIA->ItemsNumber = PIW->ItemsNumber;
	PIA->SelectedItemsNumber = PIW->SelectedItemsNumber;
	PIA->PanelItems = nullptr;
	PIA->SelectedItems = nullptr;
	PIA->CurrentItem = PIW->CurrentItem;
	PIA->TopPanelItem = PIW->TopPanelItem;
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

	memset(PIA,0,sizeof(oldfar::PanelInfo));
}

int WINAPI FarControlA(HANDLE hPlugin,int Command,void *Param)
{
	static oldfar::PanelInfo PanelInfoA={0},AnotherPanelInfoA={0};
	static int Reenter=0;

	if (hPlugin==INVALID_HANDLE_VALUE)
		hPlugin=PANEL_ACTIVE;

	switch (Command)
	{
		case oldfar::FCTL_CHECKPANELSEXIST:
			return FarControl(hPlugin,FCTL_CHECKPANELSEXIST,0,(INT_PTR)Param);
		case oldfar::FCTL_CLOSEPLUGIN:
		{
			wchar_t *ParamW = nullptr;

			if (Param)
				ParamW = AnsiToUnicode((const char *)Param);

			int ret = FarControl(hPlugin,FCTL_CLOSEPANEL,0,(INT_PTR)ParamW);

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
			PanelInfo PI;
			int ret = FarControl(hPlugin,FCTL_GETPANELINFO,0,(INT_PTR)&PI);
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

						for (int i=0; i<PI.ItemsNumber; i++)
						{
							int NewPPISize=FarControl(hPlugin,FCTL_GETPANELITEM,i,0);

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

							FarControl(hPlugin,FCTL_GETPANELITEM,i,(INT_PTR)PPI);
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

						for (int i=0; i<PI.SelectedItemsNumber; i++)
						{
							int NewPPISize=FarControl(hPlugin,FCTL_GETSELECTEDPANELITEM,i,0);

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

							FarControl(hPlugin,FCTL_GETSELECTEDPANELITEM,i,(INT_PTR)PPI);
							if(PPI)
							{
								ConvertPanelItemToAnsi(*PPI,OldPI->SelectedItems[i]);
							}
						}

						if (PPI)
							xf_free(PPI);
					}
				}

				wchar_t CurDir[sizeof(OldPI->CurDir)];
				FarControl(hPlugin,FCTL_GETPANELDIR,sizeof(OldPI->CurDir),(INT_PTR)CurDir);
				UnicodeToOEM(CurDir,OldPI->CurDir,sizeof(OldPI->CurDir));
				wchar_t ColumnTypes[sizeof(OldPI->ColumnTypes)];
				FarControl(hPlugin,FCTL_GETCOLUMNTYPES,sizeof(OldPI->ColumnTypes),(INT_PTR)ColumnTypes);
				UnicodeToOEM(ColumnTypes,OldPI->ColumnTypes,sizeof(OldPI->ColumnTypes));
				wchar_t ColumnWidths[sizeof(OldPI->ColumnWidths)];
				FarControl(hPlugin,FCTL_GETCOLUMNWIDTHS,sizeof(OldPI->ColumnWidths),(INT_PTR)ColumnWidths);
				UnicodeToOEM(ColumnWidths,OldPI->ColumnWidths,sizeof(OldPI->ColumnWidths));
				*(oldfar::PanelInfo*)Param=*OldPI;
			}
			else
			{
				memset((oldfar::PanelInfo*)Param,0,sizeof(oldfar::PanelInfo));
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
			memset(OldPI,0,sizeof(oldfar::PanelInfo));

			if (Command==oldfar::FCTL_GETANOTHERPANELSHORTINFO)
				hPlugin=PANEL_PASSIVE;

			PanelInfo PI;
			int ret = FarControl(hPlugin,FCTL_GETPANELINFO,0,(INT_PTR)&PI);

			if (ret)
			{
				ConvertUnicodePanelInfoToAnsi(&PI,OldPI);
				wchar_t CurDir[sizeof(OldPI->CurDir)];
				FarControl(hPlugin,FCTL_GETPANELDIR,sizeof(OldPI->CurDir),(INT_PTR)CurDir);
				UnicodeToOEM(CurDir,OldPI->CurDir,sizeof(OldPI->CurDir));
				wchar_t ColumnTypes[sizeof(OldPI->ColumnTypes)];
				FarControl(hPlugin,FCTL_GETCOLUMNTYPES,sizeof(OldPI->ColumnTypes),(INT_PTR)ColumnTypes);
				UnicodeToOEM(ColumnTypes,OldPI->ColumnTypes,sizeof(OldPI->ColumnTypes));
				wchar_t ColumnWidths[sizeof(OldPI->ColumnWidths)];
				FarControl(hPlugin,FCTL_GETCOLUMNWIDTHS,sizeof(OldPI->ColumnWidths),(INT_PTR)ColumnWidths);
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
			FarControl(hPlugin,FCTL_BEGINSELECTION,0,0);

			for (int i=0; i<OldPI->ItemsNumber; i++)
			{
				FarControl(hPlugin,FCTL_SETSELECTION,i,OldPI->PanelItems[i].Flags & oldfar::PPIF_SELECTED);
			}

			FarControl(hPlugin,FCTL_ENDSELECTION,0,0);
			return TRUE;
		}
		case oldfar::FCTL_REDRAWANOTHERPANEL:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_REDRAWPANEL:
		{
			if (!Param)
				return FarControl(hPlugin, FCTL_REDRAWPANEL,0,0);

			oldfar::PanelRedrawInfo* priA = (oldfar::PanelRedrawInfo*)Param;
			PanelRedrawInfo pri = {priA->CurrentItem,priA->TopPanelItem};
			return FarControl(hPlugin, FCTL_REDRAWPANEL,0,(INT_PTR)&pri);
		}
		case oldfar::FCTL_SETANOTHERNUMERICSORT:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_SETNUMERICSORT:
			return FarControl(hPlugin, FCTL_SETNUMERICSORT,(Param&&(*(int*)Param))?1:0,0);
		case oldfar::FCTL_SETANOTHERPANELDIR:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_SETPANELDIR:
		{
			if (!Param)
				return FALSE;

			wchar_t* Dir = AnsiToUnicode((char*)Param);
			int ret = FarControl(hPlugin, FCTL_SETPANELDIR,0,(INT_PTR)Dir);
			xf_free(Dir);
			return ret;
		}
		case oldfar::FCTL_SETANOTHERSORTMODE:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_SETSORTMODE:

			if (!Param)
				return FALSE;

			return FarControl(hPlugin, FCTL_SETSORTMODE,*(int*)Param,0);
		case oldfar::FCTL_SETANOTHERSORTORDER:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_SETSORTORDER:
			return FarControl(hPlugin, FCTL_SETSORTORDER,(Param&&(*(int*)Param))?TRUE:FALSE,0);
		case oldfar::FCTL_SETANOTHERVIEWMODE:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_SETVIEWMODE:
			return FarControl(hPlugin, FCTL_SETVIEWMODE,(Param?*(int *)Param:0),0);
		case oldfar::FCTL_UPDATEANOTHERPANEL:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_UPDATEPANEL:
			return FarControl(hPlugin, FCTL_UPDATEPANEL,Param?1:0,0);
		case oldfar::FCTL_GETCMDLINE:
		case oldfar::FCTL_GETCMDLINESELECTEDTEXT:
		{
			if (Param)
			{
				const size_t Size = 1024;
				wchar_t _s[Size], *s=_s;
				FarControl(hPlugin, FCTL_GETCMDLINE, Size, reinterpret_cast<INT_PTR>(s));
				if(Command==oldfar::FCTL_GETCMDLINESELECTEDTEXT)
				{
					CmdLineSelect cls;
					FarControl(hPlugin,FCTL_GETCMDLINESELECTION, 0, reinterpret_cast<INT_PTR>(&cls));
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

			return FarControl(hPlugin,FCTL_GETCMDLINEPOS,0,(INT_PTR)Param);
		case oldfar::FCTL_GETCMDLINESELECTION:
		{
			if (!Param)
				return FALSE;

			CmdLineSelect cls;
			int ret = FarControl(hPlugin, FCTL_GETCMDLINESELECTION,0,(INT_PTR)&cls);

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
			int ret = FarControl(hPlugin, FCTL_INSERTCMDLINE,0,(INT_PTR)s);
			xf_free(s);
			return ret;
		}
		case oldfar::FCTL_SETCMDLINE:
		{
			if (!Param)
				return FALSE;

			wchar_t* s = AnsiToUnicode((const char*)Param);
			int ret = FarControl(hPlugin, FCTL_SETCMDLINE,0,(INT_PTR)s);
			xf_free(s);
			return ret;
		}
		case oldfar::FCTL_SETCMDLINEPOS:

			if (!Param)
				return FALSE;

			return FarControl(hPlugin, FCTL_SETCMDLINEPOS,*(int*)Param,0);
		case oldfar::FCTL_SETCMDLINESELECTION:
		{
			if (!Param)
				return FALSE;

			oldfar::CmdLineSelect* clsA = (oldfar::CmdLineSelect*)Param;
			CmdLineSelect cls = {clsA->SelStart,clsA->SelEnd};
			return FarControl(hPlugin, FCTL_SETCMDLINESELECTION,0,(INT_PTR)&cls);
		}
		case oldfar::FCTL_GETUSERSCREEN:
			return FarControl(hPlugin, FCTL_GETUSERSCREEN,0,0);
		case oldfar::FCTL_SETUSERSCREEN:
			return FarControl(hPlugin, FCTL_SETUSERSCREEN,0,0);
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
	int ItemsNumber;
	int ret=FarGetDirList(strDir, &pItems, &ItemsNumber);

	size_t PathOffset = ExtractFilePath(strDir).GetLength() + 1;

	if (ret && ItemsNumber)
	{
		//+sizeof(int) чтоб хранить ItemsNumber ибо в FarFreeDirListA как то надо знать
		*pPanelItem=(oldfar::PluginPanelItem *)xf_malloc(ItemsNumber*sizeof(oldfar::PluginPanelItem)+sizeof(int));

		if (*pPanelItem)
		{
			*pItemsNumber = ItemsNumber;
			**((int **)pPanelItem) = ItemsNumber;
			(*((int **)pPanelItem))++;
			memset(*pPanelItem,0,ItemsNumber*sizeof(oldfar::PluginPanelItem));

			for (int i=0; i<ItemsNumber; i++)
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

		FarFreeDirList(pItems,ItemsNumber);
	}

	return ret;
}

int WINAPI FarGetPluginDirListA(INT_PTR PluginNumber,HANDLE hPlugin,const char *Dir,oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber)
{
	if (!Dir || !*Dir || !pPanelItem || !pItemsNumber)
		return FALSE;

	*pPanelItem=nullptr;
	*pItemsNumber=0;
	string strDir(Dir);

	PluginPanelItem *pPanelItemW;
	int ItemsNumber;
	int ret=FarGetPluginDirList(PluginNumber, hPlugin, strDir, &pPanelItemW, &ItemsNumber);

	if (ret && ItemsNumber)
	{
		//+sizeof(int) чтоб хранить ItemsNumber ибо в FarFreeDirListA как то надо знать
		*pPanelItem=(oldfar::PluginPanelItem *)xf_malloc(ItemsNumber*sizeof(oldfar::PluginPanelItem)+sizeof(int));

		if (*pPanelItem)
		{
			*pItemsNumber = ItemsNumber;
			**((int **)pPanelItem) = ItemsNumber;
			(*((int **)pPanelItem))++;
			memset(*pPanelItem,0,ItemsNumber*sizeof(oldfar::PluginPanelItem));

			for (int i=0; i<ItemsNumber; i++)
			{
				ConvertPanelItemToAnsi(pPanelItemW[i],(*pPanelItem)[i]);
			}
		}
		else
		{
			ret = FALSE;
		}

		FarFreePluginDirList(pPanelItemW, ItemsNumber);
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

INT_PTR WINAPI FarAdvControlA(INT_PTR ModuleNumber,int Command,void *Param)
{
	static char *ErrMsg1 = nullptr;
	static char *ErrMsg2 = nullptr;
	static char *ErrMsg3 = nullptr;

	switch (Command)
	{
		case oldfar::ACTL_GETFARVERSION:
		{
			VersionInfo Info;
			FarAdvControl(ModuleNumber, ACTL_GETFARMANAGERVERSION, &Info);
			DWORD FarVer = Info.Major<<8|Info.Minor|Info.Build<<16;
			int OldFarVer;
			GeneralCfg->GetValue(L"wrapper",L"version",&OldFarVer,FarVer);

			if (
			    //не выше текущей версии
			    (LOWORD(OldFarVer)<LOWORD(FarVer) || ((LOWORD(OldFarVer)==LOWORD(FarVer)) && HIWORD(OldFarVer)<HIWORD(FarVer))) &&
			    //и не ниже 1.70.1
			    LOWORD(OldFarVer)>=0x0146 && HIWORD(OldFarVer)>=0x1)
				FarVer=OldFarVer;

			if (Param)
				*(DWORD*)Param=FarVer;

			return FarVer;
		}
		case oldfar::ACTL_CONSOLEMODE:
			return IsFullscreen()?TRUE:FALSE;

		case oldfar::ACTL_GETSYSWORDDIV:
		{
			INT_PTR Length = FarAdvControl(ModuleNumber, ACTL_GETSYSWORDDIV, nullptr);

			if (Param)
			{
				wchar_t *SysWordDiv = (wchar_t*)xf_malloc((Length+1)*sizeof(wchar_t));
				FarAdvControl(ModuleNumber, ACTL_GETSYSWORDDIV, SysWordDiv);
				UnicodeToOEM(SysWordDiv,(char*)Param,oldfar::NM);
				xf_free(SysWordDiv);
			}

			return Length;
		}
		case oldfar::ACTL_WAITKEY:
			return FarAdvControl(ModuleNumber, ACTL_WAITKEY, Param);
		case oldfar::ACTL_GETCOLOR:
			return FarAdvControl(ModuleNumber, ACTL_GETCOLOR, Param);
		case oldfar::ACTL_GETARRAYCOLOR:
			return FarAdvControl(ModuleNumber, ACTL_GETARRAYCOLOR, Param);
		case oldfar::ACTL_EJECTMEDIA:
			return FarAdvControl(ModuleNumber, ACTL_EJECTMEDIA, Param);
		case oldfar::ACTL_KEYMACRO:
		{
			if (!Param) return FALSE;

			oldfar::ActlKeyMacro *kmA=(oldfar::ActlKeyMacro *)Param;
			FAR_MACRO_CONTROL_COMMANDS Command = MCTL_LOADALL;
			int Param1=0;
			bool Process=true;

			MacroCheckMacroText kmW={0};
			kmW.Text.StructSize=sizeof(MacroParseResult);

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
					kmW.Text.SequenceText=AnsiToUnicode(kmA->PlainText.SequenceText);

					if (kmA->PlainText.Flags&oldfar::KSFLAGS_DISABLEOUTPUT) kmW.Text.Flags|=KMFLAGS_DISABLEOUTPUT;

					if (kmA->PlainText.Flags&oldfar::KSFLAGS_NOSENDKEYSTOPLUGINS) kmW.Text.Flags|=KMFLAGS_NOSENDKEYSTOPLUGINS;

					if (kmA->PlainText.Flags&oldfar::KSFLAGS_REG_MULTI_SZ) kmW.Text.Flags|=KMFLAGS_REG_MULTI_SZ;

					break;

				case oldfar::MCMD_CHECKMACRO:
					Command=MCTL_SENDSTRING;
					Param1=MSSC_CHECK;
					kmW.Text.SequenceText=AnsiToUnicode(kmA->PlainText.SequenceText);
					break;

				default:
					Process=false;
					break;
			}

			INT_PTR res=0;

			if (Process)
			{
				res = farMacroControl(0,Command,Param1,(INT_PTR)&kmW);

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

							if (kmW.Text.SequenceText)
								xf_free((void*)kmW.Text.SequenceText);

							break;
						}

						case MSSC_POST:

							if (kmW.Text.SequenceText)
								xf_free((void*)kmW.Text.SequenceText);

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

			oldfar::KeySequence *ksA = (oldfar::KeySequence*)Param;

			if (!ksA->Count || !ksA->Sequence)
				return FALSE;

			MacroRecord MRec={0};

			if (ksA->Flags&oldfar::KSFLAGS_DISABLEOUTPUT)
				MRec.Flags|=MFLAGS_DISABLEOUTPUT;

			if (ksA->Flags&oldfar::KSFLAGS_NOSENDKEYSTOPLUGINS)
				MRec.Flags|=MFLAGS_NOSENDKEYSTOPLUGINS;

			if (ksA->Flags&oldfar::KSFLAGS_REG_MULTI_SZ)
				MRec.Flags|=MFLAGS_REG_MULTI_SZ;

			MRec.BufferSize=ksA->Count;

			DWORD* Sequence = (DWORD*)xf_malloc(MRec.BufferSize*sizeof(DWORD));
			for (int i=0; i<MRec.BufferSize; i++)
			{
				Sequence[i]=OldKeyToKey(ksA->Sequence[i]);
			}

			if (MRec.BufferSize == 1)
				MRec.Buffer=(DWORD *)(DWORD_PTR)Sequence[0];
			else
				MRec.Buffer=Sequence;

			INT_PTR ret = CtrlObject->Macro.PostNewMacro(&MRec,TRUE,TRUE);
			xf_free(Sequence);

			return ret;
		}
		case oldfar::ACTL_GETSHORTWINDOWINFO:
		case oldfar::ACTL_GETWINDOWINFO:
		{
			if (!Param)
				return FALSE;

			oldfar::WindowInfo *wiA = (oldfar::WindowInfo *)Param;
			WindowInfo wi={wiA->Pos};
			INT_PTR ret = FarAdvControl(ModuleNumber, ACTL_GETWINDOWINFO, &wi);

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
						FarAdvControl(ModuleNumber,ACTL_GETWINDOWINFO,&wi);
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
			return FarAdvControl(ModuleNumber, ACTL_GETWINDOWCOUNT, 0);
		case oldfar::ACTL_SETCURRENTWINDOW:
			return FarAdvControl(ModuleNumber, ACTL_SETCURRENTWINDOW, Param);
		case oldfar::ACTL_COMMIT:
			return FarAdvControl(ModuleNumber, ACTL_COMMIT, 0);
		case oldfar::ACTL_GETFARHWND:
			return FarAdvControl(ModuleNumber, ACTL_GETFARHWND, 0);
		case oldfar::ACTL_GETSYSTEMSETTINGS:
		{
			INT_PTR ss = FarAdvControl(ModuleNumber, ACTL_GETSYSTEMSETTINGS, 0);
			INT_PTR ret = 0;

			if (ss&FSS_CLEARROATTRIBUTE)          ret|=oldfar::FSS_CLEARROATTRIBUTE;

			if (ss&FSS_DELETETORECYCLEBIN)        ret|=oldfar::FSS_DELETETORECYCLEBIN;

			if (ss&FSS_USESYSTEMCOPYROUTINE)      ret|=oldfar::FSS_USESYSTEMCOPYROUTINE;

			if (ss&FSS_COPYFILESOPENEDFORWRITING) ret|=oldfar::FSS_COPYFILESOPENEDFORWRITING;

			if (ss&FSS_CREATEFOLDERSINUPPERCASE)  ret|=oldfar::FSS_CREATEFOLDERSINUPPERCASE;

			if (ss&FSS_SAVECOMMANDSHISTORY)       ret|=oldfar::FSS_SAVECOMMANDSHISTORY;

			if (ss&FSS_SAVEFOLDERSHISTORY)        ret|=oldfar::FSS_SAVEFOLDERSHISTORY;

			if (ss&FSS_SAVEVIEWANDEDITHISTORY)    ret|=oldfar::FSS_SAVEVIEWANDEDITHISTORY;

			if (ss&FSS_USEWINDOWSREGISTEREDTYPES) ret|=oldfar::FSS_USEWINDOWSREGISTEREDTYPES;

			if (ss&FSS_AUTOSAVESETUP)             ret|=oldfar::FSS_AUTOSAVESETUP;

			if (ss&FSS_SCANSYMLINK)               ret|=oldfar::FSS_SCANSYMLINK;

			return ret;
		}
		case oldfar::ACTL_GETPANELSETTINGS:
		{
			INT_PTR ps = FarAdvControl(ModuleNumber, ACTL_GETPANELSETTINGS, 0);
			INT_PTR ret = 0;

			if (ps&FPS_SHOWHIDDENANDSYSTEMFILES)    ret|=oldfar::FPS_SHOWHIDDENANDSYSTEMFILES;

			if (ps&FPS_HIGHLIGHTFILES)              ret|=oldfar::FPS_HIGHLIGHTFILES;

			if (ps&FPS_AUTOCHANGEFOLDER)            ret|=oldfar::FPS_AUTOCHANGEFOLDER;

			if (ps&FPS_SELECTFOLDERS)               ret|=oldfar::FPS_SELECTFOLDERS;

			if (ps&FPS_ALLOWREVERSESORTMODES)       ret|=oldfar::FPS_ALLOWREVERSESORTMODES;

			if (ps&FPS_SHOWCOLUMNTITLES)            ret|=oldfar::FPS_SHOWCOLUMNTITLES;

			if (ps&FPS_SHOWSTATUSLINE)              ret|=oldfar::FPS_SHOWSTATUSLINE;

			if (ps&FPS_SHOWFILESTOTALINFORMATION)   ret|=oldfar::FPS_SHOWFILESTOTALINFORMATION;

			if (ps&FPS_SHOWFREESIZE)                ret|=oldfar::FPS_SHOWFREESIZE;

			if (ps&FPS_SHOWSCROLLBAR)               ret|=oldfar::FPS_SHOWSCROLLBAR;

			if (ps&FPS_SHOWBACKGROUNDSCREENSNUMBER) ret|=oldfar::FPS_SHOWBACKGROUNDSCREENSNUMBER;

			if (ps&FPS_SHOWSORTMODELETTER)          ret|=oldfar::FPS_SHOWSORTMODELETTER;

			return ret;
		}
		case oldfar::ACTL_GETINTERFACESETTINGS:
		{
			INT_PTR is = FarAdvControl(ModuleNumber, ACTL_GETINTERFACESETTINGS, 0);
			INT_PTR ret = 0;

			if (is&FIS_CLOCKINPANELS)                  ret|=oldfar::FIS_CLOCKINPANELS;

			if (is&FIS_CLOCKINVIEWERANDEDITOR)         ret|=oldfar::FIS_CLOCKINVIEWERANDEDITOR;

			if (is&FIS_MOUSE)                          ret|=oldfar::FIS_MOUSE;

			if (is&FIS_SHOWKEYBAR)                     ret|=oldfar::FIS_SHOWKEYBAR;

			if (is&FIS_ALWAYSSHOWMENUBAR)              ret|=oldfar::FIS_ALWAYSSHOWMENUBAR;

			if (is&FIS_SHOWTOTALCOPYPROGRESSINDICATOR) ret|=oldfar::FIS_SHOWTOTALCOPYPROGRESSINDICATOR;

			if (is&FIS_SHOWCOPYINGTIMEINFO)            ret|=oldfar::FIS_SHOWCOPYINGTIMEINFO;

			if (is&FIS_USECTRLPGUPTOCHANGEDRIVE)       ret|=oldfar::FIS_USECTRLPGUPTOCHANGEDRIVE;

			return ret;
		}
		case oldfar::ACTL_GETCONFIRMATIONS:
		{
			INT_PTR cs = FarAdvControl(ModuleNumber, ACTL_GETCONFIRMATIONS, 0);
			INT_PTR ret = 0;

			if (cs&FCS_COPYOVERWRITE)          ret|=oldfar::FCS_COPYOVERWRITE;

			if (cs&FCS_MOVEOVERWRITE)          ret|=oldfar::FCS_MOVEOVERWRITE;

			if (cs&FCS_DRAGANDDROP)            ret|=oldfar::FCS_DRAGANDDROP;

			if (cs&FCS_DELETE)                 ret|=oldfar::FCS_DELETE;

			if (cs&FCS_DELETENONEMPTYFOLDERS)  ret|=oldfar::FCS_DELETENONEMPTYFOLDERS;

			if (cs&FCS_INTERRUPTOPERATION)     ret|=oldfar::FCS_INTERRUPTOPERATION;

			if (cs&FCS_DISCONNECTNETWORKDRIVE) ret|=oldfar::FCS_DISCONNECTNETWORKDRIVE;

			if (cs&FCS_RELOADEDITEDFILE)       ret|=oldfar::FCS_RELOADEDITEDFILE;

			if (cs&FCS_CLEARHISTORYLIST)       ret|=oldfar::FCS_CLEARHISTORYLIST;

			if (cs&FCS_EXIT)                   ret|=oldfar::FCS_EXIT;

			return ret;
		}
		case oldfar::ACTL_GETDESCSETTINGS:
		{
			INT_PTR ds = FarAdvControl(ModuleNumber, ACTL_GETDESCSETTINGS, 0);
			INT_PTR ret = 0;

			if (ds&FDS_UPDATEALWAYS)      ret|=oldfar::FDS_UPDATEALWAYS;

			if (ds&FDS_UPDATEIFDISPLAYED) ret|=oldfar::FDS_UPDATEIFDISPLAYED;

			if (ds&FDS_SETHIDDEN)         ret|=oldfar::FDS_SETHIDDEN;

			if (ds&FDS_UPDATEREADONLY)    ret|=oldfar::FDS_UPDATEREADONLY;

			return ret;
		}
		case oldfar::ACTL_SETARRAYCOLOR:
		{
			if (!Param) return FALSE;

			oldfar::FarSetColors *scA = (oldfar::FarSetColors *)Param;
			FarSetColors sc = {0, scA->StartIndex, scA->ColorCount, scA->Colors};

			if (scA->Flags&oldfar::FCLR_REDRAW) sc.Flags|=FSETCLR_REDRAW;

			return FarAdvControl(ModuleNumber, ACTL_SETARRAYCOLOR, &sc);
		}
		case oldfar::ACTL_GETWCHARMODE:
			return TRUE;
		case oldfar::ACTL_GETPLUGINMAXREADDATA:
			return FarAdvControl(ModuleNumber, ACTL_GETPLUGINMAXREADDATA, 0);
		case oldfar::ACTL_GETDIALOGSETTINGS:
		{
			INT_PTR ds = FarAdvControl(ModuleNumber, ACTL_GETDIALOGSETTINGS, 0);
			INT_PTR ret = 0;

			if (ds&oldfar::FDIS_HISTORYINDIALOGEDITCONTROLS)    ret|=FDIS_HISTORYINDIALOGEDITCONTROLS;

			if (ds&oldfar::FDIS_HISTORYINDIALOGEDITCONTROLS)    ret|=FDIS_HISTORYINDIALOGEDITCONTROLS;

			if (ds&oldfar::FDIS_PERSISTENTBLOCKSINEDITCONTROLS) ret|=FDIS_PERSISTENTBLOCKSINEDITCONTROLS;

			if (ds&oldfar::FDIS_BSDELETEUNCHANGEDTEXT)          ret|=FDIS_BSDELETEUNCHANGEDTEXT;

			return ret;
		}
		case oldfar::ACTL_REMOVEMEDIA:
		case oldfar::ACTL_GETMEDIATYPE:
			return FALSE;
		case oldfar::ACTL_REDRAWALL:
			return FarAdvControl(ModuleNumber, ACTL_REDRAWALL, 0);
	}

	return FALSE;
}

UINT GetEditorCodePageA()
{
	EditorInfo info={0};
	FarEditorControl(-1,ECTL_GETINFO,0,(INT_PTR)&info);
	UINT CodePage=info.CodePage;
	CPINFO cpi;

	if (!GetCPInfo(CodePage, &cpi) || cpi.MaxCharSize>1)
		CodePage=GetACP();

	return CodePage;
}

int GetEditorCodePageFavA()
{
	UINT CodePage=GetEditorCodePageA(),nCP;
	DWORD selectType,Index=0,FavIndex=2;
	string sTableName;
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
		while (GeneralCfg->EnumValues(FavoriteCodePagesKey,Index++,sTableName,&selectType))
		{
			if (!(selectType&CPST_FAVORITE))
				continue;

			nCP=_wtoi(sTableName);

			if (nCP==CodePage)
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
					if (!GeneralCfg->EnumValues(FavoriteCodePagesKey,Index++,strTableName,&selectType)) return CP_AUTODETECT;

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
		case oldfar::ECTL_GETCOLOR:
			if(Param)
			{
				Command = OldCommand==oldfar::ECTL_ADDCOLOR?ECTL_ADDCOLOR:ECTL_GETCOLOR;
				oldfar::EditorColor* ecA = static_cast<oldfar::EditorColor*>(Param);
				EditorColor ec={};
				ec.StringNumber = ecA->StringNumber;
				if(Command == ECTL_ADDCOLOR)
				{
					ec.StartPos = ecA->StartPos;
					ec.EndPos = ecA->EndPos;
					Colors::ColorToFarColor(ecA->Color,ec.Color);
					if(ecA->Color&oldfar::ECF_TAB1) ec.Color.Flags|=ECF_TAB1;
				}
				else
				{
					ec.ColorItem = ecA->ColorItem;
				}
				int Result = FarEditorControl(-1, Command, 0, reinterpret_cast<INT_PTR>(&ec));
				if(Result && Command == ECTL_GETCOLOR)
				{
					ecA->StartPos = ec.StartPos;
					ecA->EndPos = ec.EndPos;
					ecA->Color = Colors::FarColorToColor(ec.Color);
					if(ec.Color.Flags&ECF_TAB1) ecA->Color|=oldfar::ECF_TAB1;
				}
				return Result;
			}
			return FALSE;

		case oldfar::ECTL_GETSTRING:
		{
			EditorGetString egs;
			oldfar::EditorGetString *oegs=(oldfar::EditorGetString *)Param;

			if (!oegs) return FALSE;

			egs.StringNumber=oegs->StringNumber;
			int ret=FarEditorControl(-1,ECTL_GETSTRING,0,(INT_PTR)&egs);

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
			return FarEditorControl(-1,ECTL_INSERTTEXT,0,(INT_PTR)strP.CPtr());
		}
		case oldfar::ECTL_GETINFO:
		{
			EditorInfo ei={0};
			oldfar::EditorInfo *oei=(oldfar::EditorInfo *)Param;

			if (!oei)
				return FALSE;

			int ret=FarEditorControl(-1,ECTL_GETINFO,0,(INT_PTR)&ei);

			if (ret)
			{
				if (fn)
					xf_free(fn);

				memset(oei,0,sizeof(*oei));
				size_t FileNameSize=FarEditorControl(-1,ECTL_GETFILENAME,0,0);

				if (FileNameSize)
				{
					LPWSTR FileName=new wchar_t[FileNameSize];
					FarEditorControl(-1,ECTL_GETFILENAME,0,(INT_PTR)FileName);
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
			EditorSaveFile newsf = {0,0};

			if (Param)
			{
				oldfar::EditorSaveFile *oldsf = (oldfar::EditorSaveFile*) Param;
				newsf.FileName=(oldsf->FileName)?AnsiToUnicode(oldsf->FileName):nullptr;
				newsf.FileEOL=(oldsf->FileEOL)?AnsiToUnicode(oldsf->FileEOL):nullptr;
			}

			int ret = FarEditorControl(-1,ECTL_SAVEFILE, 0, Param?(INT_PTR)&newsf:0);

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

			return FarEditorControl(-1,ECTL_PROCESSINPUT, 0, (INT_PTR)Param);
		}
		case oldfar::ECTL_PROCESSKEY:
		{
			return FarEditorControl(-1,ECTL_PROCESSKEY, 0, (INT_PTR)(DWORD_PTR)OldKeyToKey((DWORD)(DWORD_PTR)Param));
		}
		case oldfar::ECTL_READINPUT:	//BUGBUG?
		{
			int ret = FarEditorControl(-1,ECTL_READINPUT, 0, (INT_PTR)Param);

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
			switch ((INT_PTR)Param)
			{
				case 0:
				case -1:
					return FarEditorControl(-1,ECTL_SETKEYBAR, 0, (INT_PTR)Param);
				default:
					oldfar::KeyBarTitles* oldkbt = (oldfar::KeyBarTitles*)Param;
					KeyBarTitles newkbt;
					ConvertKeyBarTitlesA(oldkbt, &newkbt);
					int ret = FarEditorControl(-1,ECTL_SETKEYBAR, 0, (INT_PTR)&newkbt);
					FreeUnicodeKeyBarTitles(&newkbt);
					return ret;
			}
		}
		case oldfar::ECTL_SETPARAM:
		{
			EditorSetParameter newsp = {};

			if (Param)
			{
				oldfar::EditorSetParameter *oldsp= (oldfar::EditorSetParameter*) Param;
				newsp.iParam = oldsp->iParam;

				switch (oldsp->Type)
				{
					case oldfar::ESPT_AUTOINDENT:				newsp.Type = ESPT_AUTOINDENT; break;
					case oldfar::ESPT_CHARCODEBASE:			newsp.Type = ESPT_CHARCODEBASE; break;
					case oldfar::ESPT_CURSORBEYONDEOL:	newsp.Type = ESPT_CURSORBEYONDEOL; break;
					case oldfar::ESPT_LOCKMODE:					newsp.Type = ESPT_LOCKMODE; break;
					case oldfar::ESPT_SAVEFILEPOSITION:	newsp.Type = ESPT_SAVEFILEPOSITION; break;
					case oldfar::ESPT_TABSIZE:					newsp.Type = ESPT_TABSIZE; break;
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

								if ((UINT)newsp.iParam==CP_AUTODETECT) return FALSE;

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
						int ret = FarEditorControl(-1,ECTL_SETPARAM, 0, (INT_PTR)&newsp);

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
						newsp.Size = FarEditorControl(-1,ECTL_SETPARAM, 0, (INT_PTR)&newsp);
						newsp.wszParam = (wchar_t*)xf_malloc(newsp.Size*sizeof(wchar_t));

						if (newsp.wszParam)
						{
							int ret = FarEditorControl(-1,ECTL_SETPARAM, 0, (INT_PTR)&newsp);
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

			return FarEditorControl(-1,ECTL_SETPARAM, 0, Param?(INT_PTR)&newsp:0);
		}
		case oldfar::ECTL_SETSTRING:
		{
			EditorSetString newss = {0,0,0,0};

			if (Param)
			{
				oldfar::EditorSetString *oldss = (oldfar::EditorSetString*) Param;
				newss.StringNumber=oldss->StringNumber;
				UINT CodePage=GetEditorCodePageA();
				newss.StringText=(oldss->StringText)?AnsiToUnicodeBin(oldss->StringText, oldss->StringLength,CodePage):nullptr;
				newss.StringEOL=(oldss->StringEOL)?AnsiToUnicode(oldss->StringEOL,CodePage):nullptr;
				newss.StringLength=oldss->StringLength;
			}

			int ret = FarEditorControl(-1,ECTL_SETSTRING, 0, Param?(INT_PTR)&newss:0);

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

			int ret = FarEditorControl(-1,ECTL_SETTITLE, 0, (INT_PTR)newtit);

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
		case oldfar::ECTL_TURNOFFMARKINGBLOCK:	Command = ECTL_TURNOFFMARKINGBLOCK; break;
		case oldfar::ECTL_ADDSTACKBOOKMARK:			Command = ECTL_ADDSTACKBOOKMARK; break;
		case oldfar::ECTL_PREVSTACKBOOKMARK:		Command = ECTL_PREVSTACKBOOKMARK; break;
		case oldfar::ECTL_NEXTSTACKBOOKMARK:		Command = ECTL_NEXTSTACKBOOKMARK; break;
		case oldfar::ECTL_CLEARSTACKBOOKMARKS:	Command = ECTL_CLEARSTACKBOOKMARKS; break;
		case oldfar::ECTL_DELETESTACKBOOKMARK:	Command = ECTL_DELETESTACKBOOKMARK; break;
		case oldfar::ECTL_GETSTACKBOOKMARKS:		Command = ECTL_GETSTACKBOOKMARKS; break;
		default:
			return FALSE;
	}
	return FarEditorControl(-1, Command, 0, reinterpret_cast<INT_PTR>(Param));
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

			ViewerInfo viW;
			viW.StructSize = sizeof(ViewerInfo); //BUGBUG?

			if (FarViewerControl(-1,VCTL_GETINFO, 0, (INT_PTR)&viW) == FALSE) return FALSE;

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
			return FarViewerControl(-1,VCTL_QUIT,0, 0);
		case oldfar::VCTL_REDRAW:
			return FarViewerControl(-1,VCTL_REDRAW,0, 0);
		case oldfar::VCTL_SETKEYBAR:
		{
			switch ((INT_PTR)Param)
			{
				case 0:
				case -1:
					return FarViewerControl(-1,VCTL_SETKEYBAR,0, (INT_PTR)Param);
				default:
					oldfar::KeyBarTitles* kbtA = (oldfar::KeyBarTitles*)Param;
					KeyBarTitles kbt;
					ConvertKeyBarTitlesA(kbtA, &kbt);
					int ret=FarViewerControl(-1,VCTL_SETKEYBAR,0, (INT_PTR)&kbt);
					FreeUnicodeKeyBarTitles(&kbt);
					return ret;
			}
		}
		case oldfar::VCTL_SETPOSITION:
		{
			if (!Param) return FALSE;

			oldfar::ViewerSetPosition* vspA = (oldfar::ViewerSetPosition*)Param;
			ViewerSetPosition vsp;
			vsp.Flags = 0;

			if (vspA->Flags&oldfar::VSP_NOREDRAW)    vsp.Flags|=VSP_NOREDRAW;

			if (vspA->Flags&oldfar::VSP_PERCENT)     vsp.Flags|=VSP_PERCENT;

			if (vspA->Flags&oldfar::VSP_RELATIVE)    vsp.Flags|=VSP_RELATIVE;

			if (vspA->Flags&oldfar::VSP_NORETNEWPOS) vsp.Flags|=VSP_NORETNEWPOS;

			vsp.StartPos = vspA->StartPos;
			vsp.LeftPos = vspA->LeftPos;
			int ret = FarViewerControl(-1,VCTL_SETPOSITION,0, (INT_PTR)&vsp);
			vspA->StartPos = vsp.StartPos;
			return ret;
		}
		case oldfar::VCTL_SELECT:
		{
			if (!Param) return FarViewerControl(-1,VCTL_SELECT,0, 0);

			oldfar::ViewerSelect* vsA = (oldfar::ViewerSelect*)Param;
			ViewerSelect vs = {vsA->BlockStartPos,vsA->BlockLen};
			return FarViewerControl(-1,VCTL_SELECT,0, (INT_PTR)&vs);
		}
		case oldfar::VCTL_SETMODE:
		{
			if (!Param) return FALSE;

			oldfar::ViewerSetMode* vsmA = (oldfar::ViewerSetMode*)Param;
			ViewerSetMode vsm={};

			switch (vsmA->Type)
			{
				case oldfar::VSMT_HEX:      vsm.Type = VSMT_HEX;      break;
				case oldfar::VSMT_WRAP:     vsm.Type = VSMT_WRAP;     break;
				case oldfar::VSMT_WORDWRAP: vsm.Type = VSMT_WORDWRAP; break;
			}

			vsm.iParam = vsmA->iParam;

			if (vsmA->Flags&oldfar::VSMFL_REDRAW) vsm.Flags|=VSMFL_REDRAW;

			return FarViewerControl(-1,VCTL_SETMODE,0, (INT_PTR)&vsm);
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

		FormatString sTableName;
		UINT nCP = ConvertCharTableToCodePage(Command);

		if (nCP==CP_AUTODETECT) return -1;

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
		sTableName<<fmt::Width(5)<<nCP<<BoxSymbols[BS_V1]<<L" "<<codePageName;
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

	Xlat(strLine.GetBuffer(),StartPos,EndPos,NewFlags);
	strLine.ReleaseBuffer();
	strLine.GetCharString(Line,strLine.GetLength()+1);
	return Line;
}

int WINAPI GetFileOwnerA(const char *Computer,const char *Name, char *Owner)
{
	string strComputer(Computer), strName(Name), strOwner;
	int Ret=GetFileOwner(strComputer,strName,strOwner);
	strOwner.GetCharString(Owner,oldfar::NM);
	return Ret;
}

static BOOL PrepareModulePath(const wchar_t *ModuleName)
{
	string strModulePath;
	strModulePath = ModuleName;
	CutToSlash(strModulePath); //??
	return FarChDir(strModulePath);
}

static void CheckScreenLock()
{
	if (ScrBuf.GetLockCount() > 0 && !CtrlObject->Macro.PeekKey())
	{
		ScrBuf.SetLockCount(0);
		ScrBuf.Flush();
	}
}



PluginA::PluginA(PluginManager *owner, const wchar_t *lpwszModuleName):
	m_owner(owner),
	m_strModuleName(lpwszModuleName),
	m_strCacheName(lpwszModuleName),
	m_hModule(nullptr),
	RootKey(nullptr)
	//more initialization here!!!
{
	wchar_t *p = m_strCacheName.GetBuffer();
	while (*p)
	{
		if (*p == L'\\')
			*p = L'/';

		p++;
	}

	m_strCacheName.ReleaseBuffer();
	ClearExports();
	memset(&PI,0,sizeof(PI));
	memset(&OPI,0,sizeof(OPI));
}

PluginA::~PluginA()
{
	if (RootKey) xf_free(RootKey);

	FreePluginInfo();
	FreeOpenPanelInfo();
	Lang.Close();
}


bool PluginA::LoadFromCache(const FAR_FIND_DATA_EX &FindData)
{
	unsigned __int64 id = PlCacheCfg->GetCacheID(m_strCacheName);

	if (id)
	{
		if (PlCacheCfg->IsPreload(id))   //PF_PRELOAD plugin, skip cache
			return Load();

		{
			string strCurPluginID;
			strCurPluginID.Format(
			    L"%I64x%x%x",
			    FindData.nFileSize,
			    FindData.ftCreationTime.dwLowDateTime,
			    FindData.ftLastWriteTime.dwLowDateTime
			);

			string strPluginID = PlCacheCfg->GetSignature(id);

			if (StrCmp(strPluginID, strCurPluginID))   //одинаковые ли бинарники?
				return false;
		}

		pOpenPanel=(PLUGINOPENPANEL)PlCacheCfg->GetExport(id,wszReg_OpenPanel);
		pOpenFilePlugin=(PLUGINOPENFILEPLUGIN)PlCacheCfg->GetExport(id,wszReg_OpenFilePlugin);
		pSetFindList=(PLUGINSETFINDLIST)PlCacheCfg->GetExport(id,wszReg_SetFindList);
		pProcessEditorInput=(PLUGINPROCESSEDITORINPUT)PlCacheCfg->GetExport(id,wszReg_ProcessEditorInput);
		pProcessEditorEvent=(PLUGINPROCESSEDITOREVENT)PlCacheCfg->GetExport(id,wszReg_ProcessEditorEvent);
		pProcessViewerEvent=(PLUGINPROCESSVIEWEREVENT)PlCacheCfg->GetExport(id,wszReg_ProcessViewerEvent);
		pProcessDialogEvent=(PLUGINPROCESSDIALOGEVENT)PlCacheCfg->GetExport(id,wszReg_ProcessDialogEvent);
		pConfigure=(PLUGINCONFIGURE)PlCacheCfg->GetExport(id,wszReg_Configure);
		WorkFlags.Set(PIWF_CACHED); //too much "cached" flags
		return true;
	}

	return false;
}

bool PluginA::SaveToCache()
{
	if (pGetPluginInfo ||
	        pOpenPanel ||
	        pOpenFilePlugin ||
	        pSetFindList ||
	        pProcessEditorInput ||
	        pProcessEditorEvent ||
	        pProcessViewerEvent ||
	        pProcessDialogEvent)
	{
		PluginInfo Info;
		GetPluginInfo(&Info);

		PlCacheCfg->BeginTransaction();

		PlCacheCfg->DeleteCache(m_strCacheName);
		unsigned __int64 id = PlCacheCfg->CreateCache(m_strCacheName);

		{
			bool bPreload = (Info.Flags & PF_PRELOAD);
			PlCacheCfg->SetPreload(id, bPreload);
			WorkFlags.Change(PIWF_PRELOADED, bPreload);

			if (bPreload)
			{
				PlCacheCfg->EndTransaction();
				return true;
			}
		}

		{
			string strCurPluginID;
			FAR_FIND_DATA_EX fdata;
			apiGetFindDataEx(m_strModuleName, fdata);
			strCurPluginID.Format(
			    L"%I64x%x%x",
			    fdata.nFileSize,
			    fdata.ftCreationTime.dwLowDateTime,
			    fdata.ftLastWriteTime.dwLowDateTime
			);
			PlCacheCfg->SetSignature(id, strCurPluginID);
		}

		GUID guid = {0,0,0,{0,0,0,0,0,0,0,0}};
		for (int i = 0; i < Info.DiskMenu.Count; i++)
		{
			guid.Data1 = i;
			PlCacheCfg->SetDiskMenuItem(id, i, Info.DiskMenu.Strings[i], GuidToStr(guid));
		}

		for (int i = 0; i < Info.PluginMenu.Count; i++)
		{
			guid.Data1 = i;
			PlCacheCfg->SetPluginsMenuItem(id, i, Info.PluginMenu.Strings[i], GuidToStr(guid));
		}

		for (int i = 0; i < Info.PluginConfig.Count; i++)
		{
			guid.Data1 = i;
			PlCacheCfg->SetPluginsConfigMenuItem(id, i, Info.PluginConfig.Strings[i], GuidToStr(guid));
		}

		PlCacheCfg->SetCommandPrefix(id, NullToEmpty(Info.CommandPrefix));
		PlCacheCfg->SetFlags(id, Info.Flags);

		PlCacheCfg->SetExport(id, wszReg_OpenPanel, pOpenPanel!=nullptr);
		PlCacheCfg->SetExport(id, wszReg_OpenFilePlugin, pOpenFilePlugin!=nullptr);
		PlCacheCfg->SetExport(id, wszReg_SetFindList, pSetFindList!=nullptr);
		PlCacheCfg->SetExport(id, wszReg_ProcessEditorInput, pProcessEditorInput!=nullptr);
		PlCacheCfg->SetExport(id, wszReg_ProcessEditorEvent, pProcessEditorEvent!=nullptr);
		PlCacheCfg->SetExport(id, wszReg_ProcessViewerEvent, pProcessViewerEvent!=nullptr);
		PlCacheCfg->SetExport(id, wszReg_ProcessDialogEvent, pProcessDialogEvent!=nullptr);
		PlCacheCfg->SetExport(id, wszReg_Configure, pConfigure!=nullptr);

		PlCacheCfg->EndTransaction();

		return true;
	}

	return false;
}

bool PluginA::LoadData(void)
{
	WorkFlags.Set(PIWF_DATALOADED);
	return true;
}

bool PluginA::Load()
{
	if (WorkFlags.Check(PIWF_DONTLOADAGAIN))
		return false;

	if (!WorkFlags.Check(PIWF_DATALOADED)&&!LoadData())
		return false;

	if (m_hModule)
		return true;

	if (!m_hModule)
	{
		string strCurPath, strCurPlugDiskPath;
		wchar_t Drive[]={0,L' ',L':',0}; // ставим 0, как признак того, что вертать обратно ненадо!
		apiGetCurrentDirectory(strCurPath);

		if (IsLocalPath(m_strModuleName))  // если указан локальный путь, то...
		{
			Drive[0] = L'=';
			Drive[1] = m_strModuleName.At(0);
			apiGetEnvironmentVariable(Drive,strCurPlugDiskPath);
		}

		PrepareModulePath(m_strModuleName);
		m_hModule = LoadLibraryEx(m_strModuleName,nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);
		GuardLastError Err;
		FarChDir(strCurPath);

		if (Drive[0]) // вернем ее (переменную окружения) обратно
			SetEnvironmentVariable(Drive,strCurPlugDiskPath);
	}

	if (!m_hModule)
	{
		//чтоб не пытаться загрузить опять а то ошибка будет постоянно показываться.
		WorkFlags.Set(PIWF_DONTLOADAGAIN);

		if (!Opt.LoadPlug.SilentLoadPlugin) //убрать в PluginSet
		{
			SetMessageHelp(L"ErrLoadPlugin");
			Message(MSG_WARNING|MSG_ERRORTYPE|MSG_NOPLUGINS,1,MSG(MError),MSG(MPlgLoadPluginError),m_strModuleName,MSG(MOk));
		}

		return false;
	}

	WorkFlags.Clear(PIWF_CACHED);
	pSetStartupInfo=(PLUGINSETSTARTUPINFO)GetProcAddress(m_hModule,NFMP_SetStartupInfo);
	pOpenPanel=(PLUGINOPENPANEL)GetProcAddress(m_hModule,NFMP_OpenPanel);
	pOpenFilePlugin=(PLUGINOPENFILEPLUGIN)GetProcAddress(m_hModule,NFMP_OpenFilePlugin);
	pClosePanel=(PLUGINCLOSEPANEL)GetProcAddress(m_hModule,NFMP_ClosePanel);
	pGetPluginInfo=(PLUGINGETPLUGININFO)GetProcAddress(m_hModule,NFMP_GetPluginInfo);
	pGetOpenPanelInfo=(PLUGINGETOPENPANELINFO)GetProcAddress(m_hModule,NFMP_GetOpenPanelInfo);
	pGetFindData=(PLUGINGETFINDDATA)GetProcAddress(m_hModule,NFMP_GetFindData);
	pFreeFindData=(PLUGINFREEFINDDATA)GetProcAddress(m_hModule,NFMP_FreeFindData);
	pGetVirtualFindData=(PLUGINGETVIRTUALFINDDATA)GetProcAddress(m_hModule,NFMP_GetVirtualFindData);
	pFreeVirtualFindData=(PLUGINFREEVIRTUALFINDDATA)GetProcAddress(m_hModule,NFMP_FreeVirtualFindData);
	pSetDirectory=(PLUGINSETDIRECTORY)GetProcAddress(m_hModule,NFMP_SetDirectory);
	pGetFiles=(PLUGINGETFILES)GetProcAddress(m_hModule,NFMP_GetFiles);
	pPutFiles=(PLUGINPUTFILES)GetProcAddress(m_hModule,NFMP_PutFiles);
	pDeleteFiles=(PLUGINDELETEFILES)GetProcAddress(m_hModule,NFMP_DeleteFiles);
	pMakeDirectory=(PLUGINMAKEDIRECTORY)GetProcAddress(m_hModule,NFMP_MakeDirectory);
	pProcessHostFile=(PLUGINPROCESSHOSTFILE)GetProcAddress(m_hModule,NFMP_ProcessHostFile);
	pSetFindList=(PLUGINSETFINDLIST)GetProcAddress(m_hModule,NFMP_SetFindList);
	pConfigure=(PLUGINCONFIGURE)GetProcAddress(m_hModule,NFMP_Configure);
	pExitFAR=(PLUGINEXITFAR)GetProcAddress(m_hModule,NFMP_ExitFAR);
	pProcessKey=(PLUGINPROCESSKEY)GetProcAddress(m_hModule,NFMP_ProcessKey);
	pProcessEvent=(PLUGINPROCESSEVENT)GetProcAddress(m_hModule,NFMP_ProcessEvent);
	pCompare=(PLUGINCOMPARE)GetProcAddress(m_hModule,NFMP_Compare);
	pProcessEditorInput=(PLUGINPROCESSEDITORINPUT)GetProcAddress(m_hModule,NFMP_ProcessEditorInput);
	pProcessEditorEvent=(PLUGINPROCESSEDITOREVENT)GetProcAddress(m_hModule,NFMP_ProcessEditorEvent);
	pProcessViewerEvent=(PLUGINPROCESSVIEWEREVENT)GetProcAddress(m_hModule,NFMP_ProcessViewerEvent);
	pProcessDialogEvent=(PLUGINPROCESSDIALOGEVENT)GetProcAddress(m_hModule,NFMP_ProcessDialogEvent);
	pMinFarVersion=(PLUGINMINFARVERSION)GetProcAddress(m_hModule,NFMP_GetMinFarVersion);
	bool bUnloaded = false;

	if (!CheckMinFarVersion(bUnloaded) || !SetStartupInfo(bUnloaded))
	{
		if (!bUnloaded)
			Unload();

		//чтоб не пытаться загрузить опять а то ошибка будет постоянно показываться.
		WorkFlags.Set(PIWF_DONTLOADAGAIN);

		return false;
	}

	FuncFlags.Set(PICFF_LOADED);
	SaveToCache();
	return true;
}

static void CreatePluginStartupInfoA(PluginA *pPlugin, oldfar::PluginStartupInfo *PSI, oldfar::FarStandardFunctions *FSF)
{
	static oldfar::PluginStartupInfo StartupInfo={0};
	static oldfar::FarStandardFunctions StandardFunctions={0};

	// заполняем структуру StandardFunctions один раз!!!
	if (!StandardFunctions.StructSize)
	{
		StandardFunctions.StructSize=sizeof(StandardFunctions);
		StandardFunctions.sprintf=sprintf;
		StandardFunctions.snprintf=_snprintf;
		StandardFunctions.sscanf=sscanf;
		StandardFunctions.qsort=FarQsort;
		StandardFunctions.qsortex=FarQsortEx;
		StandardFunctions.atoi=FarAtoiA;
		StandardFunctions.atoi64=FarAtoi64A;
		StandardFunctions.itoa=FarItoaA;
		StandardFunctions.itoa64=FarItoa64A;
		StandardFunctions.bsearch=FarBsearch;
		StandardFunctions.LIsLower   =LocalIslower;
		StandardFunctions.LIsUpper   =LocalIsupper;
		StandardFunctions.LIsAlpha   =LocalIsalpha;
		StandardFunctions.LIsAlphanum=LocalIsalphanum;
		StandardFunctions.LUpper     =LocalUpper;
		StandardFunctions.LUpperBuf  =LocalUpperBuf;
		StandardFunctions.LLowerBuf  =LocalLowerBuf;
		StandardFunctions.LLower     =LocalLower;
		StandardFunctions.LStrupr    =LocalStrupr;
		StandardFunctions.LStrlwr    =LocalStrlwr;
		StandardFunctions.LStricmp   =LStricmp;
		StandardFunctions.LStrnicmp  =LStrnicmp;
		StandardFunctions.Unquote=UnquoteA;
		StandardFunctions.LTrim=RemoveLeadingSpacesA;
		StandardFunctions.RTrim=RemoveTrailingSpacesA;
		StandardFunctions.Trim=RemoveExternalSpacesA;
		StandardFunctions.TruncStr=TruncStrA;
		StandardFunctions.TruncPathStr=TruncPathStrA;
		StandardFunctions.QuoteSpaceOnly=QuoteSpaceOnlyA;
		StandardFunctions.PointToName=PointToNameA;
		StandardFunctions.GetPathRoot=GetPathRootA;
		StandardFunctions.AddEndSlash=AddEndSlashA;
		StandardFunctions.CopyToClipboard=CopyToClipboardA;
		StandardFunctions.PasteFromClipboard=PasteFromClipboardA;
		StandardFunctions.FarKeyToName=FarKeyToNameA;
		StandardFunctions.FarNameToKey=KeyNameToKeyA;
		StandardFunctions.FarInputRecordToKey=InputRecordToKeyA;
		StandardFunctions.XLat=XlatA;
		StandardFunctions.GetFileOwner=GetFileOwnerA;
		StandardFunctions.GetNumberOfLinks=GetNumberOfLinksA;
		StandardFunctions.FarRecursiveSearch=FarRecursiveSearchA;
		StandardFunctions.MkTemp=FarMkTempA;
		StandardFunctions.DeleteBuffer=DeleteBufferA;
		StandardFunctions.ProcessName=ProcessNameA;
		StandardFunctions.MkLink=FarMkLinkA;
		StandardFunctions.ConvertNameToReal=ConvertNameToRealA;
		StandardFunctions.GetReparsePointInfo=FarGetReparsePointInfoA;
		StandardFunctions.ExpandEnvironmentStr=ExpandEnvironmentStrA;
	}

	if (!StartupInfo.StructSize)
	{
		StartupInfo.StructSize=sizeof(StartupInfo);
		StartupInfo.Menu=FarMenuFnA;
		StartupInfo.Dialog=FarDialogFnA;
		StartupInfo.GetMsg=FarGetMsgFnA;
		StartupInfo.Message=FarMessageFnA;
		StartupInfo.Control=FarControlA;
		StartupInfo.SaveScreen=FarSaveScreen;
		StartupInfo.RestoreScreen=FarRestoreScreen;
		StartupInfo.GetDirList=FarGetDirListA;
		StartupInfo.GetPluginDirList=FarGetPluginDirListA;
		StartupInfo.FreeDirList=FarFreeDirListA;
		StartupInfo.Viewer=FarViewerA;
		StartupInfo.Editor=FarEditorA;
		StartupInfo.CmpName=FarCmpNameA;
		StartupInfo.CharTable=FarCharTableA;
		StartupInfo.Text=FarTextA;
		StartupInfo.EditorControl=FarEditorControlA;
		StartupInfo.ViewerControl=FarViewerControlA;
		StartupInfo.ShowHelp=FarShowHelpA;
		StartupInfo.AdvControl=FarAdvControlA;
		StartupInfo.DialogEx=FarDialogExA;
		StartupInfo.SendDlgMessage=FarSendDlgMessageA;
		StartupInfo.DefDlgProc=FarDefDlgProcA;
		StartupInfo.InputBox=FarInputBoxA;
	}

	*PSI=StartupInfo;
	*FSF=StandardFunctions;
	PSI->ModuleNumber=(INT_PTR)pPlugin;
	PSI->FSF=FSF;
	pPlugin->GetModuleName().GetCharString(PSI->ModuleName,sizeof(PSI->ModuleName));
	PSI->RootKey=nullptr;
}

struct ExecuteStruct
{
	int id; //function id
	union
	{
		INT_PTR nResult;
		HANDLE hResult;
		BOOL bResult;
	};

	union
	{
		INT_PTR nDefaultResult;
		HANDLE hDefaultResult;
		BOOL bDefaultResult;
	};

	bool bUnloaded;
};


static UINT64 OEMApiCnt=0;

void apiSetFileApisToOEM()
{
	SetFileApisToOEM();
	OEMApiCnt++;
}

void apiRevertFileApis()
{
	OEMApiCnt--;
	if(!OEMApiCnt)
	{
		SetFileApisToANSI();
	}
}

#define EXECUTE_FUNCTION(function, es) \
	{ \
		apiSetFileApisToOEM(); \
		es.nResult = 0; \
		es.nDefaultResult = 0; \
		es.bUnloaded = false; \
		if ( Opt.ExceptRules ) \
		{ \
			__try \
			{ \
				function; \
			} \
			__except(xfilter(es.id, GetExceptionInformation(), this, 0)) \
			{ \
				m_owner->UnloadPlugin(this, es.id, true); \
				es.bUnloaded = true; \
				ProcessException=FALSE; \
			} \
		} \
		else \
		{ \
			function; \
		} \
		apiRevertFileApis(); \
	}


#define EXECUTE_FUNCTION_EX(function, es) \
	{ \
		apiSetFileApisToOEM(); \
		es.bUnloaded = false; \
		es.nResult = 0; \
		if ( Opt.ExceptRules ) \
		{ \
			__try \
			{ \
				es.nResult = (INT_PTR)function; \
			} \
			__except(xfilter(es.id, GetExceptionInformation(), this, 0)) \
			{ \
				m_owner->UnloadPlugin(this, es.id, true); \
				es.bUnloaded = true; \
				es.nResult = es.nDefaultResult; \
				ProcessException=FALSE; \
			} \
		} \
		else \
		{ \
			es.nResult = (INT_PTR)function; \
		} \
		apiRevertFileApis(); \
	}


bool PluginA::SetStartupInfo(bool &bUnloaded)
{
	if (pSetStartupInfo && !ProcessException)
	{
		oldfar::PluginStartupInfo _info;
		oldfar::FarStandardFunctions _fsf;
		CreatePluginStartupInfoA(this, &_info, &_fsf);
		// скорректирем адреса и плагино-зависимые поля
		strRootKey = Opt.strRegRoot;
		strRootKey += L"\\Plugins";
		RootKey = UnicodeToAnsi(strRootKey);
		_info.RootKey = RootKey;
		ExecuteStruct es;
		es.id = EXCEPT_SETSTARTUPINFO;
		EXECUTE_FUNCTION(pSetStartupInfo(&_info), es);

		if (es.bUnloaded)
		{
			bUnloaded = true;
			return false;
		}
	}

	return true;
}

bool PluginA::CheckMinFarVersion(bool &bUnloaded)
{
	if (pMinFarVersion && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_MINFARVERSION;
		es.nDefaultResult = 0;
		EXECUTE_FUNCTION_EX(pMinFarVersion(), es);

		if (es.bUnloaded)
		{
			bUnloaded = true;
			return false;
		}
	}

	return true;
}

int PluginA::Unload(bool bExitFAR)
{
	int nResult = TRUE;

	if (bExitFAR)
		ExitFAR();

	if (!WorkFlags.Check(PIWF_CACHED))
	{
		nResult = FreeLibrary(m_hModule);
		ClearExports();
	}

	m_hModule = nullptr;
	FuncFlags.Clear(PICFF_LOADED); //??
	return nResult;
}

bool PluginA::IsPanelPlugin()
{
	return pSetFindList ||
	       pGetFindData ||
	       pGetVirtualFindData ||
	       pSetDirectory ||
	       pGetFiles ||
	       pPutFiles ||
	       pDeleteFiles ||
	       pMakeDirectory ||
	       pProcessHostFile ||
	       pProcessKey ||
	       pProcessEvent ||
	       pCompare ||
	       pGetOpenPanelInfo ||
	       pFreeFindData ||
	       pFreeVirtualFindData ||
	       pClosePanel;
}

HANDLE PluginA::Open(int OpenFrom, const GUID& Guid, INT_PTR Item)
{
	ChangePriority *ChPriority = new ChangePriority(THREAD_PRIORITY_NORMAL);

	CheckScreenLock(); //??

	{
//		string strCurDir;
//		CtrlObject->CmdLine->GetCurDir(strCurDir);
//		FarChDir(strCurDir);
		g_strDirToSet.Clear();
	}

	HANDLE hResult = INVALID_HANDLE_VALUE;

	if (Load() && pOpenPanel && !ProcessException)
	{
		//CurPluginItem=this; //BUGBUG
		ExecuteStruct es;
		es.id = EXCEPT_OPEN;
		es.hDefaultResult = INVALID_HANDLE_VALUE;
		es.hResult = INVALID_HANDLE_VALUE;
		char *ItemA = nullptr;

		if (Item && (OpenFrom == OPEN_COMMANDLINE  || OpenFrom == OPEN_SHORTCUT))
		{
			ItemA = UnicodeToAnsi((const wchar_t *)Item);
			Item = (INT_PTR)ItemA;
		}
		if (OpenFrom == OPEN_LEFTDISKMENU || OpenFrom == OPEN_RIGHTDISKMENU || OpenFrom == OPEN_PLUGINSMENU || OpenFrom == OPEN_EDITOR || OpenFrom == OPEN_VIEWER)
		{
			Item=Guid.Data1;
		}
		if (OpenFrom == OPEN_RIGHTDISKMENU)
		{
			OpenFrom = OPEN_LEFTDISKMENU;
		}

		EXECUTE_FUNCTION_EX(pOpenPanel(OpenFrom,Item), es);

		if (ItemA) xf_free(ItemA);

		hResult = es.hResult;
		//CurPluginItem=nullptr; //BUGBUG
		/*    CtrlObject->Macro.SetRedrawEditor(TRUE); //BUGBUG

		    if ( !es.bUnloaded )
		    {

		      if(OpenFrom == OPEN_EDITOR &&
		         !CtrlObject->Macro.IsExecuting() &&
		         CtrlObject->Plugins.CurEditor &&
		         CtrlObject->Plugins.CurEditor->IsVisible() )
		      {
		        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
		        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
		        CtrlObject->Plugins.CurEditor->Show();
		      }
		      if (hInternal!=INVALID_HANDLE_VALUE)
		      {
		        PluginHandle *hPlugin=new PluginHandle;
		        hPlugin->InternalHandle=es.hResult;
		        hPlugin->PluginNumber=(INT_PTR)this;
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
    int DataSize,
    int OpMode
)
{
	HANDLE hResult = INVALID_HANDLE_VALUE;

	if (Load() && pOpenFilePlugin && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_OPENFILEPLUGIN;
		es.hDefaultResult = INVALID_HANDLE_VALUE;
		char *NameA = nullptr;

		if (Name)
			NameA = UnicodeToAnsi(Name);

		EXECUTE_FUNCTION_EX(pOpenFilePlugin(NameA, Data, DataSize), es);

		if (NameA) xf_free(NameA);

		hResult = es.hResult;
	}

	return hResult;
}


int PluginA::SetFindList(
    HANDLE hPlugin,
    const PluginPanelItem *PanelItem,
    int ItemsNumber
)
{
	BOOL bResult = FALSE;

	if (pSetFindList && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_SETFINDLIST;
		es.bDefaultResult = FALSE;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		EXECUTE_FUNCTION_EX(pSetFindList(hPlugin, PanelItemA, ItemsNumber), es);
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

	if (Load() && pProcessEditorInput && !ProcessException)
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

		EXECUTE_FUNCTION_EX(pProcessEditorInput(Ptr), es);
		bResult = es.bResult;
	}

	return bResult;
}

int PluginA::ProcessEditorEvent(
    int Event,
    PVOID Param
)
{
	if (Load() && pProcessEditorEvent && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSEDITOREVENT;
		es.nDefaultResult = 0;
		EXECUTE_FUNCTION_EX(pProcessEditorEvent(Event, Param), es);
	}

	return 0; //oops!
}

int PluginA::ProcessViewerEvent(
    int Event,
    void *Param
)
{
	if (Load() && pProcessViewerEvent && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSVIEWEREVENT;
		es.nDefaultResult = 0;
		EXECUTE_FUNCTION_EX(pProcessViewerEvent(Event, Param), es);
	}

	return 0; //oops, again!
}

int PluginA::ProcessDialogEvent(
    int Event,
    void *Param
)
{
	BOOL bResult = FALSE;

	if (Load() && pProcessDialogEvent && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSDIALOGEVENT;
		es.bDefaultResult = FALSE;
		EXECUTE_FUNCTION_EX(pProcessDialogEvent(Event, Param), es);
		bResult = es.bResult;
	}

	return bResult;
}

int PluginA::GetVirtualFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelItem,
    int *pItemsNumber,
    const wchar_t *Path
)
{
	BOOL bResult = FALSE;

	if (pGetVirtualFindData && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETVIRTUALFINDDATA;
		es.bDefaultResult = FALSE;
		pVFDPanelItemA = nullptr;
		size_t Size=StrLength(Path)+1;
		LPSTR PathA=new char[Size];
		UnicodeToOEM(Path,PathA,Size);
		EXECUTE_FUNCTION_EX(pGetVirtualFindData(hPlugin, &pVFDPanelItemA, pItemsNumber, PathA), es);
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
    int ItemsNumber
)
{
	FreeUnicodePanelItem(PanelItem, ItemsNumber);

	if (pFreeVirtualFindData && !ProcessException && pVFDPanelItemA)
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEVIRTUALFINDDATA;
		EXECUTE_FUNCTION(pFreeVirtualFindData(hPlugin, pVFDPanelItemA, ItemsNumber), es);
		pVFDPanelItemA = nullptr;
	}
}



int PluginA::GetFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber,
    int Move,
    const wchar_t **DestPath,
    int OpMode
)
{
	int nResult = -1;

	if (pGetFiles && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETFILES;
		es.nDefaultResult = -1;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		char DestA[oldfar::NM];
		UnicodeToOEM(*DestPath,DestA,sizeof(DestA));
		EXECUTE_FUNCTION_EX(pGetFiles(hPlugin, PanelItemA, ItemsNumber, Move, DestA, OpMode), es);
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
    int ItemsNumber,
    int Move,
    int OpMode
)
{
	int nResult = -1;

	if (pPutFiles && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PUTFILES;
		es.nDefaultResult = -1;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		EXECUTE_FUNCTION_EX(pPutFiles(hPlugin, PanelItemA, ItemsNumber, Move, OpMode), es);
		FreePanelItemA(PanelItemA,ItemsNumber);
		nResult = (int)es.nResult;
	}

	return nResult;
}

int PluginA::DeleteFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (pDeleteFiles && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_DELETEFILES;
		es.bDefaultResult = FALSE;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		EXECUTE_FUNCTION_EX(pDeleteFiles(hPlugin, PanelItemA, ItemsNumber, OpMode), es);
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

	if (pMakeDirectory && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_MAKEDIRECTORY;
		es.nDefaultResult = -1;
		char NameA[oldfar::NM];
		UnicodeToOEM(*Name,NameA,sizeof(NameA));
		EXECUTE_FUNCTION_EX(pMakeDirectory(hPlugin, NameA, OpMode), es);
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
    int ItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (pProcessHostFile && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSHOSTFILE;
		es.bDefaultResult = FALSE;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		EXECUTE_FUNCTION_EX(pProcessHostFile(hPlugin, PanelItemA, ItemsNumber, OpMode), es);
		FreePanelItemA(PanelItemA,ItemsNumber);
		bResult = es.bResult;
	}

	return bResult;
}


int PluginA::ProcessEvent(
    HANDLE hPlugin,
    int Event,
    PVOID Param
)
{
	BOOL bResult = FALSE;

	if (pProcessEvent && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSEVENT;
		es.bDefaultResult = FALSE;
		PVOID ParamA = Param;

		if (Param && (Event == FE_COMMAND || Event == FE_CHANGEVIEWMODE))
			ParamA = (PVOID)UnicodeToAnsi((const wchar_t *)Param);

		EXECUTE_FUNCTION_EX(pProcessEvent(hPlugin, Event, ParamA), es);

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

	if (pCompare && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_COMPARE;
		es.nDefaultResult = -2;
		oldfar::PluginPanelItem *Item1A = nullptr;
		oldfar::PluginPanelItem *Item2A = nullptr;
		ConvertPanelItemsArrayToAnsi(Item1,Item1A,1);
		ConvertPanelItemsArrayToAnsi(Item2,Item2A,1);
		EXECUTE_FUNCTION_EX(pCompare(hPlugin, Item1A, Item2A, Mode), es);
		FreePanelItemA(Item1A,1);
		FreePanelItemA(Item2A,1);
		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginA::GetFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelItem,
    int *pItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (pGetFindData && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETFINDDATA;
		es.bDefaultResult = FALSE;
		pFDPanelItemA = nullptr;
		EXECUTE_FUNCTION_EX(pGetFindData(hPlugin, &pFDPanelItemA, pItemsNumber, OpMode), es);
		bResult = es.bResult;

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
    int ItemsNumber
)
{
	FreeUnicodePanelItem(PanelItem, ItemsNumber);

	if (pFreeFindData && !ProcessException && pFDPanelItemA)
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEFINDDATA;
		EXECUTE_FUNCTION(pFreeFindData(hPlugin, pFDPanelItemA, ItemsNumber), es);
		pFDPanelItemA = nullptr;
	}
}

int PluginA::ProcessKey(HANDLE hPlugin,const INPUT_RECORD *Rec, bool Pred)
{
	BOOL bResult = FALSE;
    int VirtKey;
    int dwControlState;

    //BUGBUG: здесь можно проще.
    TranslateKeyToVK(InputRecordToKey(Rec),VirtKey,dwControlState);
	if (pProcessKey && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSKEY;
		es.bDefaultResult = TRUE; // do not pass this key to far on exception
		EXECUTE_FUNCTION_EX(pProcessKey(hPlugin, VirtKey|(Pred?PKF_PREPROCESS:0), dwControlState), es);
		bResult = es.bResult;
	}

	return bResult;
}


void PluginA::ClosePanel(
    HANDLE hPlugin
)
{
	if (pClosePanel && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_CLOSEPANEL;
		EXECUTE_FUNCTION(pClosePanel(hPlugin), es);
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

	if (pSetDirectory && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_SETDIRECTORY;
		es.bDefaultResult = FALSE;
		char *DirA = UnicodeToAnsi(Dir);
		EXECUTE_FUNCTION_EX(pSetDirectory(hPlugin, DirA, OpMode), es);

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

	memset(&OPI,0,sizeof(OPI));
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

	if (pGetOpenPanelInfo && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETOPENPANELINFO;
		oldfar::OpenPanelInfo InfoA={0};
		EXECUTE_FUNCTION(pGetOpenPanelInfo(hPlugin, &InfoA), es);
		ConvertOpenPanelInfo(InfoA,pInfo);
	}
}


int PluginA::Configure(const GUID& Guid)
{
	BOOL bResult = FALSE;

	if (Load() && pConfigure && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_CONFIGURE;
		es.bDefaultResult = FALSE;
		EXECUTE_FUNCTION_EX(pConfigure(Guid.Data1), es);
		bResult = es.bResult;
	}

	return bResult;
}

void PluginA::FreePluginInfo()
{
	if (PI.DiskMenu.Count)
	{
		for (int i=0; i<PI.DiskMenu.Count; i++)
			xf_free((void *)PI.DiskMenu.Strings[i]);

		xf_free((void *)PI.DiskMenu.Guids);
		xf_free((void *)PI.DiskMenu.Strings);
	}

	if (PI.PluginMenu.Count)
	{
		for (int i=0; i<PI.PluginMenu.Count; i++)
			xf_free((void *)PI.PluginMenu.Strings[i]);

		xf_free((void *)PI.PluginMenu.Guids);
		xf_free((void *)PI.PluginMenu.Strings);
	}

	if (PI.PluginConfig.Count)
	{
		for (int i=0; i<PI.PluginConfig.Count; i++)
			xf_free((void *)PI.PluginConfig.Strings[i]);

		xf_free((void *)PI.PluginConfig.Guids);
		xf_free((void *)PI.PluginConfig.Strings);
	}

	if (PI.CommandPrefix)
		xf_free((void *)PI.CommandPrefix);

	memset(&PI,0,sizeof(PI));
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
	memset(pi, 0, sizeof(PluginInfo));

	if (pGetPluginInfo && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETPLUGININFO;
		oldfar::PluginInfo InfoA={0};
		EXECUTE_FUNCTION(pGetPluginInfo(&InfoA), es);

		if (!es.bUnloaded)
		{
			ConvertPluginInfo(InfoA, pi);
			return true;
		}
	}

	return false;
}

void PluginA::ExitFAR()
{
	if (pExitFAR && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_EXITFAR;
		EXECUTE_FUNCTION(pExitFAR(), es);
	}
}

void PluginA::ClearExports()
{
	pSetStartupInfo=0;
	pOpenPanel=0;
	pOpenFilePlugin=0;
	pClosePanel=0;
	pGetPluginInfo=0;
	pGetOpenPanelInfo=0;
	pGetFindData=0;
	pFreeFindData=0;
	pGetVirtualFindData=0;
	pFreeVirtualFindData=0;
	pSetDirectory=0;
	pGetFiles=0;
	pPutFiles=0;
	pDeleteFiles=0;
	pMakeDirectory=0;
	pProcessHostFile=0;
	pSetFindList=0;
	pConfigure=0;
	pExitFAR=0;
	pProcessKey=0;
	pProcessEvent=0;
	pCompare=0;
	pProcessEditorInput=0;
	pProcessEditorEvent=0;
	pProcessViewerEvent=0;
	pProcessDialogEvent=0;
	pMinFarVersion=0;
}
