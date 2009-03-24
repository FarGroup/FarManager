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
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "editor.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "filelist.hpp"
#include "cmdline.hpp"
#include "chgprior.hpp"
#include "namelist.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "CFileMask.hpp"
#include "filefilter.hpp"
#include "farexcpt.hpp"
#include "syslog.hpp"
#include "localOEM.hpp"
#include "gettable.hpp"
#include "registry.hpp"

#define DLG_HEIGHT 23
#define DLG_WIDTH 74
#define PARTIAL_DLG_STR_LEN 30
#define CHAR_TABLE_SIZE 5

#define LIST_DELTA  64
static DWORD LIST_INDEX_NONE = (DWORD)-1;

// Список найденных файлов. Индекс из списка хранится в меню.
static LPFINDLIST  *FindList;
static DWORD       FindListCapacity;
static DWORD       FindListCount;
// Список архивов. Если файл найден в архиве, то FindList->ArcIndex указывает сюда.
static LPARCLIST   *ArcList;
static DWORD       ArcListCapacity;
static DWORD       ArcListCount;

static CriticalSection ffCS; // чтобы не ресайзили список найденных файлов, пока мы из него читаем
static CriticalSection statusCS; // чтобы не писали в FindMessage/FindFileCount/FindDirCount пока мы оттуда читаем


static DWORD FindFileArcIndex;
// Используются для отправки файлов на временную панель.
// индекс текущего элемента в списке и флаг для отправки.
static DWORD FindExitIndex;
enum {FIND_EXIT_NONE, FIND_EXIT_SEARCHAGAIN, FIND_EXIT_GOTO, FIND_EXIT_PANEL};
static int FindExitCode;
//static char FindFileArcName[NM];

static string strFindMask, strFindStr;
static int SearchMode,CmpCase,WholeWords,SearchInArchives,SearchHex;

static int FindFoldersChanged;
static int SearchFromChanged;
static int DlgWidth,DlgHeight;
static volatile int StopSearch,PauseSearch,SearchDone,LastFoundNumber,FindFileCount,FindDirCount,WriteDataUsed,PrepareFilesListUsed;
static string strFindMessage;
static string strLastDirName;
static int FindMessageReady,FindCountReady,FindPositionChanged;
static string strPluginSearchPath;
static HANDLE hDlg;
static int RecurseLevel;
static int BreakMainThread;
static int PluginMode;

static HANDLE hPluginMutex;

static int UseAllCodePages=FALSE,UseFilter=0;
static UINT CodePage = (UINT)-1;
static int EnableSearchInFirst=FALSE;
static __int64 SearchInFirst=_i64(0);

static const unsigned int readBufferSize = 32768;
static const unsigned int readBufferSizeW = sizeof(wchar_t)*readBufferSize;
static char *readBuffer;
static wchar_t *readBufferW;
static int codePagesCount;
static struct CodePageInfo
{
	UINT CodePage;
	UINT MaxCharSize;
	wchar_t LastSymbol;
	bool WordFound;
} *codePages;
static unsigned char *hexFindString;
static size_t hexFindStringSize;
static wchar_t *findString;

static size_t *skipCharsTable; /* Officially called: bad character shift */

/* $ 01.07.2001 IS
   Объект "маска файлов". Именно его будем использовать для проверки имени
   файла на совпадение с искомым.
*/
static CFileMask FileMaskForFindFile;

/* $ 05.10.2003 KM
   Указатель на объект фильтра операций
*/
static FileFilter *Filter;

int _cdecl SortItems(const void *p1,const void *p2)
{
  PluginPanelItem *Item1=(PluginPanelItem *)p1;
  PluginPanelItem *Item2=(PluginPanelItem *)p2;
  string strN1, strN2;
  if (*Item1->FindData.lpwszFileName)
    strN1 = Item1->FindData.lpwszFileName;
  if (*Item2->FindData.lpwszFileName)
    strN2 = Item2->FindData.lpwszFileName;

  CutToSlash(strN1);
  CutToSlash(strN2);
  return StrCmpI(strN2,strN1);
}

