/*
findfile.cpp

Поиск (Alt-F7)
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

#include "headers.hpp"
#pragma hdrstop

#include "findfile.hpp"
#include "flink.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "filelist.hpp"
#include "cmdline.hpp"
#include "chgprior.hpp"
#include "namelist.hpp"
#include "scantree.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "CFileMask.hpp"
#include "filefilter.hpp"
#include "farexcpt.hpp"
#include "syslog.hpp"
#include "localOEM.hpp"
#include "codepage.hpp"
#include "registry.hpp"
#include "cddrv.hpp"
#include "TaskBar.hpp"
#include "interf.hpp"
#include "palette.hpp"
#include "message.hpp"
#include "delete.hpp"
#include "datetime.hpp"
#include "drivemix.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "mix.hpp"
#include "constitle.hpp"
#include "DlgGuid.hpp"

#define CHAR_TABLE_SIZE 5

#define LIST_DELTA  64

DWORD LIST_INDEX_NONE = (DWORD)-1;

// Список найденных файлов. Индекс из списка хранится в меню.
struct FINDLIST
{
	FAR_FIND_DATA_EX FindData;
	size_t ArcIndex;
	DWORD Used;
//  BYTE Addons[6];
};

FINDLIST **FindList;
size_t FindListCapacity=0;
size_t FindListCount=0;

// Список архивов. Если файл найден в архиве, то FindList->ArcIndex указывает сюда.
struct ARCLIST
{
	string strArcName;
	HANDLE hPlugin;    // Plugin handle
	DWORD Flags;       // OpenPluginInfo.Flags
	string strRootPath; // Root path in plugin after opening.
};

ARCLIST **ArcList;
size_t ArcListCapacity=0;
size_t ArcListCount=0;

CriticalSection ffCS; // чтобы не ресайзили список найденных файлов, пока мы из него читаем
CriticalSection statusCS; // чтобы не писали в FindMessage/FindFileCount/FindDirCount пока мы оттуда читаем

size_t FindFileArcIndex;
// Используются для отправки файлов на временную панель.
// индекс текущего элемента в списке и флаг для отправки.
DWORD FindExitIndex;
int FindExitCode;

string strFindMask, strFindStr;
int SearchMode,CmpCase,WholeWords,SearchInArchives,SearchHex;

bool FindFoldersChanged;
bool SearchFromChanged;
short DlgWidth, DlgHeight;
volatile int StopSearch,PauseSearch,SearchDone,LastFoundNumber,FindFileCount,FindDirCount;
string strFindMessage;
string strFindPercentMessage;
string strLastDirName;
int FindMessageReady,FindCountReady,FindPositionChanged;
bool FindMessagePercentReady;
string strPluginSearchPath;

int RecurseLevel;
bool BreakMainThread;
int PluginMode;

HANDLE hPluginMutex;

bool UseFilter=false;
UINT CodePage=CP_AUTODETECT;
UINT64 SearchInFirst=0;

#define readBufferSizeA 32768
#define readBufferSize (readBufferSizeA*sizeof(wchar_t))

char *readBufferA;
wchar_t *readBuffer;
int codePagesCount;

struct CodePageInfo
{
	UINT CodePage;
	UINT MaxCharSize;
	wchar_t LastSymbol;
	bool WordFound;
} *codePages;

unsigned char *hexFindString;
size_t hexFindStringSize;
wchar_t *findString,*findStringBuffer;

size_t *skipCharsTable;
int favoriteCodePages = 0;

TaskBar *TB=NULL;

bool InFileSearchInited=false;

CFileMask FileMaskForFindFile;

FileFilter *Filter;

enum
{
	FIND_EXIT_NONE,
	FIND_EXIT_SEARCHAGAIN,
	FIND_EXIT_GOTO,
	FIND_EXIT_PANEL
};

enum ADVANCEDDLG
{
	AD_DOUBLEBOX,
	AD_TEXT_SEARCHFIRST,
	AD_EDIT_SEARCHFIRST,
	AD_CHECKBOX_FINDALTERNATESTREAMS,
	AD_SEPARATOR1,
	AD_BUTTON_OK,
	AD_BUTTON_CANCEL,
};

enum FINDASKDLG
{
	FAD_DOUBLEBOX,
	FAD_TEXT_MASK,
	FAD_EDIT_MASK,
	FAD_SEPARATOR0,
	FAD_TEXT_TEXTHEX,
	FAD_EDIT_TEXT,
	FAD_EDIT_HEX,
	FAD_TEXT_CP,
	FAD_COMBOBOX_CP,
	FAD_SEPARATOR1,
	FAD_CHECKBOX_CASE,
	FAD_CHECKBOX_WHOLEWORDS,
	FAD_CHECKBOX_HEX,
	FAD_CHECKBOX_ARC,
	FAD_CHECKBOX_DIRS,
	FAD_CHECKBOX_LINKS,
	FAD_SEPARATOR_2,
	FAD_SEPARATOR_3,
	FAD_TEXT_WHERE,
	FAD_COMBOBOX_WHERE,
	FAD_SEPARATOR_4,
	FAD_CHECKBOX_FILTER,
	FAD_SEPARATOR_5,
	FAD_BUTTON_FIND,
	FAD_BUTTON_DRIVE,
	FAD_BUTTON_FILTER,
	FAD_BUTTON_ADVANCED,
	FAD_BUTTON_CANCEL,
};

enum FINDASKDLGCOMBO
{
	FADC_ALLDISKS,
	FADC_ALLBUTNET,
	FADC_PATH,
	FADC_ROOT,
	FADC_FROMCURRENT,
	FADC_INCURRENT,
	FADC_SELECTED,
};

enum FINDDLG
{
	FD_DOUBLEBOX,
	FD_LISTBOX,
	FD_SEPARATOR1,
	FD_TEXT_STATUS,
	FD_TEXT_STATUS_PERCENTS,
	FD_SEPARATOR2,
	FD_BUTTON_NEW,
	FD_BUTTON_GOTO,
	FD_BUTTON_VIEW,
	FD_BUTTON_PANEL,
	FD_BUTTON_STOP,
};

/*
int _cdecl SortItems(const void *p1,const void *p2)
{
	PluginPanelItem *Item1=(PluginPanelItem *)p1;
	PluginPanelItem *Item2=(PluginPanelItem *)p2;
	string strN1=Item1->FindData.lpwszFileName;
	string strN2=Item2->FindData.lpwszFileName;
	CutToSlash(strN1);
	CutToSlash(strN2);
	return StrCmpI(strN2,strN1);
}
*/

void InitInFileSearch()
{
	if (!InFileSearchInited && !strFindStr.IsEmpty())
	{
		size_t findStringCount = strFindStr.GetLength();
		// Инициализируем буферы чтения из файла
		readBufferA = (char *)xf_malloc(readBufferSizeA);
		readBuffer = (wchar_t *)xf_malloc(readBufferSize);

		if (!SearchHex)
		{
			// Формируем строку поиска
			if (!CmpCase)
			{
				findStringBuffer = (wchar_t *)xf_malloc(2*findStringCount*sizeof(wchar_t));
				findString=findStringBuffer;

				for (size_t index = 0; index<strFindStr.GetLength(); index++)
				{
					wchar_t ch = strFindStr[index];

					if (IsCharLower(ch))
					{
						findString[index]=Upper(ch);
						findString[index+findStringCount]=ch;
					}
					else
					{
						findString[index]=ch;
						findString[index+findStringCount]=Lower(ch);
					}
				}
			}
			else
				findString = strFindStr.GetBuffer();

			// Инизиализируем данные для алгоритма поиска
			skipCharsTable = (size_t *)xf_malloc((WCHAR_MAX+1)*sizeof(size_t));

			for (size_t index = 0; index < WCHAR_MAX+1; index++)
				skipCharsTable[index] = findStringCount;

			for (size_t index = 0; index < findStringCount-1; index++)
				skipCharsTable[findString[index]] = findStringCount-1-index;

			if (!CmpCase)
				for (size_t index = 0; index < findStringCount-1; index++)
					skipCharsTable[findString[index+findStringCount]] = findStringCount-1-index;

			// Формируем список кодовых страниц
			if (CodePage == CP_AUTODETECT)
			{
				DWORD data;
				string codePageName;
				bool hasSelected = false;

				// Проверяем наличие выбранных страниц символов
				for (int i=0; EnumRegValue(FavoriteCodePagesKey, i, codePageName, (BYTE *)&data, sizeof(data)); i++)
				{
					if (data & CPST_FIND)
					{
						hasSelected = true;
						break;
					}
				}

				// Добавляем стандартные таблицы символов
				if (!hasSelected)
				{
					codePagesCount = StandardCPCount;
					codePages = (CodePageInfo *)xf_malloc(codePagesCount*sizeof(CodePageInfo));
					codePages[0].CodePage = GetOEMCP();
					codePages[1].CodePage = GetACP();
					codePages[2].CodePage = CP_UTF7;
					codePages[3].CodePage = CP_UTF8;
					codePages[4].CodePage = CP_UNICODE;
					codePages[5].CodePage = CP_REVERSEBOM;
				}
				else
				{
					codePagesCount = 0;
					codePages = NULL;
				}

				// Добавляем стандартные таблицы символов
				for (int i=0; EnumRegValue(FavoriteCodePagesKey, i, codePageName, (BYTE *)&data, sizeof(data)); i++)
				{
					if (data & (hasSelected?CPST_FIND:CPST_FAVORITE))
					{
						UINT codePage = _wtoi(codePageName);

						// Проверяем дубли
						if (!hasSelected)
						{
							bool isDouble = false;

							for (int j = 0; j<StandardCPCount; j++)
								if (codePage == codePages[0].CodePage)
								{
									isDouble =true;
									break;
								}

							if (isDouble)
								continue;
						}

						codePages = (CodePageInfo *)xf_realloc((void *)codePages, ++codePagesCount*sizeof(CodePageInfo));
						codePages[codePagesCount-1].CodePage = codePage;
					}
				}
			}
			else
			{
				codePagesCount = 1;
				codePages = (CodePageInfo *)xf_malloc(codePagesCount*sizeof(CodePageInfo));
				codePages[0].CodePage = CodePage;
			}

			for (int index = 0; index<codePagesCount; index++)
			{
				CodePageInfo *cp = codePages+index;

				if (IsUnicodeCodePage(cp->CodePage))
					cp->MaxCharSize = 2;
				else
				{
					CPINFO cpi;

					if (!GetCPInfo(cp->CodePage, &cpi))
						cpi.MaxCharSize = 0; //Считаем, что ошибка и потом такие таблицы в поиске пропускаем

					cp->MaxCharSize = cpi.MaxCharSize;
				}

				cp->LastSymbol = NULL;
				cp->WordFound = false;
			}
		}
		else
		{
			// Формируем hex-строку для поиска
			hexFindStringSize = 0;

			if (SearchHex)
			{
				bool flag = false;
				hexFindString = (unsigned char *)xf_malloc((findStringCount-findStringCount/3+1)/2);

				for (size_t index = 0; index < strFindStr.GetLength(); index++)
				{
					wchar_t symbol = strFindStr.At(index);
					byte offset = 0;

					if (symbol >= L'a' && symbol <= L'f')
						offset = 87;
					else if (symbol >= L'A' && symbol <= L'F')
						offset = 55;
					else if (symbol >= L'0' && symbol <= L'9')
						offset = 48;
					else
						continue;

					if (!flag)
						hexFindString[hexFindStringSize++] = ((byte)symbol-offset)<<4;
					else
						hexFindString[hexFindStringSize-1] |= ((byte)symbol-offset);

					flag = !flag;
				}
			}

			// Инизиализируем данные для аглоритма поиска
			skipCharsTable = (size_t *)xf_malloc((255+1)*sizeof(size_t));

			for (size_t index = 0; index < 255+1; index++)
				skipCharsTable[index] = hexFindStringSize;

			for (size_t index = 0; index < (size_t)hexFindStringSize-1; index++)
				skipCharsTable[hexFindString[index]] = hexFindStringSize-1-index;
		}

		InFileSearchInited=true;
	}
}

void ReleaseInFileSearch()
{
	if (InFileSearchInited && !strFindStr.IsEmpty())
	{
		if (readBufferA)
		{
			xf_free(readBufferA);
			readBufferA=NULL;
		}

		if (readBuffer)
		{
			xf_free(readBuffer);
			readBuffer=NULL;
		}

		if (skipCharsTable)
		{
			xf_free(skipCharsTable);
			skipCharsTable=NULL;
		}

		if (codePages)
		{
			xf_free(codePages);
			codePages=NULL;
		}

		if (findStringBuffer)
		{
			xf_free(findStringBuffer);
			findStringBuffer=NULL;
		}

		if (hexFindString)
		{
			xf_free(hexFindString);
			hexFindString=NULL;
		}

		InFileSearchInited=false;
	}
}

string &PrepareDriveNameStr(string &strSearchFromRoot)
{
	string strCurDir;
	CtrlObject->CmdLine->GetCurDir(strCurDir);
	GetPathRoot(strCurDir,strCurDir);
	DeleteEndSlash(strCurDir);

	if (
	    strCurDir.IsEmpty()||
	    (CtrlObject->Cp()->ActivePanel->GetMode()==PLUGIN_PANEL && CtrlObject->Cp()->ActivePanel->IsVisible())
	)
	{
		strSearchFromRoot = MSG(MSearchFromRootFolder);
	}
	else
	{
		strSearchFromRoot= MSG(MSearchFromRootOfDrive);
		strSearchFromRoot+=L" ";
		strSearchFromRoot+=strCurDir;
	}

	return strSearchFromRoot;
}

// Проверяем символ на принадлежность разделителям слов
bool IsWordDiv(const wchar_t symbol)
{
	// Так же разделителем является конец строки и пробельные символы
	return symbol==0||IsSpace(symbol)||IsEol(symbol)||IsWordDiv(Opt.strWordDiv,symbol);
}

void ClearAllLists()
{
	CriticalSectionLock Lock(ffCS);
	FindFileArcIndex=LIST_INDEX_NONE;

	if (FindList)
	{
		for (size_t i=0; i<FindListCount; i++)
		{
			delete FindList[i];
		}

		xf_free(FindList);
	}

	FindList=NULL;
	FindListCapacity=FindListCount=0;

	if (ArcList)
	{
		for (size_t i=0; i<ArcListCount; i++)
		{
			delete ArcList[i];
		}

		xf_free(ArcList);
	}

	ArcList=NULL;
	ArcListCapacity=ArcListCount=0;
}

bool FindListGrow()
{
	bool Result=false;
	size_t Delta=(FindListCapacity<256)?LIST_DELTA:FindListCapacity/2;
	FINDLIST** NewList=reinterpret_cast<FINDLIST**>(xf_realloc(FindList,(FindListCapacity+Delta)*sizeof(*FindList)));

	if (NewList)
	{
		FindList=NewList;
		FindListCapacity+=Delta;
		Result=true;
	}

	return Result;
}

