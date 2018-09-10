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

#include "print.hpp"

#include "panel.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "filelist.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "TPreRedrawFunc.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "delete.hpp"
#include "mix.hpp"
#include "lang.hpp"
#include "plugins.hpp"
#include "strmix.hpp"
#include "global.hpp"

#include "platform.fs.hpp"

#include "common/io.hpp"
#include "common/range.hpp"

#include "format.hpp"

static void AddToPrintersMenu(VMenu2 *PrinterList, range<const PRINTER_INFO_4W*> Printers)
{
	// Получаем принтер по умолчанию
	string strDefaultPrinter;
	os::GetDefaultPrinter(strDefaultPrinter);

	// Признак наличия принтера по умолчанию
	bool bDefaultPrinterFound = false;

	// Заполняем список принтеров
	for (const auto& printer: Printers)
	{
		MenuItemEx Item(printer.pPrinterName);

		if (strDefaultPrinter == printer.pPrinterName)
		{
			bDefaultPrinterFound = true;
			Item.SetCheck(TRUE);
			Item.SetSelect(TRUE);
		}
		Item.ComplexUserData = Item.Name;
		PrinterList->AddItem(Item);
	}

	if (!bDefaultPrinterFound)
		PrinterList->SetSelectPos(0, 1);
}

void PrintFiles(FileList* SrcPanel)
{
	try
	{
		const auto SelCount = SrcPanel->GetSelCount();
		if (!SelCount)
			return;

		const auto Enumerator = SrcPanel->enum_selected();
		const auto DirsCount = std::accumulate(ALL_CONST_RANGE(Enumerator), size_t{}, [](size_t Count, const os::fs::find_data& i)
		{
			return Count + (i.Attributes & FILE_ATTRIBUTE_DIRECTORY? 1 : 0);
		});

		if (DirsCount == SelCount)
			return;

		DWORD Needed = 0, Returned;
		EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 4, nullptr, 0, &Needed, &Returned);

		if (!Needed)
			return;

		block_ptr<PRINTER_INFO_4W> pi(Needed);

		if (!EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 4, reinterpret_cast<BYTE*>(pi.get()), Needed, &Needed, &Returned))
			throw MAKE_FAR_EXCEPTION(msg(lng::MCannotEnumeratePrinters));

		string strPrinterName;

		{
			string strTitle;
			if (SelCount == 1)
			{
				os::fs::find_data Data;
				if (!SrcPanel->get_first_selected(Data))
					return;

				auto Name = Data.FileName;
				strTitle = format(msg(lng::MPrintTo), inplace::quote_unconditional(TruncStr(Name, 50)));
			}
			else
			{
				strTitle = format(msg(lng::MPrintFilesTo), SelCount - DirsCount);
			}

			const auto PrinterList = VMenu2::create(strTitle, {}, ScrY - 4);
			PrinterList->SetMenuFlags(VMENU_WRAPMODE | VMENU_SHOWAMPERSAND);
			PrinterList->SetPosition({ -1, -1, 0, 0 });
			AddToPrintersMenu(PrinterList.get(), { pi.get(), Returned });

			if (PrinterList->Run() < 0)
				return;

		if (const auto NamePtr = PrinterList->GetComplexUserDataPtr<string>())
				strPrinterName = *NamePtr;
		}

		os::printer_handle Printer;

		if (!OpenPrinter(UNSAFE_CSTR(strPrinterName), &ptr_setter(Printer), nullptr))
			throw MAKE_FAR_EXCEPTION(msg(lng::MCannotOpenPrinter));

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
		const auto PluginMode = SrcPanel->GetMode() == panel_mode::PLUGIN_PANEL && !Global->CtrlObject->Plugins->UseFarCommand(hPlugin, PLUGIN_FARGETFILE);

		for (const auto& i : SrcPanel->enum_selected())
		{
			if (i.Attributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			delayed_deleter Deleter;

			string FileName;

			if (PluginMode)
			{
				const auto strTempDir = MakeTemp();
				if (!os::fs::create_directory(strTempDir))
					throw MAKE_FAR_EXCEPTION(L"create_directory error"sv);

				const auto ListItem = SrcPanel->GetLastSelectedItem();
				if (!ListItem)
					throw MAKE_FAR_EXCEPTION(L"GetLastSelectedItem error"sv);

				PluginPanelItemHolder PanelItem;
				SrcPanel->FileListToPluginItem(*ListItem, PanelItem);

				if (!Global->CtrlObject->Plugins->GetFile(hPlugin, &PanelItem.Item, strTempDir, FileName, OPM_SILENT))
				{
					os::fs::remove_directory(strTempDir);
					throw MAKE_FAR_EXCEPTION(L"GetFile error"sv);
				}

				Deleter.set(FileName);
			}
			else
			{
				FileName = i.FileName;
			}

			try
			{
				const auto SrcFile = os::fs::file(FileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING);
				if (!SrcFile)
					throw MAKE_FAR_EXCEPTION(L"Cannot open file"sv);

				os::fs::filebuf StreamBuffer(SrcFile, std::ios::in);
				std::istream Stream(&StreamBuffer);
				Stream.exceptions(Stream.badbit | Stream.failbit);

				DOC_INFO_1 di1{ UNSAFE_CSTR(FileName) };

				if (!StartDocPrinter(Printer.native_handle(), 1, reinterpret_cast<BYTE*>(&di1)))
					throw MAKE_FAR_EXCEPTION(L"StartDocPrinter error"sv);

				SCOPE_EXIT{ EndDocPrinter(Printer.native_handle()); };

				for (;;)
				{
					char Buffer[8192];
					const auto Read = io::read(Stream, Buffer);
					if (!Read)
						break;

					DWORD Written;
					if (!WritePrinter(Printer.native_handle(), Buffer, static_cast<DWORD>(Read), &Written))
						throw MAKE_FAR_EXCEPTION(L"WritePrinter error"sv);
				}

				SrcPanel->ClearLastGetSelection();
			}
			catch (const far_exception& e)
			{
				if (Message(MSG_WARNING, e.get_error_state(),
					msg(lng::MPrintTitle),
					{
						msg(lng::MCannotPrint),
						i.FileName
					},
					{ lng::MSkip, lng::MCancel }) != Message::first_button)
					break;
			}
		}
	}
	catch (const far_exception& e)
	{
		Message(MSG_WARNING, e.get_error_state(),
			msg(lng::MPrintTitle),
			{},
			{ lng::MOk });
	}

	SrcPanel->Redraw();
}