void InitInFileSearch()
{
	if (!strFindStr.IsEmpty())
	{
		size_t findStringCount = strFindStr.GetLength();
		// Инициализируем буферы чтения из файла
		readBuffer = (char *)xf_malloc(readBufferSize);
		readBufferW = (wchar_t *)xf_malloc(readBufferSizeW);

		if (!SearchHex)
		{
			// Формируем строку поиска
			if (!CmpCase)
			{
				findString = (wchar_t *)xf_malloc(2*findStringCount*sizeof(wchar_t));
				for (size_t index = 0; index<strFindStr.GetLength(); index++)
				{
					wchar_t ch = strFindStr[index];
					if (IsCharLowerW(ch))
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

			// Инизиализируем данные для аглоритма поиска
			skipCharsTable = (size_t *)xf_malloc((WCHAR_MAX+1)*sizeof(size_t));
			for (size_t index = 0; index < WCHAR_MAX+1; index++)
				skipCharsTable[index] = findStringCount;
			for (size_t index = 0; index < findStringCount-1; index++)
				skipCharsTable[findString[index]] = findStringCount-1-index;
			if (!CmpCase)
				for (size_t index = 0; index < findStringCount-1; index++)
					skipCharsTable[findString[index+findStringCount]] = findStringCount-1-index;

			// Формируем список кодовых страниц
			if (UseAllCodePages)
			{
				// Добавляем стандартные таблицы символов
				const int standardCodePagesCount = 6;
				codePagesCount = standardCodePagesCount;
				codePages = (CodePageInfo *)xf_malloc(codePagesCount*sizeof(CodePageInfo));
				codePages[0].CodePage = GetOEMCP();
				codePages[1].CodePage = GetACP();
				codePages[2].CodePage = CP_UTF7;
				codePages[3].CodePage = CP_UTF8;
				codePages[4].CodePage = CP_UNICODE;
				codePages[5].CodePage = CP_REVERSEBOM;
				// Добавляем выбранные таблицы символов
				DWORD data;
				UnicodeString codePageName;
				for(DWORD i=0;EnumRegValue(FavoriteCodePagesKey, i, codePageName, (BYTE *)&data, sizeof(data));i++)
				{
					if (data)
					{
						UINT codePage = _wtoi(codePageName);
						if (codePage != codePages[0].CodePage && codePage != codePages[1].CodePage)
						{
							codePages = (CodePageInfo *)xf_realloc((void *)codePages, ++codePagesCount*sizeof(CodePageInfo));
							codePages[codePagesCount-1].CodePage = codePage;
						}
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
				if (IsUnicodeCP(CodePage))
					cp->MaxCharSize = 2;
				else
				{
					CPINFO cpi;
					if (!GetCPInfo(CodePage, &cpi))
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
	}
}

void ReleaseInFileSearch()
{
	if (!strFindStr.IsEmpty())
	{
		xf_free(readBuffer);
		xf_free(readBufferW);
		xf_free(skipCharsTable);
		if (!SearchHex)
		{
			xf_free(codePages);
			if (!CmpCase)
				xf_free(findString);
		}
		else
			xf_free(hexFindString);
	}
}

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
	FAD_CHECKBOX_ADVANCED,
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

LONG_PTR WINAPI FindFiles::MainDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  Dialog* Dlg=(Dialog*)hDlg;
  const wchar_t *FindText=MSG(MFindFileText),*FindHex=MSG(MFindFileHex),*FindCode=MSG(MFindFileCodePage);

  switch(Msg)
  {
    case DN_INITDIALOG:
    {
      /* $ 21.09.2003 KM
         Переключение видимости строки ввода искомого текста
				в зависимости от Dlg->Item[FAD_CHECKBOX_HEX].Selected
      */
			bool Hex=(Dlg->Item[FAD_CHECKBOX_HEX]->Selected!=0);
			Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,FAD_EDIT_TEXT,!Hex);
			Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,FAD_EDIT_HEX,Hex);
			Dialog::SendDlgMessage(hDlg,DM_ENABLE,FAD_TEXT_CP,!Hex);
			Dialog::SendDlgMessage(hDlg,DM_ENABLE,FAD_COMBOBOX_CP,!Hex);
			Dialog::SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_CASE,!Hex);
			Dialog::SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_WHOLEWORDS,!Hex);
			Dialog::SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_DIRS,!Hex);

			Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,FAD_EDIT_TEXT,1);
			Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,FAD_EDIT_HEX,1);
			Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,FAD_TEXT_TEXTHEX,(LONG_PTR)(Dlg->Item[FAD_CHECKBOX_HEX]->Selected?FindHex:FindText));
			Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,FAD_TEXT_CP,(LONG_PTR)FindCode);

      /* Установка запомненных ранее параметров */
			UseAllCodePages=Opt.CodePage.AllPages;
			CodePage=Opt.CodePage.CodePage;
      /* -------------------------------------- */

			AddCodepagesToList(hDlg, FAD_COMBOBOX_CP, UseAllCodePages ? CP_AUTODETECT : CodePage, false, true);

      FindFoldersChanged = FALSE;
      SearchFromChanged=FALSE;

      return TRUE;
    }
    case DN_LISTCHANGE:
    {
			if (Param1==FAD_COMBOBOX_CP)
      {
				UINT cp = (UINT)Dialog::SendDlgMessage (hDlg, DM_LISTGETDATA, FAD_COMBOBOX_CP, Dialog::SendDlgMessage (hDlg, DM_LISTGETCURPOS, FAD_COMBOBOX_CP, NULL));
				UseAllCodePages = (cp == CP_AUTODETECT);
				if (!UseAllCodePages) {
					CodePage = cp;
				}
      }
      return TRUE;
    }
    /* 22.11.2001 VVM
      ! Сбрасыватьсостояние FindFolders при вводе текста.
        Но только если не меняли этот состояние вручную */
    case DN_BTNCLICK:
    {
      Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
      string strSearchFromRoot;

      /* $ 23.11.2001 KM
         - БЛИН! Правил глюк и глюк добавил :-(
           Перестали работать чекбоксы и радиобатоны.
      */
      /* $ 22.11.2001 KM
         - Забыли однако, что DN_BTNCLICK работает со всеми "нажимающимися"
           контролами, только с кнопками немного по-другому, поэтому простое
           нажатие ENTER на кнопках Find или Cancel ни к чему не приводило.
      */
			switch(Param1)
			{
			case FAD_BUTTON_FIND:
			case FAD_BUTTON_CANCEL:
				return FALSE;

			case FAD_BUTTON_DRIVE:
				{
					IsRedrawFramesInProcess++;
					ActivePanel->ChangeDisk();
					// Ну что ж, раз пошла такая пьянка рефрешить фреймы
					// будем таким способом.
					//FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
					FrameManager->ResizeAllFrame();
					IsRedrawFramesInProcess--;

					PrepareDriveNameStr(strSearchFromRoot, PARTIAL_DLG_STR_LEN);

					FarListGetItem item={FADC_ROOT};
					Dialog::SendDlgMessage(hDlg,DM_LISTGETITEM,FAD_COMBOBOX_WHERE,(LONG_PTR)&item);
					item.Item.Text=strSearchFromRoot;
					Dialog::SendDlgMessage(hDlg,DM_LISTUPDATE,FAD_COMBOBOX_WHERE,(LONG_PTR)&item);

					PluginMode=CtrlObject->Cp()->ActivePanel->GetMode()==PLUGIN_PANEL;
					Dialog::SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_DIRS,PluginMode?FALSE:TRUE);

					item.ItemIndex=FADC_ALLDISKS;
					Dialog::SendDlgMessage(hDlg,DM_LISTGETITEM,FAD_COMBOBOX_WHERE,(LONG_PTR)&item);
					if(PluginMode)
						item.Item.Flags|=LIF_DISABLE;
					else
						item.Item.Flags&=~LIF_DISABLE;
					Dialog::SendDlgMessage(hDlg,DM_LISTUPDATE,FAD_COMBOBOX_WHERE,(LONG_PTR)&item);

					item.ItemIndex=FADC_ALLBUTNET;
					Dialog::SendDlgMessage(hDlg,DM_LISTGETITEM,FAD_COMBOBOX_WHERE,(LONG_PTR)&item);
					if(PluginMode)
						item.Item.Flags|=LIF_DISABLE;
					else
						item.Item.Flags&=~LIF_DISABLE;
					Dialog::SendDlgMessage(hDlg,DM_LISTUPDATE,FAD_COMBOBOX_WHERE,(LONG_PTR)&item);
				}
				break;

			case FAD_BUTTON_FILTER:
				Filter->FilterEdit();
				break;

			case FAD_BUTTON_ADVANCED:
				AdvancedDialog();
				break;

			case FAD_CHECKBOX_DIRS:
				FindFoldersChanged = TRUE;
				break;

			case FAD_CHECKBOX_HEX:
				{
					Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

					/* $ 21.09.2003 KM
						 Переключение видимости строки ввода искомого текста
						 в зависимости от установленного чекбокса hex mode
					*/
					string strDataStr;
					Transform(strDataStr,(LPCWSTR)Dialog::SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,Param2?FAD_EDIT_TEXT:FAD_EDIT_HEX,NULL),Param2?L'X':L'S');
					Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,Param2?FAD_EDIT_HEX:FAD_EDIT_TEXT,(LONG_PTR)(const wchar_t*)strDataStr);
					Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,FAD_EDIT_TEXT,!Param2);
					Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,FAD_EDIT_HEX,Param2);
					Dialog::SendDlgMessage(hDlg,DM_ENABLE,FAD_TEXT_CP,!Param2);
					Dialog::SendDlgMessage(hDlg,DM_ENABLE,FAD_COMBOBOX_CP,!Param2);
					Dialog::SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_CASE,!Param2);
					Dialog::SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_WHOLEWORDS,!Param2);
					Dialog::SendDlgMessage(hDlg,DM_ENABLE,FAD_CHECKBOX_DIRS,!Param2);

					Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,FAD_TEXT_TEXTHEX,(LONG_PTR)(const wchar_t*)(Param2?FindHex:FindText));

					if (strDataStr.GetLength()>0)
					{
						int UnchangeFlag=(int)Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,FAD_EDIT_TEXT,-1);
						Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,FAD_EDIT_HEX,UnchangeFlag);
					}

					Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
				}
				break;

			case FAD_CHECKBOX_ADVANCED:
				EnableSearchInFirst=(int)Param2;
				break;
}
      return TRUE;
    }
    case DN_EDITCHANGE:
    {
      FarDialogItem &Item=*reinterpret_cast<FarDialogItem*>(Param2);

			if (Param1==FAD_EDIT_TEXT)
      {
        if(!FindFoldersChanged)
        // Строка "Содержащих текст"
        {

          BOOL Checked = (Item.PtrData && *Item.PtrData)?FALSE:Opt.FindOpt.FindFolders;
					Dialog::SendDlgMessage(hDlg, DM_SETCHECK, FAD_CHECKBOX_DIRS, Checked?BSTATE_CHECKED:BSTATE_UNCHECKED);
        }
				return TRUE;
      }
			else if(Param1==FAD_COMBOBOX_WHERE)
				SearchFromChanged=TRUE;
			break;

    }
    case DN_HOTKEY:
    {
			if (Param1==FAD_TEXT_TEXTHEX)
      {
				Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,Dlg->Item[FAD_CHECKBOX_HEX]->Selected?FAD_EDIT_HEX:FAD_EDIT_TEXT,0);
        return FALSE;
      }
    }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
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

  // По-умолчанию дополнительные параметры не используются
  EnableSearchInFirst=FALSE;

  strFindMask = strLastFindMask;
  strFindStr = strLastFindStr;
  BreakMainThread=0;

  strSearchFromRoot = MSG(MSearchFromRootFolder);

  FindList = NULL;
  ArcList = NULL;
  hPluginMutex=CreateMutexW(NULL,FALSE,NULL);

  do
  {
    ClearAllLists();

    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();

    PrepareDriveNameStr(strSearchFromRoot,PARTIAL_DLG_STR_LEN);

    const wchar_t *MasksHistoryName=L"Masks",*TextHistoryName=L"SearchText";
		const wchar_t *HexMask=L"HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH";

		static const wchar_t VSeparator[]={BoxSymbols[BS_T_H1V1],BoxSymbols[BS_V1],BoxSymbols[BS_V1],BoxSymbols[BS_V1],BoxSymbols[BS_B_H1V1],0};

    static struct DialogDataEx FindAskDlgData[]=
    {
			/* 00 */DI_DOUBLEBOX,3,1,74,20,0,0,0,0,(const wchar_t *)MFindFileTitle,
			/* 01 */DI_TEXT,5,2,0,2,0,0,0,0,(const wchar_t *)MFindFileMasks,
			/* 02 */DI_EDIT,5,3,72,3,1,(DWORD_PTR)MasksHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,0,L"",
			/* 03 */DI_TEXT,3,4,0,4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
			/* 04 */DI_TEXT,5,5,0,5,0,0,0,0,L"",
			/* 05 */DI_EDIT,5,6,72,6,0,(DWORD_PTR)TextHistoryName,DIF_HISTORY,0,L"",
			/* 06 */DI_FIXEDIT,5,6,72,6,0,(DWORD_PTR)HexMask,DIF_MASKEDIT,0,L"",
			/* 07 */DI_TEXT,5,7,0,7,0,0,0,0,L"",
			/* 08 */DI_COMBOBOX,5,8,72,8,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,L"",
			/* 09 */DI_TEXT,3,9,0,9,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
			/* 10 */DI_CHECKBOX,5,10,0,10,0,0,0,0,(const wchar_t *)MFindFileCase,
			/* 11 */DI_CHECKBOX,5,11,0,11,0,0,0,0,(const wchar_t *)MFindFileWholeWords,
			/* 12 */DI_CHECKBOX,5,12,0,12,0,0,0,0,(const wchar_t *)MSearchForHex,
			/* 13 */DI_CHECKBOX,40,10,0,10,0,0,0,0,(const wchar_t *)MFindArchives,
			/* 14 */DI_CHECKBOX,40,11,0,11,0,0,0,0,(const wchar_t *)MFindFolders,
			/* 15 */DI_CHECKBOX,40,12,0,12,0,0,0,0,(const wchar_t *)MFindSymLinks,
			/* 16 */DI_TEXT,3,13,0,13,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
			/* 17 */DI_VTEXT,38,9,0,9,0,0,DIF_BOXCOLOR,0,VSeparator,
			/* 18 */DI_TEXT,5,14,0,14,0,0,0,0,(const wchar_t *)MSearchWhere,
			/* 19 */DI_COMBOBOX,5,15,72,15,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,L"",
			/* 20 */DI_TEXT,3,16,0,16,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
			/* 21 */DI_CHECKBOX,5,17,0,17,0,0,0,0,(const wchar_t *)MFindUseFilter,
			/* 22 */DI_CHECKBOX,40,17,0,17,0,0,0,0,(const wchar_t *)MFindAdvancedOptions,
			/* 23 */DI_TEXT,3,18,0,18,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
			/* 24 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MFindFileFind,
			/* 25 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindFileDrive,
			/* 26 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindFileSetFilter,
			/* 27 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindFileAdvanced,
			/* 28 */DI_BUTTON,0,19,0,19,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel,
    };

    MakeDialogItemsEx(FindAskDlgData,FindAskDlg);

    if ( strFindStr.IsEmpty() )
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

		if(PluginMode)
		{
			OpenPluginInfo Info;
			CtrlObject->Plugins.GetOpenPluginInfo(ActivePanel->GetPluginHandle(),&Info);

			if(!(Info.Flags & OPIF_REALNAMES))
				FindAskDlg[FAD_CHECKBOX_ARC].Flags |= DIF_DISABLE;

			if(FADC_ALLDISKS+SearchMode==FADC_ALLDISKS || FADC_ALLDISKS+SearchMode==FADC_ALLBUTNET)
			{
				li[FADC_ALLDISKS].Flags=0;
				li[FADC_ALLBUTNET].Flags=0;
				li[FADC_ROOT].Flags|=LIF_SELECTED;
			}
			li[FADC_ALLDISKS].Flags|=LIF_DISABLE;
			li[FADC_ALLBUTNET].Flags|=LIF_DISABLE;

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
		FindAskDlg[FAD_CHECKBOX_FILTER].Selected=UseFilter;

    // Включить дополнительные опции поиска. KM
		FindAskDlg[FAD_CHECKBOX_ADVANCED].Selected=EnableSearchInFirst;

    while (1)
    {
      int ExitCode;
      {
				Dialog Dlg(FindAskDlg,countof(FindAskDlg),MainDlgProc);

        Dlg.SetHelp(L"FindFile");
        Dlg.SetPosition(-1,-1,78,22);
        Dlg.Process();
        ExitCode=Dlg.GetExitCode();
      }

      //Рефреш текущему времени для фильтра сразу после выхода из диалога
      Filter->UpdateCurrentTime();

			if(ExitCode!=FAD_BUTTON_FIND)
      {
        CloseHandle(hPluginMutex);
        return;
      }

      /* Запоминание установленных параметров */
			Opt.CodePage.AllPages=UseAllCodePages;
			if (!UseAllCodePages) {
				Opt.CodePage.CodePage=CodePage;
			}
      /****************************************/

      /* $ 01.07.2001 IS
         Проверим маску на корректность
      */
			if( FindAskDlg[FAD_EDIT_MASK].strData.IsEmpty() )             // если строка с масками пуста,
					FindAskDlg[FAD_EDIT_MASK].strData = L"*"; // то считаем, что маска есть "*"

			if(FileMaskForFindFile.Set(FindAskDlg[FAD_EDIT_MASK].strData, 0))
           break;
    }

		CmpCase=FindAskDlg[FAD_CHECKBOX_CASE].Selected;
		WholeWords=FindAskDlg[FAD_CHECKBOX_WHOLEWORDS].Selected;
		SearchHex=FindAskDlg[FAD_CHECKBOX_HEX].Selected;
		SearchInArchives=FindAskDlg[FAD_CHECKBOX_ARC].Selected;
    if (FindFoldersChanged)
			Opt.FindOpt.FindFolders=FindAskDlg[FAD_CHECKBOX_DIRS].Selected;

		if (!PluginMode && FindAskDlg[FAD_CHECKBOX_LINKS].Selected != Opt.FindOpt.FindSymLinks)
    {
			Opt.FindOpt.FindSymLinks=FindAskDlg[FAD_CHECKBOX_LINKS].Selected;
    }

    // Запомнить признак использования фильтра. KM
		Opt.FindOpt.UseFilter=UseFilter=FindAskDlg[FAD_CHECKBOX_FILTER].Selected;

		strFindMask = !FindAskDlg[FAD_EDIT_MASK].strData.IsEmpty() ? FindAskDlg[FAD_EDIT_MASK].strData:L"*";

    if (SearchHex)
    {
			strFindStr = FindAskDlg[FAD_EDIT_HEX].strData;
      RemoveTrailingSpaces(strFindStr);
    }
    else
			strFindStr = FindAskDlg[FAD_EDIT_TEXT].strData;

    if ( !strFindStr.IsEmpty())
    {
      strGlobalSearchString = strFindStr;
      GlobalSearchCase=CmpCase;
      GlobalSearchWholeWords=WholeWords;
      GlobalSearchHex=SearchHex;
    }

		switch(FindAskDlg[FAD_COMBOBOX_WHERE].ListPos)
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
    if ( !strFindStr.IsEmpty() )
      Editor::SetReplaceMode(FALSE);
  } while (FindFilesProcess());
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

enum ADVANCEDDLG
{
	AD_DOUBLEBOX,
	AD_TEXT_SEARCHFIRST,
	AD_EDIT_SEARCHFIRST,
	AD_SEPARATOR1,
	AD_BUTTON_OK,
	AD_BUTTON_CANCEL,
};

LONG_PTR WINAPI FindFiles::AdvancedDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  switch (Msg)
  {
    case DN_CLOSE:
			if (Param1 == AD_BUTTON_OK)
      {
        string strTemp;
				wchar_t *temp = strTemp.GetBuffer(Dialog::SendDlgMessage(hDlg,DM_GETTEXTPTR,AD_EDIT_SEARCHFIRST,0));
				Dialog::SendDlgMessage(hDlg,DM_GETTEXTPTR,AD_EDIT_SEARCHFIRST,(LONG_PTR)temp);
        if (!CheckFileSizeStringFormat(temp))
        {
          Message(MSG_WARNING,1,MSG(MFindFileAdvancedTitle),MSG(MBadFileSizeFormat),MSG(MOk));
          return FALSE;
        }
      }
      break;
  }

  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

void FindFiles::AdvancedDialog()
{
  static struct DialogDataEx AdvancedDlgData[]=
  {
    /* 00 */DI_DOUBLEBOX,3,1,38,6,0,0,0,0,(const wchar_t *)MFindFileAdvancedTitle,
    /* 01 */DI_TEXT,5,2,0,2,0,0,0,0,(const wchar_t *)MFindFileSearchFirst,
    /* 02 */DI_EDIT,5,3,36,3,0,0,0,0,L"",
    /* 03 */DI_TEXT,3,4,0,4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
    /* 04 */DI_BUTTON,0,5,0,5,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
    /* 05 */DI_BUTTON,0,5,0,5,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel,
  };

  MakeDialogItemsEx(AdvancedDlgData,AdvancedDlg);

  // Установим размер, который будет использован для ограничения поиска строки
	AdvancedDlg[AD_EDIT_SEARCHFIRST].strData = Opt.FindOpt.strSearchInFirstSize;

	Dialog Dlg(AdvancedDlg,countof(AdvancedDlg),AdvancedDlgProc);

  Dlg.SetHelp(L"FindFileAdvanced");
  Dlg.SetPosition(-1,-1,38+4,6+2);
  Dlg.Process();
  int ExitCode=Dlg.GetExitCode();

	if (ExitCode==AD_BUTTON_OK)
  {
		Opt.FindOpt.strSearchInFirstSize = AdvancedDlg[AD_EDIT_SEARCHFIRST].strData;
    SearchInFirst=ConvertFileSizeString(Opt.FindOpt.strSearchInFirstSize);
  }
}

int FindFiles::GetPluginFile(DWORD ArcIndex, struct PluginPanelItem *PanelItem,
                             const wchar_t *DestPath, string &strResultName)
{
  _ALGO(CleverSysLog clv(L"FindFiles::GetPluginFile()"));
  HANDLE hPlugin = ArcList[ArcIndex]->hPlugin;
  string strSaveDir;
  struct OpenPluginInfo Info;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

  strSaveDir = Info.CurDir;
  AddEndSlash(strSaveDir);

  CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_SILENT|OPM_FIND);

  string strFileName;

  strFileName = ArcList[ArcIndex]->strRootPath;
  SetPluginDirectory(strFileName,hPlugin);

  strFileName = PanelItem->FindData.lpwszFileName;
  SetPluginDirectory(strFileName,hPlugin);

  int nItemsNumber;
  struct PluginPanelItem *pItems;
  int nResult = 0;

  wchar_t *lpFileName = xf_wcsdup (PanelItem->FindData.lpwszFileName);
  wchar_t *lpFileNameShort = xf_wcsdup (PanelItem->FindData.lpwszAlternateFileName);

  const wchar_t *lpFileNameToFind = PointToName(RemovePseudoBackSlash(lpFileName));
  const wchar_t *lpFileNameToFindShort = PointToName(RemovePseudoBackSlash(lpFileNameShort));

  if ( CtrlObject->Plugins.GetFindData (
      hPlugin,
      &pItems,
      &nItemsNumber,
      OPM_FIND
      ) )
  {
    for (int i = 0; i < nItemsNumber; i++)
    {
      PluginPanelItem *pItem = &pItems[i];
      PluginPanelItem Item = *pItem;

      wchar_t *lpwszFileName = xf_wcsdup(NullToEmpty(pItem->FindData.lpwszFileName));
      Item.FindData.lpwszFileName = xf_wcsdup (PointToName(RemovePseudoBackSlash(lpwszFileName)));
      xf_free (lpwszFileName);

      lpwszFileName = xf_wcsdup(NullToEmpty(pItem->FindData.lpwszAlternateFileName));
      Item.FindData.lpwszAlternateFileName = xf_wcsdup (PointToName(RemovePseudoBackSlash(lpwszFileName)));
      xf_free (lpwszFileName);

      if ( !StrCmp (lpFileNameToFind, Item.FindData.lpwszFileName) &&
           !StrCmp (lpFileNameToFindShort, Item.FindData.lpwszAlternateFileName) )
      {
        nResult = CtrlObject->Plugins.GetFile (
                            hPlugin,
                            &Item,
                            DestPath,
                            strResultName,
                            OPM_SILENT|OPM_FIND
                            );

        apiFreeFindData(&Item.FindData);
        break;
      }
      apiFreeFindData(&Item.FindData);
    }

    CtrlObject->Plugins.FreeFindData (hPlugin, pItems, nItemsNumber);
  }

  xf_free (lpFileName);
  xf_free (lpFileNameShort);

  CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_SILENT|OPM_FIND);
