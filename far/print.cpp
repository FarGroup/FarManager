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
#include "language.hpp"
#include "plugins.hpp"

#define PRINTER_INFO_LEVEL 4
#define GENERATE_PRINTER_INFO(prefix, value) prefix##value##W
#define PRINTER_INFO_X(level) GENERATE_PRINTER_INFO(PRINTER_INFO_, level)
#define PRINTER_INFO PRINTER_INFO_X(PRINTER_INFO_LEVEL)

static void AddToPrintersMenu(VMenu2 *PrinterList, PRINTER_INFO *pi, int PrinterNumber)
{
	// Получаем принтер по умолчанию
	string strDefaultPrinter;
	DWORD pcchBuffer = 0;

	if (!GetDefaultPrinter(nullptr, &pcchBuffer) && ERROR_INSUFFICIENT_BUFFER==GetLastError())
	{
		wchar_t_ptr Buffer(pcchBuffer);
		if (!GetDefaultPrinter(Buffer.get(), &pcchBuffer))
			strDefaultPrinter.clear();
		else
			strDefaultPrinter.assign(Buffer.get(), Buffer.size());
	}

	// Признак наличия принтера по умолчанию
	bool bDefaultPrinterFound = false;

	// Заполняем список принтеров
	FOR(const auto& printer, make_range(pi, pi + PrinterNumber))
	{
		MenuItemEx Item(printer.pPrinterName);

		if (strDefaultPrinter == printer.pPrinterName)
		{
			bDefaultPrinterFound = true;
			Item.SetCheck(TRUE);
			Item.SetSelect(TRUE);
		}

		PrinterList->SetUserData(Item.strName.data(), (Item.strName.size()+1)*sizeof(wchar_t), PrinterList->AddItem(Item));
	}

	if (!bDefaultPrinterFound)
		PrinterList->SetSelectPos(0, 1);
}

void PrintFiles(FileList* SrcPanel)
{
	_ALGO(CleverSysLog clv(L"Alt-F5 (PrintFiles)"));
	string strPrinterName;
	DWORD Needed = 0, Returned;
	DWORD FileAttr;
	string strSelName;
	size_t DirsCount=0;
	size_t SelCount=SrcPanel->GetSelCount();

	if (!SelCount)
	{
		_ALGO(SysLog(L"Error: !SelCount"));
		return;
	}

	// проверка каталогов
	_ALGO(SysLog(L"Check for FILE_ATTRIBUTE_DIRECTORY"));
	SrcPanel->GetSelName(nullptr,FileAttr);

	while (SrcPanel->GetSelName(&strSelName,FileAttr))
	{
		if (TestParentFolderName(strSelName) || (FileAttr & FILE_ATTRIBUTE_DIRECTORY))
			DirsCount++;
	}

	if (DirsCount==SelCount)
		return;

	EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, PRINTER_INFO_LEVEL, nullptr, 0, &Needed, &Returned);

	if (!Needed)
		return;

	block_ptr<PRINTER_INFO> pi(Needed);

	if (!EnumPrinters(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS,nullptr,PRINTER_INFO_LEVEL,(LPBYTE)pi.get(),Needed,&Needed,&Returned))
	{
		Global->CatchError();
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MPrintTitle),MSG(MCannotEnumeratePrinters),MSG(MOk));
		return;
	}

	{
		_ALGO(CleverSysLog clv2(L"Show Menu"));
		LangString strTitle;
		string strName;

		if (SelCount==1)
		{
			SrcPanel->GetSelName(nullptr,FileAttr);
			SrcPanel->GetSelName(&strName,FileAttr);
			strSelName = TruncStr(strName,50);
			strTitle = MPrintTo;
			strTitle << InsertQuote(strSelName);
		}
		else
		{
			_ALGO(SysLog(L"Correct: SelCount-=DirsCount"));
			SelCount-=DirsCount;
			strTitle = MPrintFilesTo;
			strTitle << SelCount;
		}

		auto PrinterList = VMenu2::create(strTitle, nullptr, 0, ScrY - 4);
		PrinterList->SetFlags(VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
		PrinterList->SetPosition(-1,-1,0,0);
		AddToPrintersMenu(PrinterList.get(), pi.get(), Returned);

		if (PrinterList->Run()<0)
		{
			_ALGO(SysLog(L"ESC"));
			return;
		}

		strPrinterName = NullToEmpty(static_cast<const wchar_t*>(PrinterList->GetUserData(nullptr, 0)));
	}

	HANDLE hPrinter;

	if (!OpenPrinter(UNSAFE_CSTR(strPrinterName), &hPrinter,nullptr))
	{
		Global->CatchError();
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MPrintTitle),MSG(MCannotOpenPrinter),
		        strPrinterName.data(),MSG(MOk));
		_ALGO(SysLog(L"Error: Cannot Open Printer"));
		return;
	}

	{
		_ALGO(CleverSysLog clv3(L"Print selected Files"));
		SCOPED_ACTION(SaveScreen);

		auto PR_PrintMsg = [](){ Message(0, 0, MSG(MPrintTitle), MSG(MPreparingForPrinting)); };

		SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<PreRedrawItem>(PR_PrintMsg));
		SetCursorType(false, 0);
		PR_PrintMsg();
		auto hPlugin=SrcPanel->GetPluginHandle();
		int PluginMode=SrcPanel->GetMode()==PLUGIN_PANEL &&
		               !Global->CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARGETFILE);
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
					api::CreateDirectory(strTempDir,nullptr);
					auto ListItem = SrcPanel->GetLastSelectedItem();
					if (ListItem)
					{
						PluginPanelItem PanelItem;
						FileList::FileListToPluginItem(*ListItem, &PanelItem);

						if (Global->CtrlObject->Plugins->GetFile(hPlugin,&PanelItem,strTempDir,strTempName,OPM_SILENT))
							FileName = strTempName;
						else
							api::RemoveDirectory(strTempDir);

						FreePluginPanelItem(PanelItem);
					}
				}
			}
			else
				FileName = strSelName;

			api::File SrcFile;
			if(SrcFile.Open(FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING))
			{
				DOC_INFO_1 di1 = {UNSAFE_CSTR(FileName)};

				if (StartDocPrinter(hPrinter,1,(LPBYTE)&di1))
				{
					char Buffer[8192];
					size_t Read;
					DWORD Written;
					Success=TRUE;

					while (SrcFile.Read(Buffer, sizeof(Buffer), Read) && Read > 0)
					{
						if (!WritePrinter(hPrinter, Buffer, static_cast<DWORD>(Read), &Written))
						{
							Global->CatchError();
							Success = FALSE;
							break;
						}
					}
					EndDocPrinter(hPrinter);
				}
				SrcFile.Close();
			}

			if (!strTempName.empty())
			{
				DeleteFileWithFolder(strTempName);
			}

			if (Success)
				SrcPanel->ClearLastGetSelection();
			else
			{
				if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MPrintTitle),MSG(MCannotPrint),
				            strSelName.data(),MSG(MSkip),MSG(MCancel)))
					break;
			}
		}

		ClosePrinter(hPrinter);
	}

	SrcPanel->Redraw();
}
