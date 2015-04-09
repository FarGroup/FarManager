/*
findfile.cpp

Поиск (Alt-F7)
*/
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

#include "findfile.hpp"
#include "flink.hpp"
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
#include "filemasks.hpp"
#include "filefilter.hpp"
#include "farexcpt.hpp"
#include "syslog.hpp"
#include "codepage.hpp"
#include "cddrv.hpp"
#include "TaskBar.hpp"
#include "interf.hpp"
#include "colormix.hpp"
#include "message.hpp"
#include "delete.hpp"
#include "datetime.hpp"
#include "drivemix.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "mix.hpp"
#include "constitle.hpp"
#include "DlgGuid.hpp"
#include "synchro.hpp"
#include "console.hpp"
#include "wakeful.hpp"
#include "panelmix.hpp"
#include "setattr.hpp"
#include "keyboard.hpp"
#include "configdb.hpp"
#include "plugins.hpp"
#include "language.hpp"
#include "filestr.hpp"
#include "exitcode.hpp"

// Список архивов. Если файл найден в архиве, то FindList->ArcIndex указывает сюда.
struct ArcListItem
{
	string strArcName;
	PluginHandle* hPlugin;    // Plugin handle
	UINT64 Flags;       // OpenPanelInfo.Flags
	string strRootPath; // Root path in plugin after opening.
};

// Список найденных файлов. Индекс из списка хранится в меню.
struct FindListItem
{
	os::FAR_FIND_DATA FindData;
	ArcListItem* Arc;
	DWORD Used;
	void* Data;
	FARPANELITEMFREECALLBACK FreeData;
};

class InterThreadData
{
private:
	CriticalSection DataCS;
	ArcListItem* FindFileArcItem;
	int Percent;
	int LastFoundNumber;
	int FileCount;
	int DirCount;

	std::list<FindListItem> FindList;
	std::list<ArcListItem> ArcList;
	string strFindMessage;

public:
	InterThreadData() {Init();}
	~InterThreadData() { ClearAllLists(); }

	void Init()
	{
		SCOPED_ACTION(CriticalSectionLock)(DataCS);
		FindFileArcItem = nullptr;
		Percent=0;
		LastFoundNumber=0;
		FileCount=0;
		DirCount=0;
		FindList.clear();
		ArcList.clear();
		strFindMessage.clear();
	}

	std::list<FindListItem>& GetFindList() {return FindList;}

	void Lock() {DataCS.lock();}
	void Unlock() {DataCS.unlock();}

	ArcListItem* GetFindFileArcItem()
	{
		SCOPED_ACTION(CriticalSectionLock)(DataCS);
		return FindFileArcItem;
	}

	void SetFindFileArcItem(ArcListItem* Value)
	{
		SCOPED_ACTION(CriticalSectionLock)(DataCS);
		FindFileArcItem = Value;
	}

	int GetPercent() const { return Percent; }

	void SetPercent(int Value)
	{
		SCOPED_ACTION(CriticalSectionLock)(DataCS);
		Percent = Value;
	}

	int GetLastFoundNumber() const { return LastFoundNumber; }

	void SetLastFoundNumber(int Value)
	{
		SCOPED_ACTION(CriticalSectionLock)(DataCS);
		LastFoundNumber = Value;
	}

	int GetFileCount() const { return FileCount; }

	void SetFileCount(int Value)
	{
		SCOPED_ACTION(CriticalSectionLock)(DataCS);
		FileCount = Value;
	}

	int GetDirCount() const { return DirCount; }

	void SetDirCount(int Value)
	{
		SCOPED_ACTION(CriticalSectionLock)(DataCS);
		DirCount = Value;
	}

	size_t GetFindListCount() const { return FindList.size(); }

	void GetFindMessage(string& To)
	{
		SCOPED_ACTION(CriticalSectionLock)(DataCS);
		To=strFindMessage;
	}

	void SetFindMessage(const string& From)
	{
		SCOPED_ACTION(CriticalSectionLock)(DataCS);
		strFindMessage=From;
	}

	void ClearAllLists()
	{
		SCOPED_ACTION(CriticalSectionLock)(DataCS);
		FindFileArcItem = nullptr;

		if (!FindList.empty())
		{
			std::for_each(CONST_RANGE(FindList, i)
			{
				if (i.FreeData)
				{
					FarPanelItemFreeInfo info={sizeof(FarPanelItemFreeInfo),nullptr};
					if(i.Arc)
					{
						info.hPlugin=i.Arc->hPlugin;
					}
					i.FreeData(i.Data,&info);
				}
			});
			FindList.clear();
		}

		ArcList.clear();
	}

	ArcListItem& AddArcListItem(const string& ArcName, PluginHandle* hPlugin, UINT64 dwFlags, const string& RootPath)
	{
		SCOPED_ACTION(CriticalSectionLock)(DataCS);

		ArcListItem NewItem;
		NewItem.strArcName = ArcName;
		NewItem.hPlugin = hPlugin;
		NewItem.Flags = dwFlags;
		NewItem.strRootPath = RootPath;
		AddEndSlash(NewItem.strRootPath);
		ArcList.emplace_back(NewItem);
		return ArcList.back();
	}

	FindListItem& AddFindListItem(const os::FAR_FIND_DATA& FindData, void* Data, FARPANELITEMFREECALLBACK FreeData)
	{
		SCOPED_ACTION(CriticalSectionLock)(DataCS);

		FindListItem NewItem;
		NewItem.FindData = FindData;
		NewItem.Arc = nullptr;
		NewItem.Data = Data;
		NewItem.FreeData = FreeData;
		FindList.emplace_back(NewItem);
		return FindList.back();
	}
};


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
	AD_SEPARATOR1,
	AD_TEXT_COLUMNSFORMAT,
	AD_EDIT_COLUMNSFORMAT,
	AD_TEXT_COLUMNSWIDTH,
	AD_EDIT_COLUMNSWIDTH,
	AD_SEPARATOR2,
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
	FAD_CHECKBOX_NOTCONTAINING,
	FAD_CHECKBOX_ARC,
	FAD_CHECKBOX_DIRS,
	FAD_CHECKBOX_LINKS,
	FAD_CHECKBOX_STREAMS,
	FAD_SEPARATOR_2,
	FAD_SEPARATOR_3,
	FAD_TEXT_WHERE,
	FAD_COMBOBOX_WHERE,
	FAD_CHECKBOX_FILTER,
	FAD_SEPARATOR_4,
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

struct FindFiles::CodePageInfo
{
	CodePageInfo(uintptr_t CodePage):
		CodePage(CodePage),
		MaxCharSize(0),
		LastSymbol(0),
		WordFound(false)
	{
	}

	uintptr_t CodePage;
	UINT MaxCharSize;
	wchar_t LastSymbol;
	bool WordFound;

	void initialize()
	{
		if (IsUnicodeCodePage(CodePage))
			MaxCharSize = 2;
		else
		{
			CPINFO cpi;

			if (!GetCPInfo(CodePage, &cpi))
				cpi.MaxCharSize = 0; //Считаем, что ошибка и потом такие таблицы в поиске пропускаем

			MaxCharSize = cpi.MaxCharSize;
		}

		LastSymbol = 0;
		WordFound = false;
	}
};

void FindFiles::InitInFileSearch()
{
	if (!InFileSearchInited && !strFindStr.empty())
	{
		size_t findStringCount = strFindStr.size();
		// Инициализируем буферы чтения из файла
		const size_t readBufferSize = 32768;

		readBufferA.resize(readBufferSize);
		readBuffer.resize(readBufferSize);

		if (!SearchHex)
		{
			// Формируем строку поиска
			if (!CmpCase)
			{
				string UpperVersion(strFindStr), LowerVersion(strFindStr);
				ToUpper(UpperVersion);
				ToLower(LowerVersion);
				findStringBuffer = UpperVersion + LowerVersion;
				findString = findStringBuffer.data();
			}
			else
				findString = strFindStr.data();

			// Инициализируем данные для алгоритма поиска
			skipCharsTable.assign(std::numeric_limits<wchar_t>::max() + 1, findStringCount);

			for (size_t index = 0; index < findStringCount-1; index++)
				skipCharsTable[findString[index]] = findStringCount-1-index;

			if (!CmpCase)
				for (size_t index = 0; index < findStringCount-1; index++)
					skipCharsTable[findString[index+findStringCount]] = findStringCount-1-index;

			// Формируем список кодовых страниц
			if (CodePage == CP_SET)
			{
				// Проверяем наличие выбранных страниц символов
				const auto CpEnum = Codepages().GetFavoritesEnumerator();
				bool hasSelected = std::any_of(CONST_RANGE(CpEnum, i) { return i.second & CPST_FIND; });

				if (hasSelected)
				{
					m_CodePages.clear();
				}
				else
				{
					// Добавляем стандартные таблицы символов
					const uintptr_t Predefined[] = { GetOEMCP(), GetACP(), CP_UTF8, CP_UNICODE, CP_REVERSEBOM };
					m_CodePages.insert(m_CodePages.end(), ALL_CONST_RANGE(Predefined));
				}

				// Добавляем избранные таблицы символов
				std::for_each(CONST_RANGE(CpEnum, i)
				{
					if (i.second & (hasSelected?CPST_FIND:CPST_FAVORITE))
					{
						uintptr_t codePage = std::stoi(i.first);

						// Проверяем дубли
						if (hasSelected || !std::any_of(CONST_RANGE(m_CodePages, i) { return i.CodePage == CodePage; }))
							m_CodePages.emplace_back(codePage);
					}
				});
			}
			else
			{
				m_CodePages.emplace_back(CodePage);
				m_Autodetection = CodePage == CP_DEFAULT;
			}

			std::for_each(RANGE(m_CodePages, i)
			{
				i.initialize();
			});
		}
		else
		{
			// Формируем hex-строку для поиска
			if (SearchHex)
			{
				hexFindString = HexStringToBlob(strFindStr.data());
			}

			// Инициализируем данные для аглоритма поиска
			skipCharsTable.assign(std::numeric_limits<unsigned char>::max() + 1, hexFindString.size());

			for (size_t index = 0; index < hexFindString.size() - 1; index++)
				skipCharsTable[hexFindString[index]] = hexFindString.size() - 1 - index;
		}

		InFileSearchInited=true;
	}
}

void FindFiles::ReleaseInFileSearch()
{
	if (InFileSearchInited && !strFindStr.empty())
	{
		clear_and_shrink(readBufferA);
		clear_and_shrink(readBuffer);
		clear_and_shrink(skipCharsTable);
		hexFindString.clear();
		m_CodePages.clear();
		InFileSearchInited=false;
	}
}

string& FindFiles::PrepareDriveNameStr(string &strSearchFromRoot)
{
	auto strCurDir = Global->CtrlObject->CmdLine()->GetCurDir();
	GetPathRoot(strCurDir,strCurDir);
	DeleteEndSlash(strCurDir);

	if (
	    strCurDir.empty()||
	    (Global->CtrlObject->Cp()->ActivePanel()->GetMode()==PLUGIN_PANEL && Global->CtrlObject->Cp()->ActivePanel()->IsVisible())
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
bool FindFiles::IsWordDiv(const wchar_t symbol)
{
	// Так же разделителем является конец строки и пробельные символы
	return !symbol||IsSpace(symbol)||IsEol(symbol)||::IsWordDiv(Global->Opt->strWordDiv,symbol);
}

#if defined(MANTIS_0002207)
static intptr_t GetUserDataFromPluginItem(const wchar_t *Name, const PluginPanelItem * const* PanelData,size_t ItemCount)
{
	intptr_t UserData=0;

	if (Name && *Name)
	{
		for (size_t Index=0; Index < ItemCount; ++Index)
		{
			if (!StrCmp(PanelData[Index]->FileName,Name))
			{
				UserData=(intptr_t)PanelData[Index]->UserData.Data;
				break;
			}
		}
	}

	return UserData;
}
#endif

void FindFiles::SetPluginDirectory(const string& DirName, PluginHandle* hPlugin, bool UpdatePanel, UserDataItem *UserData)
{
	if (!DirName.empty())
	{
		string strName(DirName);
		//const wchar_t* DirPtr = ;
		const wchar_t* NamePtr = PointToName(strName.data());

		if (NamePtr != strName.data())
		{
			string Dir = strName.substr(0, NamePtr - strName.data());

			// force plugin to update its file list (that can be empty at this time)
			// if not done SetDirectory may fail
			{
				size_t FileCount=0;
				PluginPanelItem *PanelData=nullptr;

				if (Global->CtrlObject->Plugins->GetFindData(hPlugin,&PanelData,&FileCount,OPM_SILENT))
					Global->CtrlObject->Plugins->FreeFindData(hPlugin,PanelData,FileCount,true);
			}

			DeleteEndSlash(Dir);
			if (!Dir.empty())
			{
				Global->CtrlObject->Plugins->SetDirectory(hPlugin, Dir.data(), OPM_SILENT, UserData);
			}
			else
			{
				Global->CtrlObject->Plugins->SetDirectory(hPlugin,L"\\",OPM_SILENT);
			}
		}

		// Отрисуем панель при необходимости.
		if (UpdatePanel)
		{
			Global->CtrlObject->Cp()->ActivePanel()->Update(UPDATE_KEEP_SELECTION);
			Global->CtrlObject->Cp()->ActivePanel()->GoToFile(NamePtr);
			Global->CtrlObject->Cp()->ActivePanel()->Show();
		}
	}
}

intptr_t FindFiles::AdvancedDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	switch (Msg)
	{
		case DN_CLOSE:

			if (Param1==AD_BUTTON_OK)
			{
				auto Data = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, AD_EDIT_SEARCHFIRST, 0));

				if (Data && *Data && !CheckFileSizeStringFormat(Data))
				{
					Message(MSG_WARNING,1,MSG(MFindFileAdvancedTitle),MSG(MBadFileSizeFormat),MSG(MOk));
					return FALSE;
				}
			}

			break;
		default:
			break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

void FindFiles::AdvancedDialog()
{
	FarDialogItem AdvancedDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,52,11,0,nullptr,nullptr,0,MSG(MFindFileAdvancedTitle)},
		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(MFindFileSearchFirst)},
		{DI_EDIT,5,3,50,3,0,nullptr,nullptr,0,Global->Opt->FindOpt.strSearchInFirstSize.data()},
		{DI_TEXT,-1,4,0,4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_TEXT,5,5, 0, 5,0,nullptr,nullptr,0,MSG(MFindAlternateModeTypes)},
		{DI_EDIT,5,6,50, 6,0,nullptr,nullptr,0,Global->Opt->FindOpt.strSearchOutFormat.data()},
		{DI_TEXT,5,7, 0, 7,0,nullptr,nullptr,0,MSG(MFindAlternateModeWidths)},
		{DI_EDIT,5,8,50, 8,0,nullptr,nullptr,0,Global->Opt->FindOpt.strSearchOutFormatWidth.data()},
		{DI_TEXT,-1,9,0,9,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,10,0,10,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,0,10,0,10,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	auto AdvancedDlg = MakeDialogItemsEx(AdvancedDlgData);
	auto Dlg = Dialog::create(AdvancedDlg, &FindFiles::AdvancedDlgProc);
	Dlg->SetHelp(L"FindFileAdvanced");
	Dlg->SetPosition(-1,-1,52+4,13);
	Dlg->Process();
	int ExitCode=Dlg->GetExitCode();

	if (ExitCode==AD_BUTTON_OK)
	{
		Global->Opt->FindOpt.strSearchInFirstSize = AdvancedDlg[AD_EDIT_SEARCHFIRST].strData;
		SearchInFirst=ConvertFileSizeString(Global->Opt->FindOpt.strSearchInFirstSize);

		Global->Opt->SetSearchColumns(AdvancedDlg[AD_EDIT_COLUMNSFORMAT].strData, AdvancedDlg[AD_EDIT_COLUMNSWIDTH].strData);
	}
}