//  SetPluginDirectory(ArcList[ArcIndex].RootPath,hPlugin);
  SetPluginDirectory(strSaveDir,hPlugin);

  return nResult;
}

enum FINDDLG
{
	FD_DOUBLEBOX,
	FD_LISTBOX,
	FD_SEPARATOR1,
	FD_TEXT_STATUS,
	FD_SEPARATOR2,
	FD_BUTTON_NEW,
	FD_BUTTON_GOTO,
	FD_BUTTON_VIEW,
	FD_BUTTON_PANEL,
	FD_BUTTON_STOP,
};

LONG_PTR WINAPI FindFiles::FindDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  Dialog* Dlg=(Dialog*)hDlg;
	VMenu *ListBox=Dlg->Item[FD_LISTBOX]->ListPtr;

  CriticalSectionLock Lock(Dlg->CS);

  switch(Msg)
  {
    case DN_DRAWDIALOGDONE:
    {
      Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
      // Переместим фокус на кнопку [Go To]
      if((FindDirCount || FindFileCount) && !FindPositionChanged)
      {
        FindPositionChanged=TRUE;
				Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,FD_BUTTON_GOTO,0);
      }
//      else
//        ScrBuf.Flush();
      return TRUE;
    }
/*
    case DN_HELP: // в режиме поиска отключим вывод помощи, ибо... артефакты с прорисовкой
    {
      return !SearchDone?NULL:Param2;
    }
*/
    case DN_MOUSECLICK:
    {
      /* $ 07.04.2003 VVM
         ! Если во время поиска выбрать найденный файл мышью - вешаемся.
         Не надо посылать сообщение на закрытие диалога. Оно ни к чему... */
      SMALL_RECT drect,rect;
      LONG_PTR Ret = FALSE;
      Dialog::SendDlgMessage(hDlg,DM_GETDLGRECT,0,(LONG_PTR)&drect);
			Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,FD_LISTBOX,(LONG_PTR)&rect);
			if (Param1==FD_LISTBOX && ((MOUSE_EVENT_RECORD *)Param2)->dwMousePosition.X<drect.Left+rect.Right)
      {
				Ret = FindDlgProc(hDlg,DN_BTNCLICK,FD_BUTTON_GOTO,0); // emulates a [ Go to ] button pressing
      }
      return Ret;
    }

    case DN_KEY:
    {
      if (!StopSearch && ((Param2==KEY_ESC) || (Param2 == KEY_F10)) )
      {
        PauseSearch=TRUE;
        IsProcessAssignMacroKey++; // запретим спец клавиши
                                   // т.е. в этом диалоге нельзя нажать Alt-F9!
        int LocalRes=TRUE;
        if (Opt.Confirm.Esc)
          LocalRes=AbortMessage();
        IsProcessAssignMacroKey--;
        PauseSearch=FALSE;
        StopSearch=LocalRes;

        return TRUE;
      }

      if (!ListBox)
      {
        return TRUE;
      }

      if(Param2 == KEY_LEFT || Param2 == KEY_RIGHT || Param2 == KEY_NUMPAD4 || Param2 == KEY_NUMPAD6)
        FindPositionChanged = TRUE;

      // некторые спец.клавиши всеже отбработаем.
      if(Param2 == KEY_CTRLALTSHIFTPRESS || Param2 == KEY_ALTF9)
      {
        IsProcessAssignMacroKey--;
        FrameManager->ProcessKey((DWORD)Param2);
        IsProcessAssignMacroKey++;
        return TRUE;
      }

			if (Param1==FD_BUTTON_STOP && (Param2==KEY_RIGHT || Param2 == KEY_NUMPAD6 || Param2==KEY_TAB))
      {
        FindPositionChanged = TRUE;
				Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,FD_BUTTON_NEW,0);
        return TRUE;
      }
			else if (Param1==FD_BUTTON_NEW && (Param2==KEY_LEFT || Param2 == KEY_NUMPAD4 || Param2==KEY_SHIFTTAB))
      {
        FindPositionChanged = TRUE;
				Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,FD_BUTTON_STOP,0);
        return TRUE;
      }
      else if (Param2==KEY_UP || Param2==KEY_DOWN ||
               Param2==KEY_NUMPAD2 || Param2==KEY_NUMPAD8 ||

               Param2==KEY_PGUP || Param2==KEY_PGDN ||
               Param2==KEY_NUMPAD3 || Param2==KEY_NUMPAD9 ||

               Param2==KEY_HOME || Param2==KEY_END ||
               Param2==KEY_NUMPAD1 || Param2==KEY_NUMPAD7 ||

               Param2==KEY_MSWHEEL_LEFT || Param2==KEY_MSWHEEL_RIGHT ||
               Param2==KEY_MSWHEEL_UP || Param2==KEY_MSWHEEL_DOWN)
      {
        ListBox->ProcessKey((int)Param2);
        return TRUE;
      }
      else if (Param2==KEY_F3 || Param2==KEY_NUMPAD5 || Param2==KEY_SHIFTNUMPAD5 || Param2==KEY_F4 ||
								((Param2==KEY_ENTER||Param2==KEY_NUMENTER) && Dialog::SendDlgMessage(hDlg,DM_GETFOCUS,0,0) == FD_BUTTON_VIEW)
              )
      {
        if (ListBox->GetItemCount()==0)
        {
          return TRUE;
        }

				if((Param2==KEY_ENTER||Param2==KEY_NUMENTER) && Dialog::SendDlgMessage(hDlg,DM_GETFOCUS,0,0) == FD_BUTTON_VIEW)
          Param2=KEY_F3;

        ffCS.Enter ();

        DWORD ItemIndex = (DWORD)(DWORD_PTR)ListBox->GetUserData(NULL, 0);
				int RemoveTemp=FALSE;
				int ClosePlugin=FALSE; // Плагины надо закрывать, если открыли.
				string strSearchFileName;
				string strTempDir;

				if (FindList[ItemIndex]->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					ffCS.Leave ();

					return TRUE;
				}
				string strFileName=FindList[ItemIndex]->FindData.strFileName;
				// FindFileArcIndex нельзя здесь использовать
				// Он может быть уже другой.
				if ((FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE) &&
					(!(ArcList[FindList[ItemIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
				{
					string strFindArcName = ArcList[FindList[ItemIndex]->ArcIndex]->strArcName;
					if (ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin == INVALID_HANDLE_VALUE)
					{
						char *Buffer=new char[Opt.PluginMaxReadData];
						if (Buffer)
						{
							FILE *ProcessFile=_wfopen(strFindArcName,L"rb");
							if (ProcessFile==NULL)
							{
								delete[] Buffer;

								ffCS.Leave ();
								return TRUE;
							}
							int ReadSize=(int)fread(Buffer,1,Opt.PluginMaxReadData,ProcessFile);
							fclose(ProcessFile);

							int SavePluginsOutput=DisablePluginsOutput;
							DisablePluginsOutput=TRUE;

							WaitForSingleObject(hPluginMutex,INFINITE);

							ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = CtrlObject->Plugins.OpenFilePlugin(strFindArcName,(unsigned char *)Buffer,ReadSize, 0);

							ReleaseMutex(hPluginMutex);

							DisablePluginsOutput=SavePluginsOutput;

							delete[] Buffer;

							if (ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin == (HANDLE)-2 ||
									ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin == INVALID_HANDLE_VALUE)
							{
								ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = INVALID_HANDLE_VALUE;

								ffCS.Leave ();
								return TRUE;
							}
							ClosePlugin = TRUE;
						}
					}

					PluginPanelItem FileItem;
					memset(&FileItem,0,sizeof(FileItem));

					apiFindDataExToData(&FindList[ItemIndex]->FindData, &FileItem.FindData);

					FarMkTempEx(strTempDir); // А проверка на NULL???
					apiCreateDirectory(strTempDir, NULL);

					// if (!CtrlObject->Plugins.GetFile(ArcList[FindList[ItemIndex].ArcIndex].hPlugin,&FileItem,TempDir,SearchFileName,OPM_SILENT|OPM_FIND))
					WaitForSingleObject(hPluginMutex,INFINITE);
					if (!GetPluginFile(FindList[ItemIndex]->ArcIndex,&FileItem,strTempDir,strSearchFileName))
					{
						apiFreeFindData(&FileItem.FindData);

						apiRemoveDirectory(strTempDir);
						if (ClosePlugin)
						{
							CtrlObject->Plugins.ClosePlugin(ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin);
							ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = INVALID_HANDLE_VALUE;
						}
						ReleaseMutex(hPluginMutex);

						ffCS.Leave ();
						return FALSE;
					}
					else
					{
						apiFreeFindData(&FileItem.FindData);

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
					if(apiGetFileAttributes(strSearchFileName) == INVALID_FILE_ATTRIBUTES && apiGetFileAttributes(FindList[ItemIndex]->FindData.strAlternateFileName) != INVALID_FILE_ATTRIBUTES)
						strSearchFileName = FindList[ItemIndex]->FindData.strAlternateFileName;
				}

				DWORD FileAttr;
				if ((FileAttr=apiGetFileAttributes(strSearchFileName))!=INVALID_FILE_ATTRIBUTES)
				{
					string strOldTitle;
					apiGetConsoleTitle (strOldTitle);

					if (Param2==KEY_F3 || Param2==KEY_NUMPAD5 || Param2==KEY_SHIFTNUMPAD5)
					{
						int ListSize=ListBox->GetItemCount();
						NamesList ViewList;
						// Возьмем все файлы, которые имеют реальные имена...
						if(Opt.FindOpt.CollectFiles)
						{
							DWORD Index;
							for (int I=0;I<ListSize;I++)
							{
								Index = (DWORD)(DWORD_PTR)ListBox->GetUserData(NULL, 0, I);
								LPFINDLIST PtrFindList=FindList[Index];
								if((PtrFindList->ArcIndex == LIST_INDEX_NONE)||(ArcList[PtrFindList->ArcIndex]->Flags & OPIF_REALNAMES))
								{
									// Не учитывали файлы в архивах с OPIF_REALNAMES
									if ( !PtrFindList->FindData.strFileName.IsEmpty() && !(PtrFindList->FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
										ViewList.AddName(PtrFindList->FindData.strFileName, PtrFindList->FindData.strAlternateFileName);
								}
							} /* for */

							string strCurDir = FindList[ItemIndex]->FindData.strFileName;

							ViewList.SetCurName(strCurDir);
						}
						Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
						Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
						{
							FileViewer ShellViewer (strSearchFileName,FALSE,FALSE,FALSE,-1,NULL,(FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE)?NULL:(Opt.FindOpt.CollectFiles?&ViewList:NULL));
							ShellViewer.SetDynamicallyBorn(FALSE);
							ShellViewer.SetEnableF6(TRUE);
							// FindFileArcIndex нельзя здесь использовать
							// Он может быть уже другой.
							if ((FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE) &&
									(!(ArcList[FindList[ItemIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
								ShellViewer.SetSaveToSaveAs(TRUE);
							//!Mutex.Unlock();

							ffCS.Leave (); // чтобы поиск продолжался, пока мы тут кнопки давим

							IsProcessVE_FindFile++;
							FrameManager->EnterModalEV();
							FrameManager->ExecuteModal();
							FrameManager->ExitModalEV();
							IsProcessVE_FindFile--;

							ffCS.Enter ();

							// заставляем рефрешится экран
							FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
						}

						Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
						Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
					}
					else
					{
						Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
						Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
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
									xstrncpy(MsgFullFileName,SearchFileName,sizeof(MsgFullFileName)-1);
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
										Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
										Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
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
								FileEditor ShellEditor (strSearchFileName,CP_AUTODETECT,0);
								ShellEditor.SetDynamicallyBorn(FALSE);
								ShellEditor.SetEnableF6 (TRUE);
								// FindFileArcIndex нельзя здесь использовать
								// Он может быть уже другой.
								if ((FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE) &&
										(!(ArcList[FindList[ItemIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
									ShellEditor.SetSaveToSaveAs(TRUE);

								ffCS.Leave (); // чтобы поиск продолжался, пока мы тут кнопки давим
								IsProcessVE_FindFile++;
								FrameManager->EnterModalEV();
								FrameManager->ExecuteModal ();
								FrameManager->ExitModalEV();
								IsProcessVE_FindFile--;

								ffCS.Enter ();
								// заставляем рефрешится экран
								FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
							}
						}
						Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
						Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
					}
					SetConsoleTitleW (strOldTitle);
				}
				if (RemoveTemp)
				{
					DeleteFileWithFolder(strSearchFileName);
				}

        ffCS.Leave ();

        return TRUE;
      }
      return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
    }
    case DN_BTNCLICK:
    {

      FindPositionChanged = TRUE;

			if (Param1==FD_BUTTON_NEW)
      {
        StopSearch=TRUE;
        FindExitCode=FIND_EXIT_SEARCHAGAIN;
        return FALSE;
      }
			else if (Param1==FD_BUTTON_STOP)
      {
        if (StopSearch)
          return FALSE;
        StopSearch=TRUE;
        return TRUE;
      }
			else if (Param1==FD_BUTTON_GOTO)
      {
        if (!ListBox)
        {
          return FALSE;
        }

        // Переход будем делать так же после выхода из диалога.
        // Причину смотри для [ Panel ]
        if (ListBox->GetItemCount()==0)
        {
          return (TRUE);
        }
        FindExitIndex = (DWORD)(DWORD_PTR)ListBox->GetUserData(NULL, 0);
        FindExitCode = FIND_EXIT_GOTO;
        return (FALSE);
      }
			else if (Param1==FD_BUTTON_VIEW)
      {
				FindDlgProc(hDlg,DN_KEY,FD_LISTBOX,KEY_F3);
        return TRUE;
      }
			else if (Param1==FD_BUTTON_PANEL)
      {
        if (!ListBox)
        {
          return FALSE;
        }

        // На панель будем посылать не в диалоге, а после.
        // После окончания поиска. Иначе возможна ситуация, когда мы
        // ищем на панели, потом ее грохаем и создаем новую (а поиск-то
        // идет!) и в результате ФАР трапается.
        if (ListBox->GetItemCount()==0)
        {
          return (TRUE);
        }
        FindExitCode = FIND_EXIT_PANEL;
        FindExitIndex = (DWORD)(DWORD_PTR)ListBox->GetUserData(NULL, 0);
        return (FALSE);
      }
    }
    case DN_CTLCOLORDLGLIST:
    {
      if (Param2)
        ((struct FarListColors *)Param2)->Colors[VMenuColorDisabled]=FarColorToReal(COL_DIALOGLISTTEXT);
      return TRUE;
    }
    case DN_CLOSE:
    {
      StopSearch=TRUE;
      return TRUE;
    }
    /* 10.08.2001 KM
       Изменение размеров диалога поиска при изменении размеров консоли.
    */
    case DN_RESIZECONSOLE:
    {
      COORD coord=(*(COORD*)Param2);
      SMALL_RECT rect;
      int IncY=coord.Y-DlgHeight-3;
      int I;

      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
			for (I=0;I<=FD_BUTTON_STOP;I++)
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,I,FALSE);

      Dialog::SendDlgMessage(hDlg,DM_GETDLGRECT,0,(LONG_PTR)&rect);
      coord.X=rect.Right-rect.Left+1;
      DlgHeight+=IncY;
			coord.Y=DlgHeight+1;

      if (IncY>0)
        Dialog::SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(LONG_PTR)&coord);

			for (I=0;I<FD_SEPARATOR1;I++)
      {
        Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,I,(LONG_PTR)&rect);
        rect.Bottom+=(short)IncY;
        Dialog::SendDlgMessage(hDlg,DM_SETITEMPOSITION,I,(LONG_PTR)&rect);
      }

			for (I=FD_SEPARATOR1;I<=FD_BUTTON_STOP;I++)
      {
        Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,I,(LONG_PTR)&rect);
        if (I==2)
          rect.Left=-1;
        rect.Top+=(short)IncY;
        Dialog::SendDlgMessage(hDlg,DM_SETITEMPOSITION,I,(LONG_PTR)&rect);
      }

      if (!(IncY>0))
        Dialog::SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(LONG_PTR)&coord);

			for (I=0;I<=FD_BUTTON_STOP;I++)
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,I,TRUE);
      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);

      return TRUE;
    }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

int FindFiles::FindFilesProcess()
{
  _ALGO(CleverSysLog clv(L"FindFiles::FindFilesProcess()"));
  // В статической структуре нужны и статические переменные
  static string strTitle = L"";
  static string strSearchStr;

  /* $ 05.10.2003 KM
     Если используется фильтр операций, то во время поиска сообщаем об этом
  */
  if ( !strFindMask.IsEmpty() )
    if (UseFilter)
      strTitle.Format (L"%s: %s (%s)",MSG(MFindFileTitle),(const wchar_t*)strFindMask,MSG(MFindUsingFilter));
    else
      strTitle.Format (L"%s: %s",MSG(MFindFileTitle),(const wchar_t*)strFindMask);
  else
    if (UseFilter)
      strTitle.Format (L"%s (%s)",MSG(MFindFileTitle),MSG(MFindUsingFilter));
    else
      strTitle.Format (L"%s", MSG(MFindFileTitle));

  if ( !strFindStr.IsEmpty() )
  {
    string strTemp, strFStr;

    strFStr = strFindStr;

    TruncStrFromEnd(strFStr,10);
    strTemp.Format (L" \"%s\"", (const wchar_t*)strFStr);
    strSearchStr.Format(MSG(MFindSearchingIn), (const wchar_t*)strTemp);
  }
  else
    strSearchStr.Format (MSG(MFindSearchingIn), L"");

  struct DialogDataEx FindDlgData[]={
	/* 00 */DI_DOUBLEBOX,3,1,DLG_WIDTH,DLG_HEIGHT-3,0,0,DIF_SHOWAMPERSAND,0,strTitle,
	/* 01 */DI_LISTBOX,4,2,73,DLG_HEIGHT-8,0,0,DIF_LISTNOBOX,0,(const wchar_t*)0,
	/* 02 */DI_TEXT,-1,DLG_HEIGHT-7,0,DLG_HEIGHT-7,0,0,DIF_BOXCOLOR|DIF_SEPARATOR2,0,L"",
	/* 03 */DI_TEXT,5,DLG_HEIGHT-6,0,DLG_HEIGHT-6,0,0,DIF_SHOWAMPERSAND,0,strSearchStr,
	/* 04 */DI_TEXT,3,DLG_HEIGHT-5,0,DLG_HEIGHT-5,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
	/* 05 */DI_BUTTON,0,DLG_HEIGHT-4,0,DLG_HEIGHT-4,1,0,DIF_CENTERGROUP,1,(const wchar_t *)MFindNewSearch,
	/* 06 */DI_BUTTON,0,DLG_HEIGHT-4,0,DLG_HEIGHT-4,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindGoTo,
	/* 07 */DI_BUTTON,0,DLG_HEIGHT-4,0,DLG_HEIGHT-4,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindView,
	/* 08 */DI_BUTTON,0,DLG_HEIGHT-4,0,DLG_HEIGHT-4,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindPanel,
	/* 09 */DI_BUTTON,0,DLG_HEIGHT-4,0,DLG_HEIGHT-4,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindStop
  };

  MakeDialogItemsEx(FindDlgData,FindDlg);

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  DlgHeight=DLG_HEIGHT-2;
  DlgWidth=FindDlg[0].X2-FindDlg[0].X1-4;

  int IncY=Max(0,ScrY-DLG_HEIGHT);
  FindDlg[0].Y2+=IncY;
  FindDlg[1].Y2+=IncY;
  FindDlg[2].Y1+=IncY;
  FindDlg[3].Y1+=IncY;
  FindDlg[4].Y1+=IncY;
  FindDlg[5].Y1+=IncY;
  FindDlg[6].Y1+=IncY;
  FindDlg[7].Y1+=IncY;
  FindDlg[8].Y1+=IncY;
  FindDlg[9].Y1+=IncY;

  DlgHeight+=IncY;

  if (PluginMode)
  {
    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    HANDLE hPlugin=ActivePanel->GetPluginHandle();
    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

    FindFileArcIndex = AddArcListItem(Info.HostFile, hPlugin, Info.Flags, Info.CurDir);

    if (FindFileArcIndex == LIST_INDEX_NONE)
      return(FALSE);

    if ((Info.Flags & OPIF_REALNAMES)==0)
    {
      FindDlg[8].Type=DI_TEXT;
      FindDlg[8].strData=L"";
    }
  }

	bool AnySetFindList=false;
	for (int i=0;i<CtrlObject->Plugins.PluginsCount; i++)
	{
		if(CtrlObject->Plugins.PluginsData[i]->HasSetFindList())
		{
			AnySetFindList=true;
			break;
		}
	}
	if(!AnySetFindList)
	{
		FindDlg[8].Type=DI_TEXT;
		FindDlg[8].strData=L"";
	}

	Dialog Dlg=Dialog(FindDlg,countof(FindDlg),FindDlgProc);
  hDlg=(HANDLE)&Dlg;
//  pDlg->SetDynamicallyBorn();
  Dlg.SetHelp(L"FindFileResult");
	Dlg.SetPosition(-1,-1,DLG_WIDTH+4,DLG_HEIGHT-1+IncY);
  // Надо бы показать диалог, а то инициализация элементов запаздывает
  // иногда при поиске и первые элементы не добавляются
  Dlg.InitDialog();
  Dlg.Show();

  LastFoundNumber=0;
  SearchDone=FALSE;
  StopSearch=FALSE;
  PauseSearch=FALSE;
  WriteDataUsed=FALSE;
  PrepareFilesListUsed=0;
  FindFileCount=FindDirCount=0;
  FindExitIndex = LIST_INDEX_NONE;
  FindExitCode = FIND_EXIT_NONE;
  FindMessageReady=FindCountReady=FindPositionChanged=0;
  strLastDirName = L"";
  strFindMessage = L"";

  if (PluginMode)
  {
    if (_beginthread(PreparePluginList,0,NULL)==(unsigned long)-1)
      return FALSE;
  }
  else
  {
    if (_beginthread(PrepareFilesList,0,NULL)==(unsigned long)-1)
      return FALSE;
  }

  // Нитка для вывода в диалоге информации о ходе поиска
  if (_beginthread(WriteDialogData,0,NULL)==(unsigned long)-1)
    return FALSE;

  IsProcessAssignMacroKey++; // отключим все спец. клавиши
  Dlg.Process();
  IsProcessAssignMacroKey--;

  while(WriteDataUsed || PrepareFilesListUsed > 0)
    Sleep(10);

  ::hDlg=NULL;

  switch (FindExitCode)
  {
    case FIND_EXIT_SEARCHAGAIN:
    {
      return TRUE;
    }
    case FIND_EXIT_PANEL:
    // Отработаем переброску на временную панель
    {
      DWORD ListSize = FindListCount;
      PluginPanelItem *PanelItems=new PluginPanelItem[ListSize];
      if (PanelItems==NULL)
        ListSize=0;
      int ItemsNumber=0;
      for (DWORD i=0;i<ListSize;i++)
      {
        if (StrLength(FindList[i]->FindData.strFileName)>0 && FindList[i]->Used)
        // Добавляем всегда, если имя задано
        {
          // Для плагинов с виртуальными именами заменим имя файла на имя архива.
          // панель сама уберет лишние дубли.
          int IsArchive = ((FindList[i]->ArcIndex != LIST_INDEX_NONE) &&
                          !(ArcList[FindList[i]->ArcIndex]->Flags&OPIF_REALNAMES));

          // Добавляем только файлы или имена архивов или папки когда просили
          if (IsArchive || (Opt.FindOpt.FindFolders && !SearchHex) ||
              !(FindList[i]->FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
          {
            if (IsArchive)
              FindList[i]->FindData.strFileName = ArcList[FindList[i]->ArcIndex]->strArcName;

            PluginPanelItem *pi=&PanelItems[ItemsNumber++];
            memset(pi,0,sizeof(*pi));

            apiFindDataExToData (&FindList[i]->FindData, &pi->FindData);

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
        } /* if */
      } /* for */

      HANDLE hNewPlugin=CtrlObject->Plugins.OpenFindListPlugin(PanelItems,ItemsNumber);
      if (hNewPlugin!=INVALID_HANDLE_VALUE)
      {
        Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
        Panel *NewPanel=CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
        NewPanel->SetPluginMode(hNewPlugin,L"",true);
        NewPanel->SetVisible(TRUE);
        NewPanel->Update(0);
//        if (FindExitIndex != LIST_INDEX_NONE)
//          NewPanel->GoToFile(FindList[FindExitIndex].FindData.cFileName);
        NewPanel->Show();
      }

      for (int i = 0; i < ItemsNumber; i++)
          apiFreeFindData (&PanelItems[i].FindData);

      delete[] PanelItems;
      break;
    } /* case FIND_EXIT_PANEL */
    case FIND_EXIT_GOTO:
    {
      string strFileName=FindList[FindExitIndex]->FindData.strFileName;
      Panel *FindPanel=CtrlObject->Cp()->ActivePanel;

      if ((FindList[FindExitIndex]->ArcIndex != LIST_INDEX_NONE) &&
          (!(ArcList[FindList[FindExitIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
      {
        HANDLE hPlugin = ArcList[FindList[FindExitIndex]->ArcIndex]->hPlugin;
        if (hPlugin == INVALID_HANDLE_VALUE)
        {
          string strArcName, strArcPath;

          strArcName = ArcList[FindList[FindExitIndex]->ArcIndex]->strArcName;
          if (FindPanel->GetType()!=FILE_PANEL)
            FindPanel=CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);

          strArcPath = strArcName;

          CutToSlash(strArcPath);
          FindPanel->SetCurDir(strArcPath,TRUE);
          hPlugin=((FileList *)FindPanel)->OpenFilePlugin(strArcName,FALSE);
          if (hPlugin==(HANDLE)-2)
            hPlugin = INVALID_HANDLE_VALUE;
        } /* if */
        if (hPlugin != INVALID_HANDLE_VALUE)
        {
          struct OpenPluginInfo Info;
          CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

          /* $ 19.01.2003 KM
             Уточнение перехода в нужный каталог плагина.
          */
          if (SearchMode==FFSEARCH_ROOT ||
              SearchMode==FFSEARCH_ALL ||
              SearchMode==FFSEARCH_ALL_BUTNETWORK ||
              SearchMode==FFSEARCH_INPATH)
            CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_FIND);

          SetPluginDirectory(strFileName,hPlugin,TRUE);
        }
      } /* if */
      else
      {
        string strSetName;
        int Length;
        if ((Length=(int)strFileName.GetLength())==0)
          break;

				if (Length>1 && IsSlash(strFileName.At(Length-1)) && strFileName.At(Length-2)!=L':')
          strFileName.SetLength(Length-1);

				if ( (apiGetFileAttributes(strFileName)==INVALID_FILE_ATTRIBUTES) && (GetLastError () != ERROR_ACCESS_DENIED))
          break;
        {
          const wchar_t *NamePtr = PointToName(strFileName);

          strSetName = NamePtr;

          strFileName.SetLength(NamePtr-(const wchar_t *)strFileName);

          Length=(int)strFileName.GetLength();

					if (Length>1 && IsSlash(strFileName.At(Length-1)) && strFileName.At(Length-2)!=L':')
            strFileName.SetLength(Length-1);
        }
        if ( strFileName.IsEmpty() )
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
          Length=(int)strDirTmp.GetLength();

					if (Length>1 && IsSlash(strDirTmp.At(Length-1)) && strDirTmp.At(Length-2)!=L':')
            strDirTmp.SetLength(Length-1);

          if (0!=StrCmpI(strFileName, strDirTmp))
            FindPanel->SetCurDir(strFileName,TRUE);
        }
        if ( !strSetName.IsEmpty() )
          FindPanel->GoToFile(strSetName);
        FindPanel->Show();
        FindPanel->SetFocus();
      }
      break;
    } /* case FIND_EXIT_GOTO */
  } /* switch */

  return FALSE;
}


void FindFiles::SetPluginDirectory(const wchar_t *DirName,HANDLE hPlugin,int UpdatePanel)
{
  string strName;
  wchar_t *StartName,*EndName;
  int IsPluginDir;

  /* $ 19.01.2003 KM
     Восстановлю поведение до 4 беты. Если в DirName есть
     символ '\x1' значит это путь из плагина. Таким образом
     легче определить плагиновые пути и, соответственно,
     сделать правильный переход.
  */
  strName = DirName;
  if ( strName.GetLength() > 0 )
    IsPluginDir=strName.Contains(L'\x1')?TRUE:FALSE;
  else
    IsPluginDir=FALSE;

  StartName = strName.GetBuffer();
  while(IsPluginDir?(EndName=wcschr(StartName,L'\x1'))!=NULL:(EndName=wcschr(StartName,L'\\'))!=NULL)
  {
    *EndName=0;
    // RereadPlugin
    {
      int FileCount=0;
      PluginPanelItem *PanelData=NULL;
      if (CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&FileCount,OPM_FIND))
        CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,FileCount);
    }
    CtrlObject->Plugins.SetDirectory(hPlugin,StartName,OPM_FIND);
    StartName=EndName+1;
  }

  //   Отрисуем панель при необходимости.
  if (UpdatePanel)
  {
    CtrlObject->Cp()->ActivePanel->Update(UPDATE_KEEP_SELECTION);
    if (!CtrlObject->Cp()->ActivePanel->GoToFile(StartName))
      CtrlObject->Cp()->ActivePanel->GoToFile(DirName);
    CtrlObject->Cp()->ActivePanel->Show();
  }

  //strName.ReleaseBuffer(); Не надо. Строка все ровно удаляется, лишний вызов StrLength.
}

void FindFiles::DoScanTree(string& strRoot, FAR_FIND_DATA_EX& FindData, string& strFullName)
{
	{
		ScanTree ScTree(FALSE,!(SearchMode==FFSEARCH_CURRENT_ONLY||SearchMode==FFSEARCH_INPATH),Opt.FindOpt.FindSymLinks);

		string strSelName;
		DWORD FileAttr;
		if (SearchMode==FFSEARCH_SELECTED)
			CtrlObject->Cp()->ActivePanel->GetSelName(NULL,FileAttr);

		InitInFileSearch();

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

			ScTree.SetFindPath(strCurRoot,L"*.*");

			statusCS.Enter ();

			strFindMessage = strCurRoot;
			FindMessageReady=TRUE;

			statusCS.Leave ();

			while (!StopSearch && ScTree.GetNextName(&FindData,strFullName))
			{
				while (PauseSearch)
					Sleep(10);

				/* $ 30.09.2003 KM
					Отфильтруем файлы не попадающие в действующий фильтр
				*/
				if (UseFilter)
				{
					enumFileInFilterType foundType;
					if (!Filter->FileInFilter(&FindData,&foundType))
					{
						// сюда заходим, если не попали в фильтр или попали в Exclude-фильтр
						if ((FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && foundType==FIFT_EXCLUDE)
							ScTree.SkipDir(); // скипаем только по Exclude-фильтру, т.к. глубже тоже нужно просмотреть
						continue;
					}
				}

				/* $ 14.06.2004 KM
					Уточнение действия при обработке каталогов
				*/
				if (((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strFindStr.IsEmpty()) ||
					  (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !strFindStr.IsEmpty()))

				{
					statusCS.Enter();

					strFindMessage = strFullName;
					FindMessageReady=TRUE;

					statusCS.Leave();
				}

				if (IsFileIncluded(NULL,strFullName,FindData.dwFileAttributes))
					AddMenuRecord(strFullName,&FindData);

				if (SearchInArchives)
					ArchiveSearch(strFullName);
			}
			if (SearchMode!=FFSEARCH_SELECTED)
				break;
		}

		ReleaseInFileSearch();

	}
}
void _cdecl FindFiles::DoPrepareFileList(string& strRoot, FAR_FIND_DATA_EX& FindData, string& strFullName)
{
  TRY {
    string *strPathEnv = new string;
    wchar_t *PathEnv=NULL, *Ptr=NULL;
    PrepareFilesListUsed++;
    DWORD DiskMask=FarGetLogicalDrives();

    //string strRoot; //BUGBUG
    CtrlObject->CmdLine->GetCurDir(strRoot);

    if (SearchMode==FFSEARCH_INPATH && strPathEnv)
    {
      apiGetEnvironmentVariable(L"PATH",*strPathEnv);
      PathEnv = strPathEnv->GetBuffer(strPathEnv->GetLength()+1);
      if (PathEnv)
      {
        PathEnv[strPathEnv->GetLength()]=0;
        Ptr=PathEnv;
        while (*Ptr)
        {
          if (*Ptr==L';')
            *Ptr=0;
          ++Ptr;
        }
      }
      Ptr=PathEnv;
    }

    for (int CurrentDisk=0;;CurrentDisk++,DiskMask>>=1)
    {
      if (SearchMode==FFSEARCH_ALL ||
          SearchMode==FFSEARCH_ALL_BUTNETWORK)
      {
        if (DiskMask==0)
          break;

        if ((DiskMask & 1)==0)
          continue;

        strRoot.Format (L"%c:\\", L'A'+CurrentDisk);
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
        GetPathRootOne(strRoot,strRoot);
      else if (SearchMode==FFSEARCH_INPATH)
      {
        if(!*Ptr)
          break;
        strRoot = Ptr;
        Ptr+=StrLength(Ptr)+1;
      }

      DoScanTree(strRoot, FindData, strFullName);

      if (SearchMode!=FFSEARCH_ALL && SearchMode!=FFSEARCH_ALL_BUTNETWORK && SearchMode!=FFSEARCH_INPATH)
        break;
    }

    delete strPathEnv;

    while (!StopSearch && FindMessageReady)
      Sleep(10);

    statusCS.Enter ();

    strFindMessage.Format (MSG(MFindDone),FindFileCount,FindDirCount);

    SetFarTitle(strFindMessage);

    SearchDone=TRUE;
    FindMessageReady=TRUE;

    statusCS.Leave ();

    PrepareFilesListUsed--;
  }
  EXCEPT (xfilter((int)(INT_PTR)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1))
  {
    TerminateProcess( GetCurrentProcess(), 1);
  }
}
void _cdecl FindFiles::PrepareFilesList(void *Param)
{
  FAR_FIND_DATA_EX FindData;
  string strFullName;
  string strRoot;

  DoPrepareFileList(strRoot, FindData, strFullName);

}

void FindFiles::ArchiveSearch(const wchar_t *ArcName)
{
  _ALGO(CleverSysLog clv(L"FindFiles::ArchiveSearch()"));
  _ALGO(SysLog(L"ArcName='%s'",(ArcName?ArcName:L"NULL")));
  char *Buffer=new char[Opt.PluginMaxReadData];
  if ( !Buffer )
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
    BreakMainThread=TRUE;
    _ALGO(SysLog(L"return: hArc==(HANDLE)-2"));
    return;
  }

  if (hArc==INVALID_HANDLE_VALUE)
  {
    _ALGO(SysLog(L"return: hArc==INVALID_HANDLE_VALUE"));
    return;
  }

  int SaveSearchMode=SearchMode;
  DWORD SaveArcIndex = FindFileArcIndex;
  {
    SearchMode=FFSEARCH_FROM_CURRENT;
    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hArc,&Info);

    FindFileArcIndex = AddArcListItem(ArcName, hArc, Info.Flags, Info.CurDir);
    /* $ 11.12.2001 VVM
      - Запомним каталог перед поиском в архиве.
        И если ничего не нашли - не рисуем его снова */
    {
      string strSaveDirName, strSaveSearchPath;
      DWORD SaveListCount = FindListCount;
      /* $ 19.01.2003 KM
         Запомним пути поиска в плагине, они могут измениться.
      */
      strSaveSearchPath = strPluginSearchPath;

      strSaveDirName = strLastDirName;
      strLastDirName = L"";
      PreparePluginList((void *)1);
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

/* $ 01.07.2001 IS
   Используем FileMaskForFindFile вместо GetCommaWord
*/
int FindFiles::IsFileIncluded(PluginPanelItem *FileItem,const wchar_t *FullName,DWORD FileAttr)
{
  CriticalSectionLock Lock (ffCS);

  int FileFound=FileMaskForFindFile.Compare(FullName);
  HANDLE hPlugin=INVALID_HANDLE_VALUE;
  if (FindFileArcIndex != LIST_INDEX_NONE)
    hPlugin = ArcList[FindFileArcIndex]->hPlugin;
  while(FileFound)
  {
    /* $ 24.09.2003 KM
       Если включен режим поиска hex-кодов, тогда папки в поиск не включаем
    */
    /* $ 17.01.2002 VVM
      ! Поскольку работу с поиском в папках вынесли в диалог -
        флаг в плагине потерял свою актуальность */
    if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) && ((Opt.FindOpt.FindFolders==0) || SearchHex))
//        ((hPlugin == INVALID_HANDLE_VALUE) ||
//        (ArcList[FindFileArcIndex].Flags & OPIF_FINDFOLDERS)==0))
      return FALSE;

    if ( !strFindStr.IsEmpty() && FileFound)
    {
      FileFound=FALSE;
      if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
        break;
      string strSearchFileName;
      int RemoveTemp=FALSE;
      if ((hPlugin != INVALID_HANDLE_VALUE) && (ArcList[FindFileArcIndex]->Flags & OPIF_REALNAMES)==0)
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
          ReleaseMutex(hPluginMutex);
        RemoveTemp=TRUE;
      }
      else
      {
        strSearchFileName = FullName;
      }

      if (LookForString(strSearchFileName))
        FileFound=TRUE;

      if (RemoveTemp)
      {
        DeleteFileWithFolder(strSearchFileName);
      }
    }
    break;
  }
  return(FileFound);
}


void FindFiles::AddMenuRecord(const wchar_t *FullName, FAR_FIND_DATA *FindData)
{
    FAR_FIND_DATA_EX fdata;

    apiFindDataToDataEx (FindData, &fdata);

    AddMenuRecord (FullName, &fdata);
}

void FindFiles::AddMenuRecord(const wchar_t *FullName, FAR_FIND_DATA_EX *FindData)
{
  string strMenuText, strFileText, strSizeText;
  string strDateStr, strTimeStr, strDate;
  MenuItemEx ListItem;

  Dialog* Dlg=(Dialog*)hDlg;
  if(!Dlg)
  {
    return;
  }

  VMenu *ListBox=Dlg->Item[1]->ListPtr;

  if (!ListBox)
  {
    return;
  }

  ListItem.Clear ();

  if (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
  {
    strSizeText.Format (L"%13s", MSG(MFindFileFolder));
  }
  else
  {
    wchar_t *wszSizeText = strSizeText.GetBuffer (100); //BUGBUG, function!!!

    _ui64tow(FindData->nFileSize, wszSizeText, 10);

    strSizeText.ReleaseBuffer ();
  }
  const wchar_t *DisplayName=FindData->strFileName;
  /* $ 24.03.2002 KM
     В плагинах принудительно поставим указатель в имени на имя
     для корректного его отображения в списке, отбросив путь,
     т.к. некоторые плагины возвращают имя вместе с полным путём,
     к примеру временная панель.
  */
  if (FindFileArcIndex != LIST_INDEX_NONE)
    DisplayName=PointToName(DisplayName);
	string strDisplayName=DisplayName;
	TruncStrFromEnd(strDisplayName,24);
	strFileText.Format (L" %-25.25s%c%13.13s",(LPCWSTR)strDisplayName,BoxSymbols[BS_V1],(const wchar_t*)strSizeText);


  /* $ 05.10.2003 KM
     При использовании фильтра по дате будем отображать тот тип
     даты, который задан в фильтре.

     $ 24.01.2007 AY
     Так как теперь есть больше чем один фильтр то этот момент я
     пока не знаю как красиво сделать.

  if (UseFilter && Filter->GetParams()->FDate.Used)
  {
    switch(Filter->GetParams()->FDate.DateType)
    {
      case FDATE_MODIFIED:
        // Отображаем дату последнего изменения
        ConvertDate(FindData->ftLastWriteTime,strDateStr,strTimeStr,5);
        break;
      case FDATE_CREATED:
        // Отображаем дату создания
        ConvertDate(FindData->ftCreationTime,strDateStr,strTimeStr,5);
        break;
      case FDATE_OPENED:
        // Отображаем дату последнего доступа
        ConvertDate(FindData->ftLastAccessTime,strDateStr,strTimeStr,5);
        break;
    }
  }
  else
  */
    // Отображаем дату последнего изменения
    ConvertDate(FindData->ftLastWriteTime,strDateStr,strTimeStr,5);
		strDate.Format (L"%c%s%c%s",BoxSymbols[BS_V1],(const wchar_t*)strDateStr,BoxSymbols[BS_V1],(const wchar_t*)strTimeStr);
  strFileText += strDate;

  /* $ 05.10.2003 KM
     Отобразим в панели поиска атрибуты найденных файлов
  */
	const wchar_t AttrStr[]=
	{
		BoxSymbols[BS_V1],
		FindData->dwFileAttributes&FILE_ATTRIBUTE_READONLY? L'R':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM?L'S':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN?L'H':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE?L'A':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT? L'L':FindData->dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE?L'$':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED?L'C':FindData->dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED?L'E':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_TEMPORARY?L'T':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED?L'I':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_OFFLINE?L'O':L' ',
		FindData->dwFileAttributes&FILE_ATTRIBUTE_VIRTUAL?L'V':L' ',
		0
	};

	strFileText += AttrStr;
  strMenuText=strFileText;//.Format (L" %-*.*s",DlgWidth-2,DlgWidth-2, (const wchar_t*)strFileText);

  string strPathName;
  strPathName = FullName;

  RemovePseudoBackSlash(strPathName);

  CutToSlash(strPathName);

  if ( strPathName.IsEmpty() )
      strPathName = L".\\";

  AddEndSlash(strPathName);

  if ( StrCmpI(strPathName,strLastDirName)!=0)
  {
    if ( !strLastDirName.IsEmpty() )
    {
			ListItem.Flags|=LIF_SEPARATOR;
			ListBox->AddItem(&ListItem);
			ListItem.Flags&=~LIF_SEPARATOR;
    }
    ffCS.Enter ();

    strLastDirName = strPathName;

    if ((FindFileArcIndex != LIST_INDEX_NONE) &&
        (!(ArcList[FindFileArcIndex]->Flags & OPIF_REALNAMES)) &&
        (!ArcList[FindFileArcIndex]->strArcName.IsEmpty()))
    {
			string strArcPathName=ArcList[FindFileArcIndex]->strArcName;
			strArcPathName+=L":";
			if(!IsSlash(strPathName.At(0)))
				AddEndSlash(strArcPathName);
			strArcPathName+=(!StrCmp(strPathName,L".\\")?L"\\":(const wchar_t*)strPathName);
      strPathName = strArcPathName;
    }

		ListItem.strName=TruncPathStr(strPathName,DLG_WIDTH-8);

    DWORD ItemIndex = AddFindListItem(FindData);
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

      ListBox->SetUserData((void*)(DWORD_PTR)ItemIndex,sizeof(ItemIndex),
                           ListBox->AddItem(&ListItem));
    }

    ffCS.Leave ();
  }

  /* $ 24.03.2002 KM
     Дополнительно добавляем в список найденного
     папки. Так было в 1.65 и в 3 бете, но при
     полной переделке поиска я это где-то отломил,
     теперь возвращаю на место.
  */

  ffCS.Enter ();

  DWORD ItemIndex = AddFindListItem(FindData);
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
  ListItem.strName = strMenuText;

  ffCS.Leave ();
  /* $ 17.01.2002 VVM
    ! Выделять будем не в структуре, а в списке. Дабы не двоилось выделение */
//  ListItem.SetSelect(!FindFileCount);

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

// Проверяем символ на принадлежность разделителям слов
bool IsWordDiv(const wchar_t symbol) {
	// Проверяем символ на принадлежность FAR-м разделителям
	wchar_t *wordDiv = Opt.strWordDiv.GetBuffer();
	size_t length = Opt.strWordDiv.GetLength();
	for (size_t index = 0; index<length; index++) {
		if (symbol == wordDiv[index]) {
			return true;
		}
	}
	// Так же разделителем является конец строки и пробельные символы
	return symbol==0||IsSpace(symbol)||IsEol(symbol) ? true : false;
}

int FindFiles::LookForString(const wchar_t *Name)
{
	#define RETURN(r) { result = (r); goto exit; }
	#define CONTINUE(r) { if ((r) || !UseAllCodePages || (cpIndex==codePagesCount-1)) RETURN(r) else continue; }

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
	unsigned __int64 alreadyRead = _i64(0);
	// Смещение на которое мы отступили при переходе между блоками
	int offset=0;
	if (SearchHex)
		offset = (int)hexFindStringSize-1;

	// Основной цикл чтения из файла
	while (!StopSearch && ReadFile(fileHandle, readBuffer, !EnableSearchInFirst || alreadyRead+readBufferSize<=(unsigned __int64)SearchInFirst ? readBufferSize : static_cast<int>(SearchInFirst-alreadyRead), &readBlockSize, NULL))
	{
		// Увеличиваем счётчик прочитыннх байт
		alreadyRead += readBlockSize;
		// Для hex и обыкновенного поиска разные ветки
		if (SearchHex)
		{
			// Выходим, если ничего не прочитали или прочитали мало
			if (readBlockSize == 0 || readBlockSize<hexFindStringSize)
				RETURN (FALSE)
			// Ищем
			if (FindStringBMH((unsigned char *)readBuffer, readBlockSize)!=-1)
				RETURN (TRUE)
		}
		else
		{
			for (int cpIndex = 0; cpIndex<codePagesCount; cpIndex++)
			{
				// Информация о кодовой странице
				CodePageInfo *cpi = codePages+cpIndex;
				// Пропускаем ошибочные кодовые страницы
				if (!cpi->MaxCharSize)
					CONTINUE (FALSE)
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
					CONTINUE (WholeWords && cpi->WordFound)
				// Выходим, если прочитали меньше размера строки поиска и нет поиска по словам
				if (readBlockSize < findStringCount && !(WholeWords && cpi->WordFound))
					CONTINUE (FALSE)
				// Количество символов в выходном буфере
				unsigned int bufferCount;
				// Буфер для поиска
				wchar_t *buffer;
				// Перегоняем буфер в UTF-16
				if (IsUnicodeCP(cpi->CodePage))
				{
					// Вычисляем размер буфера в UTF-16
					bufferCount = readBlockSize/sizeof(wchar_t);
					// Выходим, если размер буфера меньше длины строки посика
					if (bufferCount < findStringCount)
						CONTINUE (FALSE)
					// Копируем буфер чтения в буфер сравнения
					if (cpi->CodePage==CP_REVERSEBOM)
					{
						// Для UTF-16 (big endian) преобразуем буфер чтения в буфер сравнения
						bufferCount = LCMapStringW(
								LOCALE_NEUTRAL,//LOCALE_INVARIANT,
								LCMAP_BYTEREV,
								(wchar_t *)readBuffer,
								(int)bufferCount,
								readBufferW,
								(int)readBufferSizeW
							);
						if (!bufferCount)
							CONTINUE (FALSE)
						// Устанавливаем буфер стравнения
						buffer = readBufferW;
					}
					else
					{
						// Если поиск в UTF-16 (little endian), то используем исходный буфер
						buffer = (wchar_t *)readBuffer;
					}
				}
				else
				{
					// Конвертируем буфер чтения из кодировки поиска в UTF-16
					bufferCount = MultiByteToWideChar(
							cpi->CodePage,
							0,
							(char *)readBuffer,
							readBlockSize,
							readBufferW,
							readBufferSizeW
						);
					// Выходим, если нам не удалось сконвертировать строку
					if (!bufferCount)
						CONTINUE (FALSE)
					// Если прочитали меньше размера строки поиска и поиска по словам, то проверяем
					// первый символ блока на разделитель и выходим
					// Если у нас поиск по словам и в конце предыдущего блока было вхождение
					if (WholeWords && cpi->WordFound)
					{
						// Если конец файла, то считаем, что есть разделитель в конце
						if (findStringCount-1>=bufferCount)
							RETURN (TRUE)
						// Проверяем первый символ текущего блока с учётом обратного смещения, которое делается
						// при переходе между блоками
						cpi->LastSymbol = readBufferW[findStringCount-1];
						if (IsWordDiv(cpi->LastSymbol))
							RETURN (TRUE)
						// Если размер буфера меньше размера слова, то выходим
						if (readBlockSize < findStringCount)
							CONTINUE (FALSE)
					}
					// Устанавливаем буфер стравнения
					buffer = readBufferW;
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
						RETURN (TRUE)
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
								RETURN (TRUE)
						}
						else
							cpi->WordFound = true;
					}
				} while (++index<=bufferCount-findStringCount);

				// Выходим, если мы вышли за пределы количества байт разрешённых для поиска
				if (EnableSearchInFirst && (unsigned __int64)SearchInFirst>=alreadyRead)
					CONTINUE (FALSE)
				// Запоминаем последний символ блока
				cpi->LastSymbol = buffer[bufferCount-1];
			}
			// Получаем смещение на которое мы отступили при переходе между блоками
			offset = (int)((UseAllCodePages?sizeof(wchar_t):codePages->MaxCharSize)*(findStringCount-1));
		}
		// Если мы потенциально прочитали не весь файл
		if (readBlockSize==readBufferSize)
		{
			// Отступаем назад на длину слова поиска минус 1
			if (!apiSetFilePointerEx(fileHandle, -1*offset, NULL, FILE_CURRENT))
				RETURN (FALSE)
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

void FindFiles::DoPreparePluginList(void* Param, string& strSaveDir)
{
  TRY {

    Sleep(200);
    HANDLE hPlugin=ArcList[FindFileArcIndex]->hPlugin;
    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
    strSaveDir = Info.CurDir;
		strPluginSearchPath=strSaveDir+L"\x1";
    WaitForSingleObject(hPluginMutex,INFINITE);
    if (SearchMode==FFSEARCH_ROOT ||
        SearchMode==FFSEARCH_ALL ||
        SearchMode==FFSEARCH_ALL_BUTNETWORK ||
        SearchMode==FFSEARCH_INPATH)
      CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_FIND);
    ReleaseMutex(hPluginMutex);
    RecurseLevel=0;
		InitInFileSearch();
    ScanPluginTree(hPlugin,ArcList[FindFileArcIndex]->Flags);
		ReleaseInFileSearch();
    WaitForSingleObject(hPluginMutex,INFINITE);
    if (SearchMode==FFSEARCH_ROOT ||
        SearchMode==FFSEARCH_ALL ||
        SearchMode==FFSEARCH_ALL_BUTNETWORK ||
        SearchMode==FFSEARCH_INPATH)
      CtrlObject->Plugins.SetDirectory(hPlugin,strSaveDir,OPM_FIND);
    ReleaseMutex(hPluginMutex);
    while (!StopSearch && FindMessageReady)
      Sleep(10);
    if (Param==NULL)
    {
      statusCS.Enter();

      strFindMessage.Format(MSG(MFindDone),FindFileCount,FindDirCount);
      FindMessageReady=TRUE;
      SearchDone=TRUE;

      statusCS.Leave();
    }
  }
  EXCEPT (xfilter((int)(INT_PTR)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1))
  {
    TerminateProcess( GetCurrentProcess(), 1);
  }
}

void _cdecl FindFiles::PreparePluginList(void *Param)
{
  string strSaveDir;

  DoPreparePluginList(Param, strSaveDir);
}

void FindFiles::ScanPluginTree(HANDLE hPlugin, DWORD Flags)
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
    ReleaseMutex(hPluginMutex);
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
    for (int I=0;I<ItemCount && !StopSearch;I++)
    {
      while (PauseSearch)
        Sleep(10);

      PluginPanelItem *CurPanelItem=PanelData+I;
      string strCurName=CurPanelItem->FindData.lpwszFileName;
      string strFullName;
      if (StrCmp(strCurName,L".")==0 || TestParentFolderName(strCurName))
        continue;
      if (Flags & OPIF_REALNAMES)
      {
        strFullName = strCurName;
      }
      else
      {
        strFullName = strPluginSearchPath;
        strFullName += strCurName;
      }

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
          RemovePseudoBackSlash(strFindMessage);
          FindMessageReady=TRUE;

          statusCS.Leave();

        }

        if (IsFileIncluded(CurPanelItem,strCurName,CurPanelItem->FindData.dwFileAttributes))
          AddMenuRecord(strFullName,&CurPanelItem->FindData);

        if (SearchInArchives && (hPlugin != INVALID_HANDLE_VALUE) && (Flags & OPIF_REALNAMES))
          ArchiveSearch(strFullName);
      }
    }
  }
  if (SearchMode!=FFSEARCH_CURRENT_ONLY)
  {
    for (int I=0;I<ItemCount && !StopSearch;I++)
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
        if (!strCurName.Contains(L'\x1') && CtrlObject->Plugins.SetDirectory(hPlugin,strCurName,OPM_FIND))
        {
          ReleaseMutex(hPluginMutex);

          strPluginSearchPath += strCurName;
          strPluginSearchPath += L"\x1";
          ScanPluginTree(hPlugin, Flags);
          if (strPluginSearchPath.RPos(pos,L'\x1'))
            strPluginSearchPath.SetLength(pos);
        }
        if (strPluginSearchPath.RPos(pos,L'\x1'))
          strPluginSearchPath.SetLength(pos+1);
        else
          strPluginSearchPath.SetLength(0);
        WaitForSingleObject(hPluginMutex,INFINITE);
        if (!CtrlObject->Plugins.SetDirectory(hPlugin,L"..",OPM_FIND))
          StopSearch=TRUE;
        ReleaseMutex(hPluginMutex);
        if (StopSearch) break;
      }
      else
        ReleaseMutex(hPluginMutex);
    }
  }
  CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,ItemCount);
  RecurseLevel--;
}

void _cdecl FindFiles::WriteDialogData(void *Param)
{
  string strDataStr;

  WriteDataUsed=TRUE;

  while( true )
  {
    Dialog* Dlg=(Dialog*)hDlg;

    if( !Dlg )
      break;

    VMenu *ListBox=Dlg->Item[1]->ListPtr;

    if (ListBox && !PauseSearch && !ScreenSaverActive)
    {
      if (BreakMainThread)
        StopSearch=TRUE;

      if (FindCountReady)
      {
        statusCS.Enter ();

        strDataStr.Format (L" %s: %d ", MSG(MFindFound),FindFileCount+FindDirCount);

        statusCS.Leave ();

        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(LONG_PTR)(const wchar_t*)strDataStr);

        FindCountReady=FALSE;
      }

      if (FindMessageReady)
      {
        string strSearchStr;

        if ( !strFindStr.IsEmpty() )
        {
          string strTemp, strFStr;

          strFStr = strFindStr;

          TruncStrFromEnd(strFStr,10);

          strTemp.Format (L" \"%s\"", (const wchar_t*)strFStr);
          strSearchStr.Format (MSG(MFindSearchingIn), (const wchar_t*)strTemp);
        }
        else
          strSearchStr.Format (MSG(MFindSearchingIn), L"");

        int Wid1=(int)strSearchStr.GetLength();
        int Wid2=DlgWidth-(int)strSearchStr.GetLength()-1;

        if (SearchDone)
        {
          Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

          strDataStr = MSG(MFindCancel);
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,9,(LONG_PTR)(const wchar_t*)strDataStr);

          statusCS.Enter ();
          strDataStr.Format (L"%-*.*s",DlgWidth,DlgWidth, (const wchar_t*)strFindMessage);
          statusCS.Leave ();

          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,3,(LONG_PTR)(const wchar_t*)strDataStr);

          strDataStr = L"";
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(LONG_PTR)(const wchar_t*)strDataStr);

          Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);

          SetFarTitle(strFindMessage);
          StopSearch=TRUE;
        }
        else
        {
          statusCS.Enter ();

          TruncPathStr(strFindMessage, Wid2);

          strDataStr.Format(L"%-*.*s %-*.*s",Wid1,Wid1, (const wchar_t*)strSearchStr,Wid2,Wid2,(const wchar_t*)strFindMessage);
          statusCS.Leave ();

          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,3,(LONG_PTR)(const wchar_t*)strDataStr);
        }

        FindMessageReady=FALSE;
      }

      if (LastFoundNumber && ListBox)
      {
        LastFoundNumber=0;

        if ( ListBox->UpdateRequired () )
          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,1,1);
      }
    }

    if (StopSearch && SearchDone && !FindMessageReady && !FindCountReady && !LastFoundNumber)
        break;

    Sleep(20);
  }

  WriteDataUsed=FALSE;
}