bool ArcListGrow()
{
	bool Result=false;
	size_t Delta=(ArcListCapacity<256)?LIST_DELTA:ArcListCapacity/2;
	ARCLIST** NewList=reinterpret_cast<ARCLIST**>(xf_realloc(ArcList,(ArcListCapacity+Delta)*sizeof(*ArcList)));

	if (NewList)
	{
		ArcList = NewList;
		ArcListCapacity+= Delta;
		Result=true;
	}

	return Result;
}

void SetPluginDirectory(const wchar_t *DirName,HANDLE hPlugin,bool UpdatePanel=false)
{
	if (DirName && *DirName)
	{
		string strName(DirName);
		wchar_t* DirPtr = strName.GetBuffer();
		wchar_t* NamePtr = (wchar_t*) PointToName(DirPtr);

		if (NamePtr != DirPtr)
		{
			*(NamePtr-1) = 0;
			// force plugin to update its file list (that can be empty at this time)
			// if not done SetDirectory may fail
			{
				int FileCount=0;
				PluginPanelItem *PanelData=NULL;

				if (CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&FileCount,OPM_SILENT))
				{
					CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,FileCount);
				}
			}

			if (*DirPtr)
			{
				CtrlObject->Plugins.SetDirectory(hPlugin,DirPtr,OPM_SILENT);
			}
			else
			{
				CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_SILENT);
			}
		}

		// Отрисуем панель при необходимости.
		if (UpdatePanel)
		{
			CtrlObject->Cp()->ActivePanel->Update(UPDATE_KEEP_SELECTION);
			CtrlObject->Cp()->ActivePanel->GoToFile(NamePtr);
			CtrlObject->Cp()->ActivePanel->Show();
		}

		//strName.ReleaseBuffer(); Не надо. Строка все ровно удаляется, лишний вызов StrLength.
	}
}

LONG_PTR WINAPI AdvancedDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	switch (Msg)
	{
		case DN_CLOSE:

			if (Param1==AD_BUTTON_OK)
			{
				LPCWSTR Data=reinterpret_cast<LPCWSTR>(SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,AD_EDIT_SEARCHFIRST,NULL));

				if (Data && *Data && !CheckFileSizeStringFormat(Data))
				{
					Message(MSG_WARNING,1,MSG(MFindFileAdvancedTitle),MSG(MBadFileSizeFormat),MSG(MOk));
					return FALSE;
				}
			}

			break;
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

void AdvancedDialog()
{
	DialogDataEx AdvancedDlgData[]=
	{
		/* 00 */DI_DOUBLEBOX,3,1,52,7,0,0,0,0,MSG(MFindFileAdvancedTitle),
		/* 01 */DI_TEXT,5,2,0,2,0,0,0,0,MSG(MFindFileSearchFirst),
		/* 02 */DI_EDIT,5,3,50,3,0,0,0,0,Opt.FindOpt.strSearchInFirstSize,
		/* 03 */DI_CHECKBOX,5,4,0,4,0,Opt.FindOpt.FindAlternateStreams,0,0,MSG(MFindAlternateStreams),
		/* 04 */DI_TEXT,3,5,0,5,0,0,DIF_SEPARATOR,0,L"",
		/* 05 */DI_BUTTON,0,6,0,6,0,0,DIF_CENTERGROUP,1,MSG(MOk),
		/* 06 */DI_BUTTON,0,6,0,6,0,0,DIF_CENTERGROUP,0,MSG(MCancel),
	};
	MakeDialogItemsEx(AdvancedDlgData,AdvancedDlg);
	Dialog Dlg(AdvancedDlg,countof(AdvancedDlg),AdvancedDlgProc);
	Dlg.SetHelp(L"FindFileAdvanced");
	Dlg.SetPosition(-1,-1,52+4,7+2);
	Dlg.Process();
	int ExitCode=Dlg.GetExitCode();

	if (ExitCode==AD_BUTTON_OK)
	{
		Opt.FindOpt.strSearchInFirstSize = AdvancedDlg[AD_EDIT_SEARCHFIRST].strData;
		SearchInFirst=ConvertFileSizeString(Opt.FindOpt.strSearchInFirstSize);
		Opt.FindOpt.FindAlternateStreams=(AdvancedDlg[AD_CHECKBOX_FINDALTERNATESTREAMS].Selected==BSTATE_CHECKED);
	}
}

LONG_PTR WINAPI MainDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	switch (Msg)
	{
		case DN_INITDIALOG:
		{
			bool Hex=(SendDlgMessage(hDlg,DM_GETCHECK,FAD_CHECKBOX_HEX,NULL)==BSTATE_CHECKED);
			SendDlgMessage(hDlg,DM_SHOWITEM,FAD_EDIT_TEXT,!Hex);
			SendDlgMessage(hDlg,DM_SHOWITEM,FAD_EDIT_HEX,Hex);
			SendDlgMessage(hDlg,DM_ENABLE,FAD_TEXT_CP,!Hex);
			SendDlgMessage(hDlg,DM_ENABLE,FAD_COMBOBOX_CP,!Hex);
			SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_CASE,!Hex);
			SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_WHOLEWORDS,!Hex);
			SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_DIRS,!Hex);
			SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,FAD_EDIT_TEXT,1);
			SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,FAD_EDIT_HEX,1);
			SendDlgMessage(hDlg,DM_SETTEXTPTR,FAD_TEXT_TEXTHEX,(LONG_PTR)(Hex?MSG(MFindFileHex):MSG(MFindFileText)));
			SendDlgMessage(hDlg,DM_SETTEXTPTR,FAD_TEXT_CP,(LONG_PTR)MSG(MFindFileCodePage));
			SendDlgMessage(hDlg,DM_SETCOMBOBOXEVENT,FAD_COMBOBOX_CP,CBET_KEY);
			FarListTitles Titles={0,NULL,0,MSG(MFindFileCodePageBottom)};
			SendDlgMessage(hDlg,DM_LISTSETTITLES,FAD_COMBOBOX_CP,reinterpret_cast<LONG_PTR>(&Titles));
			/* Установка запомненных ранее параметров */
			CodePage = Opt.FindCodePage;
			/* -------------------------------------- */
			favoriteCodePages = FillCodePagesList(hDlg, FAD_COMBOBOX_CP, CodePage, false, true);
			// Текущее значение в в списке выбора кодовых страниц в общем случае модет не совпадать с CodePage,
			// так что получаем CodePage из списка выбора
			FarListPos Position;
			SendDlgMessage(hDlg, DM_LISTGETCURPOS, FAD_COMBOBOX_CP, (LONG_PTR)&Position);
			FarListGetItem Item = { Position.SelectPos };
			SendDlgMessage(hDlg, DM_LISTGETITEM, FAD_COMBOBOX_CP, (LONG_PTR)&Item);
			CodePage = (UINT)SendDlgMessage(hDlg, DM_LISTGETDATA, FAD_COMBOBOX_CP, Position.SelectPos);
			FindFoldersChanged = false;
			SearchFromChanged = false;
			return TRUE;
		}
		case DN_CLOSE:
		{
			switch (Param1)
			{
				case FAD_BUTTON_FIND:
				{
					LPCWSTR Mask=(LPCWSTR)SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,FAD_EDIT_MASK,NULL);

					if (!Mask||!*Mask)
						Mask=L"*";

					return FileMaskForFindFile.Set(Mask,0);
				}
				case FAD_BUTTON_DRIVE:
				{
					IsRedrawFramesInProcess++;
					CtrlObject->Cp()->ActivePanel->ChangeDisk();
					// Ну что ж, раз пошла такая пьянка рефрешить фреймы
					// будем таким способом.
					//FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
					FrameManager->ResizeAllFrame();
					IsRedrawFramesInProcess--;
					string strSearchFromRoot;
					PrepareDriveNameStr(strSearchFromRoot);
					FarListGetItem item={FADC_ROOT};
					SendDlgMessage(hDlg,DM_LISTGETITEM,FAD_COMBOBOX_WHERE,(LONG_PTR)&item);
					item.Item.Text=strSearchFromRoot;
					SendDlgMessage(hDlg,DM_LISTUPDATE,FAD_COMBOBOX_WHERE,(LONG_PTR)&item);
					PluginMode=CtrlObject->Cp()->ActivePanel->GetMode()==PLUGIN_PANEL;
					SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_DIRS,PluginMode?FALSE:TRUE);
					item.ItemIndex=FADC_ALLDISKS;
					SendDlgMessage(hDlg,DM_LISTGETITEM,FAD_COMBOBOX_WHERE,(LONG_PTR)&item);

					if (PluginMode)
						item.Item.Flags|=LIF_GRAYED;
					else
						item.Item.Flags&=~LIF_GRAYED;

					SendDlgMessage(hDlg,DM_LISTUPDATE,FAD_COMBOBOX_WHERE,(LONG_PTR)&item);
					item.ItemIndex=FADC_ALLBUTNET;
					SendDlgMessage(hDlg,DM_LISTGETITEM,FAD_COMBOBOX_WHERE,(LONG_PTR)&item);

					if (PluginMode)
						item.Item.Flags|=LIF_GRAYED;
					else
						item.Item.Flags&=~LIF_GRAYED;

					SendDlgMessage(hDlg,DM_LISTUPDATE,FAD_COMBOBOX_WHERE,(LONG_PTR)&item);
				}
				break;
				case FAD_BUTTON_FILTER:
					Filter->FilterEdit();
					break;
				case FAD_BUTTON_ADVANCED:
					AdvancedDialog();
					break;
				case -2:
				case -1:
				case FAD_BUTTON_CANCEL:
					return TRUE;
			}

			return FALSE;
		}
		case DN_BTNCLICK:
		{
			switch (Param1)
			{
				case FAD_CHECKBOX_DIRS:
					FindFoldersChanged = true;
					break;
				case FAD_CHECKBOX_HEX:
				{
					SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
					string strDataStr;
					Transform(strDataStr,(LPCWSTR)SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,Param2?FAD_EDIT_TEXT:FAD_EDIT_HEX,NULL),Param2?L'X':L'S');
					SendDlgMessage(hDlg,DM_SETTEXTPTR,Param2?FAD_EDIT_HEX:FAD_EDIT_TEXT,(LONG_PTR)(const wchar_t*)strDataStr);
					SendDlgMessage(hDlg,DM_SHOWITEM,FAD_EDIT_TEXT,!Param2);
					SendDlgMessage(hDlg,DM_SHOWITEM,FAD_EDIT_HEX,Param2);
					SendDlgMessage(hDlg,DM_ENABLE,FAD_TEXT_CP,!Param2);
					SendDlgMessage(hDlg,DM_ENABLE,FAD_COMBOBOX_CP,!Param2);
					SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_CASE,!Param2);
					SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_WHOLEWORDS,!Param2);
					SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_DIRS,!Param2);
					SendDlgMessage(hDlg,DM_SETTEXTPTR,FAD_TEXT_TEXTHEX,(LONG_PTR)(const wchar_t*)(Param2?MSG(MFindFileHex):MSG(MFindFileText)));

					if (strDataStr.GetLength()>0)
					{
						int UnchangeFlag=(int)SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,FAD_EDIT_TEXT,-1);
						SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,FAD_EDIT_HEX,UnchangeFlag);
					}

					SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
				}
				break;
			}

			break;
		}
		case DM_KEY:
		{
			switch (Param1)
			{
				case FAD_COMBOBOX_CP:
				{
					switch (Param2)
					{
						case KEY_INS:
						case KEY_NUMPAD0:
						case KEY_SPACE:
						{
							// Обработка установки/снятия флажков для стандартных и любимых таблиц символов
							// Получаем текущую позицию в выпадающем списке таблиц символов
							FarListPos Position;
							SendDlgMessage(hDlg, DM_LISTGETCURPOS, FAD_COMBOBOX_CP, (LONG_PTR)&Position);
							// Получаем номер выбранной таблицы симолов
							FarListGetItem Item = { Position.SelectPos };
							SendDlgMessage(hDlg, DM_LISTGETITEM, FAD_COMBOBOX_CP, (LONG_PTR)&Item);
							UINT SelectedCodePage = (UINT)SendDlgMessage(hDlg, DM_LISTGETDATA, FAD_COMBOBOX_CP, Position.SelectPos);
							// Разрешаем отмечать только стандартные и любимые таблицы символов
							int FavoritesIndex = 2 + StandardCPCount + 2;

							if (Position.SelectPos > 1 && Position.SelectPos < FavoritesIndex + (favoriteCodePages ? favoriteCodePages + 1 : 0))
							{
								// Преобразуем номер таблицы сиволов к строке
								string strCodePageName;
								strCodePageName.Format(L"%u", SelectedCodePage);
								// Получаем текущее состояние флага в реестре
								int SelectType = 0;
								GetRegKey(FavoriteCodePagesKey, strCodePageName, SelectType, 0);

								// Отмечаем/разотмечаем таблицу символов
								if (Item.Item.Flags & LIF_CHECKED)
								{
									// Для стандартных таблиц символов просто удаляем значение из рееста, для
									// любимых же оставляем в реестре флаг, что таблица символов любимая
									if (SelectType & CPST_FAVORITE)
										SetRegKey(FavoriteCodePagesKey, strCodePageName, CPST_FAVORITE);
									else
										DeleteRegValue(FavoriteCodePagesKey, strCodePageName);

									Item.Item.Flags &= ~LIF_CHECKED;
								}
								else
								{
									SetRegKey(FavoriteCodePagesKey, strCodePageName, CPST_FIND | (SelectType & CPST_FAVORITE ?  CPST_FAVORITE : 0));
									Item.Item.Flags |= LIF_CHECKED;
								}

								// Обновляем текущий элемент в выпадающем списке
								SendDlgMessage(hDlg, DM_LISTUPDATE, FAD_COMBOBOX_CP, (LONG_PTR)&Item);

								if (Position.SelectPos<FavoritesIndex + (favoriteCodePages ? favoriteCodePages + 1 : 0)-2)
								{
									FarListPos Pos={Position.SelectPos+1,Position.TopPos};
									SendDlgMessage(hDlg, DM_LISTSETCURPOS, FAD_COMBOBOX_CP,reinterpret_cast<LONG_PTR>(&Pos));
								}

								// Обрабатываем случай, когда таблица символов может присутствовать, как в стандартных, так и в любимых,
								// т.е. выбор/снятие флага автоматичекски происходуит у обоих элементов
								bool bStandardCodePage = Position.SelectPos < FavoritesIndex;

								for (int Index = bStandardCodePage ? FavoritesIndex : 0; Index < (bStandardCodePage ? FavoritesIndex + favoriteCodePages : FavoritesIndex); Index++)
								{
									// Получаем элемент таблицы симолов
									FarListGetItem CheckItem = { Index };
									SendDlgMessage(hDlg, DM_LISTGETITEM, FAD_COMBOBOX_CP, (LONG_PTR)&CheckItem);

									// Обрабатываем только таблицы симовлов
									if (!(CheckItem.Item.Flags&LIF_SEPARATOR))
									{
										if (SelectedCodePage == (UINT)SendDlgMessage(hDlg, DM_LISTGETDATA, FAD_COMBOBOX_CP, Index))
										{
											if (Item.Item.Flags & LIF_CHECKED)
												CheckItem.Item.Flags |= LIF_CHECKED;
											else
												CheckItem.Item.Flags &= ~LIF_CHECKED;

											SendDlgMessage(hDlg, DM_LISTUPDATE, FAD_COMBOBOX_CP, (LONG_PTR)&CheckItem);
											break;
										}
									}
								}
							}
						}
						break;
					}
				}
				break;
			}

			break;
		}
		case DN_EDITCHANGE:
		{
			FarDialogItem &Item=*reinterpret_cast<FarDialogItem*>(Param2);

			switch (Param1)
			{
				case FAD_EDIT_TEXT:

					// Строка "Содержащих текст"
					if (!FindFoldersChanged)
					{
						BOOL Checked = (Item.PtrData && *Item.PtrData)?FALSE:Opt.FindOpt.FindFolders;
						SendDlgMessage(hDlg, DM_SETCHECK, FAD_CHECKBOX_DIRS, Checked?BSTATE_CHECKED:BSTATE_UNCHECKED);
					}

					return TRUE;
				case FAD_COMBOBOX_CP:
				{
					// Получаем выбранную в выпадающем списке таблицу символов
					CodePage = (UINT)SendDlgMessage(hDlg, DM_LISTGETDATA, FAD_COMBOBOX_CP, SendDlgMessage(hDlg, DM_LISTGETCURPOS, FAD_COMBOBOX_CP, NULL));
				}
				return TRUE;
				case FAD_COMBOBOX_WHERE:
					SearchFromChanged=true;
					return TRUE;
			}
		}
		case DN_HOTKEY:
		{
			if (Param1==FAD_TEXT_TEXTHEX)
			{
				bool Hex=(SendDlgMessage(hDlg,DM_GETCHECK,FAD_CHECKBOX_HEX,NULL)==BSTATE_CHECKED);
				SendDlgMessage(hDlg,DM_SETFOCUS,Hex?FAD_EDIT_HEX:FAD_EDIT_TEXT,0);
				return FALSE;
			}
		}
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool GetPluginFile(size_t ArcIndex, const FAR_FIND_DATA_EX *FindData,const wchar_t *DestPath, string &strResultName)
{
	_ALGO(CleverSysLog clv(L"FindFiles::GetPluginFile()"));
	HANDLE hPlugin=ArcList[ArcIndex]->hPlugin;
	OpenPluginInfo Info;
	CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
	string strSaveDir = NullToEmpty(Info.CurDir);
	AddEndSlash(strSaveDir);
	CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_SILENT);
	//SetPluginDirectory(ArcList[ArcIndex]->strRootPath,hPlugin);
	SetPluginDirectory(FindData->strFileName,hPlugin);
	const wchar_t *lpFileNameToFind = PointToName(FindData->strFileName);
	const wchar_t *lpFileNameToFindShort = PointToName(FindData->strAlternateFileName);
	PluginPanelItem *pItems;
	int nItemsNumber;
	bool nResult=false;

	if (CtrlObject->Plugins.GetFindData(hPlugin,&pItems,&nItemsNumber,OPM_SILENT))
	{
		for (int i=0; i<nItemsNumber; i++)
		{
			PluginPanelItem Item = pItems[i];
			Item.FindData.lpwszFileName=const_cast<LPWSTR>(PointToName(NullToEmpty(pItems[i].FindData.lpwszFileName)));
			Item.FindData.lpwszAlternateFileName=const_cast<LPWSTR>(PointToName(NullToEmpty(pItems[i].FindData.lpwszAlternateFileName)));

			if (!StrCmp(lpFileNameToFind,Item.FindData.lpwszFileName) && !StrCmp(lpFileNameToFindShort,Item.FindData.lpwszAlternateFileName))
			{
				nResult=(CtrlObject->Plugins.GetFile(hPlugin,&Item,DestPath,strResultName,OPM_SILENT)!=0);
				break;
			}
		}

		CtrlObject->Plugins.FreeFindData(hPlugin,pItems,nItemsNumber);
	}

	CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_SILENT);
	//SetPluginDirectory(ArcList[ArcIndex].RootPath,hPlugin);
	SetPluginDirectory(strSaveDir,hPlugin);
	return nResult;
}

