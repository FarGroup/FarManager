/*
print.cpp

Печать (Alt-F5)
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

#include "panel.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "filelist.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "print.hpp"
#include "delete.hpp"
#include "pathmix.hpp"
#include "mix.hpp"
#include "lang.hpp"
#include "plugins.hpp"
#include "colormix.hpp"
#include "strmix.hpp"

#define PRINTER_INFO_LEVEL 4
#define DETAIL_PRINTER_INFO_N_IMPL(level) PRINTER_INFO_##level##W
#define PRINTER_INFO_N(level) DETAIL_PRINTER_INFO_N_IMPL(level)
#define PRINTER_INFO PRINTER_INFO_N(PRINTER_INFO_LEVEL)

static void AddToPrintersMenu(VMenu2 *PrinterList, const PRINTER_INFO *pi, int PrinterNumber)
{
	// Получаем принтер по умолчанию
	string strDefaultPrinter;
	os::GetDefaultPrinter(strDefaultPrinter);

	// Признак наличия принтера по умолчанию
	bool bDefaultPrinterFound = false;

	// Заполняем список принтеров
	for (const auto& printer: make_range(pi, PrinterNumber))
	{
		MenuItemEx Item(printer.pPrinterName);

		if (strDefaultPrinter == printer.pPrinterName)
		{
			bDefaultPrinterFound = true;
			Item.SetCheck(TRUE);
			Item.SetSelect(TRUE);
		}
		Item.UserData = Item.strName;
		PrinterList->AddItem(Item);
	}

	if (!bDefaultPrinterFound)
		PrinterList->SetSelectPos(0, 1);
}

void PrintFiles(FileList* SrcPanel)
{
	_ALGO(CleverSysLog clv(L"Alt-F5 (PrintFiles)"));
	string strPrinterName;
	DWORD Needed = 0, Returned;
	size_t DirsCount=0;
	size_t SelCount=SrcPanel->GetSelCount();

	if (!SelCount)
	{
		_ALGO(SysLog(L"Error: !SelCount"));
		return;
	}

	// проверка каталогов
	{
		_ALGO(SysLog(L"Check for FILE_ATTRIBUTE_DIRECTORY"));
		DWORD FileAttr;
		string strSelName;
		SrcPanel->GetSelName(nullptr, FileAttr);
		while (SrcPanel->GetSelName(&strSelName, FileAttr))
		{
			if (TestParentFolderName(strSelName) || (FileAttr & FILE_ATTRIBUTE_DIRECTORY))
				DirsCount++;
		}
	}

	if (DirsCount==SelCount)
		return;

	EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, PRINTER_INFO_LEVEL, nullptr, 0, &Needed, &Returned);

	if (!Needed)
		return;

	block_ptr<PRINTER_INFO> pi(Needed);

	if (!EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, PRINTER_INFO_LEVEL, reinterpret_cast<BYTE*>(pi.get()), Needed, &Needed, &Returned))
	{
		const auto ErrorState = error_state::fetch();

		Message(MSG_WARNING, ErrorState,
			msg(lng::MPrintTitle),
			{
				msg(lng::MCannotEnumeratePrinters)
			},
			{ lng::MOk });
		return;
	}

	{
		_ALGO(CleverSysLog clv2(L"Show Menu"));
		string strTitle;
		if (SelCount==1)
		{
			DWORD FileAttr;
			string strSelName;
			SrcPanel->GetSelName(nullptr,FileAttr);
			SrcPanel->GetSelName(&strSelName, FileAttr);
			strSelName = inplace::quote_unconditional(TruncStr(strSelName, 50));
			strTitle = format(lng::MPrintTo, strSelName);
		}
		else
		{
			_ALGO(SysLog(L"Correct: SelCount-=DirsCount"));
			SelCount-=DirsCount;
			strTitle = format(lng::MPrintFilesTo, SelCount);
		}

		const auto PrinterList = VMenu2::create(strTitle, nullptr, 0, ScrY - 4);
		PrinterList->SetMenuFlags(VMENU_WRAPMODE | VMENU_SHOWAMPERSAND);
		PrinterList->SetPosition(-1,-1,0,0);
		AddToPrintersMenu(PrinterList.get(), pi.get(), Returned);

		if (PrinterList->Run()<0)
		{
			_ALGO(SysLog(L"ESC"));
			return;
		}

		if (const auto NamePtr = PrinterList->GetUserDataPtr<string>())
			strPrinterName = *NamePtr;
	}

	os::printer_handle Printer;

	if (!OpenPrinter(UNSAFE_CSTR(strPrinterName), &ptr_setter(Printer), nullptr))
	{
		const auto ErrorState = error_state::fetch();

		Message(MSG_WARNING, ErrorState,
			msg(lng::MPrintTitle),
			{
				msg(lng::MCannotOpenPrinter),
				strPrinterName
			},
			{ lng::MOk });
		_ALGO(SysLog(L"Error: Cannot Open Printer"));
		return;
	}

	{
		_ALGO(CleverSysLog clv3(L"Print selected Files"));
		SCOPED_ACTION(SaveScreen);

		const auto& PR_PrintMsg = []
		{
			Message(0, 
				msg(lng::MPrintTitle),
				{
					msg(lng::MPreparingForPrinting)
				},
				{});
		};

		SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<PreRedrawItem>(PR_PrintMsg));
		SetCursorType(false, 0);
		PR_PrintMsg();
		const auto hPlugin = SrcPanel->GetPluginHandle();
		int PluginMode = SrcPanel->GetMode() == panel_mode::PLUGIN_PANEL &&
		               !Global->CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARGETFILE);

		DWORD FileAttr;
		string strSelName;
		SrcPanel->GetSelName(nullptr,FileAttr);
		while (SrcPanel->GetSelName(&strSelName,FileAttr))
		{
			if (TestParentFolderName(strSelName) || (FileAttr & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			int Success=FALSE;
			string FileName;
			string strTempDir, strTempName;

			if (PluginMode)
			{
				if (FarMkTempEx(strTempDir))
				{
					os::fs::create_directory(strTempDir);
					if (const auto ListItem = SrcPanel->GetLastSelectedItem())
					{
						PluginPanelItemHolder PanelItem;
						SrcPanel->FileListToPluginItem(*ListItem, PanelItem);

						if (Global->CtrlObject->Plugins->GetFile(hPlugin, &PanelItem.Item, strTempDir, strTempName, OPM_SILENT))
							FileName = strTempName;
						else
							os::fs::remove_directory(strTempDir);
					}
				}
			}
			else
				FileName = strSelName;

			error_state ErrorState;

			if(const auto SrcFile = os::fs::file(FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING))
			{
				DOC_INFO_1 di1 = {UNSAFE_CSTR(FileName)};

				if (StartDocPrinter(Printer.native_handle(), 1, reinterpret_cast<BYTE*>(&di1)))
				{
					char Buffer[8192];
					size_t Read;
					DWORD Written;
					Success=TRUE;

					while (SrcFile.Read(Buffer, sizeof(Buffer), Read) && Read > 0)
					{
						if (!WritePrinter(Printer.native_handle(), Buffer, static_cast<DWORD>(Read), &Written))
						{
							ErrorState = error_state::fetch();
							Success = FALSE;
							break;
						}
					}
					EndDocPrinter(Printer.native_handle());
				}
			}

			if (!strTempName.empty())
			{
				DeleteFileWithFolder(strTempName);
			}

			if (Success)
				SrcPanel->ClearLastGetSelection();
			else
			{
				if (Message(MSG_WARNING, ErrorState,
					msg(lng::MPrintTitle),
					{
						msg(lng::MCannotPrint),
						strSelName
					},
					{ lng::MSkip, lng::MCancel }) != Message::first_button)
					break;
			}
		}
	}

	SrcPanel->Redraw();
}