BOOL FindFiles::FindListGrow()
{
  DWORD Delta = (FindListCapacity < 256)?LIST_DELTA:FindListCapacity/2;
  LPFINDLIST* NewList = (LPFINDLIST*)xf_realloc(FindList, (FindListCapacity + Delta) * sizeof(*FindList));
  if (NewList)
  {
    FindList = NewList;
    FindListCapacity+= Delta;
    return(TRUE);
  }
  return(FALSE);
}

BOOL FindFiles::ArcListGrow()
{
  DWORD Delta = (ArcListCapacity < 256)?LIST_DELTA:ArcListCapacity/2;
  LPARCLIST* NewList = (LPARCLIST*)xf_realloc(ArcList, (ArcListCapacity + Delta) * sizeof(*ArcList));
  if (NewList)
  {
    ArcList = NewList;
    ArcListCapacity+= Delta;
    return(TRUE);
  }
  return(FALSE);
}

DWORD FindFiles::AddFindListItem(FAR_FIND_DATA_EX *FindData)
{
  if ((FindListCount == FindListCapacity) &&
      (!FindListGrow()))
    return(LIST_INDEX_NONE);

  FindList[FindListCount] = new _FINDLIST;

  FindList[FindListCount]->FindData = *FindData;
  FindList[FindListCount]->ArcIndex = LIST_INDEX_NONE;
  return(FindListCount++);
}