size_t AddArcListItem(const wchar_t *ArcName,HANDLE hPlugin,DWORD dwFlags,const wchar_t *RootPath)
{
	if ((ArcListCount == ArcListCapacity) && (!ArcListGrow()))
		return LIST_INDEX_NONE;

	ArcList[ArcListCount] = new ARCLIST;
	ArcList[ArcListCount]->strArcName = ArcName;
	ArcList[ArcListCount]->hPlugin = hPlugin;
	ArcList[ArcListCount]->Flags = dwFlags;
	ArcList[ArcListCount]->strRootPath = RootPath;
	AddEndSlash(ArcList[ArcListCount]->strRootPath);
	return ArcListCount++;
}

size_t AddFindListItem(FAR_FIND_DATA_EX *FindData)
{
	if ((FindListCount == FindListCapacity)&&(!FindListGrow()))
		return LIST_INDEX_NONE;

	FindList[FindListCount] = new FINDLIST;
	FindList[FindListCount]->FindData = *FindData;
	FindList[FindListCount]->ArcIndex = LIST_INDEX_NONE;
	return FindListCount++;
}

// Алгоритма Бойера-Мура-Хорспула поиска подстроки (Unicode версия)
const int FindStringBMH(const wchar_t* searchBuffer, size_t searchBufferCount)
{
	size_t findStringCount = strFindStr.GetLength();
	const wchar_t *buffer = searchBuffer;
	const wchar_t *findStringLower = CmpCase ? NULL : findString+findStringCount;
	size_t lastBufferChar = findStringCount-1;

	while (searchBufferCount>=findStringCount)
	{
		for (size_t index = lastBufferChar; buffer[index]==findString[index] || (CmpCase ? 0 : buffer[index]==findStringLower[index]); index--)
			if (index == 0)
				return static_cast<int>(buffer-searchBuffer);

		size_t offset = skipCharsTable[buffer[lastBufferChar]];
		searchBufferCount -= offset;
		buffer += offset;
	}

	return -1;
}

// Алгоритма Бойера-Мура-Хорспула поиска подстроки (Char версия)
const int FindStringBMH(const unsigned char* searchBuffer, size_t searchBufferCount)
{
	const unsigned char *buffer = searchBuffer;
	size_t lastBufferChar = hexFindStringSize-1;

	while (searchBufferCount>=hexFindStringSize)
	{
		for (size_t index = lastBufferChar; buffer[index]==hexFindString[index]; index--)
			if (index == 0)
				return static_cast<int>(buffer-searchBuffer);

		size_t offset = skipCharsTable[buffer[lastBufferChar]];
		searchBufferCount -= offset;
		buffer += offset;
	}

	return -1;
}


int LookForString(const wchar_t *Name)
{
#define RETURN(r) { result = (r); goto exit; }
#define CONTINUE(r) { if ((r) || cpIndex==codePagesCount-1) RETURN(r) else continue; }
	// Длина строки поиска
	size_t findStringCount;

	// Если строки поиска пустая, то считаем, что мы всегда что-нибудь найдём
	if ((findStringCount = strFindStr.GetLength()) == 0)
		return (TRUE);

	// Результат поиска
	BOOL result = FALSE;
	// Открываем файл
	HANDLE fileHandle = apiCreateFile(
	                        Name,
	                        FILE_READ_ATTRIBUTES | FILE_READ_DATA | FILE_WRITE_ATTRIBUTES,
	                        FILE_SHARE_READ | FILE_SHARE_WRITE,
	                        NULL,
	                        OPEN_EXISTING,
	                        FILE_FLAG_SEQUENTIAL_SCAN
	                    );
	// Время последнего доступа к файлу (нужно для его восстановления после поиска)
	FILETIME lastAccessTime;
	// Признак того, что время доуступа к файлу было получено
	BOOL isTimeReaded = FALSE;

	// Если файл не удалось открыть изначальным способом, то пытаемся получить к нему
	// доступ только на чтение, иначе получаем запоминаем последнего доступа к файлу
	if (fileHandle == INVALID_HANDLE_VALUE)
		// Открыаем файл только на чтение
		fileHandle = apiCreateFile(
		                 Name,
		                 FILE_READ_DATA,
		                 FILE_SHARE_READ | FILE_SHARE_WRITE,
		                 NULL,
		                 OPEN_EXISTING,
		                 FILE_FLAG_SEQUENTIAL_SCAN
		             );
	else
		// Запоминаем время последнего доуступа к файлу
		isTimeReaded = GetFileTime(fileHandle, NULL, &lastAccessTime, NULL);

	// Если файл открыть не удалось, то считаем, что ничего не нашли
	if (fileHandle==INVALID_HANDLE_VALUE)
		return (FALSE);

	// Количество считанных из файла байт
	DWORD readBlockSize = 0;
	// Количество прочитанных из файла байт
	unsigned __int64 alreadyRead = 0;
	// Смещение на которое мы отступили при переходе между блоками
	int offset=0;

	if (SearchHex)
		offset = (int)hexFindStringSize-1;

	UINT64 FileSize=0;
	apiGetFileSizeEx(fileHandle,FileSize);

	if (SearchInFirst)
	{
		FileSize=Min(SearchInFirst,FileSize);
	}

	UINT LastPercents=0;

	// Основной цикл чтения из файла
	while (!StopSearch && ReadFile(fileHandle,readBufferA,(!SearchInFirst || alreadyRead+readBufferSizeA<=SearchInFirst)?readBufferSizeA:static_cast<DWORD>(SearchInFirst-alreadyRead),&readBlockSize,NULL))
	{
		UINT Percents=static_cast<UINT>(FileSize?alreadyRead*100/FileSize:0);

		if (Percents!=LastPercents)
		{
			statusCS.Enter();
			strFindPercentMessage.Format(L"%3d%%",Percents);
			statusCS.Leave();
			FindMessagePercentReady=true;
			LastPercents=Percents;
		}

		// Увеличиваем счётчик прочитыннх байт
		alreadyRead += readBlockSize;

		// Для hex и обыкновенного поиска разные ветки
		if (SearchHex)
		{
			// Выходим, если ничего не прочитали или прочитали мало
			if (readBlockSize == 0 || readBlockSize<hexFindStringSize)
				RETURN(FALSE)

				// Ищем
				if (FindStringBMH((unsigned char *)readBufferA, readBlockSize)!=-1)
					RETURN(TRUE)
				}
		else
		{
			for (int cpIndex = 0; cpIndex<codePagesCount; cpIndex++)
			{
				// Информация о кодовой странице
				CodePageInfo *cpi = codePages+cpIndex;

				// Пропускаем ошибочные кодовые страницы
				if (!cpi->MaxCharSize)
					CONTINUE(FALSE)

					// Если начало файла очищаем информацию о поиске по словам
					if (WholeWords && alreadyRead==readBlockSize)
					{
						cpi->WordFound = false;
						cpi->LastSymbol = NULL;
					}

				// Если ничего не прочитали
				if (readBlockSize == 0)
					// Если поиск по словам и в конце предыдущего блока было что-то найдено,
					// то считаем, что нашли то, что нужно
					CONTINUE(WholeWords && cpi->WordFound)

					// Выходим, если прочитали меньше размера строки поиска и нет поиска по словам
					if (readBlockSize < findStringCount && !(WholeWords && cpi->WordFound))
						CONTINUE(FALSE)
						// Количество символов в выходном буфере
						unsigned int bufferCount;

				// Буфер для поиска
				wchar_t *buffer;

				// Перегоняем буфер в UTF-16
				if (IsUnicodeCodePage(cpi->CodePage))
				{
					// Вычисляем размер буфера в UTF-16
					bufferCount = readBlockSize/sizeof(wchar_t);

					// Выходим, если размер буфера меньше длины строки посика
					if (bufferCount < findStringCount)
						CONTINUE(FALSE)

						// Копируем буфер чтения в буфер сравнения
						if (cpi->CodePage==CP_REVERSEBOM)
						{
							// Для UTF-16 (big endian) преобразуем буфер чтения в буфер сравнения
							bufferCount = LCMapStringW(
							                  LOCALE_NEUTRAL,//LOCALE_INVARIANT,
							                  LCMAP_BYTEREV,
							                  (wchar_t *)readBufferA,
							                  (int)bufferCount,
							                  readBuffer,
							                  readBufferSize
							              );

							if (!bufferCount)
								CONTINUE(FALSE)
								// Устанавливаем буфер стравнения
								buffer = readBuffer;
						}
						else
						{
							// Если поиск в UTF-16 (little endian), то используем исходный буфер
							buffer = (wchar_t *)readBufferA;
						}
				}
				else
				{
					// Конвертируем буфер чтения из кодировки поиска в UTF-16
					bufferCount = MultiByteToWideChar(
					                  cpi->CodePage,
					                  0,
					                  (char *)readBufferA,
					                  readBlockSize,
					                  readBuffer,
					                  readBufferSize
					              );

					// Выходим, если нам не удалось сконвертировать строку
					if (!bufferCount)
						CONTINUE(FALSE)

						// Если прочитали меньше размера строки поиска и поиска по словам, то проверяем
						// первый символ блока на разделитель и выходим
						// Если у нас поиск по словам и в конце предыдущего блока было вхождение
						if (WholeWords && cpi->WordFound)
						{
							// Если конец файла, то считаем, что есть разделитель в конце
							if (findStringCount-1>=bufferCount)
								RETURN(TRUE)
								// Проверяем первый символ текущего блока с учётом обратного смещения, которое делается
								// при переходе между блоками
								cpi->LastSymbol = readBuffer[findStringCount-1];

							if (IsWordDiv(cpi->LastSymbol))
								RETURN(TRUE)

								// Если размер буфера меньше размера слова, то выходим
								if (readBlockSize < findStringCount)
									CONTINUE(FALSE)
								}

					// Устанавливаем буфер стравнения
					buffer = readBuffer;
				}

				unsigned int index = 0;

				do
				{
					// Ищем подстроку в буфере и возвращаем индекс её начала в случае успеха
					int foundIndex = FindStringBMH(buffer+index, bufferCount-index);

					// Если подстрока не найдена идём на следующий шаг
					if (foundIndex == -1)
						break;

					// Если посдстрока найдена и отключен поиск по словам, то считаем что всё хорошо
					if (!WholeWords)
						RETURN(TRUE)
						// Устанавливаем позицию в исходном буфере
						index += foundIndex;

					// Если идёт поиск по словам, то делаем соответвующие проверки
					bool firstWordDiv = false;

					// Если мы находимся вначале блока
					if (index == 0)
					{
						// Если мы находимся вначале файла, то считаем, что разделитель есть
						// Если мы находимся вначале блока, то проверяем является
						// или нет последний символ предыдущего блока разделителем
						if (alreadyRead==readBlockSize || IsWordDiv(cpi->LastSymbol))
							firstWordDiv = true;
					}
					else
					{
						// Проверяем является или нет предыдущий найденому символ блока разделителем
						cpi->LastSymbol = buffer[index-1];

						if (IsWordDiv(cpi->LastSymbol))
							firstWordDiv = true;
					}

					// Проверяем разделитель в конце, только если найден разделитель вначале
					if (firstWordDiv)
					{
						// Если блок выбран не до конца
						if (index+findStringCount!=bufferCount)
						{
							// Проверяем является или нет последующий за найденым символ блока разделителем
							cpi->LastSymbol = buffer[index+findStringCount];

							if (IsWordDiv(cpi->LastSymbol))
								RETURN(TRUE)
							}
						else
							cpi->WordFound = true;
					}
				}
				while (++index<=bufferCount-findStringCount);

				// Выходим, если мы вышли за пределы количества байт разрешённых для поиска
				if (SearchInFirst && SearchInFirst>=alreadyRead)
					CONTINUE(FALSE)
					// Запоминаем последний символ блока
					cpi->LastSymbol = buffer[bufferCount-1];
			}

			// Получаем смещение на которое мы отступили при переходе между блоками
			offset = (int)((CodePage==CP_AUTODETECT?sizeof(wchar_t):codePages->MaxCharSize)*(findStringCount-1));
		}

		// Если мы потенциально прочитали не весь файл
		if (readBlockSize==readBufferSizeA)
		{
			// Отступаем назад на длину слова поиска минус 1
			if (!apiSetFilePointerEx(fileHandle, -1*offset, NULL, FILE_CURRENT))
				RETURN(FALSE)
				alreadyRead -= offset;
		}
	}

exit:

	// Восстаналиваем время доступа
	if (isTimeReaded)
		SetFileTime(fileHandle, NULL, &lastAccessTime, NULL);

	// Закрываем хэндл файла
	CloseHandle(fileHandle);
	// Возвращаем результат
	return (result);
#undef CONTINUE
#undef RETURN
}