intptr_t FindFiles::MainDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	const auto SetAllCpTitle = [&]()
	{
		const int TitlePosition = 1;
		const auto CpEnum = Codepages().GetFavoritesEnumerator();
		auto Title = MSG(std::any_of(CONST_RANGE(CpEnum, i) { return i.second & CPST_FIND; }) ? MFindFileSelectedCodePages : MFindFileAllCodePages);
		Dlg->GetAllItem()[FAD_COMBOBOX_CP].ListPtr->GetItemPtr(TitlePosition)->strName = Title;
		FarListPos Position = { sizeof(FarListPos) };
		Dlg->SendMessage(DM_LISTGETCURPOS, FAD_COMBOBOX_CP, &Position);
		if (Position.SelectPos == TitlePosition)
			Dlg->SendMessage(DM_SETTEXTPTR, FAD_COMBOBOX_CP, const_cast<wchar_t*>(Title));
	};

	switch (Msg)
	{
		case DN_INITDIALOG:
		{
			bool Hex=(Dlg->SendMessage(DM_GETCHECK,FAD_CHECKBOX_HEX,0)==BSTATE_CHECKED);
			Dlg->SendMessage(DM_SHOWITEM,FAD_EDIT_TEXT,ToPtr(!Hex));
			Dlg->SendMessage(DM_SHOWITEM,FAD_EDIT_HEX,ToPtr(Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_TEXT_CP,ToPtr(!Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_COMBOBOX_CP,ToPtr(!Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_CASE,ToPtr(!Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_WHOLEWORDS,ToPtr(!Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_DIRS,ToPtr(!Hex));
			Dlg->SendMessage(DM_EDITUNCHANGEDFLAG,FAD_EDIT_TEXT,ToPtr(1));
			Dlg->SendMessage(DM_EDITUNCHANGEDFLAG,FAD_EDIT_HEX,ToPtr(1));
			Dlg->SendMessage(DM_SETTEXTPTR,FAD_TEXT_TEXTHEX,const_cast<wchar_t*>(Hex?MSG(MFindFileHex):MSG(MFindFileText)));
			Dlg->SendMessage(DM_SETTEXTPTR,FAD_TEXT_CP,const_cast<wchar_t*>(MSG(MFindFileCodePage)));
			Dlg->SendMessage(DM_SETCOMBOBOXEVENT,FAD_COMBOBOX_CP,ToPtr(CBET_KEY));
			FarListTitles Titles={sizeof(FarListTitles),0,nullptr,0,MSG(MFindFileCodePageBottom)};
			Dlg->SendMessage(DM_LISTSETTITLES,FAD_COMBOBOX_CP,&Titles);
			// Установка запомненных ранее параметров
			CodePage = Global->Opt->FindCodePage;
			favoriteCodePages = Codepages().FillCodePagesList(Dlg, FAD_COMBOBOX_CP, CodePage, true, true, false, false);
			SetAllCpTitle();

			// Текущее значение в в списке выбора кодовых страниц в общем случае может не совпадать с CodePage,
			// так что получаем CodePage из списка выбора
			FarListPos Position={sizeof(FarListPos)};
			Dlg->SendMessage( DM_LISTGETCURPOS, FAD_COMBOBOX_CP, &Position);
			FarListGetItem Item = { sizeof(FarListGetItem), Position.SelectPos };
			Dlg->SendMessage( DM_LISTGETITEM, FAD_COMBOBOX_CP, &Item);
			CodePage = *(uintptr_t*)Dlg->SendMessage( DM_LISTGETDATA, FAD_COMBOBOX_CP, ToPtr(Position.SelectPos));
			return TRUE;
		}
		case DN_CLOSE:
		{
			switch (Param1)
			{
				case FAD_BUTTON_FIND:
				{
					string Mask((LPCWSTR)Dlg->SendMessage(DM_GETCONSTTEXTPTR,FAD_EDIT_MASK,0));

					if (Mask.empty())
						Mask=L"*";

					return FileMaskForFindFile->Set(Mask);
				}
				case FAD_BUTTON_DRIVE:
				{
					Global->IsRedrawWindowInProcess++;
					Global->CtrlObject->Cp()->ActivePanel()->ChangeDisk();
					// Ну что ж, раз пошла такая пьянка рефрешить окна
					// будем таким способом.
					//WindowManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
					Global->WindowManager->ResizeAllWindows();
					Global->IsRedrawWindowInProcess--;
					string strSearchFromRoot;
					PrepareDriveNameStr(strSearchFromRoot);
					FarListGetItem item={sizeof(FarListGetItem),FADC_ROOT};
					Dlg->SendMessage(DM_LISTGETITEM,FAD_COMBOBOX_WHERE,&item);
					item.Item.Text=strSearchFromRoot.data();
					Dlg->SendMessage(DM_LISTUPDATE,FAD_COMBOBOX_WHERE,&item);
					PluginMode=Global->CtrlObject->Cp()->ActivePanel()->GetMode()==PLUGIN_PANEL;
					Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_DIRS,ToPtr(!PluginMode));
					item.ItemIndex=FADC_ALLDISKS;
					Dlg->SendMessage(DM_LISTGETITEM,FAD_COMBOBOX_WHERE,&item);

					if (PluginMode)
						item.Item.Flags|=LIF_GRAYED;
					else
						item.Item.Flags&=~LIF_GRAYED;

					Dlg->SendMessage(DM_LISTUPDATE,FAD_COMBOBOX_WHERE,&item);
					item.ItemIndex=FADC_ALLBUTNET;
					Dlg->SendMessage(DM_LISTGETITEM,FAD_COMBOBOX_WHERE,&item);

					if (PluginMode)
						item.Item.Flags|=LIF_GRAYED;
					else
						item.Item.Flags&=~LIF_GRAYED;

					Dlg->SendMessage(DM_LISTUPDATE,FAD_COMBOBOX_WHERE,&item);
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
					{
						FindFoldersChanged = true;
					}
					break;

				case FAD_CHECKBOX_HEX:
				{
					Dlg->SendMessage(DM_ENABLEREDRAW,FALSE,0);
					auto Src = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, Param2? FAD_EDIT_TEXT : FAD_EDIT_HEX, 0));
					string strDataStr;
					if (Param2)
					{
						// BUGBUG, it's unclear how to represent unicode in hex
						const auto AnsiStr = narrow(Src);
						strDataStr = BlobToHexWString(AnsiStr.data(), AnsiStr.size(), 0);
					}
					else
					{
						string strSrc(Src);
						RemoveTrailingSpaces(strSrc); // BUGBUG: trailing spaces in DI_FIXEDIT. TODO: Fix in Dialog class.
						auto Blob = HexStringToBlob(strSrc.data(), L' ');
						strDataStr.assign(ALL_CONST_RANGE(Blob));
					}

					Dlg->SendMessage(DM_SETTEXTPTR,Param2?FAD_EDIT_HEX:FAD_EDIT_TEXT, UNSAFE_CSTR(strDataStr));
					auto iParam = reinterpret_cast<intptr_t>(Param2);
					Dlg->SendMessage(DM_SHOWITEM,FAD_EDIT_TEXT,ToPtr(!iParam));
					Dlg->SendMessage(DM_SHOWITEM,FAD_EDIT_HEX,ToPtr(iParam));
					Dlg->SendMessage(DM_ENABLE,FAD_TEXT_CP,ToPtr(!iParam));
					Dlg->SendMessage(DM_ENABLE,FAD_COMBOBOX_CP,ToPtr(!iParam));
					Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_CASE,ToPtr(!iParam));
					Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_WHOLEWORDS,ToPtr(!iParam));
					Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_DIRS,ToPtr(!iParam));
					Dlg->SendMessage(DM_SETTEXTPTR,FAD_TEXT_TEXTHEX, const_cast<wchar_t*>(Param2?MSG(MFindFileHex):MSG(MFindFileText)));

					if (strDataStr.size()>0)
					{
						int UnchangeFlag=(int)Dlg->SendMessage(DM_EDITUNCHANGEDFLAG,FAD_EDIT_TEXT,ToPtr(-1));
						Dlg->SendMessage(DM_EDITUNCHANGEDFLAG,FAD_EDIT_HEX,ToPtr(UnchangeFlag));
					}

					Dlg->SendMessage(DM_ENABLEREDRAW,TRUE,0);
				}
				break;
			}

			break;
		}
		case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
			if (record->EventType!=KEY_EVENT) break;
			int key = InputRecordToKey(record);
			switch (Param1)
			{
				case FAD_COMBOBOX_CP:
				{
					switch (key)
					{
						case KEY_INS:
						case KEY_NUMPAD0:
						case KEY_SPACE:
						{
							// Обработка установки/снятия флажков для стандартных и избранных таблиц символов
							// Получаем текущую позицию в выпадающем списке таблиц символов
							FarListPos Position={sizeof(FarListPos)};
							Dlg->SendMessage( DM_LISTGETCURPOS, FAD_COMBOBOX_CP, &Position);
							// Получаем номер выбранной таблицы символов
							FarListGetItem Item = { sizeof(FarListGetItem), Position.SelectPos };
							Dlg->SendMessage( DM_LISTGETITEM, FAD_COMBOBOX_CP, &Item);
							uintptr_t SelectedCodePage = *(uintptr_t*)Dlg->SendMessage( DM_LISTGETDATA, FAD_COMBOBOX_CP, ToPtr(Position.SelectPos));
							// Разрешаем отмечать только стандартные и избранные таблицы символов
							int FavoritesIndex = 2 + StandardCPCount + 2;

							if (Position.SelectPos > 1)
							{
								// Получаем текущее состояние флага в реестре
								long long SelectType = Codepages().GetFavorite(SelectedCodePage);

								// Отмечаем/разотмечаем таблицу символов
								if (Item.Item.Flags & LIF_CHECKED)
								{
									// Для стандартных таблиц символов просто удаляем значение из реестра, для
									// избранных же оставляем в реестре флаг, что таблица символов избранная
									if (SelectType & CPST_FAVORITE)
										Codepages().SetFavorite(SelectedCodePage, CPST_FAVORITE);
									else
										Codepages().DeleteFavorite(SelectedCodePage);

									Item.Item.Flags &= ~LIF_CHECKED;
								}
								else
								{
									Codepages().SetFavorite(SelectedCodePage, CPST_FIND | (SelectType & CPST_FAVORITE ? CPST_FAVORITE : 0));
									Item.Item.Flags |= LIF_CHECKED;
								}

								SetAllCpTitle();

								// Обновляем текущий элемент в выпадающем списке
								Dlg->SendMessage( DM_LISTUPDATE, FAD_COMBOBOX_CP, &Item);

								FarListPos Pos={sizeof(FarListPos),Position.SelectPos+1,Position.TopPos};
								Dlg->SendMessage( DM_LISTSETCURPOS, FAD_COMBOBOX_CP,&Pos);

								// Обрабатываем случай, когда таблица символов может присутствовать, как в стандартных, так и в избранных,
								// т.е. выбор/снятие флага автоматически происходит у обоих элементов
								bool bStandardCodePage = Position.SelectPos < FavoritesIndex;

								for (int Index = bStandardCodePage ? FavoritesIndex : 0; Index < (bStandardCodePage ? FavoritesIndex + favoriteCodePages : FavoritesIndex); Index++)
								{
									// Получаем элемент таблицы символов
									FarListGetItem CheckItem = { sizeof(FarListGetItem), Index };
									Dlg->SendMessage( DM_LISTGETITEM, FAD_COMBOBOX_CP, &CheckItem);

									// Обрабатываем только таблицы символов
									if (!(CheckItem.Item.Flags&LIF_SEPARATOR))
									{
										if (SelectedCodePage == *(UINT*)Dlg->SendMessage( DM_LISTGETDATA, FAD_COMBOBOX_CP, ToPtr(Index)))
										{
											if (Item.Item.Flags & LIF_CHECKED)
												CheckItem.Item.Flags |= LIF_CHECKED;
											else
												CheckItem.Item.Flags &= ~LIF_CHECKED;

											Dlg->SendMessage( DM_LISTUPDATE, FAD_COMBOBOX_CP, &CheckItem);
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
			auto& Item=*reinterpret_cast<FarDialogItem*>(Param2);

			switch (Param1)
			{
				case FAD_EDIT_TEXT:
					{
						// Строка "Содержащих текст"
						if (!FindFoldersChanged)
						{
							BOOL Checked = (Item.Data && *Item.Data)?FALSE:(int)Global->Opt->FindOpt.FindFolders;
							Dlg->SendMessage( DM_SETCHECK, FAD_CHECKBOX_DIRS, ToPtr(Checked?BSTATE_CHECKED:BSTATE_UNCHECKED));
						}

						return TRUE;
					}
					break;

				case FAD_COMBOBOX_CP:
				{
					// Получаем выбранную в выпадающем списке таблицу символов
					CodePage = *(uintptr_t*)Dlg->SendMessage( DM_LISTGETDATA, FAD_COMBOBOX_CP, ToPtr(Dlg->SendMessage( DM_LISTGETCURPOS, FAD_COMBOBOX_CP, 0)));
				}
				return TRUE;
				case FAD_COMBOBOX_WHERE:
					{
						SearchFromChanged=true;
					}
					return TRUE;
			}
		}
		case DN_HOTKEY:
		{
			if (Param1==FAD_TEXT_TEXTHEX)
			{
				Dlg->SendMessage(DM_SETFOCUS, FAD_EDIT_HEX,  0); // only one
				Dlg->SendMessage(DM_SETFOCUS, FAD_EDIT_TEXT, 0); // is active
				return FALSE;
			}
		}

		default:
			break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

bool FindFiles::GetPluginFile(ArcListItem* ArcItem, const os::FAR_FIND_DATA& FindData, const string& DestPath, string &strResultName, UserDataItem *UserData)
{
	_ALGO(CleverSysLog clv(L"FindFiles::GetPluginFile()"));
	OpenPanelInfo Info;
	Global->CtrlObject->Plugins->GetOpenPanelInfo(ArcItem->hPlugin,&Info);
	string strSaveDir = NullToEmpty(Info.CurDir);
	AddEndSlash(strSaveDir);
	Global->CtrlObject->Plugins->SetDirectory(ArcItem->hPlugin,L"\\",OPM_SILENT);
	//SetPluginDirectory(ArcItem->strRootPath,hPlugin);
	SetPluginDirectory(FindData.strFileName,ArcItem->hPlugin,false,UserData);
	const wchar_t *lpFileNameToFind = PointToName(FindData.strFileName);
	const wchar_t *lpFileNameToFindShort = PointToName(FindData.strAlternateFileName);
	PluginPanelItem *pItems;
	size_t nItemsNumber;
	bool nResult=false;

	if (Global->CtrlObject->Plugins->GetFindData(ArcItem->hPlugin,&pItems,&nItemsNumber,OPM_SILENT))
	{
		const auto End = pItems + nItemsNumber;
		const auto It = std::find_if(pItems, End, [&](PluginPanelItem& Item) -> bool
		{
			Item.FileName = PointToName(NullToEmpty(Item.FileName));
			Item.AlternateFileName = PointToName(NullToEmpty(Item.AlternateFileName));
			return !StrCmp(lpFileNameToFind, Item.FileName) && !StrCmp(lpFileNameToFindShort, Item.AlternateFileName);
		});

		if (It != End)
		{
			nResult = Global->CtrlObject->Plugins->GetFile(ArcItem->hPlugin, &*It, DestPath, strResultName, OPM_SILENT) != 0;
		}

		Global->CtrlObject->Plugins->FreeFindData(ArcItem->hPlugin,pItems,nItemsNumber,true);
	}

	Global->CtrlObject->Plugins->SetDirectory(ArcItem->hPlugin,L"\\",OPM_SILENT);
	SetPluginDirectory(strSaveDir,ArcItem->hPlugin);
	return nResult;
}


// Алгоритма Бойера-Мура-Хорспула поиска подстроки
template<class T, class Pred>
int FindFiles::FindStringBMH(const T* searchBuffer, size_t searchBufferCount, size_t findStringCount, Pred p) const
{
	auto buffer = searchBuffer;
	auto lastBufferChar = findStringCount - 1;

	while (searchBufferCount >= findStringCount)
	{
		for (auto index = lastBufferChar; p(buffer, index); index--)
			if (!index)
				return static_cast<int>(buffer - searchBuffer);

		auto offset = skipCharsTable[buffer[lastBufferChar]];
		searchBufferCount -= offset;
		buffer += offset;
	}

	return -1;
}

int FindFiles::FindStringBMH(const wchar_t* searchBuffer, size_t searchBufferCount) const
{
	size_t findStringCount = strFindStr.size();
	const wchar_t *findStringLower = CmpCase? nullptr : findString + findStringCount;

	return FindStringBMH(searchBuffer, searchBufferCount, findStringCount, [&](const wchar_t* Buffer, size_t index)
	{
		return Buffer[index] == findString[index] || (CmpCase ? 0 : Buffer[index] == findStringLower[index]);
	});
}

int FindFiles::FindStringBMH(const unsigned char* searchBuffer, size_t searchBufferCount) const
{
	return FindStringBMH(searchBuffer, searchBufferCount, hexFindString.size(), [&](const unsigned char* Buffer, size_t index)
	{
		return Buffer[index] == hexFindString[index];
	});
}


bool FindFiles::LookForString(const string& Name)
{
	// Длина строки поиска
	size_t findStringCount;

	// Если строки поиска пустая, то считаем, что мы всегда что-нибудь найдём
	if (!(findStringCount = strFindStr.size()))
		return true;

	os::fs::file file;
	// Открываем файл
	if(!file.Open(Name, FILE_READ_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
	{
		return false;
	}

	if (m_Autodetection)
	{
		if (!GetFileFormat(file, m_CodePages.front().CodePage))
		{
			// TODO diagnostic message
			m_CodePages.front().CodePage = GetACP();
		}
		m_CodePages.front().initialize();
	}

	// Количество считанных из файла байт
	size_t readBlockSize = 0;
	// Количество прочитанных из файла байт
	unsigned __int64 alreadyRead = 0;
	// Смещение на которое мы отступили при переходе между блоками
	size_t offset = 0;

	if (SearchHex)
		offset = hexFindString.size() - 1;

	UINT64 FileSize=0;
	file.GetSize(FileSize);

	if (SearchInFirst)
	{
		FileSize=std::min(SearchInFirst,FileSize);
	}

	UINT LastPercents=0;

	// Основной цикл чтения из файла
	while (!StopEvent.Signaled() && file.Read
		(readBufferA.data(), (!SearchInFirst || alreadyRead + readBufferA.size() <= SearchInFirst)? readBufferA.size() : SearchInFirst - alreadyRead, readBlockSize))
	{
		UINT Percents=static_cast<UINT>(FileSize?alreadyRead*100/FileSize:0);

		if (Percents!=LastPercents)
		{
			itd->SetPercent(Percents);
			LastPercents=Percents;
		}

		// Увеличиваем счётчик прочитаннх байт
		alreadyRead += readBlockSize;

		// Для hex и обыкновенного поиска разные ветки
		if (SearchHex)
		{
			// Выходим, если ничего не прочитали или прочитали мало
			if (!readBlockSize || readBlockSize < hexFindString.size())
				return false;

			// Ищем
			if (FindStringBMH((unsigned char *)readBufferA.data(), readBlockSize)!=-1)
				return true;
		}
		else
		{
			bool ErrorState = false;
			FOR(auto& i, m_CodePages)
			{
				ErrorState = false;
				// Пропускаем ошибочные кодовые страницы
				if (!i.MaxCharSize)
				{
					ErrorState = true;
					continue;
				}

				// Если начало файла очищаем информацию о поиске по словам
				if (WholeWords && alreadyRead==readBlockSize)
				{
					i.WordFound = false;
					i.LastSymbol = 0;
				}

				// Если ничего не прочитали
				if (!readBlockSize)
				{
					// Если поиск по словам и в конце предыдущего блока было что-то найдено,
					// то считаем, что нашли то, что нужно
					if(WholeWords && i.WordFound)
						return true;
					else
					{
						ErrorState = true;
						continue;
					}
					// Выходим, если прочитали меньше размера строки поиска и нет поиска по словам
				}

				if (readBlockSize < findStringCount && !(WholeWords && i.WordFound))
				{
					ErrorState = true;
					continue;
				}

				// Количество символов в выходном буфере
				size_t bufferCount;

				// Буфер для поиска
				wchar_t *buffer;

				// Перегоняем буфер в UTF-16
				if (IsUnicodeCodePage(i.CodePage))
				{
					// Вычисляем размер буфера в UTF-16
					bufferCount = readBlockSize/sizeof(wchar_t);

					// Выходим, если размер буфера меньше длины строки поиска
					if (bufferCount < findStringCount)
					{
						ErrorState = true;
						continue;
					}

					// Копируем буфер чтения в буфер сравнения
					if (i.CodePage==CP_REVERSEBOM)
					{
						// Для UTF-16 (big endian) преобразуем буфер чтения в буфер сравнения
						bufferCount = LCMapStringW(
							                LOCALE_NEUTRAL,//LOCALE_INVARIANT,
							                LCMAP_BYTEREV,
							                (wchar_t *)readBufferA.data(),
							                (int)bufferCount,
							                readBuffer.data(),
							                static_cast<int>(readBuffer.size() * sizeof(VALUE_TYPE(readBuffer)))
							            );

						if (!bufferCount)
						{
							ErrorState = true;
							continue;
						}
						// Устанавливаем буфер сравнения
						buffer = readBuffer.data();
					}
					else
					{
						// Если поиск в UTF-16 (little endian), то используем исходный буфер
						buffer = (wchar_t *)readBufferA.data();
					}
				}
				else
				{
					// Конвертируем буфер чтения из кодировки поиска в UTF-16
					bufferCount = MultiByteToWideChar(i.CodePage, 0, readBufferA.data(), static_cast<int>(readBlockSize), readBuffer.data(), static_cast<int>(readBuffer.size() * sizeof(VALUE_TYPE(readBuffer))));

					// Выходим, если нам не удалось сконвертировать строку
					if (!bufferCount)
					{
						ErrorState = true;
						continue;
					}

					// Если у нас поиск по словам и в конце предыдущего блока было вхождение
					if (WholeWords && i.WordFound)
					{
						// Если конец файла, то считаем, что есть разделитель в конце
						if (findStringCount-1>=bufferCount)
							return true;

						// Проверяем первый символ текущего блока с учётом обратного смещения, которое делается
						// при переходе между блоками
						i.LastSymbol = readBuffer[findStringCount-1];

						if (IsWordDiv(i.LastSymbol))
							return true;

						// Если размер буфера меньше размера слова, то выходим
						if (readBlockSize < findStringCount)
						{
							ErrorState = true;
							continue;
						}
					}

					// Устанавливаем буфер сравнения
					buffer = readBuffer.data();
				}

				i.WordFound = false;
				unsigned int index = 0;

				do
				{
					// Ищем подстроку в буфере и возвращаем индекс её начала в случае успеха
					int foundIndex = FindStringBMH(buffer+index, bufferCount-index);

					// Если подстрока не найдена идём на следующий шаг
					if (foundIndex == -1)
						break;

					// Если подстрока найдена и отключен поиск по словам, то считаем что всё хорошо
					if (!WholeWords)
						return true;
					// Устанавливаем позицию в исходном буфере
					index += foundIndex;

					// Если идёт поиск по словам, то делаем соответствующие проверки
					bool firstWordDiv = false;

					// Если мы находимся вначале блока
					if (!index)
					{
						// Если мы находимся вначале файла, то считаем, что разделитель есть
						// Если мы находимся вначале блока, то проверяем является
						// или нет последний символ предыдущего блока разделителем
						if (alreadyRead==readBlockSize || IsWordDiv(i.LastSymbol))
							firstWordDiv = true;
					}
					else
					{
						// Проверяем является или нет предыдущий найденному символ блока разделителем
						i.LastSymbol = buffer[index-1];

						if (IsWordDiv(i.LastSymbol))
							firstWordDiv = true;
					}

					// Проверяем разделитель в конце, только если найден разделитель вначале
					if (firstWordDiv)
					{
						// Если блок выбран не до конца
						if (index+findStringCount!=bufferCount)
						{
							// Проверяем является или нет последующий за найденным символ блока разделителем
							i.LastSymbol = buffer[index+findStringCount];

							if (IsWordDiv(i.LastSymbol))
								return true;
						}
						else
							i.WordFound = true;
					}
				}
				while (++index<=bufferCount-findStringCount);

				// Выходим, если мы вышли за пределы количества байт разрешённых для поиска
				if (SearchInFirst && SearchInFirst>=alreadyRead)
				{
					ErrorState = true;
					continue;
				}
				// Запоминаем последний символ блока
				i.LastSymbol = buffer[bufferCount-1];
			}

			if (ErrorState)
				return false;

			// Получаем смещение на которое мы отступили при переходе между блоками
			offset = (CodePage == CP_SET? sizeof(wchar_t) : m_CodePages.begin()->MaxCharSize) * (findStringCount - 1);
		}

		// Если мы потенциально прочитали не весь файл
		if (readBlockSize == readBuffer.size())
		{
			// Отступаем назад на длину слова поиска минус 1
			if (!file.SetPointer(-1ll*offset, nullptr, FILE_CURRENT))
				return false;
			alreadyRead -= offset;
		}
	}

	return false;
}

bool FindFiles::IsFileIncluded(PluginPanelItem* FileItem, const string& FullName, DWORD FileAttr, const string &strDisplayName)
{
	bool FileFound=FileMaskForFindFile->Compare(PointToName(FullName));
	ArcListItem* ArcItem = itd->GetFindFileArcItem();
	PluginHandle* hPlugin=nullptr;
	if(ArcItem)
	{
		hPlugin = ArcItem->hPlugin;
	}

	if (FileFound)
	{
		// Если включен режим поиска hex-кодов, тогда папки в поиск не включаем
		if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) && (!Global->Opt->FindOpt.FindFolders || SearchHex))
			return false;

		if (!strFindStr.empty() && FileFound)
		{
			FileFound=false;

			if (!(FileAttr & FILE_ATTRIBUTE_DIRECTORY))
			{
				itd->SetFindMessage(strDisplayName);

				string strSearchFileName;
				bool RemoveTemp=false;

				if (hPlugin)
				{
					if (!Global->CtrlObject->Plugins->UseFarCommand(hPlugin, PLUGIN_FARGETFILES))
					{
						string strTempDir;
						FarMkTempEx(strTempDir); // А проверка на nullptr???
						os::CreateDirectory(strTempDir,nullptr);

						bool GetFileResult=false;
						{
							SCOPED_ACTION(CriticalSectionLock)(PluginCS);
							GetFileResult=Global->CtrlObject->Plugins->GetFile(hPlugin,FileItem,strTempDir,strSearchFileName,OPM_SILENT|OPM_FIND)!=FALSE;
						}
						if (GetFileResult)
						{
							RemoveTemp=true;
						}
						else
						{
							os::RemoveDirectory(strTempDir);
						}
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

				if (!strSearchFileName.empty())
				{
					bool StringFound = LookForString(strSearchFileName);
					if ((StringFound && !NotContaining) || (!StringFound && NotContaining))
						FileFound=true;
				}

				if (RemoveTemp)
				{
					DeleteFileWithFolder(strSearchFileName);
				}
			}
		}
	}

	return FileFound;
}

intptr_t FindFiles::FindDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	SCOPED_ACTION(CriticalSectionLock)(PluginCS);
	auto& ListBox = Dlg->GetAllItem()[FD_LISTBOX].ListPtr;

	static bool Recurse=false;

	if(!Finalized && !Recurse)
	{
		Recurse=true;
		if (m_TimeCheck)
		{
			if (!StopEvent.Signaled())
			{
				LangString strDataStr(MFindFound);
				strDataStr << itd->GetFileCount() << itd->GetDirCount();
				Dlg->SendMessage(DM_SETTEXTPTR,FD_SEPARATOR1, UNSAFE_CSTR(strDataStr));

				LangString strSearchStr(MFindSearchingIn);

				if (!strFindStr.empty())
				{
					string strFStr(strFindStr);
					TruncStrFromEnd(strFStr,10);
					string strTemp(L" \"");
					strTemp+=strFStr+=L"\"";
					strSearchStr << strTemp;
				}
				else
					strSearchStr << L"";

				string strFM;
				itd->GetFindMessage(strFM);
				SMALL_RECT Rect;
				Dlg->SendMessage( DM_GETITEMPOSITION, FD_TEXT_STATUS, &Rect);
				TruncStrFromCenter(strFM, Rect.Right-Rect.Left+1 - static_cast<int>(strSearchStr.size()) - 1);
				Dlg->SendMessage( DM_SETTEXTPTR, FD_TEXT_STATUS, UNSAFE_CSTR(strSearchStr + L" " + strFM));
				Dlg->SendMessage( DM_SETTEXTPTR,FD_TEXT_STATUS_PERCENTS, UNSAFE_CSTR(str_printf(L"%3d%%",itd->GetPercent())));

				if (itd->GetLastFoundNumber())
				{
					itd->SetLastFoundNumber(0);

					if (ListBox->UpdateRequired())
						Dlg->SendMessage(DM_SHOWITEM,FD_LISTBOX,ToPtr(1));
				}
			}
		}
		Recurse=false;
	}

	if(!Finalized && StopEvent.Signaled())
	{
		LangString strMessage(MFindDone);
		strMessage << itd->GetFileCount() << itd->GetDirCount();
		Dlg->SendMessage( DM_ENABLEREDRAW, FALSE, 0);
		Dlg->SendMessage( DM_SETTEXTPTR, FD_SEPARATOR1, nullptr);
		Dlg->SendMessage( DM_SETTEXTPTR, FD_TEXT_STATUS, UNSAFE_CSTR(strMessage));
		Dlg->SendMessage( DM_SETTEXTPTR, FD_TEXT_STATUS_PERCENTS, nullptr);
		Dlg->SendMessage( DM_SETTEXTPTR, FD_BUTTON_STOP, const_cast<wchar_t*>(MSG(MFindCancel)));
		Dlg->SendMessage( DM_ENABLEREDRAW, TRUE, 0);
		ConsoleTitle::SetFarTitle(strMessage);
		TB.reset();
		Finalized=true;
	}

	switch (Msg)
	{
	case DN_INITDIALOG:
		{
			Dlg->GetAllItem()[FD_LISTBOX].ListPtr->SetFlags(VMENU_NOMERGEBORDER);
		}
		break;

	case DN_DRAWDLGITEMDONE: //???
	case DN_DRAWDIALOGDONE:
		{
			Dlg->DefProc(Msg,Param1,Param2);

			// Переместим фокус на кнопку [Go To]
			if ((itd->GetDirCount() || itd->GetFileCount()) && !FindPositionChanged)
			{
				FindPositionChanged=true;
				Dlg->SendMessage(DM_SETFOCUS,FD_BUTTON_GOTO,0);
			}
			return TRUE;
		}
		break;

	case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
			if (record->EventType!=KEY_EVENT) break;
			int key = InputRecordToKey(record);
			switch (key)
			{
			case KEY_ESC:
			case KEY_F10:
				{
					if (!StopEvent.Signaled())
					{
						PauseEvent.Reset();
						bool LocalRes=true;
						if (Global->Opt->Confirm.Esc)
							LocalRes=ConfirmAbortOp()!=0;
						PauseEvent.Set();
						if(LocalRes)
						{
							StopEvent.Set();
						}
						return TRUE;
					}
				}
				break;

			case KEY_ALTF9:
			case KEY_RALTF9:
			case KEY_F11:
			case KEY_CTRLW:
			case KEY_RCTRLW:
				{
					Global->WindowManager->ProcessKey(Manager::Key(key));
					return TRUE;
				}
				break;

			case KEY_RIGHT:
			case KEY_NUMPAD6:
			case KEY_TAB:
				{
					if (Param1==FD_BUTTON_STOP)
					{
						FindPositionChanged=true;
						Dlg->SendMessage(DM_SETFOCUS,FD_BUTTON_NEW,0);
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
						FindPositionChanged=true;
						Dlg->SendMessage(DM_SETFOCUS,FD_BUTTON_STOP,0);
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
			case KEY_ALTLEFT:
			case KEY_RALTLEFT:
			case KEY_ALT|KEY_NUMPAD4:
			case KEY_RALT|KEY_NUMPAD4:
			case KEY_MSWHEEL_LEFT:
			case KEY_ALTRIGHT:
			case KEY_RALTRIGHT:
			case KEY_ALT|KEY_NUMPAD6:
			case KEY_RALT|KEY_NUMPAD6:
			case KEY_MSWHEEL_RIGHT:
			case KEY_ALTSHIFTLEFT:
			case KEY_RALTSHIFTLEFT:
			case KEY_ALT|KEY_SHIFT|KEY_NUMPAD4:
			case KEY_RALT|KEY_SHIFT|KEY_NUMPAD4:
			case KEY_ALTSHIFTRIGHT:
			case KEY_RALTSHIFTRIGHT:
			case KEY_ALT|KEY_SHIFT|KEY_NUMPAD6:
			case KEY_RALT|KEY_SHIFT|KEY_NUMPAD6:
			case KEY_ALTHOME:
			case KEY_RALTHOME:
			case KEY_ALT|KEY_NUMPAD7:
			case KEY_RALT|KEY_NUMPAD7:
			case KEY_ALTEND:
			case KEY_RALTEND:
			case KEY_ALT|KEY_NUMPAD1:
			case KEY_RALT|KEY_NUMPAD1:
				{
					ListBox->ProcessKey(Manager::Key(key));
					return TRUE;
				}
				break;

			/*
			case KEY_CTRLA:
			case KEY_RCTRLA:
			{
				if (!ListBox->GetItemCount())
				{
					return TRUE;
				}

				size_t ItemIndex = *static_cast<size_t*>(ListBox->GetUserData(nullptr,0));

				FINDLIST FindItem;
				itd->GetFindListItem(ItemIndex, FindItem);

				if (ShellSetFileAttributes(nullptr,FindItem.FindData.strFileName))
				{
					itd->SetFindListItem(ItemIndex, FindItem);
					Dlg->SendMessage(DM_REDRAW,0,0);
				}
				return TRUE;
			}
			*/

			case KEY_F3:
			case KEY_NUMPAD5:
			case KEY_SHIFTNUMPAD5:
			case KEY_F4:
				{
					if (!ListBox->GetItemCount())
					{
						return TRUE;
					}

					auto FindItem = *reinterpret_cast<FindListItem**>(ListBox->GetUserData(nullptr,0));
					bool RemoveTemp=false;
					string strSearchFileName;
					string strTempDir;

					if (FindItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						return TRUE;
					}

					bool real_name = true;

					if(FindItem->Arc)
					{
						if(!(FindItem->Arc->Flags & OPIF_REALNAMES))
						{
							// Плагины надо закрывать, если открыли.
							bool ClosePanel=false;
							real_name = false;

							string strFindArcName = FindItem->Arc->strArcName;
							if(!FindItem->Arc->hPlugin)
							{
								int SavePluginsOutput=Global->DisablePluginsOutput;
								Global->DisablePluginsOutput=TRUE;
								{
									SCOPED_ACTION(CriticalSectionLock)(PluginCS);
									FindItem->Arc->hPlugin = Global->CtrlObject->Plugins->OpenFilePlugin(&strFindArcName, 0, OFP_SEARCH);
								}
								Global->DisablePluginsOutput=SavePluginsOutput;

								if (FindItem->Arc->hPlugin == PANEL_STOP ||
										!FindItem->Arc->hPlugin)
								{
									FindItem->Arc->hPlugin = nullptr;
									return TRUE;
								}

								ClosePanel = true;
							}
							FarMkTempEx(strTempDir);
							os::CreateDirectory(strTempDir, nullptr);
							SCOPED_ACTION(CriticalSectionLock)(PluginCS);
							UserDataItem UserData = {FindItem->Data,FindItem->FreeData};
							bool bGet=GetPluginFile(FindItem->Arc,FindItem->FindData,strTempDir,strSearchFileName,&UserData);
							if (!bGet)
							{
								os::RemoveDirectory(strTempDir);

								if (ClosePanel)
								{
									Global->CtrlObject->Plugins->ClosePanel(FindItem->Arc->hPlugin);
									FindItem->Arc->hPlugin = nullptr;
								}
								return FALSE;
							}
							else
							{
								if (ClosePanel)
								{
									Global->CtrlObject->Plugins->ClosePanel(FindItem->Arc->hPlugin);
									FindItem->Arc->hPlugin = nullptr;
								}
							}
							RemoveTemp=true;
						}
					}

					if (real_name)
					{
						strSearchFileName = FindItem->FindData.strFileName;
						if (!os::fs::exists(strSearchFileName) && os::fs::exists(FindItem->FindData.strAlternateFileName))
							strSearchFileName = FindItem->FindData.strAlternateFileName;
					}

					if (os::fs::exists(strSearchFileName))
					{
						const auto strOldTitle = Console().GetTitle();

						if (key==KEY_F3 || key==KEY_NUMPAD5 || key==KEY_SHIFTNUMPAD5)
						{
							NamesList ViewList;
							int list_count = 0;

							// Возьмем все файлы, которые имеют реальные имена...
							itd->Lock();
							std::for_each(CONST_RANGE(itd->GetFindList(), i)
							{
								bool RealNames=true;
								if(i.Arc)
								{
									if(!(i.Arc->Flags & OPIF_REALNAMES))
									{
										RealNames=false;
									}
								}
								if (RealNames)
								{
									if (!i.FindData.strFileName.empty() && !(i.FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
									{
										++list_count;
										ViewList.AddName(i.FindData.strFileName);
									}
								}
							});
							itd->Unlock();
							ViewList.SetCurName(FindItem->FindData.strFileName);

							Dlg->SendMessage(DM_SHOWDIALOG,FALSE,0);
							Dlg->SendMessage(DM_ENABLEREDRAW,FALSE,0);
							{
								auto ShellViewer = FileViewer::create(strSearchFileName, FALSE, FALSE, FALSE, -1, nullptr, (list_count > 1 ? &ViewList : nullptr));
								ShellViewer->SetEnableF6(TRUE);

								if(FindItem->Arc)
								{
									if(!(FindItem->Arc->Flags & OPIF_REALNAMES))
									{
										ShellViewer->SetSaveToSaveAs(true);
									}
								}
								Global->WindowManager->ExecuteModal(ShellViewer);
								// заставляем рефрешится экран
								Global->WindowManager->ProcessKey(Manager::Key(KEY_CONSOLE_BUFFER_RESIZE));
							}
							Dlg->SendMessage(DM_ENABLEREDRAW,TRUE,0);
							Dlg->SendMessage(DM_SHOWDIALOG,TRUE,0);
						}
						else
						{
							Dlg->SendMessage(DM_SHOWDIALOG,FALSE,0);
							Dlg->SendMessage(DM_ENABLEREDRAW,FALSE,0);
							{
								/* $ 14.08.2002 VVM
								  ! Пока-что запретим из поиска переключаться в активный редактор.
								    К сожалению, манагер на это не способен сейчас
															int WindowPos=WindowManager->FindWindowByFile(MODALTYPE_EDITOR,SearchFileName);
															int SwitchTo=FALSE;
															if (WindowPos!=-1)
															{
																if (!(*WindowManager)[WindowPos]->GetCanLoseFocus(TRUE) ||
																	Global->Opt->Confirm.AllowReedit)
																{
																	char MsgFullFileName[NM];
																	xstrncpy(MsgFullFileName,SearchFileName,sizeof(MsgFullFileName));
																	int MsgCode=Message(0,2,MSG(MFindFileTitle),
																				TruncPathStr(MsgFullFileName,ScrX-16),
																				MSG(MAskReload),
																				MSG(MCurrent),MSG(MNewOpen));
																	if (!MsgCode)
																	{
																		SwitchTo=TRUE;
																	}
																	else if (MsgCode==1)
																	{
																		SwitchTo=FALSE;
																	}
																	else
																	{
																		Dlg->SendMessage(DM_ENABLEREDRAW,TRUE,0);
																		Dlg->SendMessage(DM_SHOWDIALOG,TRUE,0);
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
																(*WindowManager)[WindowPos]->SetCanLoseFocus(FALSE);
																(*WindowManager)[WindowPos]->SetDynamicallyBorn(false);
																WindowManager->ActivateWindow(WindowPos);
																WindowManager->EnterModalEV();
																WindowManager->ExecuteModal ();
																WindowManager->ExitModalEV();
																// WindowManager->ExecuteNonModal();
																// заставляем рефрешится экран
																WindowManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
															}
															else
								*/
								{
									auto ShellEditor = FileEditor::create(strSearchFileName, CP_DEFAULT, 0);
									ShellEditor->SetEnableF6(true);

									if(FindItem->Arc)
									{
										if(!(FindItem->Arc->Flags & OPIF_REALNAMES))
										{
											ShellEditor->SetSaveToSaveAs(true);
										}
									}
									auto editorExitCode=ShellEditor->GetExitCode();
									if (editorExitCode != XC_OPEN_ERROR && editorExitCode != XC_LOADING_INTERRUPTED)
									{
										Global->WindowManager->ExecuteModal(ShellEditor);
										// заставляем рефрешится экран
										Global->WindowManager->ProcessKey(Manager::Key(KEY_CONSOLE_BUFFER_RESIZE));
									}
								}
							}
							Dlg->SendMessage(DM_ENABLEREDRAW,TRUE,0);
							Dlg->SendMessage(DM_SHOWDIALOG,TRUE,0);
						}
						Console().SetTitle(strOldTitle);
					}
					if (RemoveTemp)
					{
						DeleteFileWithFolder(strSearchFileName);
					}
					return TRUE;
				}
				break;
			default:
				break;
			}
		}
		break;

	case DN_BTNCLICK:
		{
			FindPositionChanged = true;
			switch (Param1)
			{
			case FD_BUTTON_NEW:
				{
					StopEvent.Set();
					return FALSE;
				}
				break;

			case FD_BUTTON_STOP:
				{
					if(!StopEvent.Signaled())
					{
						StopEvent.Set();
						return TRUE;
					}
					else
					{
						return FALSE;
					}
				}
				break;

			case FD_BUTTON_VIEW:
				{
					INPUT_RECORD key;
					KeyToInputRecord(KEY_F3,&key);
					FindDlgProc(Dlg,DN_CONTROLINPUT,FD_LISTBOX,&key);
					return TRUE;
				}
				break;

			case FD_BUTTON_GOTO:
			case FD_BUTTON_PANEL:
				{
					// Переход и посыл на панель будем делать не в диалоге, а после окончания поиска.
					// Иначе возможна ситуация, когда мы ищем на панели, потом ее грохаем и создаем новую
					// (а поиск-то идет!) и в результате ФАР трапается.
					if(!ListBox->GetItemCount())
					{
						return TRUE;
					}
					FindExitItem = *reinterpret_cast<FindListItem**>(ListBox->GetUserData(nullptr, 0));
					TB.reset();
					return FALSE;
				}
				break;
			default:
				break;
			}
		}
		break;

	case DN_CLOSE:
		{
			BOOL Result = TRUE;
			if (Param1==FD_LISTBOX)
			{
				if(ListBox->GetItemCount())
				{
					FindDlgProc(Dlg,DN_BTNCLICK,FD_BUTTON_GOTO,0); // emulates a [ Go to ] button pressing;
				}
				else
				{
					Result = FALSE;
				}
			}
			if(Result)
			{
				StopEvent.Set();
			}
			return Result;
		}
		break;

	case DN_RESIZECONSOLE:
		{
			auto pCoord = static_cast<PCOORD>(Param2);
			SMALL_RECT DlgRect;
			Dlg->SendMessage( DM_GETDLGRECT, 0, &DlgRect);
			int DlgWidth=DlgRect.Right-DlgRect.Left+1;
			int DlgHeight=DlgRect.Bottom-DlgRect.Top+1;
			int IncX = pCoord->X - DlgWidth - 2;
			int IncY = pCoord->Y - DlgHeight - 2;
			Dlg->SendMessage( DM_ENABLEREDRAW, FALSE, 0);

			for (int i = 0; i <= FD_BUTTON_STOP; i++)
			{
				Dlg->SendMessage(DM_SHOWITEM, i, ToPtr(FALSE));
			}

			if ((IncX > 0) || (IncY > 0))
			{
				pCoord->X = DlgWidth + (IncX > 0 ? IncX : 0);
				pCoord->Y = DlgHeight + (IncY > 0 ? IncY : 0);
				Dlg->SendMessage( DM_RESIZEDIALOG, 0, pCoord);
			}

			DlgWidth += IncX;
			DlgHeight += IncY;

			for (int i = 0; i < FD_SEPARATOR1; i++)
			{
				SMALL_RECT rect;
				Dlg->SendMessage( DM_GETITEMPOSITION, i, &rect);
				rect.Right += IncX;
				rect.Bottom += IncY;
				Dlg->SendMessage( DM_SETITEMPOSITION, i, &rect);
			}

			for (int i = FD_SEPARATOR1; i <= FD_BUTTON_STOP; i++)
			{
				SMALL_RECT rect;
				Dlg->SendMessage( DM_GETITEMPOSITION, i, &rect);

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
				rect.Bottom += IncY;
				Dlg->SendMessage( DM_SETITEMPOSITION, i, &rect);
			}

			if ((IncX <= 0) || (IncY <= 0))
			{
				pCoord->X = DlgWidth;
				pCoord->Y = DlgHeight;
				Dlg->SendMessage( DM_RESIZEDIALOG, 0, pCoord);
			}

			for (int i = 0; i <= FD_BUTTON_STOP; i++)
			{
				Dlg->SendMessage( DM_SHOWITEM, i, ToPtr(TRUE));
			}

			Dlg->SendMessage( DM_ENABLEREDRAW, TRUE, 0);
			return TRUE;
		}
		break;
	default:
		break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

void FindFiles::AddMenuRecord(Dialog* Dlg,const string& FullName, const os::FAR_FIND_DATA& FindData, void* Data, FARPANELITEMFREECALLBACK FreeData)
{
	if (!Dlg)
		return;

	auto& ListBox = Dlg->GetAllItem()[FD_LISTBOX].ListPtr;

	if(!ListBox->GetItemCount())
	{
		Dlg->SendMessage( DM_ENABLE, FD_BUTTON_GOTO, ToPtr(TRUE));
		Dlg->SendMessage( DM_ENABLE, FD_BUTTON_VIEW, ToPtr(TRUE));
		if(AnySetFindList)
		{
			Dlg->SendMessage( DM_ENABLE, FD_BUTTON_PANEL, ToPtr(TRUE));
		}
		Dlg->SendMessage( DM_ENABLE, FD_LISTBOX, ToPtr(TRUE));
	}

	FormatString MenuText;

	const wchar_t *DisplayName=FindData.strFileName.data();

	MenuText << L' ';

	FOR(auto& i, Global->Opt->FindOpt.OutColumns)
	{
		int CurColumnType = static_cast<int>(i.type & 0xFF);
		int Width = i.width;
		if (!Width)
		{
			Width = GetDefaultWidth(i.type);
		}

		switch (CurColumnType)
		{
			case DIZ_COLUMN:
			case OWNER_COLUMN:
			{
				// пропускаем, не реализовано
				break;
			}
			case NAME_COLUMN:
			{
				// даже если указали, пропускаем, т.к. поле имени обязательное и идет в конце.
				break;
			}

			case ATTR_COLUMN:
			{
				MenuText << FormatStr_Attribute(FindData.dwFileAttributes, Width) << BoxSymbols[BS_V1];
				break;
			}
			case NUMSTREAMS_COLUMN:
			case STREAMSSIZE_COLUMN:
			case SIZE_COLUMN:
			case PACKED_COLUMN:
			case NUMLINK_COLUMN:
			{
				UINT64 StreamsSize=0;
				DWORD StreamsCount=0;

				if (itd->GetFindFileArcItem())
				{
					if (CurColumnType == NUMSTREAMS_COLUMN || CurColumnType == STREAMSSIZE_COLUMN)
						EnumStreams(FindData.strFileName,StreamsSize,StreamsCount);
					else if(CurColumnType == NUMLINK_COLUMN)
						StreamsCount=GetNumberOfLinks(FindData.strFileName);
				}

				MenuText << FormatStr_Size(
								FindData.nFileSize,
								FindData.nAllocationSize,
								(CurColumnType == NUMSTREAMS_COLUMN || CurColumnType == NUMLINK_COLUMN)?StreamsCount:StreamsSize,
								DisplayName,
								FindData.dwFileAttributes,
								0,
								FindData.dwReserved0,
								(CurColumnType == NUMSTREAMS_COLUMN || CurColumnType == NUMLINK_COLUMN)?STREAMSSIZE_COLUMN:CurColumnType,
								i.type,
								Width);

				MenuText << BoxSymbols[BS_V1];
				break;
			}

			case DATE_COLUMN:
			case TIME_COLUMN:
			case WDATE_COLUMN:
			case ADATE_COLUMN:
			case CDATE_COLUMN:
			case CHDATE_COLUMN:
			{
				const FILETIME *FileTime;
				switch (CurColumnType)
				{
					case CDATE_COLUMN:
						FileTime=&FindData.ftCreationTime;
						break;
					case ADATE_COLUMN:
						FileTime=&FindData.ftLastAccessTime;
						break;
					case CHDATE_COLUMN:
						FileTime=&FindData.ftChangeTime;
						break;
					case DATE_COLUMN:
					case TIME_COLUMN:
					case WDATE_COLUMN:
					default:
						FileTime=&FindData.ftLastWriteTime;
						break;
				}

				MenuText << FormatStr_DateTime(FileTime, CurColumnType, i.type, Width) << BoxSymbols[BS_V1];
				break;
			}
		}
	}


	// В плагинах принудительно поставим указатель в имени на имя
	// для корректного его отображения в списке, отбросив путь,
	// т.к. некоторые плагины возвращают имя вместе с полным путём,
	// к примеру временная панель.

	const wchar_t *DisplayName0=DisplayName;
	if (itd->GetFindFileArcItem())
		DisplayName0 = PointToName(DisplayName0);
	MenuText << DisplayName0;

	string strPathName=FullName;
	{
		size_t pos;

		if (FindLastSlash(pos,strPathName))
			strPathName.resize(pos);
		else
			strPathName.clear();
	}
	AddEndSlash(strPathName);

	if (StrCmpI(strPathName, strLastDirName))
	{
		if (ListBox->GetItemCount())
		{
			MenuItemEx ListItem;
			ListItem.Flags|=LIF_SEPARATOR;
			ListBox->AddItem(ListItem);
		}

		strLastDirName = strPathName;

		if (itd->GetFindFileArcItem())
		{
			ArcListItem* ArcItem = itd->GetFindFileArcItem();
			if(!(ArcItem->Flags & OPIF_REALNAMES) && !ArcItem->strArcName.empty())
			{
				string strArcPathName=ArcItem->strArcName;
				strArcPathName+=L":";

				if (!IsSlash(strPathName.front()))
					AddEndSlash(strArcPathName);

				strArcPathName += strPathName == L".\\"? L"\\" : strPathName.data();
				strPathName = strArcPathName;
			}
		}
		FindListItem& FindItem = itd->AddFindListItem(FindData,Data,nullptr);
		// Сбросим данные в FindData. Они там от файла
		FindItem.FindData.Clear();
		// Используем LastDirName, т.к. PathName уже может быть искажена
		FindItem.FindData.strFileName = strLastDirName;
		// Used=0 - Имя не попадёт во временную панель.
		FindItem.Used=0;
		// Поставим атрибут у каталога, чтобы он не был файлом :)
		FindItem.FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		FindItem.Arc = itd->GetFindFileArcItem();;

		auto Ptr = &FindItem;
		MenuItemEx ListItem(strPathName);
		ListBox->SetUserData(&Ptr,sizeof(Ptr),ListBox->AddItem(ListItem));
	}

	FindListItem& FindItem = itd->AddFindListItem(FindData,Data,FreeData);
	FindItem.FindData.strFileName = FullName;
	FindItem.Used=1;
	FindItem.Arc = itd->GetFindFileArcItem();

	MenuItemEx ListItem(MenuText);
	int ListPos = ListBox->AddItem(ListItem);
	auto Ptr = &FindItem;
	ListBox->SetUserData(&Ptr, sizeof(Ptr), ListPos);

	// Выделим как положено - в списке.
	int FC=itd->GetFileCount(), DC=itd->GetDirCount(), LF=itd->GetLastFoundNumber();
	if (!FC && !DC)
	{
		ListBox->SetSelectPos(ListPos, -1);
	}

	if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		DC++;
	}
	else
	{
		FC++;
	}

	LF++;

	itd->SetFileCount(FC);
	itd->SetDirCount(DC);
	itd->SetLastFoundNumber(LF);
}

void FindFiles::AddMenuRecord(Dialog* Dlg,const string& FullName, PluginPanelItem& FindData)
{
	os::FAR_FIND_DATA fdata;
	PluginPanelItemToFindDataEx(&FindData, &fdata);
	AddMenuRecord(Dlg,FullName, fdata, FindData.UserData.Data, FindData.UserData.FreeData);
	FindData.UserData.FreeData = nullptr; //передано в FINDLIST
}

void FindFiles::ArchiveSearch(Dialog* Dlg, const string& ArcName)
{
	_ALGO(CleverSysLog clv(L"FindFiles::ArchiveSearch()"));
	_ALGO(SysLog(L"ArcName='%s'",(ArcName?ArcName:L"nullptr")));

	PluginHandle* hArc;
	{
		int SavePluginsOutput = Global->DisablePluginsOutput;
		Global->DisablePluginsOutput = TRUE;
		string strArcName = ArcName;
		{
			SCOPED_ACTION(CriticalSectionLock)(PluginCS);
			hArc = Global->CtrlObject->Plugins->OpenFilePlugin(&strArcName, OPM_FIND, OFP_SEARCH);
		}
		Global->DisablePluginsOutput = SavePluginsOutput;
	}
	if (hArc==PANEL_STOP)
	{
		//StopEvent.Set(); // ??
		_ALGO(SysLog(L"return: hArc==(HANDLE)-2"));
		return;
	}

	if (!hArc)
	{
		_ALGO(SysLog(L"return: hArc==nullptr"));
		return;
	}

	FINDAREA SaveSearchMode = SearchMode;
	ArcListItem* SaveArcItem = itd->GetFindFileArcItem();
	{
		int SavePluginsOutput=Global->DisablePluginsOutput;
		Global->DisablePluginsOutput=TRUE;

		SearchMode=FINDAREA_FROM_CURRENT;
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hArc,&Info);
		itd->SetFindFileArcItem(&itd->AddArcListItem(ArcName, hArc, Info.Flags, NullToEmpty(Info.CurDir)));
		// Запомним каталог перед поиском в архиве. И если ничего не нашли - не рисуем его снова.
		{
			string strSaveDirName, strSaveSearchPath;
			size_t SaveListCount = itd->GetFindListCount();
			// Запомним пути поиска в плагине, они могут измениться.
			strSaveSearchPath = strPluginSearchPath;
			strSaveDirName = strLastDirName;
			strLastDirName.clear();
			DoPreparePluginList(Dlg,true);
			strPluginSearchPath = strSaveSearchPath;
			ArcListItem* ArcItem = itd->GetFindFileArcItem();
			{
				SCOPED_ACTION(CriticalSectionLock)(PluginCS);
				Global->CtrlObject->Plugins->ClosePanel(ArcItem->hPlugin);
			}
			ArcItem->hPlugin = nullptr;

			if (SaveListCount == itd->GetFindListCount())
				strLastDirName = strSaveDirName;
		}

		Global->DisablePluginsOutput=SavePluginsOutput;
	}
	itd->SetFindFileArcItem(SaveArcItem);
	SearchMode=SaveSearchMode;
}

void FindFiles::DoScanTree(Dialog* Dlg, const string& strRoot)
{
	ScanTree ScTree(
		false,
		!(SearchMode==FINDAREA_CURRENT_ONLY||SearchMode==FINDAREA_INPATH),
		Global->Opt->FindOpt.FindSymLinks
	);
	string strSelName;
	DWORD FileAttr;

	if (SearchMode==FINDAREA_SELECTED)
		Global->CtrlObject->Cp()->ActivePanel()->GetSelName(nullptr,FileAttr);

	while (!StopEvent.Signaled())
	{
		string strCurRoot;

		if (SearchMode==FINDAREA_SELECTED)
		{
			if (!Global->CtrlObject->Cp()->ActivePanel()->GetSelName(&strSelName,FileAttr))
				break;

			if (!(FileAttr & FILE_ATTRIBUTE_DIRECTORY) || TestParentFolderName(strSelName) || strSelName == L".")
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
		itd->SetFindMessage(strCurRoot);
		os::FAR_FIND_DATA FindData;
		string strFullName;

		while (!StopEvent.Signaled() && ScTree.GetNextName(&FindData,strFullName))
		{
			HANDLE hFindStream = INVALID_HANDLE_VALUE;
			SCOPE_EXIT{ if (hFindStream != INVALID_HANDLE_VALUE) os::FindStreamClose(hFindStream); };

			Sleep(0);
			PauseEvent.Wait();

			bool bContinue=false;
			WIN32_FIND_STREAM_DATA sd;
			bool FirstCall=true;
			string strFindDataFileName=FindData.strFileName;

			if (Global->Opt->FindOpt.FindAlternateStreams)
			{
				hFindStream=os::FindFirstStream(strFullName,FindStreamInfoStandard,&sd);
			}

			// process default streams first
			bool ProcessAlternateStreams = false;
			while (!StopEvent.Signaled())
			{
				string strFullStreamName=strFullName;

				if (ProcessAlternateStreams)
				{
					if (hFindStream != INVALID_HANDLE_VALUE)
					{
						if (!FirstCall)
						{
							if (!os::FindNextStream(hFindStream,&sd))
							{
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
							FindData.dwFileAttributes &= ~FILE_ATTRIBUTE_DIRECTORY;
						}
						else
						{
							// default stream is already processed
							continue;
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

					if (!Filter->FileInFilter(FindData, &foundType, &strFullName))
					{
						// сюда заходим, если не попали в фильтр или попали в Exclude-фильтр
						if ((FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && foundType==FIFT_EXCLUDE)
							ScTree.SkipDir(); // скипаем только по Exclude-фильтру, т.к. глубже тоже нужно просмотреть

						{
							bContinue=true;

							if (ProcessAlternateStreams)
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

				if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					itd->SetFindMessage(strFullName);
				}

				if (IsFileIncluded(nullptr,strFullStreamName,FindData.dwFileAttributes,strFullName))
				{
					AddMenuRecord(Dlg,strFullStreamName, FindData, nullptr, nullptr);
				}

				ProcessAlternateStreams = Global->Opt->FindOpt.FindAlternateStreams;
				if (!Global->Opt->FindOpt.FindAlternateStreams || hFindStream == INVALID_HANDLE_VALUE)
				{
					break;
				}
			}

			if (bContinue)
			{
				continue;
			}

			if (SearchInArchives)
				ArchiveSearch(Dlg,strFullName);
		}

		if (SearchMode!=FINDAREA_SELECTED)
			break;
	}
}

void FindFiles::ScanPluginTree(Dialog* Dlg, PluginHandle* hPlugin, UINT64 Flags, int& RecurseLevel)
{
	PluginPanelItem *PanelData=nullptr;
	size_t ItemCount=0;
	bool GetFindDataResult=false;
	{
		SCOPED_ACTION(CriticalSectionLock)(PluginCS);
		{
			if(!StopEvent.Signaled())
			{
				GetFindDataResult=Global->CtrlObject->Plugins->GetFindData(hPlugin,&PanelData,&ItemCount,OPM_FIND)!=FALSE;
			}
		}
	}
	if (!GetFindDataResult)
	{
		return;
	}

	RecurseLevel++;

	if (SearchMode!=FINDAREA_SELECTED || RecurseLevel!=1)
	{
		for (size_t I=0; I<ItemCount && !StopEvent.Signaled(); I++)
		{
			Sleep(0);
			PauseEvent.Wait();

			PluginPanelItem *CurPanelItem=PanelData+I;
			string strCurName=NullToEmpty(CurPanelItem->FileName);
			if (strCurName.empty() || strCurName == L"." || TestParentFolderName(strCurName))
				continue;

			string strFullName = strPluginSearchPath;
			strFullName += strCurName;

			if (!UseFilter || Filter->FileInFilter(*CurPanelItem))
			{
				if (CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					itd->SetFindMessage(strFullName);
				}

				if (IsFileIncluded(CurPanelItem,strCurName,CurPanelItem->FileAttributes,strFullName))
					AddMenuRecord(Dlg,strFullName, *CurPanelItem);

				if (SearchInArchives && hPlugin && (Flags & OPIF_REALNAMES))
					ArchiveSearch(Dlg,strFullName);
			}
		}
	}

	if (SearchMode!=FINDAREA_CURRENT_ONLY)
	{
		for (size_t I=0; I<ItemCount && !StopEvent.Signaled(); I++)
		{
			PluginPanelItem *CurPanelItem=PanelData+I;

			if ((CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			 && StrCmp(CurPanelItem->FileName, L".") && !TestParentFolderName(CurPanelItem->FileName)
			 && (!UseFilter || Filter->FileInFilter(*CurPanelItem))
			 && (SearchMode!=FINDAREA_SELECTED || RecurseLevel!=1 || Global->CtrlObject->Cp()->ActivePanel()->IsSelected(CurPanelItem->FileName))
			) {
				bool SetDirectoryResult=false;
				{
					SCOPED_ACTION(CriticalSectionLock)(PluginCS);
					SetDirectoryResult=Global->CtrlObject->Plugins->SetDirectory(hPlugin, CurPanelItem->FileName, OPM_FIND, &CurPanelItem->UserData)!=FALSE;
				}
				if (SetDirectoryResult)
				{
					strPluginSearchPath += CurPanelItem->FileName;
					strPluginSearchPath += L"\\";
					ScanPluginTree(Dlg, hPlugin, Flags, RecurseLevel);

					size_t pos = strPluginSearchPath.rfind(L'\\');
					if (pos != string::npos)
						strPluginSearchPath.resize(pos);

					if ((pos = strPluginSearchPath.rfind(L'\\')) != string::npos)
						strPluginSearchPath.resize(pos+1);
					else
						strPluginSearchPath.clear();

					SetDirectoryResult=false;
					{
						SCOPED_ACTION(CriticalSectionLock)(PluginCS);
						SetDirectoryResult=Global->CtrlObject->Plugins->SetDirectory(hPlugin,L"..",OPM_FIND)!=FALSE;
					}
					if (!SetDirectoryResult)
					{
						StopEvent.Set();
					}
				}
			}
		}
	}

	Global->CtrlObject->Plugins->FreeFindData(hPlugin,PanelData,ItemCount,true);
	RecurseLevel--;
}

void FindFiles::DoPrepareFileList(Dialog* Dlg)
{
	auto strRoot = Global->CtrlObject->CmdLine()->GetCurDir();
	if (strRoot.find_first_of(L";,") != string::npos)
		InsertQuote(strRoot);

	string InitString;

	if (SearchMode==FINDAREA_INPATH)
	{
		string strPathEnv;
		os::env::get_variable(L"PATH", strPathEnv);
		InitString = strPathEnv;
	}
	else if (SearchMode==FINDAREA_ROOT)
	{
		GetPathRoot(strRoot,strRoot);
		InitString = strRoot;
	}
	else if (SearchMode==FINDAREA_ALL || SearchMode==FINDAREA_ALL_BUTNETWORK)
	{
		std::list<string> Volumes;
		string strGuidVolime;
		auto Strings = os::GetLogicalDriveStrings();
		std::for_each(CONST_RANGE(Strings, i)
		{
			int DriveType=FAR_GetDriveType(i);

			if (DriveType != DRIVE_REMOVABLE && !IsDriveTypeCDROM(DriveType) && (DriveType != DRIVE_REMOTE || SearchMode != FINDAREA_ALL_BUTNETWORK))
			{
				if(os::GetVolumeNameForVolumeMountPoint(i, strGuidVolime))
				{
					Volumes.emplace_back(strGuidVolime);
				}
				InitString += i + L";";
			}
		});

		WCHAR VolumeName[50];

		bool End = false;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		for(hFind = FindFirstVolume(VolumeName, ARRAYSIZE(VolumeName)); hFind != INVALID_HANDLE_VALUE && !End; End = !FindNextVolume(hFind, VolumeName, ARRAYSIZE(VolumeName)))
		{
			int DriveType=FAR_GetDriveType(VolumeName);

			if (DriveType==DRIVE_REMOVABLE || IsDriveTypeCDROM(DriveType) || (DriveType==DRIVE_REMOTE && SearchMode==FINDAREA_ALL_BUTNETWORK))
			{
				continue;
			}

			if (std::none_of(CONST_RANGE(Volumes, i) {return i.compare(0, wcslen(VolumeName), VolumeName) == 0;}))
			{
				InitString.append(VolumeName).append(L";");
			}
		}
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindVolumeClose(hFind);
		}
	}
	else
	{
		InitString = strRoot;
	}

	std::vector<string> Strings;
	split(Strings, InitString, STLF_UNIQUE);
	FOR(const auto& i, Strings)
	{
		DoScanTree(Dlg, i);
	}

	itd->SetPercent(0);
	StopEvent.Set();
}

void FindFiles::DoPreparePluginList(Dialog* Dlg, bool Internal)
{
	ArcListItem* ArcItem = itd->GetFindFileArcItem();
	OpenPanelInfo Info;
	string strSaveDir;
	{
		SCOPED_ACTION(CriticalSectionLock)(PluginCS);
		Global->CtrlObject->Plugins->GetOpenPanelInfo(ArcItem->hPlugin,&Info);
		strSaveDir = NullToEmpty(Info.CurDir);
		if (SearchMode==FINDAREA_ROOT || SearchMode==FINDAREA_ALL || SearchMode==FINDAREA_ALL_BUTNETWORK || SearchMode==FINDAREA_INPATH)
		{
			Global->CtrlObject->Plugins->SetDirectory(ArcItem->hPlugin,L"\\",OPM_FIND);
			Global->CtrlObject->Plugins->GetOpenPanelInfo(ArcItem->hPlugin,&Info);
		}
	}

	strPluginSearchPath = NullToEmpty(Info.CurDir);

	if (!strPluginSearchPath.empty())
		AddEndSlash(strPluginSearchPath);

	int RecurseLevel=0;
	ScanPluginTree(Dlg,ArcItem->hPlugin,ArcItem->Flags, RecurseLevel);

	if (SearchMode==FINDAREA_ROOT || SearchMode==FINDAREA_ALL || SearchMode==FINDAREA_ALL_BUTNETWORK || SearchMode==FINDAREA_INPATH)
	{
		SCOPED_ACTION(CriticalSectionLock)(PluginCS);
		Global->CtrlObject->Plugins->SetDirectory(ArcItem->hPlugin,strSaveDir,OPM_FIND,&Info.UserData);
	}

	if (!Internal)
	{
		itd->SetPercent(0);
		StopEvent.Set();
	}
}

struct THREADPARAM
{
	bool PluginMode;
	Dialog* Dlg;
};

unsigned int FindFiles::ThreadRoutine(THREADPARAM* Param)
{
	try
	{
		InitInFileSearch();
		Param->PluginMode? DoPreparePluginList(Param->Dlg, false) : DoPrepareFileList(Param->Dlg);
		ReleaseInFileSearch();
	}
	catch (SException& e)
	{
		if (ProcessSEHException(L"FindFiles::ThreadRoutine", e.GetInfo()))
		{
			std::terminate();
		}
		else
		{
			throw;
		}
	}
	return 0;
}

bool FindFiles::FindFilesProcess()
{
	_ALGO(CleverSysLog clv(L"FindFiles::FindFilesProcess()"));
	// Если используется фильтр операций, то во время поиска сообщаем об этом
	string strTitle=MSG(MFindFileTitle);

	itd->Init();

	if (!strFindMask.empty())
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

	LangString strSearchStr(MFindSearchingIn);

	if (!strFindStr.empty())
	{
		string strFStr=strFindStr;
		strSearchStr << L" " + InsertQuote(TruncStrFromEnd(strFStr,10));
	}
	else
	{
		strSearchStr << L"";
	}

	int DlgWidth = ScrX + 1 - 2;
	int DlgHeight = ScrY + 1 - 2;
	FarDialogItem FindDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,DlgWidth-4,DlgHeight-2,0,nullptr,nullptr,DIF_SHOWAMPERSAND,strTitle.data()},
		{DI_LISTBOX,4,2,DlgWidth-5,DlgHeight-7,0,nullptr,nullptr,DIF_LISTNOBOX|DIF_DISABLE,0},
		{DI_TEXT,-1,DlgHeight-6,0,DlgHeight-6,0,nullptr,nullptr,DIF_SEPARATOR2,L""},
		{DI_TEXT,5,DlgHeight-5,DlgWidth-(strFindStr.empty()?6:12),DlgHeight-5,0,nullptr,nullptr,DIF_SHOWAMPERSAND,strSearchStr.data()},
		{DI_TEXT,DlgWidth-9,DlgHeight-5,DlgWidth-6,DlgHeight-5,0,nullptr,nullptr,(strFindStr.empty()?DIF_HIDDEN:0),L""},
		{DI_TEXT,-1,DlgHeight-4,0,DlgHeight-4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,DlgHeight-3,0,DlgHeight-3,0,nullptr,nullptr,DIF_FOCUS|DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MFindNewSearch)},
		{DI_BUTTON,0,DlgHeight-3,0,DlgHeight-3,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_DISABLE,MSG(MFindGoTo)},
		{DI_BUTTON,0,DlgHeight-3,0,DlgHeight-3,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_DISABLE,MSG(MFindView)},
		{DI_BUTTON,0,DlgHeight-3,0,DlgHeight-3,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_DISABLE,MSG(MFindPanel)},
		{DI_BUTTON,0,DlgHeight-3,0,DlgHeight-3,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MFindStop)},
	};
	auto FindDlg = MakeDialogItemsEx(FindDlgData);
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);

	if (PluginMode)
	{
		Panel *ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
		auto hPlugin=ActivePanel->GetPluginHandle();
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		itd->SetFindFileArcItem(&itd->AddArcListItem(NullToEmpty(Info.HostFile), hPlugin, Info.Flags, NullToEmpty(Info.CurDir)));

		if (!itd->GetFindFileArcItem())
			return false;

		if (!(Info.Flags & OPIF_REALNAMES))
		{
			FindDlg[FD_BUTTON_PANEL].Type=DI_TEXT;
			FindDlg[FD_BUTTON_PANEL].strData.clear();
		}
	}

	AnySetFindList = std::any_of(CONST_RANGE(*Global->CtrlObject->Plugins, i)
	{
		return i->HasSetFindList();
	});

	if (!AnySetFindList)
	{
		FindDlg[FD_BUTTON_PANEL].Flags|=DIF_DISABLE;
	}

	auto Dlg = Dialog::create(FindDlg, &FindFiles::FindDlgProc, this);
//  pDlg->SetDynamicallyBorn();
	Dlg->SetHelp(L"FindFileResult");
	Dlg->SetPosition(-1, -1, DlgWidth, DlgHeight);
	Dlg->SetId(FindFileResultId);
	// Надо бы показать диалог, а то инициализация элементов запаздывает
	// иногда при поиске и первые элементы не добавляются
	Dlg->InitDialog();
	Dlg->Show();

	strLastDirName.clear();

	THREADPARAM Param={PluginMode, Dlg.get()};

		TB = std::make_unique<IndeterminateTaskBar>();
		SCOPED_ACTION(wakeful);

		{
			Thread FindThread(&Thread::join, &FindFiles::ThreadRoutine, this, &Param);
			Dlg->Process();
		}

		PauseEvent.Set();
		StopEvent.Reset();

		switch (Dlg->GetExitCode())
		{
			case FD_BUTTON_NEW:
			{
				return true;
			}

			case FD_BUTTON_PANEL:
			// Отработаем переброску на временную панель
			{
				itd->Lock();
				size_t ListSize = itd->GetFindList().size();
				std::vector<PluginPanelItem> PanelItems;
				PanelItems.reserve(ListSize);

				std::for_each(RANGE(itd->GetFindList(), i)
				{
					if (!i.FindData.strFileName.empty() && i.Used)
					// Добавляем всегда, если имя задано
					{
						// Для плагинов с виртуальными именами заменим имя файла на имя архива.
						// панель сама уберет лишние дубли.
						bool IsArchive=false;
						if(i.Arc)
						{
							if(!(i.Arc->Flags&OPIF_REALNAMES))
							{
								IsArchive=true;
							}
						}
						// Добавляем только файлы или имена архивов или папки когда просили
						if (IsArchive || (Global->Opt->FindOpt.FindFolders && !SearchHex) ||
							    !(i.FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
						{
							if (IsArchive)
							{
								i.FindData.strFileName = i.Arc->strArcName;
							}
							PanelItems.emplace_back(VALUE_TYPE(PanelItems)());
							PluginPanelItem& pi = PanelItems.back();
							FindDataExToPluginPanelItem(&i.FindData, &pi);

							if (IsArchive)
								pi.FileAttributes = 0;

							if (pi.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							{
								DeleteEndSlash(const_cast<wchar_t*>(pi.FileName));
							}
						}
					}
				});
				itd->Unlock();
				auto hNewPlugin=Global->CtrlObject->Plugins->OpenFindListPlugin(PanelItems.data(), PanelItems.size());

				if (hNewPlugin)
				{
					Panel *ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
					Panel *NewPanel=Global->CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
					NewPanel->SetPluginMode(hNewPlugin,L"",true);
					NewPanel->SetVisible(true);
					NewPanel->Update(0);
					//if (FindExitItem)
					//NewPanel->GoToFile(FindExitItem->FindData.cFileName);
					NewPanel->Show();
				}

				std::for_each(ALL_RANGE(PanelItems), FreePluginPanelItem);
				break;
			}
			case FD_BUTTON_GOTO:
			case FD_LISTBOX:
			{
				string strFileName=FindExitItem->FindData.strFileName;
				Panel *FindPanel = Global->CtrlObject->Cp()->ActivePanel();

				if (FindExitItem->Arc)
				{
					if (!FindExitItem->Arc->hPlugin)
					{
						string strArcName = FindExitItem->Arc->strArcName;

						if (FindPanel->GetType()!=FILE_PANEL)
						{
							FindPanel=Global->CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);
						}

						string strArcPath=strArcName;
						CutToSlash(strArcPath);
						FindPanel->SetCurDir(strArcPath,true);
						FindExitItem->Arc->hPlugin=((FileList *)FindPanel)->OpenFilePlugin(&strArcName, FALSE, OFP_SEARCH);
						if (FindExitItem->Arc->hPlugin==PANEL_STOP)
							FindExitItem->Arc->hPlugin = nullptr;
					}

					if (FindExitItem->Arc->hPlugin)
					{
						OpenPanelInfo Info;
						Global->CtrlObject->Plugins->GetOpenPanelInfo(FindExitItem->Arc->hPlugin,&Info);

						if (SearchMode==FINDAREA_ROOT ||
							    SearchMode==FINDAREA_ALL ||
							    SearchMode==FINDAREA_ALL_BUTNETWORK ||
							    SearchMode==FINDAREA_INPATH)
							Global->CtrlObject->Plugins->SetDirectory(FindExitItem->Arc->hPlugin,L"\\",0);

						SetPluginDirectory(strFileName, FindExitItem->Arc->hPlugin, true); // ??? ,FindItem.Data ???
					}
				}
				else
				{
					string strSetName;
					size_t Length=strFileName.size();

					if (!Length)
						break;

					if (Length>1 && IsSlash(strFileName[Length-1]) && strFileName[Length-2] != L':')
						strFileName.pop_back();

					if (!os::fs::exists(strFileName) && (GetLastError() != ERROR_ACCESS_DENIED))
						break;

					const wchar_t *NamePtr = PointToName(strFileName);
					strSetName = NamePtr;

					if (Global->Opt->FindOpt.FindAlternateStreams)
					{
						size_t Pos = strSetName.find(L':');

						if (Pos != string::npos)
							strSetName.resize(Pos);
					}

					strFileName.resize(NamePtr-strFileName.data());
					Length=strFileName.size();

					if (Length>1 && IsSlash(strFileName[Length-1]) && strFileName[Length-2] != L':')
						strFileName.pop_back();

					if (strFileName.empty())
						break;

					if (FindPanel->GetType()!=FILE_PANEL &&
						    Global->CtrlObject->Cp()->GetAnotherPanel(FindPanel)->GetType()==FILE_PANEL)
						FindPanel=Global->CtrlObject->Cp()->GetAnotherPanel(FindPanel);

					if ((FindPanel->GetType()!=FILE_PANEL) || (FindPanel->GetMode()!=NORMAL_PANEL))
					// Сменим панель на обычную файловую...
					{
						FindPanel=Global->CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);
						FindPanel->SetVisible(true);
						FindPanel->Update(0);
					}

					// ! Не меняем каталог, если мы уже в нем находимся.
					// Тем самым добиваемся того, что выделение с элементов панели не сбрасывается.
					string strDirTmp = FindPanel->GetCurDir();
					Length=strDirTmp.size();

					if (Length>1 && IsSlash(strDirTmp[Length-1]) && strDirTmp[Length-2] != L':')
						strDirTmp.pop_back();

					if (StrCmpI(strFileName, strDirTmp))
						FindPanel->SetCurDir(strFileName,true);

					if (!strSetName.empty())
						FindPanel->GoToFile(strSetName);

					FindPanel->Show();
					FindPanel->SetFocus();
				}
				break;
			}
		}
	return false;
}

FindFiles::FindFiles():
	AnySetFindList(),
	CmpCase(),
	WholeWords(),
	SearchInArchives(),
	SearchHex(),
	NotContaining(),
	UseFilter(),
	m_Autodetection(),
	InFileSearchInited(),
	FindFoldersChanged(),
	SearchFromChanged(),
	FindPositionChanged(),
	Finalized(),
	PluginMode(),
	SearchMode(FINDAREA_ALL),
	favoriteCodePages(),
	readBufferA(),
	readBuffer(),
	findString(),
	CodePage(CP_DEFAULT),
	skipCharsTable(),
	SearchInFirst(),
	FindExitItem(),
	FileMaskForFindFile(std::make_unique<filemasks>()),
	Filter(std::make_unique<FileFilter>(Global->CtrlObject->Cp()->ActivePanel(), FFT_FINDFILE)),
	itd(std::make_unique<InterThreadData>()),
	PauseEvent(Event::manual, Event::signaled),
	StopEvent(Event::manual, Event::nonsignaled),
	m_TimeCheck(time_check::immediate, GetRedrawTimeout())
{
	_ALGO(CleverSysLog clv(L"FindFiles::FindFiles()"));

	static string strLastFindMask=L"*.*", strLastFindStr;

	static string strSearchFromRoot;
	strSearchFromRoot = MSG(MSearchFromRootFolder);

	static bool LastCmpCase = 0, LastWholeWords = 0, LastSearchInArchives = 0, LastSearchHex = 0, LastNotContaining = 0;

	CmpCase=LastCmpCase;
	WholeWords=LastWholeWords;
	SearchInArchives=LastSearchInArchives;
	SearchHex=LastSearchHex;
	NotContaining = LastNotContaining;
	SearchMode = static_cast<FINDAREA>(Global->Opt->FindOpt.FileSearchMode.Get());
	UseFilter=Global->Opt->FindOpt.UseFilter.Get();
	strFindMask = strLastFindMask;
	strFindStr = strLastFindStr;

	do
	{
		FindExitItem = nullptr;
		FindFoldersChanged=false;
		SearchFromChanged=false;
		FindPositionChanged=false;
		Finalized=false;
		TB.reset();
		itd->ClearAllLists();
		Panel *ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
		PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();
		PrepareDriveNameStr(strSearchFromRoot);
		static const wchar_t MasksHistoryName[] = L"Masks", TextHistoryName[] = L"SearchText";
		static const wchar_t HexMask[]=L"HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH";
		const wchar_t VSeparator[] = { BoxSymbols[BS_T_H1V1], BoxSymbols[BS_V1], BoxSymbols[BS_V1], BoxSymbols[BS_V1], BoxSymbols[BS_V1], BoxSymbols[BS_B_H1V1], 0 };
		FarDialogItem FindAskDlgData[]=
		{
			{DI_DOUBLEBOX,3,1,76,19,0,nullptr,nullptr,0,MSG(MFindFileTitle)},
			{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(MFindFileMasks)},
			{DI_EDIT,5,3,74,3,0,MasksHistoryName,nullptr,DIF_FOCUS|DIF_HISTORY|DIF_USELASTHISTORY,L""},
			{DI_TEXT,-1,4,0,4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_TEXT,5,5,0,5,0,nullptr,nullptr,0,L""},
			{DI_EDIT,5,6,74,6,0,TextHistoryName,nullptr,DIF_HISTORY,L""},
			{DI_FIXEDIT,5,6,74,6,0,nullptr,HexMask,DIF_MASKEDIT,L""},
			{DI_TEXT,5,7,0,7,0,nullptr,nullptr,0,L""},
			{DI_COMBOBOX,5,8,74,8,0,nullptr,nullptr,DIF_DROPDOWNLIST,L""},
			{DI_TEXT,-1,9,0,9,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_CHECKBOX,5,10,0,10,0,nullptr,nullptr,0,MSG(MFindFileCase)},
			{DI_CHECKBOX,5,11,0,11,0,nullptr,nullptr,0,MSG(MFindFileWholeWords)},
			{DI_CHECKBOX,5,12,0,12,0,nullptr,nullptr,0,MSG(MSearchForHex)},
			{DI_CHECKBOX,5,13,0,13,NotContaining,nullptr,nullptr,0,MSG(MSearchNotContaining)},
			{DI_CHECKBOX,41,10,0,10,0,nullptr,nullptr,0,MSG(MFindArchives)},
			{DI_CHECKBOX,41,11,0,11,0,nullptr,nullptr,0,MSG(MFindFolders)},
			{DI_CHECKBOX,41,12,0,12,0,nullptr,nullptr,0,MSG(MFindSymLinks)},
			{DI_CHECKBOX,41,13,0,13,0,nullptr,nullptr,0,MSG(MFindAlternateStreams)},
			{DI_TEXT,-1,14,0,14,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_VTEXT,39,9,0,9,0,nullptr,nullptr,DIF_BOXCOLOR,VSeparator},
			{DI_TEXT,5,15,0,15,0,nullptr,nullptr,0,MSG(MSearchWhere)},
			{DI_COMBOBOX,5,16,36,16,0,nullptr,nullptr,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,L""},
			{DI_CHECKBOX,41,16,0,16,(int)(UseFilter?BSTATE_CHECKED:BSTATE_UNCHECKED),nullptr,nullptr,DIF_AUTOMATION,MSG(MFindUseFilter)},
			{DI_TEXT,-1,17,0,17,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MFindFileFind)},
			{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MFindFileDrive)},
			{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_AUTOMATION|(UseFilter?0:DIF_DISABLE),MSG(MFindFileSetFilter)},
			{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MFindFileAdvanced)},
			{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
		};
		auto FindAskDlg = MakeDialogItemsEx(FindAskDlgData);

		if (strFindStr.empty())
			FindAskDlg[FAD_CHECKBOX_DIRS].Selected=Global->Opt->FindOpt.FindFolders;

		FarListItem li[]=
		{
			{0,MSG(MSearchAllDisks)},
			{0,MSG(MSearchAllButNetwork)},
			{0,MSG(MSearchInPATH)},
			{0,strSearchFromRoot.data()},
			{0,MSG(MSearchFromCurrent)},
			{0,MSG(MSearchInCurrent)},
			{0,MSG(MSearchInSelected)},
		};
		li[FADC_ALLDISKS+SearchMode].Flags|=LIF_SELECTED;
		FarList l={sizeof(FarList),ARRAYSIZE(li),li};
		FindAskDlg[FAD_COMBOBOX_WHERE].ListItems=&l;

		if (PluginMode)
		{
			OpenPanelInfo Info;
			Global->CtrlObject->Plugins->GetOpenPanelInfo(ActivePanel->GetPluginHandle(),&Info);

			if (!(Info.Flags & OPIF_REALNAMES))
				FindAskDlg[FAD_CHECKBOX_ARC].Flags |= DIF_DISABLE;

			if (SearchMode == FINDAREA_ALL || SearchMode == FINDAREA_ALL_BUTNETWORK)
			{
				li[FADC_ALLDISKS].Flags=0;
				li[FADC_ALLBUTNET].Flags=0;
				li[FADC_ROOT].Flags|=LIF_SELECTED;
			}

			li[FADC_ALLDISKS].Flags|=LIF_GRAYED;
			li[FADC_ALLBUTNET].Flags|=LIF_GRAYED;
			FindAskDlg[FAD_CHECKBOX_LINKS].Selected=0;
			FindAskDlg[FAD_CHECKBOX_LINKS].Flags|=DIF_DISABLE;
			FindAskDlg[FAD_CHECKBOX_STREAMS].Selected = 0;
			FindAskDlg[FAD_CHECKBOX_STREAMS].Flags |= DIF_DISABLE;
		}
		else
		{
			FindAskDlg[FAD_CHECKBOX_LINKS].Selected = Global->Opt->FindOpt.FindSymLinks;
			FindAskDlg[FAD_CHECKBOX_STREAMS].Selected = Global->Opt->FindOpt.FindAlternateStreams;
		}
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
		int ExitCode;
		auto Dlg = Dialog::create(FindAskDlg, &FindFiles::MainDlgProc, this);
		Dlg->SetAutomation(FAD_CHECKBOX_FILTER,FAD_BUTTON_FILTER,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
		Dlg->SetHelp(L"FindFile");
		Dlg->SetId(FindFileId);
		Dlg->SetPosition(-1,-1,80,21);
		Dlg->Process();
		ExitCode=Dlg->GetExitCode();
		//Рефреш текущему времени для фильтра сразу после выхода из диалога
		Filter->UpdateCurrentTime();

		if (ExitCode!=FAD_BUTTON_FIND)
		{
			return;
		}

		Global->Opt->FindCodePage = CodePage;
		CmpCase=FindAskDlg[FAD_CHECKBOX_CASE].Selected == BSTATE_CHECKED;
		WholeWords=FindAskDlg[FAD_CHECKBOX_WHOLEWORDS].Selected == BSTATE_CHECKED;
		SearchHex=FindAskDlg[FAD_CHECKBOX_HEX].Selected == BSTATE_CHECKED;
		SearchInArchives=FindAskDlg[FAD_CHECKBOX_ARC].Selected == BSTATE_CHECKED;
		NotContaining = FindAskDlg[FAD_CHECKBOX_NOTCONTAINING].Selected == BSTATE_CHECKED;

		if (FindFoldersChanged)
		{
			Global->Opt->FindOpt.FindFolders=(FindAskDlg[FAD_CHECKBOX_DIRS].Selected==BSTATE_CHECKED);
		}

		if (!PluginMode)
		{
			Global->Opt->FindOpt.FindSymLinks=(FindAskDlg[FAD_CHECKBOX_LINKS].Selected==BSTATE_CHECKED);
			Global->Opt->FindOpt.FindAlternateStreams = (FindAskDlg[FAD_CHECKBOX_STREAMS].Selected == BSTATE_CHECKED);
		}

		UseFilter=(FindAskDlg[FAD_CHECKBOX_FILTER].Selected==BSTATE_CHECKED);
		Global->Opt->FindOpt.UseFilter=UseFilter;
		strFindMask = !FindAskDlg[FAD_EDIT_MASK].strData.empty() ? FindAskDlg[FAD_EDIT_MASK].strData:L"*";

		if (SearchHex)
		{
			strFindStr = FindAskDlg[FAD_EDIT_HEX].strData;
			RemoveTrailingSpaces(strFindStr);
		}
		else
			strFindStr = FindAskDlg[FAD_EDIT_TEXT].strData;

		if (!strFindStr.empty())
		{
			Global->StoreSearchString(strFindStr, SearchHex);
			Global->GlobalSearchCase=CmpCase;
			Global->GlobalSearchWholeWords=WholeWords;
		}

		switch (FindAskDlg[FAD_COMBOBOX_WHERE].ListPos)
		{
			case FADC_ALLDISKS:
				SearchMode=FINDAREA_ALL;
				break;
			case FADC_ALLBUTNET:
				SearchMode=FINDAREA_ALL_BUTNETWORK;
				break;
			case FADC_PATH:
				SearchMode=FINDAREA_INPATH;
				break;
			case FADC_ROOT:
				SearchMode=FINDAREA_ROOT;
				break;
			case FADC_FROMCURRENT:
				SearchMode=FINDAREA_FROM_CURRENT;
				break;
			case FADC_INCURRENT:
				SearchMode=FINDAREA_CURRENT_ONLY;
				break;
			case FADC_SELECTED:
				SearchMode=FINDAREA_SELECTED;
				break;
		}

		if (SearchFromChanged)
		{
			Global->Opt->FindOpt.FileSearchMode=SearchMode;
		}

		LastCmpCase=CmpCase;
		LastWholeWords=WholeWords;
		LastSearchHex=SearchHex;
		LastSearchInArchives=SearchInArchives;
		LastNotContaining = NotContaining;
		strLastFindMask = strFindMask;
		strLastFindStr = strFindStr;

		if (!strFindStr.empty())
			Editor::SetReplaceMode(false);
	}
	while (FindFilesProcess());

	Global->CtrlObject->Cp()->ActivePanel()->SetTitle();
}

FindFiles::~FindFiles()
{
	Global->ScrBuf->ResetShadow();
}