DWORD FindFiles::AddArcListItem(const wchar_t *ArcName, HANDLE hPlugin,
                                DWORD dwFlags, const wchar_t *RootPath)
{
  if ((ArcListCount == ArcListCapacity) &&
      (!ArcListGrow()))
    return(LIST_INDEX_NONE);

  ArcList[ArcListCount] = new _ARCLIST;

  ArcList[ArcListCount]->strArcName = ArcName;
  ArcList[ArcListCount]->hPlugin = hPlugin;
  ArcList[ArcListCount]->Flags = dwFlags;
  ArcList[ArcListCount]->strRootPath = RootPath;
  AddEndSlash(ArcList[ArcListCount]->strRootPath);
  return(ArcListCount++);
}

void FindFiles::ClearAllLists()
{
  CriticalSectionLock Lock(ffCS);

  FindFileArcIndex = LIST_INDEX_NONE;

  if (FindList)
  {
      for (unsigned int i = 0; i < FindListCount; i++)
          delete FindList[i];

    xf_free(FindList);
  }
  FindList = NULL;
  FindListCapacity = FindListCount = 0;

  if (ArcList)
  {
      for (unsigned int i = 0; i < ArcListCount; i++)
          delete ArcList[i];

    xf_free(ArcList);
  }
  ArcList = NULL;
  ArcListCapacity = ArcListCount = 0;
}