bool IsFileIncluded(PluginPanelItem *FileItem,const wchar_t *FullName,DWORD FileAttr)
{
	CriticalSectionLock Lock(ffCS);
	bool FileFound=FileMaskForFindFile.Compare(FullName);
	HANDLE hPlugin=FindFileArcIndex == LIST_INDEX_NONE?INVALID_HANDLE_VALUE:ArcList[FindFileArcIndex]->hPlugin;

	while (FileFound)
	{
		/* $ 24.09.2003 KM
		   Если включен режим поиска hex-кодов, тогда папки в поиск не включаем
		*/
		/* $ 17.01.2002 VVM
		  ! Поскольку работу с поиском в папках вынесли в диалог -
		    флаг в плагине потерял свою актуальность */
		if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) && ((Opt.FindOpt.FindFolders==0) || SearchHex))
			return FALSE;

		if (!strFindStr.IsEmpty() && FileFound)
		{
			FileFound=false;

			if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
				break;

			string strSearchFileName;
			bool RemoveTemp=false;

			if (hPlugin != INVALID_HANDLE_VALUE)
			{
				if (!CtrlObject->Plugins.UseFarCommand(hPlugin, PLUGIN_FARGETFILES))
				{
					string strTempDir;
					FarMkTempEx(strTempDir); // А проверка на NULL???
					apiCreateDirectory(strTempDir,NULL);
					WaitForSingleObject(hPluginMutex,INFINITE);

					if (!CtrlObject->Plugins.GetFile(hPlugin,FileItem,strTempDir,strSearchFileName,OPM_SILENT|OPM_FIND))
					{
						ReleaseMutex(hPluginMutex);
						apiRemoveDirectory(strTempDir);
						break;
					}
					else
					{
						ReleaseMutex(hPluginMutex);
					}

					RemoveTemp=true;
				}
				else
				{
					strSearchFileName = strPluginSearchPath + FullName;
				}
			}
			else
			{
				strSearchFileName = FullName;
			}

			if (LookForString(strSearchFileName))
				FileFound=true;

			if (RemoveTemp)
			{
				DeleteFileWithFolder(strSearchFileName);
			}
		}

		break;
	}

	return FileFound;
}

LONG_PTR WINAPI FindFiles::FindDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	Dialog* Dlg=reinterpret_cast<Dialog*>(hDlg);
	VMenu *ListBox=Dlg->Item[FD_LISTBOX]->ListPtr;

	if (StopSearch && TB)
	{
		delete TB;
		TB=NULL;
	}

	CriticalSectionLock Lock(Dlg->CS);

	switch (Msg)
	{
		case DN_DRAWDIALOGDONE:
		{
			DefDlgProc(hDlg,Msg,Param1,Param2);

			// Переместим фокус на кнопку [Go To]
			if ((FindDirCount || FindFileCount) && !FindPositionChanged)
			{
				FindPositionChanged=TRUE;
				SendDlgMessage(hDlg,DM_SETFOCUS,FD_BUTTON_GOTO,0);
			}

//      else
//        ScrBuf.Flush();
			return TRUE;
		}
		case DN_KEY:
		{
			switch (Param2)
			{
				case KEY_ESC:
				case KEY_F10:
				{
					if (!StopSearch)
					{
						PauseSearch=TRUE;
						IsProcessAssignMacroKey--;
						int LocalRes=TRUE;

						if (Opt.Confirm.Esc)
							LocalRes=AbortMessage();

						IsProcessAssignMacroKey++;
						PauseSearch=FALSE;
						StopSearch=LocalRes;
						return TRUE;
					}
				}
				break;
				// Некоторые спец.клавиши все-же обработаем.
				case KEY_CTRLALTSHIFTPRESS:
				case KEY_ALTF9:
				case KEY_F11:
				case KEY_CTRLW:
				{
					IsProcessAssignMacroKey--;
					FrameManager->ProcessKey((DWORD)Param2);
					IsProcessAssignMacroKey++;
					return TRUE;
				}
				break;
				case KEY_RIGHT:
				case KEY_NUMPAD6:
				case KEY_TAB:
				{
					if (Param1==FD_BUTTON_STOP)
					{
						FindPositionChanged=TRUE;
						SendDlgMessage(hDlg,DM_SETFOCUS,FD_BUTTON_NEW,0);
						return TRUE;
					}
				}
				break;
				case KEY_LEFT:
				case KEY_NUMPAD4:
				case KEY_SHIFTTAB:
				{
					if (Param1==FD_BUTTON_NEW)
					{
						FindPositionChanged=TRUE;
						SendDlgMessage(hDlg,DM_SETFOCUS,FD_BUTTON_STOP,0);
						return TRUE;
					}
				}
				break;
				case KEY_UP:
				case KEY_DOWN:
				case KEY_NUMPAD8:
				case KEY_NUMPAD2:
				case KEY_PGUP:
				case KEY_PGDN:
				case KEY_NUMPAD9:
				case KEY_NUMPAD3:
				case KEY_HOME:
				case KEY_END:
				case KEY_NUMPAD7:
				case KEY_NUMPAD1:
				case KEY_MSWHEEL_UP:
				case KEY_MSWHEEL_DOWN:
				case KEY_ALTLEFT: case KEY_NUMPAD4|KEY_ALT: case KEY_MSWHEEL_LEFT:
				case KEY_ALTRIGHT: case KEY_NUMPAD6|KEY_ALT: case KEY_MSWHEEL_RIGHT:
				case KEY_ALTSHIFTLEFT: case KEY_NUMPAD4|KEY_ALT|KEY_SHIFT:
				case KEY_ALTSHIFTRIGHT: case KEY_NUMPAD6|KEY_ALT|KEY_SHIFT:
				case KEY_ALTHOME: case KEY_NUMPAD7|KEY_ALT:
				case KEY_ALTEND: case KEY_NUMPAD1|KEY_ALT:
				{
					ListBox->ProcessKey((int)Param2);
					return TRUE;
				}
				break;
				case KEY_F3:
				case KEY_NUMPAD5:
				case KEY_SHIFTNUMPAD5:
				case KEY_F4:
				{
					if (ListBox->GetItemCount()==0)
					{
						return TRUE;
					}

					ffCS.Enter();
					size_t ItemIndex = reinterpret_cast<size_t>(ListBox->GetUserData(NULL,0));
					int RemoveTemp=FALSE;
					int ClosePlugin=FALSE; // Плагины надо закрывать, если открыли.
					string strSearchFileName;
					string strTempDir;

					if (FindList[ItemIndex]->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						ffCS.Leave();
						return TRUE;
					}

					// FindFileArcIndex нельзя здесь использовать
					// Он может быть уже другой.
					if ((FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE) &&
					        (!(ArcList[FindList[ItemIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
					{
						string strFindArcName = ArcList[FindList[ItemIndex]->ArcIndex]->strArcName;

						if (ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin == INVALID_HANDLE_VALUE)
						{
							LPBYTE Buffer=new BYTE[Opt.PluginMaxReadData];

							if (Buffer)
							{
								HANDLE hProcessFile=apiCreateFile(strFindArcName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0);

								if (hProcessFile!=INVALID_HANDLE_VALUE)
								{
									DWORD ReadSize=0;
									bool Result=(ReadFile(hProcessFile,Buffer,Opt.PluginMaxReadData,&ReadSize,NULL)!=FALSE);
									CloseHandle(hProcessFile);

									if (Result)
									{
										int SavePluginsOutput=DisablePluginsOutput;
										DisablePluginsOutput=TRUE;
										WaitForSingleObject(hPluginMutex,INFINITE);
										ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = CtrlObject->Plugins.OpenFilePlugin(strFindArcName,Buffer,ReadSize,0);
										ReleaseMutex(hPluginMutex);
										DisablePluginsOutput=SavePluginsOutput;
										delete[] Buffer;

										if (ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin == (HANDLE)-2 ||
										        ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin == INVALID_HANDLE_VALUE)
										{
											ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = INVALID_HANDLE_VALUE;
											ffCS.Leave();
											return TRUE;
										}

										ClosePlugin = TRUE;
									}
								}
								else
								{
									delete[] Buffer;
									ffCS.Leave();
									return TRUE;
								}
							}
						}

						FarMkTempEx(strTempDir);
						apiCreateDirectory(strTempDir, NULL);
						// if (!CtrlObject->Plugins.GetFile(ArcList[FindList[ItemIndex].ArcIndex].hPlugin,&FileItem,TempDir,SearchFileName,OPM_SILENT|OPM_FIND))
						WaitForSingleObject(hPluginMutex,INFINITE);

						if (!GetPluginFile(FindList[ItemIndex]->ArcIndex,&FindList[ItemIndex]->FindData,strTempDir,strSearchFileName))
						{
							apiRemoveDirectory(strTempDir);

							if (ClosePlugin)
							{
								CtrlObject->Plugins.ClosePlugin(ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin);
								ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = INVALID_HANDLE_VALUE;
							}

							ReleaseMutex(hPluginMutex);
							ffCS.Leave();
							return FALSE;
						}
						else
						{
							if (ClosePlugin)
							{
								CtrlObject->Plugins.ClosePlugin(ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin);
								ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = INVALID_HANDLE_VALUE;
							}

							ReleaseMutex(hPluginMutex);
						}

						RemoveTemp=TRUE;
					}
					else
					{
						strSearchFileName = FindList[ItemIndex]->FindData.strFileName;

						if (apiGetFileAttributes(strSearchFileName) == INVALID_FILE_ATTRIBUTES && apiGetFileAttributes(FindList[ItemIndex]->FindData.strAlternateFileName) != INVALID_FILE_ATTRIBUTES)
							strSearchFileName = FindList[ItemIndex]->FindData.strAlternateFileName;
					}

					DWORD FileAttr=apiGetFileAttributes(strSearchFileName);

					if (FileAttr!=INVALID_FILE_ATTRIBUTES)
					{
						string strOldTitle;
						apiGetConsoleTitle(strOldTitle);

						if (Param2==KEY_F3 || Param2==KEY_NUMPAD5 || Param2==KEY_SHIFTNUMPAD5)
						{
							int ListSize=ListBox->GetItemCount();
							NamesList ViewList;

							// Возьмем все файлы, которые имеют реальные имена...
							if (Opt.FindOpt.CollectFiles)
							{
								for (int I=0; I<ListSize; I++)
								{
									FINDLIST* PtrFindList=FindList[reinterpret_cast<size_t>(ListBox->GetUserData(NULL,0,I))];

									if ((PtrFindList->ArcIndex == LIST_INDEX_NONE)||(ArcList[PtrFindList->ArcIndex]->Flags & OPIF_REALNAMES))
									{
										// Не учитывали файлы в архивах с OPIF_REALNAMES
										if (!PtrFindList->FindData.strFileName.IsEmpty() && !(PtrFindList->FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
											ViewList.AddName(PtrFindList->FindData.strFileName, PtrFindList->FindData.strAlternateFileName);
									}
								} /* for */

								string strCurDir = FindList[ItemIndex]->FindData.strFileName;
								ViewList.SetCurName(strCurDir);
							}

							SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
							SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
							{
								FileViewer ShellViewer(strSearchFileName,FALSE,FALSE,FALSE,-1,NULL,(FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE)?NULL:(Opt.FindOpt.CollectFiles?&ViewList:NULL));
								ShellViewer.SetDynamicallyBorn(FALSE);
								ShellViewer.SetEnableF6(TRUE);

								// FindFileArcIndex нельзя здесь использовать
								// Он может быть уже другой.
								if ((FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE) &&
								        (!(ArcList[FindList[ItemIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
									ShellViewer.SetSaveToSaveAs(TRUE);

								//!Mutex.Unlock();
								ffCS.Leave();  // чтобы поиск продолжался, пока мы тут кнопки давим
								IsProcessVE_FindFile++;
								FrameManager->EnterModalEV();
								FrameManager->ExecuteModal();
								FrameManager->ExitModalEV();
								IsProcessVE_FindFile--;
								ffCS.Enter();
								// заставляем рефрешится экран
								FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
							}
							SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
							SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
						}
						else
						{
							SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
							SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
							{
								/* $ 14.08.2002 VVM
								  ! Пока-что запретим из поиска переключаться в активный редактор.
								    К сожалению, манагер на это не способен сейчас
															int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR,SearchFileName);
															int SwitchTo=FALSE;
															if (FramePos!=-1)
															{
																if (!(*FrameManager)[FramePos]->GetCanLoseFocus(TRUE) ||
																	Opt.Confirm.AllowReedit)
																{
																	char MsgFullFileName[NM];
																	xstrncpy(MsgFullFileName,SearchFileName,sizeof(MsgFullFileName));
																	int MsgCode=Message(0,2,MSG(MFindFileTitle),
																				TruncPathStr(MsgFullFileName,ScrX-16),
																				MSG(MAskReload),
																				MSG(MCurrent),MSG(MNewOpen));
																	if (MsgCode==0)
																	{
																		SwitchTo=TRUE;
																	}
																	else if (MsgCode==1)
																	{
																		SwitchTo=FALSE;
																	}
																	else
																	{
																		SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
																		SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
																		return TRUE;
																	}
																}
																else
																{
																	SwitchTo=TRUE;
																}
															}
															if (SwitchTo)
															{
																(*FrameManager)[FramePos]->SetCanLoseFocus(FALSE);
																(*FrameManager)[FramePos]->SetDynamicallyBorn(FALSE);
																FrameManager->ActivateFrame(FramePos);
																IsProcessVE_FindFile++;
																FrameManager->EnterModalEV();
																FrameManager->ExecuteModal ();
																FrameManager->ExitModalEV();
																// FrameManager->ExecuteNonModal();
																IsProcessVE_FindFile--;
																// заставляем рефрешится экран
																FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
															}
															else
								*/
								{
									FileEditor ShellEditor(strSearchFileName,CP_AUTODETECT,0);
									ShellEditor.SetDynamicallyBorn(FALSE);
									ShellEditor.SetEnableF6(TRUE);

									// FindFileArcIndex нельзя здесь использовать
									// Он может быть уже другой.
									if ((FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE) &&
									        (!(ArcList[FindList[ItemIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
										ShellEditor.SetSaveToSaveAs(TRUE);

									ffCS.Leave();  // чтобы поиск продолжался, пока мы тут кнопки давим
									IsProcessVE_FindFile++;
									FrameManager->EnterModalEV();
									FrameManager->ExecuteModal();
									FrameManager->ExitModalEV();
									IsProcessVE_FindFile--;
									ffCS.Enter();
									// заставляем рефрешится экран
									FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
								}
							}
							SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
							SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
						}

						SetConsoleTitle(strOldTitle);
					}

					if (RemoveTemp)
					{
						DeleteFileWithFolder(strSearchFileName);
					}

					ffCS.Leave();
					return TRUE;
				}
			}

			return DefDlgProc(hDlg,Msg,Param1,Param2);
		}
		case DN_BTNCLICK:
		{
			FindPositionChanged = TRUE;

			switch (Param1)
			{
				case FD_BUTTON_NEW:
				{
					StopSearch=TRUE;
					FindExitCode=FIND_EXIT_SEARCHAGAIN;
					return FALSE;
				}
				case FD_BUTTON_STOP:
				{
					if (StopSearch)
						return FALSE;

					StopSearch=TRUE;
					return TRUE;
				}
				case FD_BUTTON_GOTO:
				{
					if (!ListBox)
					{
						return FALSE;
					}

					// Переход будем делать так же после выхода из диалога.
					// Причину смотри для [ Panel ]
					if (ListBox->GetItemCount()==0)
					{
						return TRUE;
					}

					FindExitIndex = (DWORD)(DWORD_PTR)ListBox->GetUserData(NULL, 0);
					FindExitCode = FIND_EXIT_GOTO;
					return FALSE;
				}
				case FD_BUTTON_VIEW:
				{
					FindDlgProc(hDlg,DN_KEY,FD_LISTBOX,KEY_F3);
					return TRUE;
				}
				case FD_BUTTON_PANEL:
				{
					if (ListBox)
					{
						// На панель будем посылать не в диалоге, а после.
						// После окончания поиска. Иначе возможна ситуация, когда мы
						// ищем на панели, потом ее грохаем и создаем новую (а поиск-то
						// идет!) и в результате ФАР трапается.
						if (ListBox->GetItemCount()==0)
						{
							return TRUE;
						}

						FindExitCode=FIND_EXIT_PANEL;
						FindExitIndex=(DWORD)(DWORD_PTR)ListBox->GetUserData(NULL, 0);

						if (TB)
						{
							delete TB;
							TB=NULL;
						}
					}

					return FALSE;
				}
			}

			break;
		}
		case DN_CTLCOLORDLGLIST:
		{
			if (Param2)
				reinterpret_cast<FarListColors*>(Param2)->Colors[VMenuColorDisabled]=FarColorToReal(COL_DIALOGLISTTEXT);

			return TRUE;
		}
		case DN_CLOSE:
		{
			if (Param1==FD_LISTBOX)
			{
				FindDlgProc(hDlg,DN_BTNCLICK,FD_BUTTON_GOTO,0); // emulates a [ Go to ] button pressing;
			}

			StopSearch=TRUE;
			return TRUE;
		}
		case DN_RESIZECONSOLE:
		{
			PCOORD pCoord = reinterpret_cast<PCOORD>(Param2);
			int IncX = pCoord->X - DlgWidth - 2;
			int IncY = pCoord->Y - DlgHeight - 2;
			SendDlgMessage(hDlg, DM_ENABLEREDRAW, FALSE, 0);

			for (int i = 0; i <= FD_BUTTON_STOP; i++)
			{
				SendDlgMessage(hDlg, DM_SHOWITEM, i, FALSE);
			}

			if ((IncX > 0) || (IncY > 0))
			{
				pCoord->X = DlgWidth + (IncX > 0 ? IncX : 0);
				pCoord->Y = DlgHeight + (IncY > 0 ? IncY : 0);
				SendDlgMessage(hDlg, DM_RESIZEDIALOG, 0, reinterpret_cast<LONG_PTR>(pCoord));
			}

			DlgWidth += IncX;
			DlgHeight += IncY;
			SMALL_RECT rect;

			for (int i = 0; i < FD_SEPARATOR1; i++)
			{
				SendDlgMessage(hDlg, DM_GETITEMPOSITION, i, reinterpret_cast<LONG_PTR>(&rect));
				rect.Right += IncX;
				rect.Bottom += IncY;
				SendDlgMessage(hDlg, DM_SETITEMPOSITION, i, reinterpret_cast<LONG_PTR>(&rect));
			}

			for (int i = FD_SEPARATOR1; i <= FD_BUTTON_STOP; i++)
			{
				SendDlgMessage(hDlg, DM_GETITEMPOSITION, i, reinterpret_cast<LONG_PTR>(&rect));

				if (i == FD_TEXT_STATUS)
				{
					rect.Right += IncX;
				}
				else if (i==FD_TEXT_STATUS_PERCENTS)
				{
					rect.Right+=IncX;
					rect.Left+=IncX;
				}

				rect.Top += IncY;
				SendDlgMessage(hDlg, DM_SETITEMPOSITION, i, reinterpret_cast<LONG_PTR>(&rect));
			}

			if ((IncX <= 0) || (IncY <= 0))
			{
				pCoord->X = DlgWidth;
				pCoord->Y = DlgHeight;
				SendDlgMessage(hDlg, DM_RESIZEDIALOG, 0, reinterpret_cast<LONG_PTR>(pCoord));
			}

			for (int i = 0; i <= FD_BUTTON_STOP; i++)
			{
				SendDlgMessage(hDlg, DM_SHOWITEM, i, TRUE);
			}

			SendDlgMessage(hDlg, DM_ENABLEREDRAW, TRUE, 0);
			return TRUE;
		}
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool FindFiles::FindFilesProcess()
{
	_ALGO(CleverSysLog clv(L"FindFiles::FindFilesProcess()"));
	// Если используется фильтр операций, то во время поиска сообщаем об этом
	string strTitle=MSG(MFindFileTitle);
	string strSearchStr;

	if (!strFindMask.IsEmpty())
	{
		strTitle+=L": ";
		strTitle+=strFindMask;

		if (UseFilter)
		{
			strTitle+=L" (";
			strTitle+=MSG(MFindUsingFilter);
			strTitle+=L")";
		}
	}
	else
	{
		if (UseFilter)
		{
			strTitle+=L" (";
			strTitle+=MSG(MFindUsingFilter);
			strTitle+=L")";
		}
	}

	if (!strFindStr.IsEmpty())
	{
		string strFStr=strFindStr;
		TruncStrFromEnd(strFStr,10);
		InsertQuote(strFStr);
		string strTemp=L" ";
		strTemp+=strFStr;
		strSearchStr.Format(MSG(MFindSearchingIn),strTemp.CPtr());
	}
	else
	{
		strSearchStr.Format(MSG(MFindSearchingIn), L"");
	}

	DlgWidth = ScrX + 1 - 2;
	DlgHeight = ScrY + 1 - 2;
	DialogDataEx FindDlgData[]=
	{
		DI_DOUBLEBOX,3,1,DlgWidth-4,DlgHeight-2,0,0,DIF_SHOWAMPERSAND,0,strTitle,
		DI_LISTBOX,4,2,DlgWidth-5,DlgHeight-7,0,0,DIF_LISTNOBOX,0,0,
		DI_TEXT,0,DlgHeight-6,0,DlgHeight-6,0,0,DIF_SEPARATOR2,0,L"",
		DI_TEXT,5,DlgHeight-5,DlgWidth-(strFindStr.IsEmpty()?6:12),DlgHeight-5,0,0,DIF_SHOWAMPERSAND,0,strSearchStr,
		DI_TEXT,DlgWidth-9,DlgHeight-5,DlgWidth-6,DlgHeight-4,0,0,(strFindStr.IsEmpty()?DIF_HIDDEN:0),0,L"",
		DI_TEXT,0,DlgHeight-4,0,DlgHeight-4,0,0,DIF_SEPARATOR,0,L"",
		DI_BUTTON,0,DlgHeight-3,0,DlgHeight-3,1,0,DIF_CENTERGROUP,1,MSG(MFindNewSearch),
		DI_BUTTON,0,DlgHeight-3,0,DlgHeight-3,0,0,DIF_CENTERGROUP,0,MSG(MFindGoTo),
		DI_BUTTON,0,DlgHeight-3,0,DlgHeight-3,0,0,DIF_CENTERGROUP,0,MSG(MFindView),
		DI_BUTTON,0,DlgHeight-3,0,DlgHeight-3,0,0,DIF_CENTERGROUP,0,MSG(MFindPanel),
		DI_BUTTON,0,DlgHeight-3,0,DlgHeight-3,0,0,DIF_CENTERGROUP,0,MSG(MFindStop),
	};
	MakeDialogItemsEx(FindDlgData,FindDlg);
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	if (PluginMode)
	{
		Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
		HANDLE hPlugin=ActivePanel->GetPluginHandle();
		OpenPluginInfo Info;
		CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
		FindFileArcIndex = AddArcListItem(Info.HostFile, hPlugin, Info.Flags, Info.CurDir);

		if (FindFileArcIndex == LIST_INDEX_NONE)
			return false;

		if ((Info.Flags & OPIF_REALNAMES)==0)
		{
			FindDlg[FD_BUTTON_PANEL].Type=DI_TEXT;
			FindDlg[FD_BUTTON_PANEL].strData.Clear();
		}
	}

	bool AnySetFindList=false;

	for (int i=0; i<CtrlObject->Plugins.GetPluginsCount(); i++)
	{
		if (CtrlObject->Plugins.GetPlugin(i)->HasSetFindList())
		{
			AnySetFindList=true;
			break;
		}
	}

	if (!AnySetFindList)
	{
		FindDlg[FD_BUTTON_PANEL].Flags|=DIF_DISABLE;
	}

	Dialog Dlg=Dialog(FindDlg,countof(FindDlg),FindDlgProc);
//  pDlg->SetDynamicallyBorn();
	Dlg.SetHelp(L"FindFileResult");
	Dlg.SetPosition(-1, -1, DlgWidth, DlgHeight);
	// Надо бы показать диалог, а то инициализация элементов запаздывает
	// иногда при поиске и первые элементы не добавляются
	Dlg.InitDialog();
	Dlg.Show();
	LastFoundNumber=0;
	SearchDone=FALSE;
	StopSearch=FALSE;
	PauseSearch=FALSE;
	FindFileCount=FindDirCount=0;
	FindExitIndex = LIST_INDEX_NONE;
	FindExitCode = FIND_EXIT_NONE;
	FindMessageReady=FindCountReady=FindPositionChanged=0;
	strLastDirName.Clear();
	strFindMessage.Clear();
	strFindPercentMessage.Clear();
	FindMessagePercentReady=false;
	HANDLE Threads[]={NULL,NULL};
#define THREAD_WORK 0
#define THREAD_DISPLAY 1
	Threads[THREAD_WORK]=CreateThread(NULL,0,PluginMode?PreparePluginList:PrepareFilesList,&Dlg,0,NULL);

	if (Threads[THREAD_WORK])
	{
		// Нитка для вывода в диалоге информации о ходе поиска
		Threads[THREAD_DISPLAY]=CreateThread(NULL,0,WriteDialogData,&Dlg,0,NULL);

		if (Threads[THREAD_DISPLAY])
		{
			TB=new TaskBar;
			IsProcessAssignMacroKey++; // отключим все спец. клавиши
			Dlg.Process();
			IsProcessAssignMacroKey--;
			WaitForMultipleObjects(2,Threads,TRUE,INFINITE);
			CloseHandle(Threads[THREAD_WORK]);
			CloseHandle(Threads[THREAD_DISPLAY]);

			switch (FindExitCode)
			{
				case FIND_EXIT_SEARCHAGAIN:
				{
					return true;
				}
				case FIND_EXIT_PANEL:
					// Отработаем переброску на временную панель
				{
					size_t ListSize = FindListCount;
					PluginPanelItem *PanelItems=new PluginPanelItem[ListSize];

					if (PanelItems==NULL)
						ListSize=0;

					int ItemsNumber=0;

					for (size_t i=0; i<ListSize; i++)
					{
						if (StrLength(FindList[i]->FindData.strFileName)>0 && FindList[i]->Used)
							// Добавляем всегда, если имя задано
						{
							// Для плагинов с виртуальными именами заменим имя файла на имя архива.
							// панель сама уберет лишние дубли.
							bool IsArchive = ((FindList[i]->ArcIndex != LIST_INDEX_NONE) &&
							                  !(ArcList[FindList[i]->ArcIndex]->Flags&OPIF_REALNAMES));

							// Добавляем только файлы или имена архивов или папки когда просили
							if (IsArchive || (Opt.FindOpt.FindFolders && !SearchHex) ||
							        !(FindList[i]->FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
							{
								if (IsArchive)
									FindList[i]->FindData.strFileName = ArcList[FindList[i]->ArcIndex]->strArcName;

								PluginPanelItem *pi=&PanelItems[ItemsNumber++];
								memset(pi,0,sizeof(*pi));
								apiFindDataExToData(&FindList[i]->FindData, &pi->FindData);

								if (IsArchive)
									pi->FindData.dwFileAttributes = 0;

								/* $ 21.03.2002 VVM
									+ Передаем имена каталогов без заключительного "\" */
								if (pi->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
								{
									int Length = StrLength(pi->FindData.lpwszFileName);

									if ((Length) && IsSlash(pi->FindData.lpwszFileName[Length-1]))
										pi->FindData.lpwszFileName[Length-1] = 0;
								}
							}
						}
					}

					HANDLE hNewPlugin=CtrlObject->Plugins.OpenFindListPlugin(PanelItems,ItemsNumber);

					if (hNewPlugin!=INVALID_HANDLE_VALUE)
					{
						Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
						Panel *NewPanel=CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
						NewPanel->SetPluginMode(hNewPlugin,L"",true);
						NewPanel->SetVisible(TRUE);
						NewPanel->Update(0);
						//if (FindExitIndex != LIST_INDEX_NONE)
						//NewPanel->GoToFile(FindList[FindExitIndex].FindData.cFileName);
						NewPanel->Show();
					}

					for (int i = 0; i < ItemsNumber; i++)
						apiFreeFindData(&PanelItems[i].FindData);

					delete[] PanelItems;
					break;
				}
				case FIND_EXIT_GOTO:
				{
					string strFileName=FindList[FindExitIndex]->FindData.strFileName;
					Panel *FindPanel=CtrlObject->Cp()->ActivePanel;

					if (FindList[FindExitIndex]->ArcIndex != LIST_INDEX_NONE)
					{
						HANDLE hPlugin = ArcList[FindList[FindExitIndex]->ArcIndex]->hPlugin;

						if (hPlugin == INVALID_HANDLE_VALUE)
						{
							string strArcName = ArcList[FindList[FindExitIndex]->ArcIndex]->strArcName;

							if (FindPanel->GetType()!=FILE_PANEL)
							{
								FindPanel=CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);
							}

							string strArcPath=strArcName;
							CutToSlash(strArcPath);
							FindPanel->SetCurDir(strArcPath,TRUE);
							hPlugin=((FileList *)FindPanel)->OpenFilePlugin(strArcName,FALSE);

							if (hPlugin==(HANDLE)-2)
								hPlugin = INVALID_HANDLE_VALUE;
						}

						if (hPlugin != INVALID_HANDLE_VALUE)
						{
							OpenPluginInfo Info;
							CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

							/* $ 19.01.2003 KM
								 Уточнение перехода в нужный каталог плагина.
							*/
							if (SearchMode==FFSEARCH_ROOT ||
							        SearchMode==FFSEARCH_ALL ||
							        SearchMode==FFSEARCH_ALL_BUTNETWORK ||
							        SearchMode==FFSEARCH_INPATH)
								CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",0);

							SetPluginDirectory(strFileName,hPlugin,TRUE);
						}
					}
					else
					{
						string strSetName;
						size_t Length=strFileName.GetLength();

						if (!Length)
							break;

						if (Length>1 && IsSlash(strFileName.At(Length-1)) && strFileName.At(Length-2)!=L':')
							strFileName.SetLength(Length-1);

						if ((apiGetFileAttributes(strFileName)==INVALID_FILE_ATTRIBUTES) && (GetLastError() != ERROR_ACCESS_DENIED))
							break;

						{
							const wchar_t *NamePtr = PointToName(strFileName);
							strSetName = NamePtr;

							if (Opt.FindOpt.FindAlternateStreams)
							{
								size_t Pos=0;

								if (strSetName.Pos(Pos,L':'))
									strSetName.SetLength(Pos);
							}

							strFileName.SetLength(NamePtr-(const wchar_t *)strFileName);
							Length=strFileName.GetLength();

							if (Length>1 && IsSlash(strFileName.At(Length-1)) && strFileName.At(Length-2)!=L':')
								strFileName.SetLength(Length-1);
						}

						if (strFileName.IsEmpty())
							break;

						if (FindPanel->GetType()!=FILE_PANEL &&
						        CtrlObject->Cp()->GetAnotherPanel(FindPanel)->GetType()==FILE_PANEL)
							FindPanel=CtrlObject->Cp()->GetAnotherPanel(FindPanel);

						if ((FindPanel->GetType()!=FILE_PANEL) || (FindPanel->GetMode()!=NORMAL_PANEL))
							// Сменим панель на обычную файловую...
						{
							FindPanel=CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);
							FindPanel->SetVisible(TRUE);
							FindPanel->Update(0);
						}

						/* $ 09.06.2001 IS
							 ! Не меняем каталог, если мы уже в нем находимся. Тем самым
								 добиваемся того, что выделение с элементов панели не сбрасывается.
						*/
						{
							string strDirTmp;
							FindPanel->GetCurDir(strDirTmp);
							Length=strDirTmp.GetLength();

							if (Length>1 && IsSlash(strDirTmp.At(Length-1)) && strDirTmp.At(Length-2)!=L':')
								strDirTmp.SetLength(Length-1);

							if (StrCmpI(strFileName, strDirTmp))
								FindPanel->SetCurDir(strFileName,TRUE);
						}

						if (!strSetName.IsEmpty())
							FindPanel->GoToFile(strSetName);

						FindPanel->Show();
						FindPanel->SetFocus();
					}

					break;
				}
			}
		}
	}

	return false;
}

void FindFiles::DoScanTree(HANDLE hDlg,string& strRoot)
{
	ScanTree ScTree(FALSE,!(SearchMode==FFSEARCH_CURRENT_ONLY||SearchMode==FFSEARCH_INPATH),Opt.FindOpt.FindSymLinks);
	string strSelName;
	DWORD FileAttr;

	if (SearchMode==FFSEARCH_SELECTED)
		CtrlObject->Cp()->ActivePanel->GetSelName(NULL,FileAttr);

	while (1)
	{
		string strCurRoot;

		if (SearchMode==FFSEARCH_SELECTED)
		{
			if (!CtrlObject->Cp()->ActivePanel->GetSelName(&strSelName,FileAttr))
				break;

			if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0 || TestParentFolderName(strSelName) ||
			        StrCmp(strSelName,L".")==0)
				continue;

			strCurRoot = strRoot;
			AddEndSlash(strCurRoot);
			strCurRoot += strSelName;
		}
		else
		{
			strCurRoot = strRoot;
		}

		ScTree.SetFindPath(strCurRoot,L"*");
		statusCS.Enter();
		strFindMessage = strCurRoot;
		FindMessageReady=TRUE;
		statusCS.Leave();
		FAR_FIND_DATA_EX FindData;
		string strFullName;

		while (!StopSearch && ScTree.GetNextName(&FindData,strFullName))
		{
			while (PauseSearch)
			{
				Sleep(1);
			}

			bool bContinue=false;
			WIN32_FIND_STREAM_DATA sd;
			HANDLE hFindStream=INVALID_HANDLE_VALUE;
			bool FirstCall=true;
			string strFindDataFileName=FindData.strFileName;

			if (Opt.FindOpt.FindAlternateStreams)
			{
				hFindStream=apiFindFirstStream(strFullName,FindStreamInfoStandard,&sd);
			}

			while (true)
			{
				string strFullStreamName=strFullName;

				if (Opt.FindOpt.FindAlternateStreams)
				{
					if (hFindStream!=INVALID_HANDLE_VALUE)
					{
						if (!FirstCall)
						{
							if (!apiFindNextStream(hFindStream,&sd))
							{
								apiFindStreamClose(hFindStream);
								break;
							}
						}
						else
						{
							FirstCall=false;
						}

						LPWSTR NameEnd=wcschr(sd.cStreamName+1,L':');

						if (NameEnd)
						{
							*NameEnd=L'\0';
						}

						if (sd.cStreamName[1]) // alternate stream
						{
							strFullStreamName+=sd.cStreamName;
							FindData.strFileName=strFindDataFileName+sd.cStreamName;
							FindData.nFileSize=sd.StreamSize.QuadPart;
						}
					}
					else
					{
						if (bContinue)
						{
							break;
						}
					}
				}

				if (UseFilter)
				{
					enumFileInFilterType foundType;

					if (!Filter->FileInFilter(&FindData,&foundType))
					{
						// сюда заходим, если не попали в фильтр или попали в Exclude-фильтр
						if ((FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && foundType==FIFT_EXCLUDE)
							ScTree.SkipDir(); // скипаем только по Exclude-фильтру, т.к. глубже тоже нужно просмотреть

						{
							bContinue=true;

							if (Opt.FindOpt.FindAlternateStreams)
							{
								continue;
							}
							else
							{
								break;
							}
						}
					}
				}

				if (((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strFindStr.IsEmpty()) ||
				        (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !strFindStr.IsEmpty()))
				{
					statusCS.Enter();
					strFindMessage = strFullName;
					FindMessageReady=TRUE;
					statusCS.Leave();
				}

				if (IsFileIncluded(NULL,strFullStreamName,FindData.dwFileAttributes))
				{
					AddMenuRecord(hDlg,strFullStreamName,&FindData);
				}

				if (!Opt.FindOpt.FindAlternateStreams || hFindStream==INVALID_HANDLE_VALUE)
				{
					break;
				}
			}

			if (bContinue)
			{
				continue;
			}

			if (SearchInArchives)
				ArchiveSearch(hDlg,strFullName);
		}

		if (SearchMode!=FFSEARCH_SELECTED)
			break;
	}
}
void FindFiles::DoPrepareFileList(HANDLE hDlg)
{
	string strRoot;
	wchar_t *Ptr=NULL;
	DWORD DiskMask=FarGetLogicalDrives();
	//string strRoot; //BUGBUG
	CtrlObject->CmdLine->GetCurDir(strRoot);

	if (SearchMode==FFSEARCH_INPATH)
	{
		string strPathEnv;
		apiGetEnvironmentVariable(L"PATH",strPathEnv);
		strPathEnv.Append(L'\0');
		wchar_t* PathEnv = strPathEnv.GetBuffer();
		Ptr = PathEnv;

		while (*PathEnv)
		{
			if (*PathEnv==L';')
				*PathEnv=0;

			++PathEnv;
		}
	}

	for (WCHAR CurrentDisk=0;; CurrentDisk++,DiskMask>>=1)
	{
		if (SearchMode==FFSEARCH_ALL ||
		        SearchMode==FFSEARCH_ALL_BUTNETWORK)
		{
			if (DiskMask==0)
				break;

			if ((DiskMask & 1)==0)
				continue;

			const wchar_t Root[]={L'A'+CurrentDisk,L':',L'\\',L'\0'};
			strRoot=Root;
			int DriveType=FAR_GetDriveType(strRoot);

			if (DriveType==DRIVE_REMOVABLE || IsDriveTypeCDROM(DriveType) ||
			        (DriveType==DRIVE_REMOTE && SearchMode==FFSEARCH_ALL_BUTNETWORK))
			{
				if (DiskMask==1)
					break;
				else
					continue;
			}
		}
		else if (SearchMode==FFSEARCH_ROOT)
		{
			GetPathRoot(strRoot,strRoot);
		}
		else if (SearchMode==FFSEARCH_INPATH)
		{
			if (!*Ptr)
				break;

			strRoot = Ptr;
			Ptr+=StrLength(Ptr)+1;
		}

		DoScanTree(hDlg, strRoot);

		if (SearchMode!=FFSEARCH_ALL && SearchMode!=FFSEARCH_ALL_BUTNETWORK && SearchMode!=FFSEARCH_INPATH)
			break;
	}

	while (!StopSearch && (FindMessageReady||FindMessagePercentReady))
	{
		Sleep(1);
	}

	statusCS.Enter();
	strFindPercentMessage.Clear();
	FindMessagePercentReady=true;
	strFindMessage.Format(MSG(MFindDone),FindFileCount,FindDirCount);
	ConsoleTitle::SetFarTitle(strFindMessage);
	SearchDone=TRUE;
	FindMessageReady=TRUE;
	statusCS.Leave();
}
DWORD WINAPI FindFiles::PrepareFilesList(void *Param)
{
	__try
	{
		InitInFileSearch();
		DoPrepareFileList(reinterpret_cast<HANDLE>(Param));
		ReleaseInFileSearch();
	}
	__except(xfilter((int)(INT_PTR)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1))
	{
		TerminateProcess(GetCurrentProcess(), 1);
	}
	return 0;
}

void FindFiles::ArchiveSearch(HANDLE hDlg,const wchar_t *ArcName)
{
	_ALGO(CleverSysLog clv(L"FindFiles::ArchiveSearch()"));
	_ALGO(SysLog(L"ArcName='%s'",(ArcName?ArcName:L"NULL")));
	char *Buffer=new char[Opt.PluginMaxReadData];

	if (!Buffer)
	{
		_ALGO(SysLog(L"ERROR: alloc buffer (size=%u)",Opt.PluginMaxReadData));
		return;
	}

	FILE *ProcessFile=_wfopen(ArcName,L"rb");

	if (ProcessFile==NULL)
	{
		delete[] Buffer;
		return;
	}

	int ReadSize=(int)fread(Buffer,1,Opt.PluginMaxReadData,ProcessFile);
	fclose(ProcessFile);
	int SavePluginsOutput=DisablePluginsOutput;
	DisablePluginsOutput=TRUE;
	string strArcName = ArcName;
	HANDLE hArc=CtrlObject->Plugins.OpenFilePlugin(strArcName,(unsigned char *)Buffer,ReadSize,OPM_FIND);
	DisablePluginsOutput=SavePluginsOutput;
	delete[] Buffer;

	if (hArc==(HANDLE)-2)
	{
		BreakMainThread=true;
		_ALGO(SysLog(L"return: hArc==(HANDLE)-2"));
		return;
	}

	if (hArc==INVALID_HANDLE_VALUE)
	{
		_ALGO(SysLog(L"return: hArc==INVALID_HANDLE_VALUE"));
		return;
	}

	int SaveSearchMode=SearchMode;
	size_t SaveArcIndex = FindFileArcIndex;
	{
		SearchMode=FFSEARCH_FROM_CURRENT;
		OpenPluginInfo Info;
		CtrlObject->Plugins.GetOpenPluginInfo(hArc,&Info);
		FindFileArcIndex = AddArcListItem(ArcName, hArc, Info.Flags, Info.CurDir);
		/* $ 11.12.2001 VVM
		  - Запомним каталог перед поиском в архиве.
		    И если ничего не нашли - не рисуем его снова */
		{
			string strSaveDirName, strSaveSearchPath;
			size_t SaveListCount = FindListCount;
			/* $ 19.01.2003 KM
			   Запомним пути поиска в плагине, они могут измениться.
			*/
			strSaveSearchPath = strPluginSearchPath;
			strSaveDirName = strLastDirName;
			strLastDirName.Clear();
			DoPreparePluginList(hDlg,true);
			strPluginSearchPath = strSaveSearchPath;
			WaitForSingleObject(hPluginMutex,INFINITE);
			CtrlObject->Plugins.ClosePlugin(ArcList[FindFileArcIndex]->hPlugin);
			ArcList[FindFileArcIndex]->hPlugin = INVALID_HANDLE_VALUE;
			ReleaseMutex(hPluginMutex);

			if (SaveListCount == FindListCount)
				strLastDirName = strSaveDirName;
		}
	}
	FindFileArcIndex = SaveArcIndex;
	SearchMode=SaveSearchMode;
}

void FindFiles::AddMenuRecord(HANDLE hDlg,const wchar_t *FullName, FAR_FIND_DATA *FindData)
{
	FAR_FIND_DATA_EX fdata;
	apiFindDataToDataEx(FindData, &fdata);
	AddMenuRecord(hDlg,FullName,&fdata);
}

void FindFiles::AddMenuRecord(HANDLE hDlg,const wchar_t *FullName, FAR_FIND_DATA_EX *FindData)
{
	if (!hDlg)
		return;

	VMenu *ListBox=reinterpret_cast<Dialog*>(hDlg)->Item[1]->ListPtr;

	if (!ListBox)
		return;

	if ((FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		if ((FindData->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) || (FindData->dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)) apiGetCompressedFileSize(FullName,FindData->nPackSize);
		else FindData->nPackSize=FindData->nFileSize;
	}

	MenuItemEx ListItem;
	ListItem.Clear();

	FormatString MenuText;

	// Отображаем дату последнего изменения
	string strDateStr, strTimeStr;
	ConvertDate(FindData->ftLastWriteTime, strDateStr, strTimeStr, 5);
	MenuText << L' ' << fmt::Width(8) << fmt::Precision(8) << strDateStr << L' ' << fmt::Width(5) << fmt::Precision(5) << strTimeStr << BoxSymbols[BS_V1];

	MenuText << fmt::Width(13);
	if (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		MenuText << MSG(MFindFileFolder);
	else
		MenuText << FindData->nFileSize;
	MenuText << BoxSymbols[BS_V1];

	/* $ 05.10.2003 KM
	   Отобразим в панели поиска атрибуты найденных файлов
	*/
	const wchar_t AttrStr[]=
	{
		FindData->dwFileAttributes&FILE_ATTRIBUTE_READONLY?L'R':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM?L'S':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN?L'H':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE?L'A':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT?L'L':FindData->dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE?L'$':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED?L'C':FindData->dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED?L'E':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_TEMPORARY?L'T':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED?L'I':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_OFFLINE?L'O':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_VIRTUAL?L'V':L' ',
		0
	};
	MenuText << AttrStr << BoxSymbols[BS_V1];

	const wchar_t *DisplayName=FindData->strFileName;
	/* $ 24.03.2002 KM
		В плагинах принудительно поставим указатель в имени на имя
		для корректного его отображения в списке, отбросив путь,
		т.к. некоторые плагины возвращают имя вместе с полным путём,
		к примеру временная панель.
	*/
	if (FindFileArcIndex != LIST_INDEX_NONE)
		DisplayName = PointToName(DisplayName);
	MenuText << DisplayName;

	string strPathName=FullName;
	{
		size_t pos;

		if (FindLastSlash(pos,strPathName))
			strPathName.SetLength(pos);
		else
			strPathName.Clear();
	}
	AddEndSlash(strPathName);

	if (StrCmpI(strPathName,strLastDirName)!=0)
	{
		if (!strLastDirName.IsEmpty())
		{
			ListItem.Flags|=LIF_SEPARATOR;
			ListBox->AddItem(&ListItem);
			ListItem.Flags&=~LIF_SEPARATOR;
		}

		ffCS.Enter();
		strLastDirName = strPathName;

		if ((FindFileArcIndex != LIST_INDEX_NONE) &&
		        (!(ArcList[FindFileArcIndex]->Flags & OPIF_REALNAMES)) &&
		        (!ArcList[FindFileArcIndex]->strArcName.IsEmpty()))
		{
			string strArcPathName=ArcList[FindFileArcIndex]->strArcName;
			strArcPathName+=L":";

			if (!IsSlash(strPathName.At(0)))
				AddEndSlash(strArcPathName);

			strArcPathName+=(!StrCmp(strPathName,L".\\")?L"\\":(const wchar_t*)strPathName);
			strPathName = strArcPathName;
		}

		ListItem.strName = strPathName;
		size_t ItemIndex = AddFindListItem(FindData);

		if (ItemIndex != LIST_INDEX_NONE)
		{
			// Сбросим данные в FindData. Они там от файла
			FindList[ItemIndex]->FindData.Clear();
			// Используем LastDirName, т.к. PathName уже может быть искажена
			FindList[ItemIndex]->FindData.strFileName = strLastDirName;
			/* $ 07.04.2002 KM
				- Вместо пустого имени используем флаг, в противном
					случае не работает переход на каталог из списка.
					Used=0 - Имя не попададёт во временную панель.
			*/
			FindList[ItemIndex]->Used=0;
			// Поставим атрибут у каталога, что-бы он не был файлом :)
			FindList[ItemIndex]->FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

			if (FindFileArcIndex != LIST_INDEX_NONE)
				FindList[ItemIndex]->ArcIndex = FindFileArcIndex;

			ListBox->SetUserData((void*)(DWORD_PTR)ItemIndex,sizeof(ItemIndex),ListBox->AddItem(&ListItem));
		}

		ffCS.Leave();
	}

	/* $ 24.03.2002 KM
		Дополнительно добавляем в список найденного
		папки. Так было в 1.65 и в 3 бете, но при
		полной переделке поиска я это где-то отломил,
		теперь возвращаю на место.
	*/
	ffCS.Enter();
	size_t ItemIndex = AddFindListItem(FindData);

	if (ItemIndex != LIST_INDEX_NONE)
	{
		FindList[ItemIndex]->FindData.strFileName = FullName;
		/* $ 07.04.2002 KM
			Used=1 - Имя попададёт во временную панель.
		*/
		FindList[ItemIndex]->Used=1;

		if (FindFileArcIndex != LIST_INDEX_NONE)
			FindList[ItemIndex]->ArcIndex = FindFileArcIndex;
	}

	ListItem.strName = MenuText.strValue();
	ffCS.Leave();
	int ListPos = ListBox->AddItem(&ListItem);
	ListBox->SetUserData((void*)(DWORD_PTR)ItemIndex,sizeof(ItemIndex), ListPos);

	// Выделим как положено - в списке.
	if (!FindFileCount && !FindDirCount)
		ListBox->SetSelectPos(ListPos, -1);

	statusCS.Enter();

	if (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		FindDirCount++;
	else
		FindFileCount++;

	statusCS.Leave();
	LastFoundNumber++;
	FindCountReady=TRUE;
}

void FindFiles::DoPreparePluginList(HANDLE hDlg,bool Internal)
{
	Sleep(1);
	HANDLE hPlugin=ArcList[FindFileArcIndex]->hPlugin;
	OpenPluginInfo Info;
	WaitForSingleObject(hPluginMutex,INFINITE);
	CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
	string strSaveDir = Info.CurDir;

	if (SearchMode==FFSEARCH_ROOT ||
	        SearchMode==FFSEARCH_ALL ||
	        SearchMode==FFSEARCH_ALL_BUTNETWORK ||
	        SearchMode==FFSEARCH_INPATH)
	{
		CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_FIND);
		CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
	}

	ReleaseMutex(hPluginMutex);
	strPluginSearchPath=Info.CurDir;

	if (!strPluginSearchPath.IsEmpty())
		AddEndSlash(strPluginSearchPath);

	RecurseLevel=0;
	ScanPluginTree(hDlg,hPlugin,ArcList[FindFileArcIndex]->Flags);
	WaitForSingleObject(hPluginMutex,INFINITE);

	if (SearchMode==FFSEARCH_ROOT ||
	        SearchMode==FFSEARCH_ALL ||
	        SearchMode==FFSEARCH_ALL_BUTNETWORK ||
	        SearchMode==FFSEARCH_INPATH)
		CtrlObject->Plugins.SetDirectory(hPlugin,strSaveDir,OPM_FIND);

	ReleaseMutex(hPluginMutex);

	while (!StopSearch && (FindMessageReady||FindMessagePercentReady))
	{
		Sleep(1);
	}

	if (!Internal)
	{
		statusCS.Enter();
		strFindPercentMessage.Clear();
		FindMessagePercentReady=true;
		strFindMessage.Format(MSG(MFindDone),FindFileCount,FindDirCount);
		FindMessageReady=TRUE;
		SearchDone=TRUE;
		statusCS.Leave();
	}
}

DWORD WINAPI FindFiles::PreparePluginList(void *Param)
{
	__try
	{
		InitInFileSearch();
		DoPreparePluginList(reinterpret_cast<HANDLE>(Param), false);
		ReleaseInFileSearch();
	}
	__except(xfilter((int)(INT_PTR)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1))
	{
		TerminateProcess(GetCurrentProcess(), 1);
	}
	return 0;
}

void FindFiles::ScanPluginTree(HANDLE hDlg,HANDLE hPlugin, DWORD Flags)
{
	PluginPanelItem *PanelData=NULL;
	int ItemCount=0;
	WaitForSingleObject(hPluginMutex,INFINITE);

	if (StopSearch || !CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&ItemCount,OPM_FIND))
	{
		ReleaseMutex(hPluginMutex);
		return;
	}
	else
	{
		ReleaseMutex(hPluginMutex);
	}

	RecurseLevel++;
	/* $ 19.01.2003 KM
	   Отключу пока сортировку, что-то результаты получаются
	   не совсем те, которые я ожидал.
	*/
	/* $ 24.03.2002 KM
	   Сортировку в плагинах по именам папок делаем в случае,
	   если имя файла содержит в себе путь, а не только при
	   OPIF_REALNAMES, в противном случае в результатах поиска
	   получается некрасивый визуальный эффект разбросанности
	   одинаковых папок по списку.
	*/
//  if (PanelData && strlen(PointToName(PanelData->FindData.cFileName))>0)
//    far_qsort((void *)PanelData,ItemCount,sizeof(*PanelData),SortItems);

	if (SearchMode!=FFSEARCH_SELECTED || RecurseLevel!=1)
	{
		for (int I=0; I<ItemCount && !StopSearch; I++)
		{
			while (PauseSearch)
			{
				Sleep(1);
			}

			PluginPanelItem *CurPanelItem=PanelData+I;
			string strCurName=CurPanelItem->FindData.lpwszFileName;
			string strFullName;

			if (StrCmp(strCurName,L".")==0 || TestParentFolderName(strCurName))
				continue;

			strFullName = strPluginSearchPath;
			strFullName += strCurName;

			/* $ 30.09.2003 KM
			  Отфильтруем файлы не попадающие в действующий фильтр
			*/
			if (!UseFilter || Filter->FileInFilter(&CurPanelItem->FindData))
			{
				/* $ 14.06.2004 KM
				  Уточнение действия при обработке каталогов
				*/
				if (((CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strFindStr.IsEmpty()) ||
				        (!(CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !strFindStr.IsEmpty()))
				{
					statusCS.Enter();
					strFindMessage = strFullName;
					FindMessageReady=TRUE;
					statusCS.Leave();
				}

				if (IsFileIncluded(CurPanelItem,strCurName,CurPanelItem->FindData.dwFileAttributes))
					AddMenuRecord(hDlg,strFullName,&CurPanelItem->FindData);

				if (SearchInArchives && (hPlugin != INVALID_HANDLE_VALUE) && (Flags & OPIF_REALNAMES))
					ArchiveSearch(hDlg,strFullName);
			}
		}
	}

	if (SearchMode!=FFSEARCH_CURRENT_ONLY)
	{
		for (int I=0; I<ItemCount && !StopSearch; I++)
		{
			PluginPanelItem *CurPanelItem=PanelData+I;
			string strCurName=CurPanelItem->FindData.lpwszFileName;

			if ((CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			        StrCmp(strCurName,L".")!=0 && !TestParentFolderName(strCurName) &&
			        (!UseFilter || Filter->FileInFilter(&CurPanelItem->FindData)) &&
			        (SearchMode!=FFSEARCH_SELECTED || RecurseLevel!=1 ||
			         CtrlObject->Cp()->ActivePanel->IsSelected(strCurName)))
			{
				WaitForSingleObject(hPluginMutex,INFINITE);
				size_t pos;

				if (CtrlObject->Plugins.SetDirectory(hPlugin,strCurName,OPM_FIND))
				{
					ReleaseMutex(hPluginMutex);
					strPluginSearchPath += strCurName;
					strPluginSearchPath += L"\\";
					ScanPluginTree(hDlg, hPlugin, Flags);

					if (strPluginSearchPath.RPos(pos,L'\\'))
						strPluginSearchPath.SetLength(pos);

					if (strPluginSearchPath.RPos(pos,L'\\'))
						strPluginSearchPath.SetLength(pos+1);
					else
						strPluginSearchPath.Clear();

					WaitForSingleObject(hPluginMutex,INFINITE);

					if (!CtrlObject->Plugins.SetDirectory(hPlugin,L"..",OPM_FIND))
						StopSearch=TRUE;

					ReleaseMutex(hPluginMutex);
				}
				else
				{
					ReleaseMutex(hPluginMutex);
				}
			}

			if (StopSearch) break;
		}
	}

	CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,ItemCount);
	RecurseLevel--;
}

void FindFiles::DoWriteDialogData(HANDLE hDlg)
{
	string strDataStr;
	Dialog* Dlg=reinterpret_cast<Dialog*>(hDlg);
	DWORD StartTime=GetTickCount();

	if (Dlg)
	{
		while (true)
		{
			VMenu *ListBox=Dlg->Item[1]->ListPtr;

			if (ListBox && !PauseSearch && !ScreenSaverActive)
			{
				if (BreakMainThread)
					StopSearch=TRUE;

				DWORD CurTime=GetTickCount();

				if (CurTime-StartTime<RedrawTimeout)
				{
					Sleep(1); //иначе крутим почти пустой цикл и жрём процессор.
					continue;
				}

				StartTime=CurTime;

				if (FindCountReady)
				{
					statusCS.Enter();
					strDataStr.Format(MSG(MFindFound),FindFileCount,FindDirCount);
					statusCS.Leave();
					SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(LONG_PTR)(const wchar_t*)strDataStr);
					FindCountReady=FALSE;
				}

				if (FindMessageReady)
				{
					string strSearchStr;

					if (!strFindStr.IsEmpty())
					{
						string strFStr(strFindStr);
						TruncStrFromEnd(strFStr,10);
						string strTemp(L" \"");
						strTemp+=strFStr+="\"";
						strSearchStr.Format(MSG(MFindSearchingIn), (const wchar_t*)strTemp);
					}
					else
						strSearchStr.Format(MSG(MFindSearchingIn), L"");

					int StatusTextWidth = DlgWidth - 10;

					if (SearchDone)
					{
						SendDlgMessage(hDlg, DM_ENABLEREDRAW, FALSE, 0);
						strDataStr = MSG(MFindCancel);
						SendDlgMessage(hDlg, DM_SETTEXTPTR, FD_BUTTON_STOP, reinterpret_cast<LONG_PTR>(strDataStr.CPtr()));
						statusCS.Enter();
						strDataStr.Format(L"%-*.*s", StatusTextWidth, StatusTextWidth, strFindMessage.CPtr());
						statusCS.Leave();
						SendDlgMessage(hDlg, DM_SETTEXTPTR, FD_TEXT_STATUS, reinterpret_cast<LONG_PTR>(strDataStr.CPtr()));
						strDataStr.Clear();
						SendDlgMessage(hDlg, DM_SETTEXTPTR, FD_SEPARATOR1, reinterpret_cast<LONG_PTR>(strDataStr.CPtr()));
						SendDlgMessage(hDlg, DM_ENABLEREDRAW, TRUE, 0);
						ConsoleTitle::SetFarTitle(strFindMessage);
						StopSearch=TRUE;
					}
					else
					{
						int Wid1 = static_cast<int>(strSearchStr.GetLength());
						int Wid2 = StatusTextWidth - Wid1 - 1;
						statusCS.Enter();
						TruncStrFromCenter(strFindMessage, Wid2);
						strDataStr.Format(L"%-*.*s %-*.*s", Wid1, Wid1, strSearchStr.CPtr(), Wid2, Wid2, strFindMessage.CPtr());
						statusCS.Leave();
						SendDlgMessage(hDlg, DM_SETTEXTPTR, FD_TEXT_STATUS, reinterpret_cast<LONG_PTR>(strDataStr.CPtr()));
					}

					FindMessageReady=FALSE;
				}

				if (FindMessagePercentReady)
				{
					statusCS.Enter();
					strDataStr=strFindPercentMessage;
					statusCS.Leave();
					SendDlgMessage(hDlg, DM_SETTEXTPTR,FD_TEXT_STATUS_PERCENTS,reinterpret_cast<LONG_PTR>(strDataStr.CPtr()));
					FindMessagePercentReady=false;
				}

				if (LastFoundNumber && ListBox)
				{
					LastFoundNumber=0;

					if (ListBox->UpdateRequired())
						SendDlgMessage(hDlg,DM_SHOWITEM,1,1);
				}
			}

			if (StopSearch && SearchDone && !FindMessageReady && !FindCountReady && !LastFoundNumber &&!FindMessagePercentReady)
				break;

			Sleep(1);
		}
	}
}
DWORD WINAPI FindFiles::WriteDialogData(void *Param)
{
	__try
	{
		DoWriteDialogData(reinterpret_cast<HANDLE>(Param));
	}
	__except(xfilter((int)(INT_PTR)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1))
	{
		TerminateProcess(GetCurrentProcess(), 1);
	}
	return 0;
}
FindFiles::FindFiles()
{
	_ALGO(CleverSysLog clv(L"FindFiles::FindFiles()"));
	static string strLastFindMask=L"*.*", strLastFindStr;
	// Статической структуре и статические переменные
	static string strSearchFromRoot;
	static int LastCmpCase=0,LastWholeWords=0,LastSearchInArchives=0,LastSearchHex=0;
	// Создадим объект фильтра
	Filter=new FileFilter(CtrlObject->Cp()->ActivePanel,FFT_FINDFILE);
	CmpCase=LastCmpCase;
	WholeWords=LastWholeWords;
	SearchInArchives=LastSearchInArchives;
	SearchHex=LastSearchHex;
	SearchMode=Opt.FindOpt.FileSearchMode;
	UseFilter=Opt.FindOpt.UseFilter;
	strFindMask = strLastFindMask;
	strFindStr = strLastFindStr;
	BreakMainThread=false;
	strSearchFromRoot = MSG(MSearchFromRootFolder);
	FindList = NULL;
	ArcList = NULL;
	hPluginMutex=CreateMutexW(NULL,FALSE,NULL);

	do
	{
		ClearAllLists();
		Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
		PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();
		PrepareDriveNameStr(strSearchFromRoot);
		const wchar_t *MasksHistoryName=L"Masks",*TextHistoryName=L"SearchText";
		const wchar_t *HexMask=L"HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH";
		static const wchar_t VSeparator[]={BoxSymbols[BS_T_H1V1],BoxSymbols[BS_V1],BoxSymbols[BS_V1],BoxSymbols[BS_V1],BoxSymbols[BS_B_H1V1],0};
		struct DialogDataEx FindAskDlgData[]=
		{
			/* 00 */DI_DOUBLEBOX,3,1,74,20,0,0,0,0,(const wchar_t *)MFindFileTitle,
			/* 01 */DI_TEXT,5,2,0,2,0,0,0,0,(const wchar_t *)MFindFileMasks,
			/* 02 */DI_EDIT,5,3,72,3,1,(DWORD_PTR)MasksHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,0,L"",
			/* 03 */DI_TEXT,3,4,0,4,0,0,DIF_SEPARATOR,0,L"",
			/* 04 */DI_TEXT,5,5,0,5,0,0,0,0,L"",
			/* 05 */DI_EDIT,5,6,72,6,0,(DWORD_PTR)TextHistoryName,DIF_HISTORY,0,L"",
			/* 06 */DI_FIXEDIT,5,6,72,6,0,(DWORD_PTR)HexMask,DIF_MASKEDIT,0,L"",
			/* 07 */DI_TEXT,5,7,0,7,0,0,0,0,L"",
			/* 08 */DI_COMBOBOX,5,8,72,8,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,L"",
			/* 09 */DI_TEXT,3,9,0,9,0,0,DIF_SEPARATOR,0,L"",
			/* 10 */DI_CHECKBOX,5,10,0,10,0,0,0,0,(const wchar_t *)MFindFileCase,
			/* 11 */DI_CHECKBOX,5,11,0,11,0,0,0,0,(const wchar_t *)MFindFileWholeWords,
			/* 12 */DI_CHECKBOX,5,12,0,12,0,0,0,0,(const wchar_t *)MSearchForHex,
			/* 13 */DI_CHECKBOX,40,10,0,10,0,0,0,0,(const wchar_t *)MFindArchives,
			/* 14 */DI_CHECKBOX,40,11,0,11,0,0,0,0,(const wchar_t *)MFindFolders,
			/* 15 */DI_CHECKBOX,40,12,0,12,0,0,0,0,(const wchar_t *)MFindSymLinks,
			/* 16 */DI_TEXT,3,13,0,13,0,0,DIF_SEPARATOR,0,L"",
			/* 17 */DI_VTEXT,38,9,0,9,0,0,DIF_BOXCOLOR,0,VSeparator,
			/* 18 */DI_TEXT,5,14,0,14,0,0,0,0,(const wchar_t *)MSearchWhere,
			/* 19 */DI_COMBOBOX,5,15,72,15,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,L"",
			/* 20 */DI_TEXT,3,16,0,16,0,0,DIF_SEPARATOR,0,L"",
			/* 21 */DI_CHECKBOX,5,17,0,17,0,0,0,0,(const wchar_t *)MFindUseFilter,
			/* 22 */DI_TEXT,3,18,0,18,0,0,DIF_SEPARATOR,0,L"",
			/* 23 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MFindFileFind,
			/* 24 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindFileDrive,
			/* 25 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindFileSetFilter,
			/* 26 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindFileAdvanced,
			/* 27 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel,
		};
		MakeDialogItemsEx(FindAskDlgData,FindAskDlg);

		if (strFindStr.IsEmpty())
			FindAskDlg[FAD_CHECKBOX_DIRS].Selected=Opt.FindOpt.FindFolders;

		FarListItem li[]=
		{
			{0,MSG(MSearchAllDisks)},
			{0,MSG(MSearchAllButNetwork)},
			{0,MSG(MSearchInPATH)},
			{0,strSearchFromRoot},
			{0,MSG(MSearchFromCurrent)},
			{0,MSG(MSearchInCurrent)},
			{0,MSG(MSearchInSelected)},
		};
		li[FADC_ALLDISKS+SearchMode].Flags|=LIF_SELECTED;
		FarList l={countof(li),li};
		FindAskDlg[FAD_COMBOBOX_WHERE].ListItems=&l;

		if (PluginMode)
		{
			OpenPluginInfo Info;
			CtrlObject->Plugins.GetOpenPluginInfo(ActivePanel->GetPluginHandle(),&Info);

			if (!(Info.Flags & OPIF_REALNAMES))
				FindAskDlg[FAD_CHECKBOX_ARC].Flags |= DIF_DISABLE;

			if (FADC_ALLDISKS+SearchMode==FADC_ALLDISKS || FADC_ALLDISKS+SearchMode==FADC_ALLBUTNET)
			{
				li[FADC_ALLDISKS].Flags=0;
				li[FADC_ALLBUTNET].Flags=0;
				li[FADC_ROOT].Flags|=LIF_SELECTED;
			}

			li[FADC_ALLDISKS].Flags|=LIF_GRAYED;
			li[FADC_ALLBUTNET].Flags|=LIF_GRAYED;
			FindAskDlg[FAD_CHECKBOX_LINKS].Selected=0;
			FindAskDlg[FAD_CHECKBOX_LINKS].Flags|=DIF_DISABLE;
		}
		else
			FindAskDlg[FAD_CHECKBOX_LINKS].Selected=Opt.FindOpt.FindSymLinks;

		/* $ 14.05.2001 DJ
		   не селектим чекбокс, если нельзя искать в архивах
		*/
		if (!(FindAskDlg[FAD_CHECKBOX_ARC].Flags & DIF_DISABLE))
			FindAskDlg[FAD_CHECKBOX_ARC].Selected=SearchInArchives;

		FindAskDlg[FAD_EDIT_MASK].strData = strFindMask;

		if (SearchHex)
			FindAskDlg[FAD_EDIT_HEX].strData = strFindStr;
		else
			FindAskDlg[FAD_EDIT_TEXT].strData = strFindStr;

		FindAskDlg[FAD_CHECKBOX_CASE].Selected=CmpCase;
		FindAskDlg[FAD_CHECKBOX_WHOLEWORDS].Selected=WholeWords;
		FindAskDlg[FAD_CHECKBOX_HEX].Selected=SearchHex;
		// Использовать фильтр. KM
		FindAskDlg[FAD_CHECKBOX_FILTER].Selected=UseFilter?BSTATE_CHECKED:BSTATE_UNCHECKED;
		int ExitCode;
		Dialog Dlg(FindAskDlg,countof(FindAskDlg),MainDlgProc);
		Dlg.SetHelp(L"FindFile");
		Dlg.SetId(FindFileId);
		Dlg.SetPosition(-1,-1,78,22);
		Dlg.Process();
		ExitCode=Dlg.GetExitCode();
		//Рефреш текущему времени для фильтра сразу после выхода из диалога
		Filter->UpdateCurrentTime();

		if (ExitCode!=FAD_BUTTON_FIND)
		{
			CloseHandle(hPluginMutex);
			return;
		}

		/* Запоминание установленных параметров */
		Opt.FindCodePage = CodePage;
		CmpCase=FindAskDlg[FAD_CHECKBOX_CASE].Selected;
		WholeWords=FindAskDlg[FAD_CHECKBOX_WHOLEWORDS].Selected;
		SearchHex=FindAskDlg[FAD_CHECKBOX_HEX].Selected;
		SearchInArchives=FindAskDlg[FAD_CHECKBOX_ARC].Selected;

		if (FindFoldersChanged)
		{
			Opt.FindOpt.FindFolders=(FindAskDlg[FAD_CHECKBOX_DIRS].Selected==BSTATE_CHECKED);
		}

		if (!PluginMode)
		{
			Opt.FindOpt.FindSymLinks=(FindAskDlg[FAD_CHECKBOX_LINKS].Selected==BSTATE_CHECKED);
		}

		// Запомнить признак использования фильтра. KM
		UseFilter=(FindAskDlg[FAD_CHECKBOX_FILTER].Selected==BSTATE_CHECKED);
		Opt.FindOpt.UseFilter=UseFilter;
		strFindMask = !FindAskDlg[FAD_EDIT_MASK].strData.IsEmpty() ? FindAskDlg[FAD_EDIT_MASK].strData:L"*";

		if (SearchHex)
		{
			strFindStr = FindAskDlg[FAD_EDIT_HEX].strData;
			RemoveTrailingSpaces(strFindStr);
		}
		else
			strFindStr = FindAskDlg[FAD_EDIT_TEXT].strData;

		if (!strFindStr.IsEmpty())
		{
			strGlobalSearchString = strFindStr;
			GlobalSearchCase=CmpCase;
			GlobalSearchWholeWords=WholeWords;
			GlobalSearchHex=SearchHex;
		}

		switch (FindAskDlg[FAD_COMBOBOX_WHERE].ListPos)
		{
			case FADC_ALLDISKS:
				SearchMode=FFSEARCH_ALL;
				break;
			case FADC_ALLBUTNET:
				SearchMode=FFSEARCH_ALL_BUTNETWORK;
				break;
			case FADC_PATH:
				SearchMode=FFSEARCH_INPATH;
				break;
			case FADC_ROOT:
				SearchMode=FFSEARCH_ROOT;
				break;
			case FADC_FROMCURRENT:
				SearchMode=FFSEARCH_FROM_CURRENT;
				break;
			case FADC_INCURRENT:
				SearchMode=FFSEARCH_CURRENT_ONLY;
				break;
			case FADC_SELECTED:
				SearchMode=FFSEARCH_SELECTED;
				break;
		}

		if (SearchFromChanged)
		{
			Opt.FindOpt.FileSearchMode=SearchMode;
		}

		LastCmpCase=CmpCase;
		LastWholeWords=WholeWords;
		LastSearchHex=SearchHex;
		LastSearchInArchives=SearchInArchives;
		strLastFindMask = strFindMask;
		strLastFindStr = strFindStr;

		if (!strFindStr.IsEmpty())
			Editor::SetReplaceMode(FALSE);
	}
	while (FindFilesProcess());

	CloseHandle(hPluginMutex);
	CtrlObject->Cp()->ActivePanel->SetTitle();
}


FindFiles::~FindFiles()
{
	FileMaskForFindFile.Free();
	ClearAllLists();
	ScrBuf.ResetShadow();

	if (Filter)
		delete Filter;
}