string &FindFiles::PrepareDriveNameStr(string &strSearchFromRoot, size_t sz)
{
  string strCurDir, strMsgStr, strMsgStr1;
  int MsgLen,DrvLen,MsgLenDiff;

  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();

  CtrlObject->CmdLine->GetCurDir(strCurDir);

  GetPathRootOne(strCurDir, strCurDir);

	DeleteEndSlash(strCurDir);

  if (strCurDir.IsEmpty() || PluginMode)
  {
    strSearchFromRoot = MSG(MSearchFromRootFolder);
    strMsgStr1 = strSearchFromRoot;

    RemoveHighlights(strMsgStr1);
    MsgLen=(int)strMsgStr1.GetLength();

    // Разница в длине строк с '&' и без. Нужно для дальнейшего
    // учёта точной длины строки, без учёта '&'
    MsgLenDiff=(int)strSearchFromRoot.GetLength()-MsgLen;
  }
  else
  {
    strMsgStr = MSG(MSearchFromRootOfDrive);
    strMsgStr1 = strMsgStr;

    RemoveHighlights(strMsgStr1);
    MsgLen=(int)strMsgStr1.GetLength();

    // Разница в длине строк с '&' и без. Нужно для дальнейшего
    // учёта точной длины строки, без учёта '&'
    MsgLenDiff=(int)strMsgStr.GetLength()-MsgLen;

    DrvLen=(int)sz-MsgLen-1-MsgLenDiff; // -1 - это пробел между строкой и диском, -MsgLenDiff - это учёт символа '&'
    if (DrvLen<7)
    {
      DrvLen=7; // Сделаем минимальный размер имени диска 7 символов
                // (учтём работу TruncPathStr с UNC, чтобы хоть что-то было видно)
      MsgLen=(int)sz-DrvLen-1-MsgLenDiff;
    }

    TruncStrFromEnd(strMsgStr,MsgLen+MsgLenDiff);
    TruncPathStr(strCurDir,DrvLen);

    strSearchFromRoot.Format (L"%s %s", (const wchar_t*)strMsgStr, (const wchar_t*)strCurDir);
  }

  //SearchFromRoot[sz-1+MsgLenDiff]=0; //BUGBUG

  return strSearchFromRoot;
}

wchar_t *FindFiles::RemovePseudoBackSlash(wchar_t *FileName)
{
	for (int i=0;FileName[i]!=0;i++)
	{
		if (FileName[i]==L'\x1')
			FileName[i]=L'\\';
	}
	return FileName;
}

string &FindFiles::RemovePseudoBackSlash(string &strFileName)
{
	wchar_t *lpwszFileName = strFileName.GetBuffer ();

	RemovePseudoBackSlash (lpwszFileName);

	strFileName.ReleaseBuffer ();

	return strFileName;
}
